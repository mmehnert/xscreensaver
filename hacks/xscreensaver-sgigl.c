/* xscreensaver, Copyright (c) 1997 Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

/* This is a kludge that lets xscreensaver work with SGI demos that expect
   to be run from `haven'.  It runs the program given on the command line,
   then waits for an X window to be created whose name is that of the 
   program.  Then, it resizes that window to fill the screen.  Run it
   like this:

           xscreensaver-sgigl /usr/demos/bin/ep -S
 */

static char *progname;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <X11/Xlib.h>

void
main(int ac, char **av)
{
  char buf [512];
  pid_t parent, forked;
  Display *dpy;
  Screen *screen;
  char *s;

  progname = av[0];

  s = strrchr(progname, '/');
  if (s) progname = s+1;

  if (ac < 1)
    {
      fprintf(stderr, "usage: %s program arguments...\n", progname);
      exit(1);
    }

  dpy = XOpenDisplay(0);
  if (!dpy)
    {
      fprintf(stderr, "%s: couldn't open display\n", progname);
      exit(1);
    }

  screen = DefaultScreenOfDisplay(dpy);
  XSelectInput (dpy, RootWindowOfScreen(screen),
		SubstructureNotifyMask);

  parent = getpid();
  switch ((int) (forked = fork ()))
    {
    case -1:
      {
	sprintf (buf, "%s: couldn't fork", progname);
	perror (buf);
	exit (1);
	break;
      }
    case 0:	/* forked */
      {
	time_t start = time((time_t) 0);
	XEvent event;
	s = strrchr(av[1], '/');
	if (s) s++;
	else s = av[1];

	while (1)
	  {
	    XNextEvent(dpy, &event);

	    if (event.xany.type == CreateNotify)
	      {
		char *name = 0;
		Window w = event.xcreatewindow.window;
		XFetchName(dpy, w, &name);
		if (name && !strcmp(name, s))
		  {
		    XMoveResizeWindow(dpy, w, 0, 0,
				      WidthOfScreen(screen),
				      HeightOfScreen(screen));
		    XSync(dpy, False);
		    fflush(stdout);
		    fflush(stderr);
		    exit(0);	/* Note that this only exits a child fork.  */
		  }
	      }

	    if (start + 5 < time((time_t) 0))
	      {
		fprintf(stderr,
		    "%s: timed out: no window named \"%s\" has been created\n",
			progname, s);
		fflush(stdout);
		fflush(stderr);
		kill(parent, SIGTERM);
		exit(1);
	      }
	  }
	break;
      }
    default:	/* foreground */
      {
	close (ConnectionNumber (dpy));		/* close display fd */
	execvp (av[1], av+1);			/* shouldn't return. */
	sprintf (buf, "%s: execvp(\"%s\") failed", progname, av[1]);
	perror (buf);
	fflush(stderr);
	fflush(stdout);
	exit (1);
	break;
      }
    }
}
