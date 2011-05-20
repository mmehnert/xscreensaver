/* passwd.c --- verifying typed passwords with the OS.
 * xscreensaver, Copyright (c) 1993-1997 Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#ifndef NO_LOCKING  /* whole file */

#ifdef __STDC__
# include <stdlib.h>
# ifdef __unix
#  include <unistd.h>
# endif
#else /* !__STDC__ */
# ifndef const
#  define const /**/
# endif
#endif /* !__STDC__ */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#if !defined(VMS) && !defined(HAVE_ADJUNCT_PASSWD)
# include <pwd.h>
#endif


#ifdef __bsdi__
# include <sys/param.h>
# if _BSDI_VERSION >= 199608
#  define BSD_AUTH
# endif
#endif /* __bsdi__ */




#define Bool int	/* blargh */
#define True 1
#define False 0


extern char *progname;

static char *encrypted_root_passwd = 0;
static char *encrypted_user_passwd = 0;


#if defined(HAVE_SHADOW)		/* passwds live in /etc/shadow */

#   include <shadow.h>
#   define PWTYPE   struct spwd *
#   define PW_PSLOT sp_pwdp
#   define GETPW    getspnam

#elif defined(HAVE_DEC_ENHANCED)	/* passwds live in /tcb/files/auth/ */

#   include <sys/security.h>
#   include <prot.h>

#   define PWTYPE   struct pr_passwd *
#   define PW_PSLOT ufld.fd_encrypt
#   define GETPW    getprpwnam

#elif defined(SCO)			/* SCO = DEC + different headers */

#   include <sys/security.h>
#   include <sys/audit.h>
#   include <prot.h>

#   define PRTYPE   struct pr_passwd *
#   define PW_PSLOT ufld.fd_encrypt
#   define GETPW    getprpwnam

#elif defined(HAVE_ADJUNCT_PASSWD)

#   include <sys/label.h>
#   include <sys/audit.h>
#   include <pwdadj.h>

#   define PRTYPE   passwd_adjunct *
#   define PWPSLOT  pwa_passwd
#   define GETPW    getpwanam

#else				/* good old vanilla, marginally secure Unix */

#   define PWTYPE   struct passwd *
#   define PWPSLOT  pw_passwd
#   define GETPW    getpwnam

#endif


/* This has to be called before we've changed our effective user ID,
   because it might need priveleges to get at the encrypted passwords.
 */
Bool
#ifdef __STDC__
lock_init (int argc, char **argv)
#else  /* !__STDC__ */
lock_init (argc, argv)
	int argc;
	char **argv;
#endif /* !__STDC__ */
{
  Bool ok = True;
  char *u;
  PWTYPE p;

#ifdef HAVE_DEC_ENHANCED      /* from M.Matsumoto <matsu@yao.sharp.co.jp> */
  set_auth_parameters(argc, argv);
  check_auth_parameters();
#endif /* HAVE_DEC_ENHANCED */

  p = GETPW ("root");

  if (p && p->PWPSLOT && p->PWPSLOT[0] != '*')
    encrypted_root_passwd = strdup(p->PWPSLOT);
  else
    {
      fprintf (stderr, "%s: couldn't get root's password\n", progname);
      encrypted_root_passwd = strdup("*");
    }

  /* It has been reported that getlogin() returns the wrong user id on some
     very old SGI systems... */

  u = (char *) getlogin ();
  if (u)
    u = strdup(u);
  else
    {
      /* getlogin() fails if not attached to a terminal; in that case, use
	 getpwuid().  (Note that in this case, we're not doing shadow stuff,
	 since all we're interested in is the name, not the password.  So
	 that should still work.  Right?) */
      struct passwd *p2 = getpwuid (getuid ());
      u = (p2->pw_name ? strdup (p2->pw_name) : 0);
    }

  p = GETPW (u);

  if (p && p->PWPSLOT &&
      /* p->PWPSLOT[0] != '*' */	/* sensible */
      (strlen (p->PWPSLOT) > 4)		/* solaris */
      )
    encrypted_user_passwd = strdup(p->PWPSLOT);
  else
    {
      fprintf (stderr, "%s: couldn't get password of \"%s\"\n", progname,
	       (u ? u : "(null)"));
      encrypted_user_passwd = strdup("*");
      ok = False;
    }

  if (u) free (u);
  return ok;
}


/* This can be called at any time, and says whether the typed password
   belongs to either the logged in user (real uid, not effective); or
   to root.
 */
Bool
#ifdef __STDC__
passwd_valid_p (const char *typed_passwd)
#else  /* !__STDC__ */
passwd_valid_p (typed_passwd)
	const char *typed_passwd;
#endif /* !__STDC__ */
{
  if (encrypted_user_passwd &&
      !strcmp ((char *) crypt (typed_passwd, encrypted_user_passwd),
	       encrypted_user_passwd))
    return 1;

  /* do not allow root to have a null password. */
  else if (typed_passwd[0] &&
	   encrypted_root_passwd &&
	   !strcmp ((char *) crypt (typed_passwd, encrypted_root_passwd),
		    encrypted_root_passwd))
    return 1;

  else
    return 0;
}

#endif /* NO_LOCKING -- whole file */
