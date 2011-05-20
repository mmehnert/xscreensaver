/* xscreensaver, Copyright (c) 1991-1998 Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#ifdef HAVE_MOTIF
# include <Xm/Xm.h>

#else  /* HAVE_ATHENA */
# include <X11/Shell.h>
# include <X11/Xaw/Form.h>
# include <X11/Xaw/Box.h>
# include <X11/Xaw/Command.h>
# include <X11/Xaw/Dialog.h>
#endif /* HAVE_ATHENA */

#include "xscreensaver.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

extern Widget splash_dialog;
extern Widget splash_form;
extern Widget splash_roger_label;
extern Widget splash_label1;
extern Widget splash_label3;
extern Widget splash_demo;
extern Widget splash_prefs;
extern Widget splash_help;

static void
splash_sink(saver_info *si)
{
  if (si->splash_dialog)
    {
      XtDestroyWidget(si->splash_dialog);
      si->splash_dialog = 0;
    }
}

static void
splash_sink_timer (XtPointer closure, XtIntervalId *id)
{
  saver_info *si = (saver_info *) closure;
  splash_sink(si);
}


static void
send_self_clientmessage (saver_info *si, Atom command)
{
  Display *dpy = si->dpy;
  Window window = si->default_screen->screensaver_window;
  XEvent event;
  event.xany.type = ClientMessage;
  event.xclient.display = si->dpy;
  event.xclient.window = window;
  event.xclient.message_type = XA_SCREENSAVER;
  event.xclient.format = 32;
  event.xclient.data.l[0] = (long) command;
  if (! XSendEvent (dpy, window, False, 0L, &event))
    fprintf (stderr, "%s: XSendEvent(dpy, 0x%x ...) failed.\n",
	     progname, (unsigned int) window);
}

static void
splash_demo_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  saver_info *si = (saver_info *) client_data;
  splash_sink(si);
  send_self_clientmessage (si, XA_DEMO);
}

static void
splash_prefs_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  saver_info *si = (saver_info *) client_data;
  splash_sink(si);
  send_self_clientmessage (si, XA_PREFS);
}

static void
splash_help_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  saver_info *si = (saver_info *) client_data;
  saver_preferences *p = &si->prefs;

  splash_sink(si);

  if (!p->help_url || !*p->help_url)
    fprintf(stderr, "%s: no Help URL has been specified.\n");
  else if (!p->load_url_command || !*p->load_url_command)
    fprintf(stderr, "%s: no URL-loading command has been specified.\n");
  else
    {
      char *buf = (char *) malloc (strlen(p->load_url_command) +
				   (strlen(p->help_url) * 2) + 10);
      sprintf(buf, p->load_url_command, p->help_url, p->help_url);
      system(buf);
    }
}

static void
make_splash_dialog (saver_info *si)
{
  saver_screen_info *ssi = si->default_screen;
  Widget parent = ssi->toplevel_shell;

  create_splash_dialog (parent, ssi->default_visual,
			DefaultColormapOfScreen (ssi->screen));
  si->splash_dialog = splash_dialog; /* gaaah... */

#ifdef HAVE_ATHENA
  XawDialogAddButton(splash_form,"demo",  splash_demo_cb, si);
  XawDialogAddButton(splash_form,"prefs", splash_prefs_cb, si);
  XawDialogAddButton(splash_form,"help",  splash_help_cb, si);
  splash_demo  = XtNameToWidget(splash_form,"demo");
  splash_prefs = XtNameToWidget(splash_form,"prefs");
  splash_help  = XtNameToWidget(splash_form,"help");

  /* Lose the label on the inner dialog. */
  {
    Widget w = XtNameToWidget(splash_form, "label");
    if (w) XtUnmanageChild(w);
  }

#else  /* HAVE_MOTIF */
  /* Another random thing necessary in 1.2.1 but not 1.1.5... */
  XtVaSetValues (splash_roger_label, XmNborderWidth, 1, 0);

  XtAddCallback (splash_demo, XmNactivateCallback, splash_demo_cb, si);
  XtAddCallback (splash_prefs, XmNactivateCallback, splash_prefs_cb, si);
  XtAddCallback (splash_help, XmNactivateCallback, splash_help_cb, si);
  XtAddCallback (splash_roger_label, XmNexposeCallback, roger, si);

#endif /* HAVE_MOTIF */

  format_into_label (splash_label1, si->version);

  XtRealizeWidget(splash_dialog);
}

void
pop_splash_dialog (saver_info *si)
{
  XtIntervalId splash_sink_id;

  if (si->prefs.splash_duration <= 0)
    return;

  if (! si->splash_dialog)
    make_splash_dialog (si);

#ifdef HAVE_ATHENA
  splash_form = splash_dialog; /* kludge */
#endif

  pop_up_dialog_box (splash_dialog, splash_form,
		     /* for debugging -- don't ask */
		     (si->prefs.debug_p ? 69 : 0) +
		     3);
  XtManageChild (splash_form);

#ifdef HAVE_ATHENA
  if (splash_roger_label)
    roger (splash_roger_label, 0, 0);
#endif /* HAVE_ATHENA */

  splash_sink_id = XtAppAddTimeOut (si->app, si->prefs.splash_duration,
				    splash_sink_timer, (XtPointer) si);
}
