/* xscreensaver, Copyright (c) 1993-1996 Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include <X11/Intrinsic.h>

#ifndef __STDC__
# define _NO_PROTO
#endif

#ifndef NO_MOTIF
# include <Xm/Xm.h>
# include <Xm/Text.h>
# include <Xm/List.h>
# include <Xm/ToggleB.h>
# define TextGetString XmTextGetString
# define TextSetString XmTextSetString

#else /* !NO_MOTIF */
  /* Athena demo code contributed by Jon A. Christopher <jac8782@tamu.edu> */
  /* Copyright 1997, with the same permissions as above. */
# include <X11/StringDefs.h>
# include <X11/Shell.h>
# include <X11/Xaw/Form.h>
# include <X11/Xaw/Box.h>
# include <X11/Xaw/List.h>
# include <X11/Xaw/Command.h>
# include <X11/Xaw/Toggle.h>
# include <X11/Xaw/Viewport.h>
# include <X11/Xaw/Dialog.h>
#endif /* !NO_MOTIF */

#include "xscreensaver.h"
#include <stdio.h>

#ifdef HAVE_MIT_SAVER_EXTENSION
extern int mit_saver_ext_event_number;
extern Window server_mit_saver_window;
#endif /* HAVE_MIT_SAVER_EXTENSION */

#ifdef HAVE_SGI_SAVER_EXTENSION
/* extern int sgi_saver_ext_event_number; */
#endif /* HAVE_SGI_SAVER_EXTENSION */

extern Bool use_mit_saver_extension;
extern Bool use_sgi_saver_extension;

extern Time timeout, cycle, lock_timeout;
#ifndef NO_LOCKING
extern Time passwd_timeout;
#endif
extern int fade_seconds, fade_ticks;
extern Bool verbose_p, install_cmap_p, fade_p, unfade_p;
extern Bool lock_p, locking_disabled_p;

static void demo_mode_hack P((char *));
static void demo_mode_done P((void));

static void focus_fuckus P((Widget dialog));

#ifdef NO_MOTIF
static void text_cb P((char *, XtPointer, XtPointer));
#else /* MOTIF */
static void text_cb P((Widget button, XtPointer, XtPointer));
#endif /* MOTIF */

extern void demo_mode_restart_process P((void));

#ifdef NO_MOTIF
# define WHERE
#else
# define WHERE extern 
#endif

WHERE Widget demo_dialog;
WHERE Widget label1;
WHERE Widget text_line;
WHERE Widget demo_form;
WHERE Widget demo_list;
WHERE Widget next, prev, done, restart, edit;

WHERE Widget resources_dialog;
WHERE Widget resources_form;
WHERE Widget res_done, res_cancel;
WHERE Widget timeout_text, cycle_text, fade_text, ticks_text;
WHERE Widget lock_time_text, passwd_time_text;
WHERE Widget verbose_toggle, cmap_toggle, fade_toggle, unfade_toggle,
  lock_toggle;

#undef WHERE


extern create_demo_dialog P((Widget));
extern create_resources_dialog P((Widget));

#ifdef NO_MOTIF

Widget buttonbox, textbox, okbox;

# define ToggleButtonSetState(toggle, state, notify) \
   XtVaSetValues(toggle, XtNstate, state,  NULL)

static int
#ifdef __STDC__
ToggleButtonGetState(Widget toggle)
#else  /* !__STDC__ */
ToggleButtonGetState(toggle)
     Widget toggle;
#endif /* !__STDC__ */
{
  int state;
  XtVaGetValues(toggle,
		XtNstate, &state,
		NULL);
  return (state);
}

static void 
#ifdef __STDC__
TextSetString(Widget w, char *b)
#else  /* !__STDC__ */
TextSetString(w,b) 
     Widget w;
     char *b;
#endif /* !__STDC__ */
{
  XtVaSetValues(w,
		XtNvalue, b,
		NULL);
}


char *
#ifdef __STDC__
TextGetString(Widget w)
#else  /* !__STDC__ */
TextGetString(w)
     Widget w;
#endif /* !__STDC__ */
{
  char *value;
  XtVaGetValues(w,
		XtNvalue, &value,
		NULL);
  return (value);
}
#endif /* NO_MOTIF */


static void
#ifdef __STDC__
focus_fuckus (Widget dialog)
#else /* ! __STDC__ */
focus_fuckus (dialog) Widget dialog;
#endif /* ! __STDC__ */
{
  XSetInputFocus (XtDisplay (dialog), XtWindow (dialog),
		  RevertToParent, CurrentTime);
}

static void
raise_screenhack_dialog P((void))
{
  XMapRaised (XtDisplay (demo_dialog), XtWindow (demo_dialog));
  if (resources_dialog)
    XMapRaised (XtDisplay (resources_dialog), XtWindow (resources_dialog));
  focus_fuckus (resources_dialog ? resources_dialog : demo_dialog);
}

static void
destroy_screenhack_dialogs P((void))
{
  if (demo_dialog) XtDestroyWidget (demo_dialog);
  if (resources_dialog) XtDestroyWidget (resources_dialog);
  demo_dialog = resources_dialog = 0;
}

#ifndef NO_MOTIF

static void
#ifdef __STDC__
text_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
text_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  char *line = XmTextGetString (button);
  demo_mode_hack (line);
}

#else /* NO_MOTIF */

static void
#ifdef __STDC__
text_cb (char *line, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
text_cb (line, client_data, call_data)
     char *line;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  demo_mode_hack (line);
}

#endif /* NO_MOTIF */


static void
#ifdef __STDC__
select_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
select_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
#ifdef NO_MOTIF
  XawListReturnStruct *item = (XawListReturnStruct*)call_data;
  /* printf("selected item %d; \"%s\"\n", item->list_index, item->string); */
  text_cb(item->string, 0, 0);
#else  /* MOTIF */
  char **hacks = (char **) client_data;
  XmListCallbackStruct *lcb = (XmListCallbackStruct *) call_data;
  XmTextSetString (text_line, hacks [lcb->item_position - 1]);
  if (lcb->reason == XmCR_DEFAULT_ACTION)
    text_cb (text_line, 0, 0);
#endif /* MOTIF */
  focus_fuckus (demo_dialog);
}

#ifndef NO_MOTIF

static void
#ifdef __STDC__
ensure_selected_item_visible (Widget list)
#else /* ! __STDC__ */
ensure_selected_item_visible (list) Widget list;
#endif /* ! __STDC__ */
{
  int *pos_list = 0;
  int pos_count = 0;
  if (XmListGetSelectedPos (list, &pos_list, &pos_count) && pos_count > 0)
    {
      int top = -2;
      int visible = 0;
      XtVaGetValues (list,
		     XmNtopItemPosition, &top,
		     XmNvisibleItemCount, &visible,
		     0);
      if (pos_list[0] >= top + visible)
	{
	  int pos = pos_list[0] - visible + 1;
	  if (pos < 0) pos = 0;
	  XmListSetPos (list, pos);
	}
      else if (pos_list[0] < top)
	{
	  XmListSetPos (list, pos_list[0]);
	}
    }
  if (pos_list)
    XtFree ((char *) pos_list);
}
#endif /* MOTIF */


static void
#ifdef __STDC__
next_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
next_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{

#ifdef NO_MOTIF
  int cnt;
  XawListReturnStruct *current=XawListShowCurrent(demo_list);
  if (current->list_index!=XAW_LIST_NONE) {
    XtVaGetValues(demo_list,
		  XtNnumberStrings, &cnt,
		  NULL);
    /* printf("num strings %d current %d\n",cnt, current->list_index); */
    if (current->list_index+1 < cnt) {
      current->list_index++;
      XawListHighlight(demo_list,current->list_index);
    }
  } else
    XawListHighlight(demo_list,1);
  current=XawListShowCurrent(demo_list);
  text_cb(current->string,0,0);

#else /* MOTIF */

  int *pos_list;
  int pos_count;
  if (! XmListGetSelectedPos (demo_list, &pos_list, &pos_count))
    XmListSelectPos (demo_list, 1, True);
  else
    {
      int pos = pos_list [0];
      XmListSelectPos (demo_list, pos + 1, True);
      XtFree ((char *) pos_list);
      if (! XmListGetSelectedPos (demo_list, &pos_list, &pos_count))
	abort ();
      if (pos_list [0] == pos)
	XmListSelectPos (demo_list, 1, True);
      XtFree ((char *) pos_list);
    }
  ensure_selected_item_visible (demo_list);
  text_cb (text_line, 0, 0);

#endif /* MOTIF */
}

static void
#ifdef __STDC__
prev_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
prev_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
#ifdef NO_MOTIF
  XawListReturnStruct *current=XawListShowCurrent(demo_list);
  if (current->list_index!=XAW_LIST_NONE) {
    if (current->list_index>=1) {
      current->list_index--;
      XawListHighlight(demo_list,current->list_index);
    }
  } else
    XawListHighlight(demo_list,1);
  current=XawListShowCurrent(demo_list);
  text_cb(current->string,0,0);

#else /* MOTIF */

  int *pos_list;
  int pos_count;
  if (! XmListGetSelectedPos (demo_list, &pos_list, &pos_count))
    XmListSelectPos (demo_list, 0, True);
  else
    {
      XmListSelectPos (demo_list, pos_list [0] - 1, True);
      XtFree ((char *) pos_list);
    }
  ensure_selected_item_visible (demo_list);
  text_cb (text_line, 0, 0);

#endif /* MOTIF */
}


static void pop_resources_dialog P((void));
static void make_resources_dialog P((Widget parent));

static void
#ifdef __STDC__
edit_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
edit_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  Widget parent = (Widget) client_data;
  if (! resources_dialog)
    make_resources_dialog (parent);
  pop_resources_dialog ();
}

static void
#ifdef __STDC__
done_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
done_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  demo_mode_done ();
}


static void
#ifdef __STDC__
restart_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
restart_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  demo_mode_restart_process ();
}

void
#ifdef __STDC__
pop_up_dialog_box (Widget dialog, Widget form, int where)
#else /* ! __STDC__ */
pop_up_dialog_box (dialog, form, where)
     Widget dialog, form;
     int where;
#endif /* ! __STDC__ */
{
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
#ifdef NO_MOTIF
  XtRealizeWidget (dialog);
#else /* MOTIF */
  XtRealizeWidget (form);
#endif /* MOTIF */
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
  XtSetArg (av [ac], XtNx, x); ac++;
  XtSetArg (av [ac], XtNy, y); ac++;
  XtSetArg (av [ac], XtNoverrideRedirect, True); ac++;
#ifndef NO_MOTIF
  XtSetArg (av [ac], XmNdefaultPosition, False); ac++;
  /* I wonder whether this does anything useful? */
  /*  XtSetArg (av [ac], XmNdialogStyle, XmDIALOG_SYSTEM_MODAL); ac++; */
#endif /* MOTIF */
  XtSetValues (dialog, av, ac);
  XtSetValues (form, av, ac);
#ifdef NO_MOTIF
  XtPopup(dialog,XtGrabNone);
#else /* MOTIF */
  XtManageChild (form);
#endif /* MOTIF */

  focus_fuckus (dialog);
}


static void
#ifdef __STDC__
make_screenhack_dialog (Widget parent, char **hacks)
#else /* ! __STDC__ */
make_screenhack_dialog (parent, hacks)
     Widget parent;
     char **hacks;
#endif /* ! __STDC__ */
{
#ifndef NO_MOTIF
  char buf [255];
  Arg av[10];
  int ac;
  char *label;
  XmString xm_label = 0;
  XmString new_xm_label;

  create_demo_dialog (parent);
  ac = 0;
  XtSetArg (av [ac], XmNlabelString, &xm_label); ac++;
  XtGetValues (label1, av, ac);
  XmStringGetLtoR (xm_label, XmSTRING_DEFAULT_CHARSET, &label);
  if (!strcmp (label, XtName (label1)))
    strcpy (buf, "ERROR: RESOURCES ARE NOT INSTALLED CORRECTLY");
  else
    sprintf (buf, label, screensaver_version);
  new_xm_label = XmStringCreate (buf, XmSTRING_DEFAULT_CHARSET);
  ac = 0;
  XtSetArg (av [ac], XmNlabelString, new_xm_label); ac++;
  XtSetValues (label1, av, ac);
  XmStringFree (new_xm_label);
  XtFree (label);

  XtAddCallback (demo_list, XmNbrowseSelectionCallback, select_cb,
		 (XtPointer) hacks);
  XtAddCallback (demo_list, XmNdefaultActionCallback, select_cb,
		 (XtPointer) hacks);

  XtAddCallback (text_line, XmNactivateCallback, text_cb, 0);
  XtAddCallback (next, XmNactivateCallback, next_cb, 0);
  XtAddCallback (prev, XmNactivateCallback, prev_cb, 0);
  XtAddCallback (done, XmNactivateCallback, done_cb, 0);
  XtAddCallback (restart, XmNactivateCallback, restart_cb, 0);
  XtAddCallback (edit, XmNactivateCallback, edit_cb, (XtPointer) parent);

  for (; *hacks; hacks++)
    {
      XmString xmstr = XmStringCreate (*hacks, XmSTRING_DEFAULT_CHARSET);
      XmListAddItem (demo_list, xmstr, 0);
      /* XmListSelectPos (widget, i, False); */
      XmStringFree (xmstr);
    }

#if 0
  /* Dialogs that have scroll-lists don't obey maxWidth!  Fuck!!  Hack it. */
  ac = 0;
  XtSetArg (av [ac], XmNmaxWidth, &max_w); ac++;
  XtGetValues (demo_dialog, av, ac); /* great, this SEGVs */
#endif



#else  /* NO_MOTIF */



  Widget subform, box, viewport, label2;

  demo_dialog = 
    XtVaCreatePopupShell("demo_dialog", transientShellWidgetClass, parent,
			 XtNtitle, NULL,
			 XtNoverrideRedirect, TRUE,
			 NULL);

  demo_form =
    XtVaCreateManagedWidget("demo_form", formWidgetClass, demo_dialog,
			    NULL);

  label1 = XtVaCreateManagedWidget("label1", labelWidgetClass, demo_form,
			    XtNleft, XtChainLeft,
			    XtNright, XtChainRight,
			    XtNtop, XtChainTop,
			    NULL);

  {
    char buf [255];
    char *text = 0;
    XtVaGetValues (label1, XtNlabel, &text, 0);
    if (!text || !strcmp (text, XtName (label1)))
      strcpy (buf, "ERROR: RESOURCES ARE NOT INSTALLED CORRECTLY");
    else
      sprintf (buf, text, screensaver_version);
    XtVaSetValues (label1, XtNlabel, buf, 0);
    XtFree (text);
  }

  label2 = XtVaCreateManagedWidget("label2", labelWidgetClass, demo_form,
			    XtNleft, XtChainLeft,
			    XtNright, XtChainRight,
			    XtNfromVert, label1,
			    NULL);

  subform=
    XtVaCreateManagedWidget("subform", formWidgetClass, demo_form,
			    XtNleft, XtChainLeft,
			    XtNright, XtChainRight,
			    XtNfromVert, label2,
			    XtNbottom, XtChainBottom,
			    NULL);
  viewport=
    XtVaCreateManagedWidget("viewport", viewportWidgetClass, subform,
			    XtNallowVert, TRUE,
			    XtNallowHoriz, TRUE,
			    XtNforceBars, TRUE,
			    NULL);

  demo_list=XtVaCreateManagedWidget("demo_list", listWidgetClass, viewport,
				    XtNverticalList, TRUE,
				    XtNdefaultColumns, 1,
				    XtNlist, hacks,
				    NULL);
  XtAddCallback(demo_list, XtNcallback, select_cb, NULL);
  box=
    XtVaCreateManagedWidget("box", boxWidgetClass, demo_form,
			    XtNfromHoriz, subform,
			    XtNfromVert, label2,
			    XtNbottom, XtChainBottom,
			    XtNright, XtChainRight,
			    NULL);
  prev=XtVaCreateManagedWidget("prev", commandWidgetClass, box, NULL);
  XtAddCallback(prev, XtNcallback, prev_cb, NULL);

  next=XtVaCreateManagedWidget("next", commandWidgetClass, box, NULL);
  XtAddCallback(next, XtNcallback, next_cb, NULL);

  edit=XtVaCreateManagedWidget("edit", commandWidgetClass, box, NULL);
  XtAddCallback(edit, XtNcallback, edit_cb, parent);

  restart=XtVaCreateManagedWidget("restart", commandWidgetClass, box, NULL);
  XtAddCallback(restart, XtNcallback, restart_cb, NULL);

  done=XtVaCreateManagedWidget("done", commandWidgetClass, box, NULL);
  XtAddCallback(done, XtNcallback, done_cb, NULL);
#endif /* NO_MOTIF */

  pop_up_dialog_box(demo_dialog, demo_form, 0);
}


/* the Screensaver Parameters dialog */

static struct resources {
  int timeout, cycle, secs, ticks, lock_time, passwd_time;
  int verb, cmap, fade, unfade, lock_p;
} res;


extern int parse_time P((char *line, Bool sec_p, Bool));

static void 
#ifdef __STDC__
hack_time_cb (Display *dpy, char *line, int *store, Bool sec_p)
#else /* ! __STDC__ */
hack_time_cb (dpy, line, store, sec_p)
     Display *dpy;
     char *line;
     int *store;
     Bool sec_p;
#endif /* ! __STDC__ */
{
  if (*line)
    {
      int value;
      value = parse_time (line, sec_p, True);
      if (value < 0)
	/*XBell (dpy, 0)*/;
      else
	*store = value;
    }
}

static void
#ifdef __STDC__
res_sec_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
res_sec_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  hack_time_cb (XtDisplay (button), TextGetString (button),
		(int *) client_data, True);
}

static void
#ifdef __STDC__
res_min_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
res_min_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  hack_time_cb (XtDisplay (button), TextGetString (button),
		(int *) client_data, False);
}

static void
#ifdef __STDC__
res_int_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
res_int_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  char *line = TextGetString (button);
  int *store = (int *) client_data;
  unsigned int value;
  char c;
  if (! *line)
    ;
  else if (sscanf (line, "%u%c", &value, &c) != 1)
    XBell (XtDisplay (button), 0);
  else
    *store = value;
}

static void
#ifdef __STDC__
res_bool_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
res_bool_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  int *store = (int *) client_data;
#ifdef NO_MOTIF
  *store = ToggleButtonGetState(button);
#else  /* MOTIF */
  *store = ((XmToggleButtonCallbackStruct *) call_data)->set;
#endif /* MOTIF */
}

static void
#ifdef __STDC__
res_cancel_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
res_cancel_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  XtDestroyWidget (resources_dialog);
  resources_dialog = 0;
  raise_screenhack_dialog ();
}


static void
#ifdef __STDC__
res_done_cb (Widget button, XtPointer client_data, XtPointer call_data)
#else /* ! __STDC__ */
res_done_cb (button, client_data, call_data)
     Widget button;
     XtPointer client_data, call_data;
#endif /* ! __STDC__ */
{
  res_cancel_cb (button, client_data, call_data);

#ifdef NO_MOTIF
  /* check all text widgets */
  res_min_cb(timeout_text, &res.timeout, NULL);
  res_min_cb(cycle_text, &res.cycle, NULL);
  res_sec_cb(fade_text, &res.secs, NULL);
  res_int_cb(ticks_text, &res.ticks, NULL);
  res_min_cb(lock_time_text, &res.lock_time, NULL);
  res_sec_cb(passwd_time_text, &res.passwd_time, NULL);
#endif

  /* Throttle the timeouts to minimum sane values. */
  if (res.timeout < 5) res.timeout = 5;
  if (res.cycle < 2) res.cycle = 2;
  if (res.passwd_time < 10) res.passwd_time = 10;

  timeout = res.timeout * 1000;
  cycle = res.cycle * 1000;
  lock_timeout = res.lock_time * 1000;
#ifndef NO_LOCKING
  passwd_timeout = res.passwd_time * 1000;
#endif
  fade_seconds = res.secs;
  fade_ticks = res.ticks;
  verbose_p = res.verb;
  install_cmap_p = res.cmap;
  fade_p = res.fade;
  unfade_p = res.unfade;
  lock_p = res.lock_p;

#if defined(HAVE_MIT_SAVER_EXTENSION) || defined(HAVE_SGI_SAVER_EXTENSION)
  if (use_mit_saver_extension || use_sgi_saver_extension)
    {
      /* Need to set the server timeout to the new one the user has picked.
       */
      int server_timeout, server_interval, prefer_blank, allow_exp;
      XGetScreenSaver (dpy, &server_timeout, &server_interval,
		       &prefer_blank, &allow_exp);
      if (server_timeout != (timeout / 1000))
	{
	  server_timeout = (timeout / 1000);
	  if (verbose_p)
	    fprintf (stderr,
		   "%s: configuring server for saver timeout of %d seconds.\n",
		     progname, server_timeout);
	  /* Leave all other parameters the same. */
	  XSetScreenSaver (dpy, server_timeout, server_interval,
			   prefer_blank, allow_exp);
	}
    }
#endif /* HAVE_MIT_SAVER_EXTENSION || HAVE_SGI_SAVER_EXTENSION */
}


static void
#ifdef __STDC__
make_resources_dialog (Widget parent)
#else /* ! __STDC__ */
make_resources_dialog (parent) Widget parent;
#endif /* ! __STDC__ */
{
#ifndef NO_MOTIF
  Arg av[10];
  int ac;

  create_resources_dialog (parent);

  XtAddCallback (res_done, XmNactivateCallback, res_done_cb, 0);
  XtAddCallback (res_cancel, XmNactivateCallback, res_cancel_cb, 0);

#define CB(widget,type,slot) \
	XtAddCallback ((widget), XmNvalueChangedCallback, (type), \
		       (XtPointer) (slot))
  CB (timeout_text,	res_min_cb,  &res.timeout);
  CB (cycle_text,	res_min_cb,  &res.cycle);
  CB (fade_text,	res_sec_cb,  &res.secs);
  CB (ticks_text,	res_int_cb,  &res.ticks);
  CB (lock_time_text,	res_min_cb,  &res.lock_time);
  CB (passwd_time_text,	res_sec_cb,  &res.passwd_time);
  CB (verbose_toggle,	res_bool_cb, &res.verb);
  CB (cmap_toggle,	res_bool_cb, &res.cmap);
  CB (fade_toggle,	res_bool_cb, &res.fade);
  CB (unfade_toggle,	res_bool_cb, &res.unfade);
  CB (lock_toggle,	res_bool_cb, &res.lock_p);
#undef CB
  ac = 0;
  XtSetArg (av[ac], XmNsensitive, False); ac++;

  if (locking_disabled_p)
    {
      XtSetValues (passwd_time_text, av, ac);
      XtSetValues (lock_time_text, av, ac);
      XtSetValues (lock_toggle, av, ac);
    }
  if (CellsOfScreen (XtScreen (parent)) <= 2)
    {
      XtSetValues (fade_text, av, ac);
      XtSetValues (ticks_text, av, ac);
      XtSetValues (cmap_toggle, av, ac);
      XtSetValues (fade_toggle, av, ac);
      XtSetValues (unfade_toggle, av, ac);
    }


#else  /* NO_MOTIF */

  Widget rlabel;

  resources_dialog = 
    XtVaCreatePopupShell("resources_dialog", transientShellWidgetClass, parent,
			 XtNtitle, NULL,
			 XtNoverrideRedirect, TRUE,
			 NULL);

  resources_form =
    XtVaCreateManagedWidget("resources_form", formWidgetClass,
			    resources_dialog,
			    NULL);

  rlabel = XtVaCreateManagedWidget("label1", labelWidgetClass, resources_form,
				   XtNleft, XtChainLeft,
				   XtNright, XtChainRight,
				   XtNtop, XtChainTop,
				   NULL);

  textbox=
    XtVaCreateManagedWidget("textbox", formWidgetClass, resources_form,
			    XtNleft, XtChainLeft,
			    XtNfromVert, rlabel,
			    NULL);
  okbox=
    XtVaCreateManagedWidget("textbox", boxWidgetClass, resources_form,
			    XtNleft, XtChainLeft,
			    XtNright, XtChainRight,
			    XtNfromVert, textbox,
			    XtNorientation, XtorientHorizontal,
			    NULL);
  timeout_text=
    XtVaCreateManagedWidget("timeout", dialogWidgetClass, textbox,
			    XtNleft, XtChainLeft,
			    XtNright, XtChainRight,
			    XtNtop, XtChainTop,
			    NULL);
  cycle_text=
    XtVaCreateManagedWidget("cycle", dialogWidgetClass, textbox,
			    XtNleft, XtChainLeft,
			    XtNright, XtChainRight,
			    XtNfromVert, timeout_text,
			    NULL);
  fade_text=
    XtVaCreateManagedWidget("fade", dialogWidgetClass, textbox,
			    XtNleft, XtChainLeft,
			    XtNright, XtChainRight,
			    XtNfromVert, cycle_text,
			    NULL);

  ticks_text =
    XtVaCreateManagedWidget("ticks", dialogWidgetClass, textbox,
			    XtNtop, XtChainTop,
			    XtNright, XtChainRight,
			    XtNfromHoriz, timeout_text,
			    NULL);

  lock_time_text =
    XtVaCreateManagedWidget("lockTime", dialogWidgetClass, textbox,
			    XtNfromVert, ticks_text,
			    XtNright, XtChainRight,
			    XtNfromHoriz, cycle_text,
			    NULL);

  passwd_time_text =
    XtVaCreateManagedWidget("passwdTime", dialogWidgetClass, textbox,
			    XtNfromVert, lock_time_text,
			    XtNright, XtChainRight,
			    XtNfromHoriz, fade_text,
			    NULL);

  buttonbox=
    XtVaCreateManagedWidget("buttonbox", boxWidgetClass, resources_form,
			    XtNfromVert, rlabel,
			    XtNfromHoriz, textbox,
			    XtNright, XtChainRight,
			    XtNorientation, XtorientVertical,
			    NULL);
  verbose_toggle =
    XtVaCreateManagedWidget("verbose", toggleWidgetClass, buttonbox,
			    NULL);
  cmap_toggle =
    XtVaCreateManagedWidget("cmap", toggleWidgetClass, buttonbox,
			    NULL);
  fade_toggle =
    XtVaCreateManagedWidget("fade", toggleWidgetClass, buttonbox,
			    NULL);
  unfade_toggle =
    XtVaCreateManagedWidget("unfade", toggleWidgetClass, buttonbox,
			    NULL);
  lock_toggle = 
    XtVaCreateManagedWidget("lock", toggleWidgetClass, buttonbox,
			    NULL);


  res_done=XtVaCreateManagedWidget("done", commandWidgetClass, okbox,
			  NULL);
  XtAddCallback(res_done, XtNcallback, res_done_cb, NULL);
  res_cancel=XtVaCreateManagedWidget("cancel", commandWidgetClass, okbox,
			  NULL);
  XtAddCallback(res_cancel, XtNcallback, res_cancel_cb, NULL);

#if 0
  /* we can't set callbacks for these, so we'll check them all if "done"
     gets pressed */
  CB (timeout_text,	res_min_cb,  &res.timeout);
  CB (cycle_text,	res_min_cb,  &res.cycle);
  CB (fade_text,	res_sec_cb,  &res.secs);
  CB (ticks_text,	res_int_cb,  &res.ticks);
  CB (lock_time_text,	res_min_cb,  &res.lock_time);
  CB (passwd_time_text,	res_sec_cb,  &res.passwd_time);
#endif
#define CB(widget,type,slot) \
	XtAddCallback ((widget), XtNcallback, (type), \
		       (XtPointer) (slot))
  CB (verbose_toggle,	res_bool_cb, &res.verb);
  CB (cmap_toggle,	res_bool_cb, &res.cmap);
  CB (fade_toggle,	res_bool_cb, &res.fade);
  CB (unfade_toggle,	res_bool_cb, &res.unfade);
  CB (lock_toggle,	res_bool_cb, &res.lock_p);
#undef CB

#endif /* NO_MOTIF */
}


static void
#ifdef __STDC__
fmt_time (char *buf, unsigned int s, int min_p)
#else /* ! __STDC__ */
fmt_time (buf, s, min_p)
     char *buf;
     unsigned int s;
     int min_p;
#endif /* ! __STDC__ */
{
  unsigned int h = 0, m = 0;
  if (s >= 60)
    {
      m += (s / 60);
      s %= 60;
    }
  if (m >= 60)
    {
      h += (m / 60);
      m %= 60;
    }
/*
  if (min_p && h == 0 && s == 0)
    sprintf (buf, "%u", m);
  else if (!min_p && h == 0 && m == 0)
    sprintf (buf, "%u", s);
  else
  if (h == 0)
    sprintf (buf, "%u:%02u", m, s);
  else
*/
    sprintf (buf, "%u:%02u:%02u", h, m, s);
}

static void
pop_resources_dialog P((void))
{
  char buf [100];

  res.timeout = timeout / 1000;
  res.cycle = cycle / 1000;
  res.lock_time = lock_timeout / 1000;
#ifndef NO_LOCKING
  res.passwd_time = passwd_timeout / 1000;
#endif
  res.secs = fade_seconds;
  res.ticks = fade_ticks;
  res.verb = verbose_p;
  res.cmap = install_cmap_p;
  res.fade = fade_p;
  res.unfade = unfade_p;
  res.lock_p = (lock_p && !locking_disabled_p);

  fmt_time (buf, res.timeout, 1);     TextSetString (timeout_text, buf);
  fmt_time (buf, res.cycle, 1);       TextSetString (cycle_text, buf);
  fmt_time (buf, res.lock_time, 1);   TextSetString (lock_time_text, buf);
  fmt_time (buf, res.passwd_time, 0); TextSetString (passwd_time_text, buf);
  fmt_time (buf, res.secs, 0);        TextSetString (fade_text, buf);
  sprintf (buf, "%u", res.ticks);     TextSetString (ticks_text, buf);

#ifdef NO_MOTIF
  ToggleButtonSetState (verbose_toggle, res.verb, True);
  ToggleButtonSetState (cmap_toggle, res.cmap, True);
  ToggleButtonSetState (fade_toggle, res.fade, True);
  ToggleButtonSetState (unfade_toggle, res.unfade, True);
  ToggleButtonSetState (lock_toggle, res.lock_p, True);
#else  /* MOTIF */
  XmToggleButtonSetState (verbose_toggle, res.verb, True);
  XmToggleButtonSetState (cmap_toggle, res.cmap, True);
  XmToggleButtonSetState (fade_toggle, res.fade, True);
  XmToggleButtonSetState (unfade_toggle, res.unfade, True);
  XmToggleButtonSetState (lock_toggle, res.lock_p, True);
#endif /* MOTIF */

  pop_up_dialog_box (resources_dialog, resources_form, 1);
}


/* The code on this page isn't actually Motif-specific */

Bool dbox_up_p = False;
Bool demo_mode_p = False;

extern XtAppContext app;
extern Widget toplevel_shell;
extern Bool use_xidle_extension;
extern Bool use_mit_saver_extension;
extern Bool use_sgi_saver_extension;
extern Time notice_events_timeout;

extern char **screenhacks;
extern char *demo_hack;

extern void notice_events_timer P((XtPointer closure, XtIntervalId *timer));
extern Bool handle_clientmessage P((XEvent *, Bool));

void
demo_mode P((void))
{
  dbox_up_p = True;
  initialize_screensaver_window ();
  raise_window (True, False);
  make_screenhack_dialog (toplevel_shell, screenhacks);
  while (demo_mode_p)
    {
      XEvent event;
      XtAppNextEvent (app, &event);
      switch (event.xany.type)
	{
	case 0:		/* synthetic "timeout" event */
	  break;

	case ClientMessage:
	  handle_clientmessage (&event, False);
	  break;

	case CreateNotify:
	  if (!use_xidle_extension &&
	      !use_mit_saver_extension &&
	      !use_sgi_saver_extension)
	    {
	      XtAppAddTimeOut (app, notice_events_timeout, notice_events_timer,
			       (XtPointer) event.xcreatewindow.window);
#ifdef DEBUG_TIMERS
	      if (verbose_p)
		printf ("%s: starting notice_events_timer for 0x%X (%lu)\n",
			progname,
			(unsigned int) event.xcreatewindow.window,
			notice_events_timeout);
#endif /* DEBUG_TIMERS */
	    }
	  break;

	case ButtonPress:
	case ButtonRelease:
	  if (!XtWindowToWidget (dpy, event.xbutton.window))
	    raise_screenhack_dialog ();
	  /* fall through */

	default:
#ifdef HAVE_MIT_SAVER_EXTENSION
	  if (event.type == mit_saver_ext_event_number)
	    {
	      /* Get the "real" server window out of the way as soon
		 as possible. */
	      if (server_mit_saver_window &&
		  window_exists_p (dpy, server_mit_saver_window))
		XUnmapWindow (dpy, server_mit_saver_window);
	    }
	  else
#endif /* HAVE_MIT_SAVER_EXTENSION */

	  XtDispatchEvent (&event);
	  break;
	}
    }
  destroy_screenhack_dialogs ();
  initialize_screensaver_window ();
  unblank_screen ();
}

static void
#ifdef __STDC__
demo_mode_hack (char *hack)
#else /* ! __STDC__ */
demo_mode_hack (hack) char *hack;
#endif /* ! __STDC__ */
{
  if (! demo_mode_p) abort ();
  kill_screenhack ();
  if (! demo_hack)
    blank_screen ();
  demo_hack = hack;
  spawn_screenhack (False);
  /* raise_screenhack_dialog(); */
}

static void
demo_mode_done P((void))
{
  kill_screenhack ();
  if (demo_hack)
    unblank_screen ();
  demo_mode_p = False;
  dbox_up_p = False;
  demo_hack = 0;
}
