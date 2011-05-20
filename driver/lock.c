/* lock.c --- handling the password dialog for locking-mode.
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

/* Athena locking code contributed by Jon A. Christopher <jac8782@tamu.edu> */
/* Copyright 1997, with the same permissions as above. */

#ifndef NO_LOCKING   /* whole file */

#include <X11/Intrinsic.h>
#include "xscreensaver.h"

#ifndef VMS
# include <pwd.h>
#endif

#ifndef __STDC__
# define _NO_PROTO
#endif

#ifdef NO_MOTIF
# undef  USE_MOTIF
# define USE_ATHENA
#else
# define USE_MOTIF
# undef  USE_ATHENA
#endif



#ifdef USE_ATHENA

# include <X11/Shell.h>
# include <X11/StringDefs.h>
# include <X11/Xaw/Text.h>
# include <X11/Xaw/Label.h>
# include <X11/Xaw/Dialog.h>

static void passwd_done_cb P((Widget, XtPointer, XtPointer));
static XtActionsRec actionsList[] =
{
    {"passwdentered", (XtActionProc) passwd_done_cb},
};

static char Translations[] =
"\
<Key>Return:   passwdentered()\
";

#else  /* USE_MOTIF */

# include <Xm/Xm.h>
# include <Xm/List.h>
# include <Xm/TextF.h>

#endif /* USE_MOTIF */

extern Widget passwd_dialog;
extern Widget passwd_form;
extern Widget roger_label;
extern Widget passwd_label1;
extern Widget passwd_label3;
extern Widget passwd_cancel;

#ifdef USE_MOTIF
extern Widget passwd_text;
extern Widget passwd_done;
#else  /* USE_ATHENA */
static Widget passwd_text = 0;	/* gag... */
static Widget passwd_done = 0;
#endif /* USE_ATHENA */



static enum { pw_read, pw_ok, pw_fail, pw_cancel, pw_time } passwd_state;
#define PASSWDLEN 80
static char typed_passwd [PASSWDLEN];


#if defined(USE_ATHENA) || (XmVersion >= 1002)
   /* The `destroy' bug apears to be fixed as of Motif 1.2.1, but
      the `verify-callback' bug is still present. */
# define DESTROY_WORKS
#endif

static void
#ifdef __STDC__
passwd_cancel_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
passwd_cancel_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  passwd_state = pw_cancel;
}

static void
#ifdef __STDC__
passwd_done_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
passwd_done_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  if (passwd_state != pw_read) return; /* already done */
#ifdef USE_ATHENA
  strncpy(typed_passwd, XawDialogGetValueString(passwd_form), PASSWDLEN);
#endif /* USE_ATHENA */
  if (passwd_valid_p (typed_passwd))
    passwd_state = pw_ok;
  else
    passwd_state = pw_fail;
}

#if defined(USE_MOTIF) && defined(VERIFY_CALLBACK_WORKS)

  /* It looks to me like adding any modifyVerify callback causes
     Motif 1.1.4 to free the the TextF_Value() twice.  I can't see
     the bug in the Motif source, but Purify complains, even if
     check_passwd_cb() is a no-op.

     Update: Motif 1.2.1 also loses, but in a different way: it
     writes beyond the end of a malloc'ed block in ModifyVerify().
     Probably this block is the text field's text.
   */

static void 
#ifdef __STDC__
check_passwd_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
check_passwd_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  XmTextVerifyCallbackStruct *vcb = (XmTextVerifyCallbackStruct *) call_data;

  if (passwd_state != pw_read)
    return;
  else if (vcb->reason == XmCR_ACTIVATE)
    {
      passwd_done_cb (0, 0, 0);
    }
  else if (vcb->text->length > 1)	/* don't allow "paste" operations */
    {
      vcb->doit = False;
    }
  else if (vcb->text->ptr != 0)
    {
      int i;
      strncat (typed_passwd, vcb->text->ptr, vcb->text->length);
      typed_passwd [vcb->endPos + vcb->text->length] = 0;
      for (i = 0; i < vcb->text->length; i++)
	vcb->text->ptr [i] = '*';
    }
}

# else /* USE_ATHENA || !VERIFY_CALLBACK_WORKS */

static void keypress P((Widget w, XEvent *event, String *av, Cardinal *ac));
static void backspace P((Widget w, XEvent *event, String *av, Cardinal *ac));
static void kill_line P((Widget w, XEvent *event, String *av, Cardinal *ac));
static void done P((Widget w, XEvent *event, String *av, Cardinal *ac));

static XtActionsRec actions[] = {{"keypress",  keypress},
				 {"backspace", backspace},
				 {"kill_line", kill_line},
				 {"done",      done}
			        };

# ifdef USE_MOTIF
#  if 0  /* oh fuck, why doesn't this work? */
static char translations[] = "\
<Key>BackSpace:		backspace()\n\
<Key>Delete:		backspace()\n\
Ctrl<Key>H:		backspace()\n\
Ctrl<Key>U:		kill_line()\n\
Ctrl<Key>X:		kill_line()\n\
Ctrl<Key>J:		done()\n\
Ctrl<Key>M:		done()\n\
<Key>:			keypress()\n\
";
#  else  /* !0 */
static char translations[] = "<Key>:keypress()";
#  endif /* !0 */
# endif /* USE_MOTIF */


static void
#ifdef __STDC__
text_field_set_string (Widget widget, char *text, int position)
#else /* ! __STDC__ */
text_field_set_string (widget, text, position)
     Widget widget;
     char *text;
     int position;
#endif /* ! __STDC__ */
{
#ifdef USE_MOTIF
  XmTextFieldSetString (widget, text);
  XmTextFieldSetInsertionPosition (widget, position);

#else /* USE_ATHENA */
  char *buf;
  int end_pos;

  XawTextBlock block;
  block.firstPos = 0;
  block.length = strlen (text);
  block.ptr = text;
  block.format = 0;
  if (block.length == 0)
    {
      buf = XawDialogGetValueString(passwd_form);
      if (buf)
	end_pos = strlen(buf);
      else
	end_pos = -1;
    }
  XawTextReplace (widget, 0, end_pos, &block);
  XawTextSetInsertionPoint (widget, position);
#endif /* USE_ATHENA */
}


static void
#ifdef __STDC__
keypress (Widget w, XEvent *event, String *argv, Cardinal *argc)
#else /* ! __STDC__ */
keypress (w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
#endif /* ! __STDC__ */
{
  int i, j;
  char s [sizeof (typed_passwd)];
  int size = XLookupString ((XKeyEvent *) event, s, sizeof (s), 0, 0);
  if (size != 1) return;

  /* hack because I can't get translations to dance to my tune... */
  if (*s == '\010') { backspace (w, event, argv, argc); return; }
  if (*s == '\177') { backspace (w, event, argv, argc); return; }
  if (*s == '\025') { kill_line (w, event, argv, argc); return; }
  if (*s == '\030') { kill_line (w, event, argv, argc); return; }
  if (*s == '\012') { done (w, event, argv, argc); return; }
  if (*s == '\015') { done (w, event, argv, argc); return; }

  i = j = strlen (typed_passwd);
  typed_passwd [i] = *s;
  s [++i] = 0;
  while (i--)
    s [i] = '*';

  text_field_set_string (passwd_text, s, j + 1);
}

static void
#ifdef __STDC__
backspace (Widget w, XEvent *event, String *argv, Cardinal *argc)
#else /* ! __STDC__ */
backspace (w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
#endif /* ! __STDC__ */
{
  char s [sizeof (typed_passwd)];
  int i = strlen (typed_passwd);
  int j = i;
  if (i == 0)
    return;
  typed_passwd [--i] = 0;
  s [i] = 0;
  while (i--)
    s [i] = '*';

  text_field_set_string (passwd_text, s, j + 1);
}

static void
#ifdef __STDC__
kill_line (Widget w, XEvent *event, String *argv, Cardinal *argc)
#else /* ! __STDC__ */
kill_line (w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
#endif /* ! __STDC__ */
{
  memset (typed_passwd, 0, sizeof (typed_passwd));
  text_field_set_string (passwd_text, "", 0);
}

static void
#ifdef __STDC__
done (Widget w, XEvent *event, String *argv, Cardinal *argc)
#else /* ! __STDC__ */
done (w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
#endif /* ! __STDC__ */
{
  passwd_done_cb (w, 0, 0);
}

#endif /* USE_ATHENA || !VERIFY_CALLBACK_WORKS */


#ifdef __STDC__
extern void skull (Display *, Window, GC, GC, int, int, int, int);
#endif

static void
#ifdef __STDC__
roger (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
roger (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  Display *dpy = XtDisplay (button);
  Screen *screen = XtScreen (button);
  Window window = XtWindow (button);
  Arg av [10];
  int ac = 0;
  XGCValues gcv;
  Colormap cmap;
  GC draw_gc, erase_gc;
  unsigned int fg, bg;
  int x, y, size;
  XWindowAttributes xgwa;
  XGetWindowAttributes (dpy, window, &xgwa);
  cmap = xgwa.colormap;
  if (xgwa.width > xgwa.height) size = xgwa.height;
  else size = xgwa.width;
  if (size > 40) size -= 30;
  x = (xgwa.width - size) / 2;
  y = (xgwa.height - size) / 2;
  XtSetArg (av [ac], XtNforeground, &fg); ac++;
  XtSetArg (av [ac], XtNbackground, &bg); ac++;
  XtGetValues (button, av, ac);
  /* if it's black on white, swap it cause it looks better (hack hack) */
  if (fg == BlackPixelOfScreen (screen) && bg == WhitePixelOfScreen (screen))
    fg = WhitePixelOfScreen (screen), bg = BlackPixelOfScreen (screen);
  gcv.foreground = bg;
  erase_gc = XCreateGC (dpy, window, GCForeground, &gcv);
  gcv.foreground = fg;
  draw_gc = XCreateGC (dpy, window, GCForeground, &gcv);
  XFillRectangle (dpy, window, erase_gc, 0, 0, xgwa.width, xgwa.height);
  skull (dpy, window, draw_gc, erase_gc, x, y, size, size);
  XFreeGC (dpy, draw_gc);
  XFreeGC (dpy, erase_gc);
}

static void
#ifdef __STDC__
make_passwd_dialog (saver_info *si)
#else  /* !__STDC__ */
make_passwd_dialog (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  char *username = 0;
  saver_screen_info *ssi = si->default_screen;
  Widget parent = ssi->toplevel_shell;

  if (ssi->demo_cmap &&
      ssi->demo_cmap != ssi->cmap &&
      ssi->demo_cmap != DefaultColormapOfScreen (ssi->screen))
    {
      XFreeColormap (si->dpy, ssi->demo_cmap);
      ssi->demo_cmap = 0;
    }

  if (ssi->default_visual == DefaultVisualOfScreen (ssi->screen))
    ssi->demo_cmap = DefaultColormapOfScreen (ssi->screen);
  else
    ssi->demo_cmap = XCreateColormap (si->dpy,
				     RootWindowOfScreen (ssi->screen),
				     ssi->default_visual, AllocNone);

  create_passwd_dialog (parent, ssi->default_visual, ssi->demo_cmap);

#ifdef USE_ATHENA

  XtVaSetValues(passwd_form, XtNvalue, typed_passwd, 0);

  XawDialogAddButton(passwd_form,"ok", passwd_done_cb, 0);
  XawDialogAddButton(passwd_form,"cancel", passwd_cancel_cb, 0);
  passwd_done = XtNameToWidget(passwd_form,"ok");
  passwd_text = XtNameToWidget(passwd_form,"value");

  XtAppAddActions(XtWidgetToApplicationContext(passwd_text), 
		  actionsList, XtNumber(actionsList));
  XtOverrideTranslations(passwd_text, XtParseTranslationTable(Translations));

#else  /* USE_MOTIF */

  XtAddCallback (passwd_done, XmNactivateCallback, passwd_done_cb, 0);
  XtAddCallback (passwd_cancel, XmNactivateCallback, passwd_cancel_cb, 0);
  XtAddCallback (roger_label, XmNexposeCallback, roger, 0);

# ifdef VERIFY_CALLBACK_WORKS
  XtAddCallback (passwd_text, XmNmodifyVerifyCallback, check_passwd_cb, 0);
  XtAddCallback (passwd_text, XmNactivateCallback, check_passwd_cb, 0);
# else
  XtAddCallback (passwd_text, XmNactivateCallback, passwd_done_cb, 0);
  XtOverrideTranslations (passwd_text, XtParseTranslationTable (translations));
# endif

# if defined(USE_MOTIF) && (XmVersion >= 1002)
  /* The focus stuff changed around; this didn't exist in 1.1.5. */
  XtVaSetValues (passwd_form, XmNinitialFocus, passwd_text, 0);
# endif

  /* Another random thing necessary in 1.2.1 but not 1.1.5... */
  XtVaSetValues (roger_label, XmNborderWidth, 2, 0);

#endif /* USE_MOTIF */

#ifndef VMS
  {
    struct passwd *pw = getpwuid (getuid ());
    username = pw->pw_name;
  }
#else  /* VMS -- from "R.S.Niranjan" <U00C782%BRKVC1@navistar.com> who says
	         that on OpenVMS 6.1, using `struct passwd' crashes... */
  username = getenv("USER");
#endif /* VMS */

  format_into_label (passwd_label1, si->version);
  format_into_label (passwd_label3, (username ? username : "???"));
}

static int passwd_idle_timer_tick;
static XtIntervalId passwd_idle_id;

static void
#ifdef __STDC__
passwd_idle_timer (XtPointer closure, XtIntervalId *id)
#else  /* !__STDC__ */
passwd_idle_timer (closure, id)
	XtPointer closure;
	XtIntervalId *id;
#endif /* !__STDC__ */
{
  saver_info *si = (saver_info *) closure;
  saver_preferences *p = &si->prefs;

  Display *dpy = XtDisplay (passwd_form);
#ifdef USE_ATHENA
  Window window = XtWindow (passwd_form);
#else  /* MOTIF */
  Window window = XtWindow (XtParent(passwd_done));
#endif /* MOTIF */
  static Dimension x, y, d, s, ss;
  static GC gc = 0;
  int max = p->passwd_timeout / 1000;

  idle_timer ((XtPointer) si, id);

  if (passwd_idle_timer_tick == max)  /* first time */
    {
      XGCValues gcv;
#ifdef USE_MOTIF
      unsigned long fg, bg, ts, bs;
      Dimension w = 0, h = 0;
      XtVaGetValues(XtParent(passwd_done),
		    XmNwidth, &w,
		    0);
      XtVaGetValues(passwd_done,
		    XmNheight, &h,
		    XmNy, &y,
		    XtNforeground, &fg,
		    XtNbackground, &bg,
		    XmNtopShadowColor, &ts,
		    XmNbottomShadowColor, &bs,
		    0);

      if (ts != bg && ts != fg)
	fg = ts;
      if (bs != bg && bs != fg)
	fg = bs;

      d = h / 2;
      if (d & 1) d++;

      x = (w / 2);

      x -= d/2;
      y += d/2;

#else  /* USE_ATHENA */

      Arg av [100];
      int ac = 0;
      unsigned long fg, bg;
      XtSetArg (av [ac], XtNheight, &d); ac++;
      XtGetValues (passwd_done, av, ac);
      ac = 0;
      XtSetArg (av [ac], XtNwidth, &x); ac++;
      XtSetArg (av [ac], XtNheight, &y); ac++;
      XtSetArg (av [ac], XtNforeground, &fg); ac++;
      XtSetArg (av [ac], XtNbackground, &bg); ac++;
      XtGetValues (passwd_form, av, ac);
      x -= d;
      y -= d;
      d -= 4;

#endif /* USE_ATHENA */

      gcv.foreground = fg;
      if (gc) XFreeGC (dpy, gc);
      gc = XCreateGC (dpy, window, GCForeground, &gcv);
      s = 360*64 / (passwd_idle_timer_tick - 1);
      ss = 90*64;
      XFillArc (dpy, window, gc, x, y, d, d, 0, 360*64);
      XSetForeground (dpy, gc, bg);
      x += 1;
      y += 1;
      d -= 2;
    }

  if (--passwd_idle_timer_tick)
    {
      passwd_idle_id = XtAppAddTimeOut (si->app, 1000, passwd_idle_timer,
					(XtPointer) si);
      XFillArc (dpy, window, gc, x, y, d, d, ss, s);
      ss += s;
    }
}

#ifdef USE_ATHENA

void
#ifdef __STDC__
pop_up_athena_dialog_box (Widget parent, Widget focus, Widget dialog,
			  Widget form, int where)
#else  /* !__STDC__ */
pop_up_athena_dialog_box (parent, focus, dialog, form, where)
     Widget parent, focus, dialog, form;
     int where;
#endif /* !__STDC__ */
{
  /* modified from demo.c */
  /* I'm sure this is the wrong way to pop up a dialog box, but I can't
     figure out how else to do it.

     It's important that the screensaver dialogs not get decorated or
     otherwise reparented by the window manager, because they need to be
     children of the *real* root window, not the WM's virtual root, in
     order for us to guarentee that they are visible above the screensaver
     window itself.
   */
  Arg av [100];
  int ac = 0;
  Dimension sw, sh, x, y, w, h;

  XtRealizeWidget(dialog);
  sw = WidthOfScreen (XtScreen (dialog));
  sh = HeightOfScreen (XtScreen (dialog));
  ac = 0;
  XtSetArg (av [ac], XtNwidth, &w); ac++;
  XtSetArg (av [ac], XtNheight, &h); ac++;
  XtGetValues (form, av, ac);
  switch (where)
    {
    case 0:	/* center it in the top-right quadrant */
      x = (sw/2 + w) / 2 + (sw/2) - w;
      y = (sh/2 + h) / 2 - h;
      break;
    case 1:	/* center it in the bottom-right quadrant */
      x = (sw/2 + w) / 2 + (sw/2) - w;
      y = (sh/2 + h) / 2 + (sh/2) - h;
      break;
    case 2:	/* center it on the screen */
      x = (sw + w) / 2 - w;
      y = (sh + h) / 2 - h;
      break;
    default:
      abort ();
    }
  if (x + w > sw) x = sw - w;
  if (y + h > sh) y = sh - h;
  ac = 0;
  XtVaSetValues(dialog,
		XtNx, x,
		XtNy, y,
		NULL);
  XtVaSetValues(form,
		XtNx, x,
		XtNy, y,
		NULL);
  XtPopup(dialog,XtGrabNone);
  steal_focus_and_colormap (focus);
}

static void
#ifdef __STDC__
passwd_set_label (char *buf, int len)
#else /* ! __STDC__ */
passwd_set_label (buf,len) char *buf; int len;
#endif /* ! __STDC__ */
{
  Widget label;
  if (!passwd_text)
    return;
  label=XtNameToWidget(XtParent(passwd_text),"*label");
  XtVaSetValues(label,
		XtNlabel, buf,
		NULL);
}
#endif /* USE_ATHENA */

static Bool
#ifdef __STDC__
pop_passwd_dialog (saver_info *si)
#else  /* !__STDC__ */
pop_passwd_dialog (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  saver_preferences *p = &si->prefs;
  saver_screen_info *ssi = si->default_screen;
  Widget parent = ssi->toplevel_shell;
  Display *dpy = XtDisplay (passwd_dialog);
  Window focus;
  int revert_to;
  int i;

  typed_passwd [0] = 0;
  passwd_state = pw_read;
  text_field_set_string (passwd_text, "", 0);

  /* In case one of the hacks has unmapped it temporarily...
     Get that sucker on stage now! */
  for (i = 0; i < si->nscreens; i++)
    XMapRaised(si->dpy, si->screens[i].screensaver_window);

  XGetInputFocus (dpy, &focus, &revert_to);
#if defined(USE_MOTIF) && !defined(DESTROY_WORKS)
  /* This fucker blows up if we destroy the widget.  I can't figure
     out why.  The second destroy phase dereferences freed memory...
     So we just keep it around; but unrealizing or unmanaging it
     doesn't work right either, so we hack the window directly. FMH.
   */
  if (XtWindow (passwd_form))
    XMapRaised (dpy, XtWindow (passwd_dialog));
#endif

#ifdef USE_ATHENA
  pop_up_athena_dialog_box (parent, passwd_text, passwd_dialog,
			    passwd_form, 2);
#else
  pop_up_dialog_box (passwd_dialog, passwd_form,
#ifdef DEBUG
		     (si->prefs.debug_p ? 69 : 0) +
#endif
		     2);
  XtManageChild (passwd_form);
#endif

#if defined(USE_MOTIF) && (XmVersion < 1002)
  /* The focus stuff changed around; this causes problems in 1.2.1
     but is necessary in 1.1.5. */
  XmProcessTraversal (passwd_text, XmTRAVERSE_CURRENT);
#endif

  passwd_idle_timer_tick = p->passwd_timeout / 1000;
  passwd_idle_id = XtAppAddTimeOut (si->app, 1000,  passwd_idle_timer,
				    (XtPointer) si);

#ifdef USE_ATHENA
  if (roger_label)
    roger(roger_label, 0, 0);
#endif /* USE_ATHENA */

#ifdef DEBUG
  if (!si->prefs.debug_p)
#endif
  XGrabServer (dpy);				/* ############ DANGER! */

  /* this call to ungrab used to be in main_loop() - see comment in
      xscreensaver.c around line 857. */
  ungrab_keyboard_and_mouse (si->dpy);

  while (passwd_state == pw_read)
    {
      XEvent event;
      XtAppNextEvent (si->app, &event);
      /* wait for timer event */
      if (event.xany.type == 0 && passwd_idle_timer_tick == 0)
	passwd_state = pw_time;
      XtDispatchEvent (&event);
    }
  XUngrabServer (dpy);
  XSync (dpy, False);				/* ###### (danger over) */

  if (passwd_state != pw_time)
    XtRemoveTimeOut (passwd_idle_id);

  if (passwd_state != pw_ok)
    {
      char *lose;
      switch (passwd_state)
	{
	case pw_time: lose = "Timed out!"; break;
	case pw_fail: lose = "Sorry!"; break;
	case pw_cancel: lose = 0; break;
	default: abort ();
	}
#ifdef USE_MOTIF
      XmProcessTraversal (passwd_cancel, 0); /* turn off I-beam */
#endif
      if (lose)
	{
#ifdef USE_ATHENA
	  /* show the message */
	  passwd_set_label(lose,strlen(lose)+1);

	  /* and clear the password line */
	  memset(typed_passwd, 0, PASSWDLEN);
	  text_field_set_string (passwd_text, "", 0);
#else
	  text_field_set_string (passwd_text, lose, strlen (lose) + 1);
#endif
	  passwd_idle_timer_tick = 1;
	  passwd_idle_id = XtAppAddTimeOut (si->app, 3000, passwd_idle_timer,
				(XtPointer) si);
	  while (1)
	    {
	      XEvent event;
	      XtAppNextEvent (si->app, &event);
	      if (event.xany.type == 0 &&	/* wait for timer event */
		  passwd_idle_timer_tick == 0)
		break;
	      XtDispatchEvent (&event);
	    }
	}
    }
  memset (typed_passwd, 0, sizeof (typed_passwd));
  text_field_set_string (passwd_text, "", 0);
  XtSetKeyboardFocus (parent, None);

#ifdef DESTROY_WORKS
  XtDestroyWidget (passwd_dialog);
  passwd_dialog = 0;
#else
  XUnmapWindow (XtDisplay (passwd_dialog), XtWindow (passwd_dialog));
#endif
  {
    XErrorHandler old_handler = XSetErrorHandler (BadWindow_ehandler);
    /* I don't understand why this doesn't refocus on the old selected
       window when MWM is running in click-to-type mode.  The value of
       `focus' seems to be correct. */
    XSetInputFocus (dpy, focus, revert_to, CurrentTime);
    XSync (dpy, False);
    XSetErrorHandler (old_handler);
  }

  /* Since we installed our colormap to display the dialog properly, put
     the old one back, so that the screensaver_window is now displayed
     properly. */
  for (i = 0; i < si->nscreens; i++)
    {
      saver_screen_info *ssi = &si->screens[i];
      if (ssi->cmap)
	XInstallColormap (si->dpy, ssi->cmap);
    }

  return (passwd_state == pw_ok ? True : False);
}

Bool
#ifdef __STDC__
unlock_p (saver_info *si)
#else  /* !__STDC__ */
unlock_p (si)
	saver_info *si;
#endif /* !__STDC__ */
{
  static Bool initted = False;
  if (! initted)
    {
#ifndef VERIFY_CALLBACK_WORKS
      XtAppAddActions (si->app, actions, XtNumber (actions));
#endif
      passwd_dialog = 0;
      initted = True;
    }
  if (! passwd_dialog)
    make_passwd_dialog (si);
  return pop_passwd_dialog (si);
}

#endif /* !NO_LOCKING -- whole file */
