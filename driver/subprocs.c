/* subprocs.c --- choosing, spawning, and killing screenhacks.
 * xscreensaver, Copyright (c) 1991, 1992, 1993, 1995, 1997
 *  Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>		/* not used for much... */

#ifndef ESRCH
#include <errno.h>
#endif

#include <sys/time.h>		/* sys/resource.h needs this for timeval */
#include <sys/resource.h>	/* for setpriority() and PRIO_PROCESS */
#include <sys/wait.h>		/* for waitpid() and associated macros */
#include <signal.h>		/* for the signal names */

#ifndef NO_SETUID
#include <pwd.h>		/* for getpwnam() and struct passwd */
#include <grp.h>		/* for getgrgid() and struct group */
#endif /* NO_SETUID */

#if !defined(SIGCHLD) && defined(SIGCLD)
#define SIGCHLD SIGCLD
#endif

#ifdef __STDC__
extern int putenv (/* const char * */);	/* getenv() is in stdlib.h... */
extern int kill (pid_t, int);		/* signal() is in sys/signal.h... */
#endif

/* This file doesn't need the Xt headers, so stub these types out... */
#undef XtPointer
#define XtAppContext void*
#define XrmDatabase  void*
#define XtIntervalId void*
#define XtPointer    void*
#define Widget       void*

#include "xscreensaver.h"
#include "yarandom.h"


extern saver_info *global_si_kludge;	/* I hate C so much... */


static void
#ifdef __STDC__
nice_subproc (int nice_level)
#else  /* !__STDC__ */
nice_subproc (nice_level)
	int nice_level;
#endif /* !__STDC__ */
{
  if (nice_level == 0)
    return;

#if defined(SYSV) || defined(SVR4) || defined(__hpux)
  {
    int old_nice = nice (0);
    int n = nice_level - old_nice;
    errno = 0;
    if (nice (n) == -1 && errno != 0)
      {
	char buf [512];
	sprintf (buf, "%s: nice(%d) failed", progname, n);
	perror (buf);
    }
  }
#elif defined(PRIO_PROCESS)
  if (setpriority (PRIO_PROCESS, getpid(), nice_level) != 0)
    {
      char buf [512];
      sprintf (buf, "%s: setpriority(PRIO_PROCESS, %d, %d) failed",
	       progname, (unsigned long) getpid(), nice_level);
      perror (buf);
    }

#else
  fprintf (stderr,
	   "%s: don't know how to change process priority on this system.\n",
	   progname);

#endif
}


static void
#ifdef __STDC__
exec_simple_command (const char *command)
#else  /* !__STDC__ */
exec_simple_command (command)
	const char *command;
#endif /* !__STDC__ */
{
  char *av[1024];
  int ac = 0;
  char *token = strtok (strdup(command), " \t");
  while (token)
    {
      av[ac++] = token;
      token = strtok(0, " \t");
    }
  av[ac] = 0;

  execvp (av[0], av);			/* shouldn't return. */

  {
    char buf [512];
    sprintf (buf, "%s: execvp() failed", progname);
    perror (buf);
    fflush(stderr);
    fflush(stdout);
    exit (1);	/* Note that this only exits a child fork.  */
  }
}


static void
#ifdef __STDC__
exec_complex_command (const char *shell, const char *command)
#else  /* !__STDC__ */
exec_complex_command (shell, command)
	const char *shell;
	const char *command;
#endif /* !__STDC__ */
{
  char *av[5];
  int ac = 0;
  char *command2 = (char *) malloc (strlen (command) + 6);
  memcpy (command2, "exec ", 5);
  memcpy (command2 + 5, command, strlen (command) + 1);

  /* Invoke the shell as "/bin/sh -c 'exec prog -arg -arg ...'" */
  av [ac++] = (char *) shell;
  av [ac++] = "-c";
  av [ac++] = command2;
  av [ac]   = 0;

  execvp (av[0], av);			/* shouldn't return. */

  {
    char buf [512];
    sprintf (buf, "%s: execvp() failed", progname);
    perror (buf);
    fflush(stderr);
    fflush(stdout);
    exit (1);	/* Note that this only exits a child fork.  */
  }
}


static void
#ifdef __STDC__
exec_screenhack (saver_info *si, const char *command)
#else  /* !__STDC__ */
exec_screenhack (si, command)
	saver_info *si;
	const char *command;
#endif /* !__STDC__ */
{
  /* I don't believe what a sorry excuse for an operating system UNIX is!

     - I want to spawn a process.
     - I want to know it's pid so that I can kill it.
     - I would like to receive a message when it dies of natural causes.
     - I want the spawned process to have user-specified arguments.

     If shell metacharacters are present (wildcards, backquotes, etc), the
     only way to parse those arguments is to run a shell to do the parsing
     for you.

     And the only way to know the pid of the process is to fork() and exec()
     it in the spawned side of the fork.

     But if you're running a shell to parse your arguments, this gives you
     the pid of the *shell*, not the pid of the *process* that you're
     actually interested in, which is an *inferior* of the shell.  This also
     means that the SIGCHLD you get applies to the shell, not its inferior.
     (Why isn't that sufficient?  I don't remember any more, but it turns
     out that it isn't.)

     So, the only solution, when metacharacters are present, is to force the
     shell to exec() its inferior.  What a fucking hack!  We prepend "exec "
     to the command string, and hope it doesn't contain unquoted semicolons
     or ampersands (we don't search for them, because we don't want to
     prohibit their use in quoted strings (messages, for example) and parsing
     out the various quote characters is too much of a pain.)

     (Actually, Clint Wong <clint@jts.com> points out that process groups
     might be used to take care of this problem; this may be worth considering
     some day, except that, 1: this code works now, so why fix it, and 2: from
     what I've seen in Emacs, dealing with process groups isn't especially
     portable.)
   */
  saver_preferences *p = &si->prefs;
  Bool hairy_p = !!strpbrk (command, "*?$&!<>[];`'\\\"");

  if (p->verbose_p)
    printf ("%s: spawning \"%s\" in pid %lu%s.\n",
	    progname, command, (unsigned long) getpid (),
	    (hairy_p ? " (via shell)" : ""));

  if (hairy_p)
    /* If it contains any shell metacharacters, do it the hard way,
       and fork a shell to parse the arguments for us. */
    exec_complex_command (p->shell, command);
  else
    /* Otherwise, we can just exec the program directly. */
    exec_simple_command (command);

  abort();	/* that shouldn't have returned. */
}


/* Global locks to avoid a race condition between the main thread and
   the SIGCHLD handler.
 */
static Bool killing = False;
static Bool suspending = False;

static const char *current_hack_name P((saver_info *si));

static void
#ifdef __STDC__
await_child_death (saver_info *si, Bool killed)
#else  /* !__STDC__ */
await_child_death (si, killed)
	saver_info *si;
	Bool killed;
#endif /* !__STDC__ */
{
  saver_preferences *p = &si->prefs;
  Bool suspended_p = False;
  int status;
  pid_t kid;
  killing = True;
  if (! si->pid)
    return;

  do
    {
      kid = waitpid (si->pid, &status, WUNTRACED);
    }
  while (kid == -1 && errno == EINTR);

  if (kid == si->pid)
    {
      if (WIFEXITED (status))
	{
	  int exit_status = WEXITSTATUS (status);
	  if (exit_status & 0x80)
	    exit_status |= ~0xFF;
	  /* One might assume that exiting with non-0 means something went
	     wrong.  But that loser xswarm exits with the code that it was
	     killed with, so it *always* exits abnormally.  Treat abnormal
	     exits as "normal" (don't mention them) if we've just killed
	     the subprocess.  But mention them if they happen on their own.
	   */
	  if (exit_status != 0 && (p->verbose_p || (! killed)))
	    fprintf (stderr,
		     "%s: child pid %lu (%s) exited abnormally (code %d).\n",
		    progname, (unsigned long) si->pid, current_hack_name(si),
		     exit_status);
	  else if (p->verbose_p)
	    printf ("%s: child pid %lu (%s) exited normally.\n",
		    progname, (unsigned long) si->pid, current_hack_name (si));
	}
      else if (WIFSIGNALED (status))
	{
	  if (!killed || WTERMSIG (status) != SIGTERM)
	    fprintf (stderr,
		     "%s: child pid %lu (%s) terminated with signal %d!\n",
		     progname, (unsigned long) si->pid, current_hack_name(si),
		     WTERMSIG(status));
	  else if (p->verbose_p)
	    printf ("%s: child pid %lu (%s) terminated with SIGTERM.\n",
		    progname, (unsigned long) si->pid, current_hack_name (si));
	}
      else if (suspending)
	{
	  suspended_p = True;
	  suspending = False; /* complain if it happens twice */
	}
      else if (WIFSTOPPED (status))
	{
	  suspended_p = True;
	  fprintf (stderr,
		   "%s: child pid %lu (%s) stopped with signal %d!\n",
		   progname, (unsigned long) si->pid,
		   current_hack_name(si), WSTOPSIG (status));
	}
      else
	fprintf (stderr, "%s: child pid %lu (%s) died in a mysterious way!",
		 progname, (unsigned long) si->pid, current_hack_name(si));
    }
  else if (kid <= 0)
    fprintf (stderr,
	     "%s: waitpid(%lu, ...) says there are no kids?  (%lu)\n",
	     progname, (unsigned long) si->pid, (unsigned long) kid);
  else
    fprintf (stderr, "%s: waitpid(%lu, ...) says proc %lu died, not %lu?\n",
	     progname, (unsigned long) si->pid, (unsigned long) kid,
	     (unsigned long) si->pid);
  killing = False;
  if (suspended_p != True)
    si->pid = 0;
}


static const char *
#ifdef __STDC__
current_hack_name (saver_info *si)
#else  /* !__STDC__ */
current_hack_name (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  saver_preferences *p = &si->prefs;
  static char name [1024];
  const char *hack = (si->demo_mode_p
		      ? si->demo_hack
		      : p->screenhacks [si->current_hack]);
  const char *in;
  char *out;

  in = hack;
  out = name;
  while (isspace(*in)) in++;		/* skip whitespace */
  while (!isspace(*in) && *in != ':')
    *out++ = *in++;			/* snarf first token */
  while (isspace(*in)) in++;		/* skip whitespace */

  if (*in == ':')			/* token was a visual name; skip it. */
    {
      in++;
      out = name;
      while (isspace(*in)) in++;		/* skip whitespace */
      while (!isspace(*in)) *out++ = *in++;	/* snarf first token */
    }
  *out = 0;
  return name;
}

static Bool
#ifdef __STDC__
select_visual_of_hack (saver_info *si, const char *hack)
#else  /* !__STDC__ */
select_visual_of_hack (si, hack)
	saver_info *si;
	const char *hack;
#endif /* !__STDC__ */
{
  saver_preferences *p = &si->prefs;
  Bool selected;
  static char vis [1024];
  const char *in = hack;
  char *out = vis;
  while (isspace(*in)) in++;		/* skip whitespace */
  while (!isspace(*in) && *in != ':')
    *out++ = *in++;			/* snarf first token */
  while (isspace(*in)) in++;		/* skip whitespace */
  *out = 0;

  if (*in == ':')
    selected = select_visual(si, vis);
  else
    selected = select_visual(si, 0);

  if (!selected && (p->verbose_p || si->demo_mode_p))
    {
      if (*in == ':') in++;
      while (isspace(*in)) in++;
      fprintf (stderr,
	       (si->demo_mode_p
		? "%s: warning, no \"%s\" visual for \"%s\".\n"
		: "%s: no \"%s\" visual; skipping \"%s\".\n"),
	       progname, (vis ? vis : "???"), in);
    }

  return selected;
}


#ifdef SIGCHLD
static void
#ifdef __STDC__
sigchld_handler (int sig)
#else /* !__STDC__ */
sigchld_handler (sig)
     int sig;
#endif /* !__STDC__ */
{
  saver_info *si = global_si_kludge;	/* I hate C so much... */

  if (killing)
    return;
  if (! si->pid)
    abort ();
  await_child_death (si, False);
}
#endif


void
init_sigchld P((void))
{
#ifdef SIGCHLD
  if (((int) signal (SIGCHLD, sigchld_handler)) == -1)
    {
      char buf [255];
      sprintf (buf, "%s: couldn't catch SIGCHLD", progname);
      perror (buf);
    }
#endif
}


void
#ifdef __STDC__
spawn_screenhack (saver_info *si, Bool first_time_p)
#else  /* !__STDC__ */
spawn_screenhack (si, first_time_p)
	saver_info *si;
	Bool first_time_p;
#endif /* !__STDC__ */
{
  saver_preferences *p = &si->prefs;
  raise_window (si, first_time_p, True, False);
  XFlush (si->dpy);

  if (p->screenhacks_count || si->demo_mode_p)
    {
      char *hack;
      pid_t forked;
      char buf [255];
      int new_hack;

      if (si->demo_mode_p)
	{
	  hack = si->demo_hack;

	  /* Ignore visual-selection failure if in demo mode. */
	  (void) select_visual_of_hack (si, hack);
	}
      else
	{
	  int retry_count = 0;

	AGAIN:
	  if (p->screenhacks_count == 1)
	    new_hack = 0;
	  else if (si->next_mode_p == 1)
	    new_hack = (si->current_hack + 1) % p->screenhacks_count;
	  else if (si->next_mode_p == 2)
	    new_hack = ((si->current_hack + p->screenhacks_count - 1)
			% p->screenhacks_count);
	  else
	    while ((new_hack = random () % p->screenhacks_count)
		   == si->current_hack)
	      ;
	  si->current_hack = new_hack;
	  hack = p->screenhacks[si->current_hack];

	  if (!select_visual_of_hack (si, hack))
	    {
	      if (++retry_count > (p->screenhacks_count*4))
		{
		  /* Uh, oops.  Odds are, there are no suitable visuals,
		     and we're looping.  Give up.  (This is totally lame,
		     what we should do is make a list of suitable hacks at
		     the beginning, then only loop over them.)
		  */
		  if (p->verbose_p)
		    fprintf(stderr,
			    "%s: no suitable visuals for these programs.\n",
			    progname);
		  return;
		}
	      else
		goto AGAIN;
	    }
	}
      si->next_mode_p = 0;


      /* If there's a visual description on the front of the command, nuke it.
       */
      {
	char *in = hack;
	while (isspace(*in)) in++;			/* skip whitespace */
	hack = in;
	while (!isspace(*in) && *in != ':') in++;	/* snarf first token */
	while (isspace(*in)) in++;			/* skip whitespace */
	if (*in == ':')
	  {
	    in++;
	    while (isspace(*in)) in++;
	    hack = in;
	  }
      }

      switch ((int) (forked = fork ()))
	{
	case -1:
	  sprintf (buf, "%s: %scouldn't fork", progname);
	  perror (buf);
	  restore_real_vroot (si);
	  saver_exit (si, 1);

	case 0:
	  close (ConnectionNumber (si->dpy));	/* close display fd */
	  nice_subproc (p->nice_inferior);
	  exec_screenhack (si, hack);		/* this does not return */
	  abort();
	  break;

	default:
	  si->pid = forked;
	  break;
	}
    }
}

void
#ifdef __STDC__
kill_screenhack (saver_info *si)
#else  /* !__STDC__ */
kill_screenhack (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  saver_preferences *p = &si->prefs;
  killing = True;
  if (! si->pid)
    return;
  if (kill (si->pid, SIGTERM) < 0)
    {
      if (errno == ESRCH)
	{
	  /* Sometimes we don't get a SIGCHLD at all!  WTF?
	     It's a race condition.  It looks to me like what's happening is
	     something like: a subprocess dies of natural causes.  There is a
	     small window between when the process dies and when the SIGCHLD
	     is (would have been) delivered.  If we happen to try to kill()
	     the process during that time, the kill() fails, because the
	     process is already dead.  But! no SIGCHLD is delivered (perhaps
	     because the failed kill() has reset some state in the kernel?)
	     Anyway, if kill() says "No such process", then we have to wait()
	     for it anyway, because the process has already become a zombie.
	     I love Unix.
	   */
	  await_child_death (si, False);
	}
      else
	{
	  char buf [255];
	  sprintf (buf, "%s: couldn't kill child process %lu", progname,
		   (unsigned long) si->pid);
	  perror (buf);
	}
    }
  else
    {
      if (p->verbose_p)
	printf ("%s: killing pid %lu.\n", progname, (unsigned long) si->pid);
      await_child_death (si, True);
    }
}


void
#ifdef __STDC__
suspend_screenhack (saver_info *si, Bool suspend_p)
#else  /* !__STDC__ */
suspend_screenhack (si, suspend_p)
	saver_info *si;
	Bool suspend_p;
#endif /* !__STDC__ */
{
  saver_preferences *p = &si->prefs;
  
  suspending = suspend_p;
  if (! si->pid)
    ;
  else if (kill (si->pid, (suspend_p ? SIGSTOP : SIGCONT)) < 0)
    {
      char buf [255];
      sprintf (buf, "%s: couldn't %s child process %lu", progname,
	       (suspend_p ? "suspend" : "resume"),
	       (unsigned long) si->pid);
      perror (buf);
    }
  else if (p->verbose_p)
    printf ("%s: %s pid %lu.\n", progname,
	    (suspend_p ? "suspending" : "resuming"), (unsigned long) si->pid);
}


/* Called when we're exiting abnormally, to kill off the subproc. */
void
#ifdef __STDC__
emergency_kill_subproc (saver_info *si)
#else  /* !__STDC__ */
emergency_kill_subproc (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  saver_preferences *p = &si->prefs;
  if (si->pid)
    {
#ifdef SIGCHLD
      signal (SIGCHLD, SIG_IGN);
#endif
      if (p->verbose_p)
	fprintf(real_stderr, "%s: kill(%d, SIGTERM)\n", progname, si->pid);
      kill(si->pid, SIGTERM);
      si->pid = 0;
    }
}

Bool
#ifdef __STDC__
screenhack_running_p (saver_info *si)
#else  /* !__STDC__ */
screenhack_running_p (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  if (si->pid) return True;
  else return False;
}


/* Restarting the xscreensaver process from scratch. */

static char **saved_argv;

void
#ifdef __STDC__
save_argv (int argc, char **argv)
#else /* !__STDC__ */
save_argv (argc, argv)
     int argc;
     char **argv;
#endif /* !__STDC__ */
{
  saved_argv = (char **) malloc ((argc + 2) * sizeof (char *));
  saved_argv [argc] = 0;
  while (argc--)
    {
      int i = strlen (argv [argc]) + 1;
      saved_argv [argc] = (char *) malloc (i);
      memcpy (saved_argv [argc], argv [argc], i);
    }
}

void
#ifdef __STDC__
restart_process (saver_info *si)
#else  /* !__STDC__ */
restart_process (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  fflush (real_stdout);
  fflush (real_stderr);
  execvp (saved_argv [0], saved_argv);	/* shouldn't return */
  {
    char buf [512];
    sprintf (buf, "%s: could not restart process", progname);
    perror(buf);
    fflush(stderr);
  }
}

/* Like restart_process(), but ensures that when it restarts,
   it comes up in demo-mode. */
void
#ifdef __STDC__
demo_mode_restart_process (saver_info *si)
#else  /* !__STDC__ */
demo_mode_restart_process (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  int i;
  for (i = 0; saved_argv [i]; i++);
  /* add the -demo switch; save_argv() left room for this. */
  saved_argv [i] = "-demo";
  saved_argv [i+1] = 0;
  restart_process (si);		/* shouldn't return */
  saved_argv [i] = 0;
  XBell(si->dpy, 0);
}

void
#ifdef __STDC__
hack_environment (saver_info *si)
#else  /* !__STDC__ */
hack_environment (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  /* Store $DISPLAY into the environment, so that the $DISPLAY variable that
     the spawned processes inherit is the same as the value of -display passed
     in on our command line (which is not necessarily the same as what our
     $DISPLAY variable is.)
   */
  char *s, buf [2048];
  int i;
  sprintf (buf, "DISPLAY=%s", DisplayString (si->dpy));
  i = strlen (buf);
  s = (char *) malloc (i+1);
  strncpy (s, buf, i+1);

  /* Allegedly, BSD 4.3 didn't have putenv(), but nobody runs such systems
     any more, right?  It's not Posix, but everyone seems to have it. */
  if (putenv (s))
    abort ();
}


/* Change the uid/gid of the screensaver process, so that it is safe for it
   to run setuid root (which it needs to do on some systems to read the 
   encrypted passwords from the passwd file.)

   hack_uid() is run before opening the X connection, so that XAuth works.
   hack_uid_warn() is called after the connection is opened and the command
   line arguments are parsed, so that the messages from hack_uid() get 
   printed after we know whether we're in `verbose' mode.
 */

#ifndef NO_SETUID

static int hack_uid_errno;
static char hack_uid_buf [255], *hack_uid_error;

void
#ifdef __STDC__
hack_uid (saver_info *si)
#else  /* !__STDC__ */
hack_uid (si)
	saver_info *si;
#endif /* !__STDC__ */
{

  /* If we've been run as setuid or setgid to someone else (most likely root)
     turn off the extra permissions so that random user-specified programs
     don't get special privileges.  (On some systems it might be necessary
     to install this as setuid root in order to read the passwd file to
     implement lock-mode...)
  */
  setgid (getgid ());
  setuid (getuid ());

  hack_uid_errno = 0;
  hack_uid_error = 0;

  /* If we're being run as root (as from xdm) then switch the user id
     to something safe. */
  if (getuid () == 0)
    {
      struct passwd *p;
      /* Locking can't work when running as root, because we have no way of
	 knowing what the user id of the logged in user is (so we don't know
	 whose password to prompt for.)
       */
      si->locking_disabled_p = True;
      si->nolock_reason = "running as root";
      p = getpwnam ("nobody");
      if (! p) p = getpwnam ("daemon");
      if (! p) p = getpwnam ("bin");
      if (! p) p = getpwnam ("sys");
      if (! p)
	{
	  hack_uid_error = "couldn't find safe uid; running as root.";
	  hack_uid_errno = -1;
	}
      else
	{
	  struct group *g = getgrgid (p->pw_gid);
	  hack_uid_error = hack_uid_buf;
	  sprintf (hack_uid_error, "changing uid/gid to %s/%s (%ld/%ld).",
		   p->pw_name, (g ? g->gr_name : "???"),
		   (long) p->pw_uid, (long) p->pw_gid);

	  /* Change the gid to be a safe one.  If we can't do that, then
	     print a warning.  We change the gid before the uid so that we
	     change the gid while still root. */
	  if (setgid (p->pw_gid) != 0)
	    {
	      hack_uid_errno = errno;
	      sprintf (hack_uid_error, "couldn't set gid to %s (%ld)",
		       (g ? g->gr_name : "???"), (long) p->pw_gid);
	    }

	  /* Now change the uid to be a safe one. */
	  if (setuid (p->pw_uid) != 0)
	    {
	      hack_uid_errno = errno;
	      sprintf (hack_uid_error, "couldn't set uid to %s (%ld)",
		       p->pw_name, (long) p->pw_uid);
	    }
	}
    }
#ifndef NO_LOCKING
 else	/* disable locking if already being run as "someone else" */
   {
     struct passwd *p = getpwuid (getuid ());
     if (!p ||
	 !strcmp (p->pw_name, "root") ||
	 !strcmp (p->pw_name, "nobody") ||
	 !strcmp (p->pw_name, "daemon") ||
	 !strcmp (p->pw_name, "bin") ||
	 !strcmp (p->pw_name, "sys"))
       {
	 si->locking_disabled_p = True;
	 si->nolock_reason = hack_uid_buf;
	 sprintf (si->nolock_reason, "running as %s", p->pw_name);
       }
   }
#endif /* NO_LOCKING */
}

void
#ifdef __STDC__
hack_uid_warn (saver_info *si)
#else  /* !__STDC__ */
hack_uid_warn (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  saver_preferences *p = &si->prefs;

  if (! hack_uid_error)
    ;
  else if (hack_uid_errno == 0)
    {
      if (p->verbose_p)
	printf ("%s: %s\n", progname, hack_uid_error);
    }
  else
    {
      char buf [255];
      sprintf (buf, "%s: %s", progname, hack_uid_error);
      if (hack_uid_errno == -1)
	fprintf (stderr, "%s\n", buf);
      else
	{
	  errno = hack_uid_errno;
	  perror (buf);
	}
    }
}

#endif /* !NO_SETUID */
