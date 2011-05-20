/* xscreensaver, Copyright (c) 1991-1997 Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xmu/SysUtil.h>

#include "xscreensaver.h"

extern Time timeout;
extern Bool screen_blanked_p;
extern Bool use_mit_saver_extension;
extern Bool use_sgi_saver_extension;


/* MIT SCREEN-SAVER server extension hackery.
 */

#ifdef HAVE_MIT_SAVER_EXTENSION

# include <X11/extensions/scrnsaver.h>
int mit_saver_ext_event_number = 0;
int mit_saver_ext_error_number = 0;
Window server_mit_saver_window = 0;

Bool
#ifdef __STDC__
query_mit_saver_extension (Display *dpy)
#else  /* !__STDC__ */
query_mit_saver_extension (dpy) Display *dpy;
#endif /* !__STDC__ */
{
  return XScreenSaverQueryExtension (dpy,
				     &mit_saver_ext_event_number,
				     &mit_saver_ext_error_number);
}

static int
#ifdef __STDC__
ignore_all_errors_ehandler (Display *dpy, XErrorEvent *error)
#else /* ! __STDC__ */
ignore_all_errors_ehandler (dpy, error) Display *dpy; XErrorEvent *error;
#endif /* ! __STDC__ */
{
  return 0;
}

static void
init_mit_saver_extension P((void))
{
  XID kill_id;
  Atom kill_type;
  Window root = RootWindowOfScreen (screen);
  Pixmap blank_pix = XCreatePixmap (dpy, root, 1, 1, 1);

  /* Kill off the old MIT-SCREEN-SAVER client if there is one.
     This tends to generate X errors, though (possibly due to a bug
     in the server extension itself?) so just ignore errors here. */
  if (XScreenSaverGetRegistered (dpy, XScreenNumberOfScreen (screen),
				 &kill_id, &kill_type)
      && kill_id != blank_pix)
    {
      int (*old_handler) ();
      old_handler = XSetErrorHandler (ignore_all_errors_ehandler);
      XKillClient (dpy, kill_id);
      XSync (dpy, False);
      XSetErrorHandler (old_handler);
    }

  XScreenSaverSelectInput (dpy, root, ScreenSaverNotifyMask);

  XScreenSaverRegister (dpy, XScreenNumberOfScreen (screen),
			(XID) blank_pix, XA_PIXMAP);
}
#endif /* HAVE_MIT_SAVER_EXTENSION */


/* SGI SCREEN_SAVER server extension hackery.
 */

#ifdef HAVE_SGI_SAVER_EXTENSION

# include <X11/extensions/XScreenSaver.h>
int sgi_saver_ext_event_number = 0;
int sgi_saver_ext_error_number = 0;

Bool
#ifdef __STDC__
query_sgi_saver_extension (Display *dpy)
#else  /* !__STDC__ */
query_sgi_saver_extension (dpy) Display *dpy;
#endif /* !__STDC__ */
{
  return XScreenSaverQueryExtension (dpy,
				     &sgi_saver_ext_event_number,
				     &sgi_saver_ext_error_number);
}

static void
init_sgi_saver_extension P((void))
{
  if (screen_blanked_p)
    /* If you mess with this while the server thinks it's active,
       the server crashes. */
    return;

  XScreenSaverDisable (dpy, XScreenNumberOfScreen(screen));
  if (! XScreenSaverEnable (dpy, XScreenNumberOfScreen(screen)))
    {
      fprintf (stderr,
       "%s: %sSGI SCREEN_SAVER extension exists, but can't be initialized;\n\
		perhaps some other screensaver program is already running?\n",
	       progname, (verbose_p ? "## " : ""));
      use_sgi_saver_extension = False;
    }
}

#endif /* HAVE_SGI_SAVER_EXTENSION */


/* Figuring out what the appropriate XSetScreenSaver() paramters are
   (one wouldn't expect this to be rocket science.)
 */

void
#ifdef __STDC__
disable_builtin_screensaver (Bool turn_off_p)
#else  /* !__STDC__ */
disable_builtin_screensaver (turn_off_p)
  Bool turn_off_p;
#endif /* !__STDC__ */
{
  int current_server_timeout, current_server_interval;
  int current_prefer_blank, current_allow_exp;
  int desired_server_timeout, desired_server_interval;
  int desired_prefer_blank, desired_allow_exp;

  XGetScreenSaver (dpy, &current_server_timeout, &current_server_interval,
		   &current_prefer_blank, &current_allow_exp);

  desired_server_timeout = current_server_timeout;
  desired_server_interval = current_server_interval;
  desired_prefer_blank = current_prefer_blank;
  desired_allow_exp = current_allow_exp;

  /* On SGIs, if interval is non-zero, it is the number of seconds after
     screen saving starts at which the monitor should be powered down.
     Obviously I don't want that, so make sure it's 0.

     Power saving is disabled if DontPreferBlanking, but in that case,
     we don't get extension events either.  So we can't turn it off that way.
   */
  desired_server_interval = 0;

  /* I suspect (but am not sure) that DontAllowExposures might have
     something to do with powering off the monitor as well. */
  desired_allow_exp = AllowExposures;

#if defined(HAVE_MIT_SAVER_EXTENSION) || defined(HAVE_SGI_SAVER_EXTENSION)
  if (use_mit_saver_extension || use_sgi_saver_extension)
    {
      desired_server_timeout = (timeout / 1000);

      /* The SGI extension won't give us events unless blanking is on.
	 I think (unsure right now) that the MIT extension is the opposite. */
      if (use_sgi_saver_extension)
	desired_prefer_blank = PreferBlanking;
      else
	desired_prefer_blank = DontPreferBlanking;
    }
  else
#endif /* HAVE_MIT_SAVER_EXTENSION || HAVE_SGI_SAVER_EXTENSION */
    {
      desired_server_timeout = 0;
    }

  if (desired_server_timeout != current_server_timeout ||
      desired_server_interval != current_server_interval ||
      desired_prefer_blank != current_prefer_blank ||
      desired_allow_exp != current_allow_exp)
    {
      if (desired_server_timeout == 0)
	printf ("%s%sisabling server builtin screensaver.\n\
	You can re-enable it with \"xset s on\".\n",
		(verbose_p ? "" : progname), (verbose_p ? "\n\tD" : ": d"));

      if (verbose_p)
	fprintf (stderr, "%s: (xset s %d %d %s %s)\n", progname,
		 desired_server_timeout, desired_server_interval,
		 (desired_prefer_blank ? "blank" : "noblank"),
		 (desired_allow_exp ? "noexpose" : "expose"));

      XSetScreenSaver (dpy, desired_server_timeout, desired_server_interval,
		       desired_prefer_blank, desired_allow_exp);
      XSync(dpy, False);
    }


#if defined(HAVE_MIT_SAVER_EXTENSION) || defined(HAVE_SGI_SAVER_EXTENSION)
  {
    static Bool extension_initted = False;
    if (!extension_initted)
      {
	extension_initted = True;
# ifdef HAVE_MIT_SAVER_EXTENSION
	if (use_mit_saver_extension) init_mit_saver_extension();
# endif
# ifdef HAVE_SGI_SAVER_EXTENSION
	if (use_sgi_saver_extension) init_sgi_saver_extension();
# endif
      }
  }
#endif /* HAVE_MIT_SAVER_EXTENSION || HAVE_SGI_SAVER_EXTENSION */

  if (turn_off_p)
    /* Turn off the server builtin saver if it is now running. */
    XForceScreenSaver (dpy, ScreenSaverReset);
}
