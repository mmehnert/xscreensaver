/* demo.c --- implements the interactive demo-mode and options dialogs.
 * xscreensaver, Copyright (c) 1993-1998 Jamie Zawinski <jwz@jwz.org>
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

#ifdef HAVE_ATHENA_KLUDGE	/* don't ask */
# undef HAVE_MOTIF
# define HAVE_ATHENA 1
#endif

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifndef VMS
# include <pwd.h>		/* for getpwuid() */
#else /* VMS */
# include "vms-pwd.h"
#endif /* VMS */

#ifdef HAVE_UNAME
# include <sys/utsname.h>	/* for uname() */
#endif /* HAVE_UNAME */

#include <stdio.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/* We don't actually use any widget internals, but these are included
   so that gdb will have debug info for the widgets... */
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>

#ifdef HAVE_MOTIF
# include <Xm/Xm.h>
# include <Xm/Text.h>
# include <Xm/List.h>
# include <Xm/ToggleB.h>
# include <Xm/MessageB.h>
# include <Xm/LabelG.h>
# include <Xm/RowColumn.h>

#else  /* HAVE_ATHENA */
  /* Athena demo code contributed by Jon A. Christopher <jac8782@tamu.edu> */
  /* Copyright 1997, with the same permissions as above. */
# include <X11/Shell.h>
# include <X11/Xaw/Form.h>
# include <X11/Xaw/Box.h>
# include <X11/Xaw/List.h>
# include <X11/Xaw/Command.h>
# include <X11/Xaw/Toggle.h>
# include <X11/Xaw/Viewport.h>
# include <X11/Xaw/Dialog.h>
# include <X11/Xaw/Scrollbar.h>
# include <X11/Xaw/Text.h>
#endif /* HAVE_ATHENA */

#include "version.h"
#include "prefs.h"
#include "resources.h"		/* for parse_time() */
#include "visual.h"		/* for has_writable_cells() */
#include "remote.h"		/* for xscreensaver_command() */

#include <stdio.h>
#include <string.h>
#include <ctype.h>


char *progname = 0;
char *progclass = "XScreenSaver";
XrmDatabase db;

char *blurb (void) { return progname; }

static void run_hack (Display *dpy, int n);

static saver_preferences *global_prefs_kludge = 0;    /* I hate C so much... */


static char *short_version = 0;

Atom XA_VROOT;
Atom XA_SCREENSAVER, XA_SCREENSAVER_RESPONSE, XA_SCREENSAVER_VERSION;
Atom XA_SCREENSAVER_TIME, XA_SCREENSAVER_ID, XA_SELECT, XA_DEMO, XA_RESTART;

extern void create_demo_dialog (Widget, Visual *, Colormap);
extern void create_resources_dialog (Widget, Visual *, Colormap);

extern Widget demo_dialog;
extern Widget label1;
extern Widget text_line;
extern Widget demo_form;
extern Widget demo_list;
extern Widget next, prev, done, restart, edit;

extern Widget resources_dialog;
extern Widget resources_form;
extern Widget res_done, res_cancel;
extern Widget timeout_text, cycle_text, fade_text, ticks_text;
extern Widget lock_time_text, passwd_time_text;
extern Widget verbose_toggle, cmap_toggle, fade_toggle, unfade_toggle,
  lock_toggle;


#ifdef HAVE_MOTIF

# define set_toggle_button_state(toggle,state) \
  XmToggleButtonSetState ((toggle), (state), True)
# define set_text_string(text_widget,string) \
  XmTextSetString ((text_widget), (string))
# define add_button_callback(button,cb,arg) \
  XtAddCallback ((button), XmNactivateCallback, (cb), (arg))
# define add_toggle_callback(button,cb,arg) \
  XtAddCallback ((button), XmNvalueChangedCallback, (cb), (arg))
# define add_text_callback add_toggle_callback

#else  /* HAVE_ATHENA */

# define set_toggle_button_state(toggle,state) \
  XtVaSetValues((toggle), XtNstate, (state),  0)
# define set_text_string(text_widget,string) \
  XtVaSetValues ((text_widget), XtNvalue, (string), 0)
# define add_button_callback(button,cb,arg) \
  XtAddCallback ((button), XtNcallback, (cb), (arg))
# define add_toggle_callback add_button_callback
# define add_text_callback(b,c,a) ERROR!

#endif /* HAVE_ATHENA */


#define disable_widget(widget) \
  XtVaSetValues((widget), XtNsensitive, False, 0)


static char *
get_text_string (Widget text_widget)
{
#ifdef HAVE_MOTIF
  return XmTextGetString (text_widget);
#else  /* HAVE_ATHENA */
  char *string = 0;
  if (XtIsSubclass(text_widget, textWidgetClass))
    XtVaGetValues (text_widget, XtNstring, &string, 0);
  else if (XtIsSubclass(text_widget, dialogWidgetClass))
    XtVaGetValues (text_widget, XtNvalue, &string, 0);
  else
    string = 0;

  return string;
#endif /* HAVE_ATHENA */
}

static char *
get_label_string (Widget label_widget)
{
#ifdef HAVE_MOTIF
  char *label = 0;
  XmString xm_label = 0;
  XtVaGetValues (label_widget, XmNlabelString, &xm_label, 0);
  if (!xm_label)
    return 0;
  XmStringGetLtoR (xm_label, XmSTRING_DEFAULT_CHARSET, &label);
  return label;
#else  /* HAVE_ATHENA */
  char *label = 0;
  XtVaGetValues (label_widget, XtNlabel, &label, 0);
  return (label ? strdup(label) : 0);
#endif /* HAVE_ATHENA */
}


static void
set_label_string (Widget label_widget, char *string)
{
#ifdef HAVE_MOTIF
  XmString xm_string = XmStringCreate (string, XmSTRING_DEFAULT_CHARSET);
  XtVaSetValues (label_widget, XmNlabelString, xm_string, 0);
  XmStringFree (xm_string);
#else  /* HAVE_ATHENA */
  XtVaSetValues (label_widget, XtNlabel, string, 0);
#endif /* HAVE_ATHENA */
}


static void
format_into_label (Widget label, const char *arg)
{
  char *text = get_label_string (label);
  char *buf = (char *) malloc ((text ? strlen(text) : 0) + strlen(arg) + 100);

  if (!text || !strcmp (text, XtName (label)))
      strcpy (buf, "ERROR: RESOURCES ARE NOT INSTALLED CORRECTLY");
    else
      sprintf (buf, text, arg);

    set_label_string (label, buf);
    free (buf);
    XtFree (text);
}


/* Why this behavior isn't automatic in *either* toolkit, I'll never know.
 */
static void
ensure_selected_item_visible (Widget list)
{
#ifdef HAVE_MOTIF
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

#else  /* HAVE_ATHENA */
# ifdef HAVE_XawViewportSetCoordinates

  int margin = 16;	/* should be line height or something. */
  int count = 0;
  int pos;
  Dimension list_h = 0, vp_h = 0;
  Dimension top_margin = 4;  /* I don't know where this value comes from */
  Position vp_x = 0, vp_y = 0, current_y;
  double cratio;
  Widget viewport = XtParent(demo_list);
  Widget sb = (viewport ? XtNameToWidget(viewport, "*vertical") : 0);
  float sb_top = 0, sb_size = 0;
  XawListReturnStruct *current = XawListShowCurrent(demo_list);
  if (!current || !sb) return;

  XtVaGetValues(demo_list,
		XtNnumberStrings, &count,
		XtNheight, &list_h,
		0);
  if (count < 2 || list_h < 10) return;

  XtVaGetValues(viewport, XtNheight, &vp_h, XtNx, &vp_x, XtNy, &vp_y, 0);
  if (vp_h < 10) return;

  XtVaGetValues(sb, XtNtopOfThumb, &sb_top, XtNshown, &sb_size, 0);
  if (sb_size <= 0) return;

  pos = current->list_index;
  cratio = ((double) pos)  / ((double) count);
  current_y = (cratio * list_h);

  if (cratio < sb_top ||
      cratio > sb_top + sb_size)
    {
      if (cratio < sb_top)
	current_y -= (vp_h - margin - margin);
      else
	current_y -= margin;

      if ((long)current_y >= (long) list_h)
	current_y = (Position) ((long)list_h - (long)vp_h);

      if ((long)current_y < (long)top_margin)
	current_y = (Position)top_margin;

      XawViewportSetCoordinates (viewport, vp_x, current_y);
    }
# endif /* HAVE_XawViewportSetCoordinates */
#endif /* HAVE_ATHENA */
}


static void
text_cb (Widget text_widget, XtPointer client_data, XtPointer call_data)
{
  Display *dpy = XtDisplay (text_widget);
  saver_preferences *p = (saver_preferences *) client_data;
  char *new_text = get_text_string (text_widget);

  int hack_number = -1;		/* 0-based */

#ifdef HAVE_ATHENA
  XawListReturnStruct *current = XawListShowCurrent(demo_list);
  hack_number = current->list_index;
#else  /* HAVE_MOTIF */
  int *pos_list = 0;
  int pos_count = 0;
  if (XmListGetSelectedPos (demo_list, &pos_list, &pos_count))
    hack_number = pos_list[0] - 1;
  if (pos_list)
    XtFree ((char *) pos_list);
#endif /* HAVE_MOTIF */

  ensure_selected_item_visible (demo_list);

  if (hack_number < 0 || hack_number >= p->screenhacks_count)
    {
      set_text_string (text_widget, "");
      XBell (dpy, 0);
    }
  else
    {
fprintf(stderr, "%d:\nold: %s\nnew: %s\n",
	hack_number, p->screenhacks [hack_number], new_text);

      if (p->screenhacks [hack_number])
	free (p->screenhacks [hack_number]);
      p->screenhacks [hack_number] = strdup (new_text);

#ifdef HAVE_MOTIF

      XmListDeselectAllItems (demo_list);
      {
	XmString xmstr = XmStringCreate (new_text, XmSTRING_DEFAULT_CHARSET);
	XmListReplaceItemsPos (demo_list, &xmstr, 1, hack_number+1);
	XmStringFree (xmstr);
      }
      XmListSelectPos (demo_list, hack_number+1, True);

#else  /* HAVE_ATHENA */

      {
	Widget vp = XtParent(demo_list);
	Widget sb = (vp ? XtNameToWidget(vp, "*vertical") : 0);
	Dimension list_h = 0;
	Position vp_x = 0, vp_y = 0;
	float sb_top = 0;

	XawListUnhighlight (demo_list);

	XtVaGetValues (vp, XtNx, &vp_x, 0);
	XtVaGetValues (sb, XtNtopOfThumb, &sb_top, 0);
	XtVaGetValues (demo_list, XtNheight, &list_h, 0);
	vp_y = (sb_top * list_h);
	XtVaSetValues (demo_list,
		       XtNlist, p->screenhacks,
		       XtNnumberStrings, p->screenhacks_count,
		       0);
	XawViewportSetCoordinates (vp, vp_x, vp_y);
	XawListHighlight (demo_list, hack_number);
      }

#endif /* HAVE_ATHENA */

      write_init_file (p, short_version);
      XSync (dpy, False);
      usleep (500000);		/* give the disk time to settle down */

      run_hack (dpy, hack_number+1);
    }
}


#ifdef HAVE_ATHENA
/* Bend over backwards to make hitting Return in the text field do the
   right thing. 
   */
static void text_enter (Widget w, XEvent *event, String *av, Cardinal *ac)
{
  text_cb (w, global_prefs_kludge, 0);	  /* I hate C so much... */
}

static XtActionsRec actions[] = {{"done",      text_enter}
			        };
static char translations[] = ("<Key>Return:	done()\n"
			      "<Key>Linefeed:	done()\n"
			      "Ctrl<Key>M:	done()\n"
			      "Ctrl<Key>J:	done()\n");
#endif /* HAVE_ATHENA */


static void
next_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
#ifdef HAVE_ATHENA
  XawListReturnStruct *current = XawListShowCurrent(demo_list);
  int cnt;
  XtVaGetValues (demo_list, XtNnumberStrings, &cnt, 0);
  if (current->list_index == XAW_LIST_NONE ||
      current->list_index + 1 >= cnt)
    current->list_index = 0;
  else
    current->list_index++;
  XawListHighlight(demo_list, current->list_index);

  ensure_selected_item_visible (demo_list);
  current = XawListShowCurrent(demo_list);
  XtVaSetValues(text_line, XtNstring, current->string, 0);

  run_hack (XtDisplay (button), current->list_index + 1);

#else  /* HAVE_MOTIF */

  saver_preferences *p = (saver_preferences *) client_data;
  int *pos_list = 0;
  int pos_count = 0;
  int pos;
  if (! XmListGetSelectedPos (demo_list, &pos_list, &pos_count))
    {
      pos = 1;
      XmListDeselectAllItems (demo_list);	/* LessTif lossage */
      XmListSelectPos (demo_list, pos, True);
    }
  else
    {
      pos = pos_list[0] + 1;
      if (pos > p->screenhacks_count)
	pos = 1;
      XmListDeselectAllItems (demo_list);	/* LessTif lossage */
      XmListSelectPos (demo_list, pos, True);
    }
     
  ensure_selected_item_visible (demo_list);
  run_hack (XtDisplay (button), pos);
  if (pos_list)
    XtFree ((char *) pos_list);

#endif /* HAVE_MOTIF */
}


static void
prev_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
#ifdef HAVE_ATHENA
  XawListReturnStruct *current = XawListShowCurrent(demo_list);
  int cnt;
  XtVaGetValues (demo_list, XtNnumberStrings, &cnt, 0);
  if (current->list_index == XAW_LIST_NONE ||
      current->list_index <= 0)
    current->list_index = cnt-1;
  else
    current->list_index--;
  XawListHighlight(demo_list, current->list_index);

  ensure_selected_item_visible (demo_list);
  current = XawListShowCurrent(demo_list);
  XtVaSetValues(text_line, XtNstring, current->string, 0);

  run_hack (XtDisplay (button), current->list_index + 1);

#else  /* HAVE_MOTIF */

  saver_preferences *p = (saver_preferences *) client_data;
  int *pos_list = 0;
  int pos_count = 0;
  int pos;
  if (! XmListGetSelectedPos (demo_list, &pos_list, &pos_count))
    {
      pos = p->screenhacks_count;
      XmListDeselectAllItems (demo_list);	/* LessTif lossage */
      XmListSelectPos (demo_list, pos, True);
    }
  else
    {
      pos = pos_list[0] - 1;
      if (pos == 0)
	pos = p->screenhacks_count;
      XmListDeselectAllItems (demo_list);	/* LessTif lossage */
      XmListSelectPos (demo_list, pos, True);
    }
     
  ensure_selected_item_visible (demo_list);
  run_hack (XtDisplay (button), pos);
  if (pos_list)
    XtFree ((char *) pos_list);

#endif /* HAVE_MOTIF */
}


static void
select_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
/*  saver_preferences *p = (saver_preferences *) client_data; */

#ifdef HAVE_ATHENA
  XawListReturnStruct *item = (XawListReturnStruct*)call_data;
  XtVaSetValues(text_line, XtNstring, item->string, 0);
  run_hack (XtDisplay (button), item->list_index + 1);

#else  /* HAVE_MOTIF */
  XmListCallbackStruct *lcb = (XmListCallbackStruct *) call_data;
  char *string = 0;
  if (lcb->item)
    XmStringGetLtoR (lcb->item, XmSTRING_DEFAULT_CHARSET, &string);
  set_text_string (text_line, (string ? string : ""));

  if (lcb->reason == XmCR_DEFAULT_ACTION && string)
    run_hack (XtDisplay (button), lcb->item_position);

  if (string)
    XtFree (string);
#endif /* HAVE_MOTIF */
}



static void pop_resources_dialog (saver_preferences *p);
static void make_resources_dialog (saver_preferences *p, Widget parent);

static void
edit_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  saver_preferences *p = (saver_preferences *) client_data;
  Widget parent = button;

  do {
    parent = XtParent(parent);
  } while (XtParent(parent));

  if (! resources_dialog)
    make_resources_dialog (p, parent);
  pop_resources_dialog (p);
}

static void
done_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  /* Save here?  Right now we don't need to, because we save every time
     the text field is edited, or the Preferences OK button is pressed.
  */
  exit (0);
}


static void
restart_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  xscreensaver_command (XtDisplay (button), XA_RESTART, 0, False);
}


static void
pop_up_dialog_box (Widget dialog, Widget form)
{
#ifdef HAVE_ATHENA
  XtRealizeWidget (dialog);
  XtPopup (dialog, XtGrabNone);
#else  /* HAVE_MOTIF */
  XtRealizeWidget (form);
  XtManageChild (form);
#endif /* HAVE_MOTIF */
  XMapRaised (XtDisplay (dialog), XtWindow (dialog));
}


static void
make_demo_dialog (Widget toplevel_shell, saver_preferences *p)
{
  Widget parent = toplevel_shell;
  char **hacks = p->screenhacks;

  create_demo_dialog (parent,
		      DefaultVisualOfScreen (XtScreen (parent)),
		      DefaultColormapOfScreen (XtScreen (parent)));
  format_into_label (label1, short_version);

  add_button_callback (next,    next_cb,    (XtPointer) p);
  add_button_callback (prev,    prev_cb,    (XtPointer) p);
  add_button_callback (done,    done_cb,    (XtPointer) p);
  if (restart)
    add_button_callback(restart,restart_cb, (XtPointer) p);
  add_button_callback (edit,    edit_cb,    (XtPointer) p);

#ifdef HAVE_MOTIF
  XtAddCallback (demo_list, XmNbrowseSelectionCallback,
		 select_cb, (XtPointer) p);
  XtAddCallback (demo_list, XmNdefaultActionCallback,
		 select_cb, (XtPointer) p);
  XtAddCallback (text_line, XmNactivateCallback, text_cb, (XtPointer) p);

  if (hacks)
    for (; *hacks; hacks++)
      {
	XmString xmstr = XmStringCreate (*hacks, XmSTRING_DEFAULT_CHARSET);
	XmListAddItem (demo_list, xmstr, 0);
	XmStringFree (xmstr);
      }

#else  /* HAVE_ATHENA */

  /* Hook up the text line. */

  XtAppAddActions(XtWidgetToApplicationContext(text_line),
		  actions, XtNumber(actions));
  XtOverrideTranslations(text_line, XtParseTranslationTable(translations));


  /* Must realize the widget before populating the list, or the dialog
     will be as wide as the longest string.
  */
  XtRealizeWidget (demo_dialog);

  XtVaSetValues (demo_list,
		 XtNlist, hacks,
		 XtNnumberStrings, p->screenhacks_count,
		 0);
  XtAddCallback (demo_list, XtNcallback, select_cb, p);

  /* Now that we've populated the list, make sure that the list is as
     wide as the dialog itself.
  */
  {
    Widget viewport = XtParent(demo_list);
    Widget subform = XtParent(viewport);
    Widget box = XtNameToWidget(demo_dialog, "*box");
    Widget label1 = XtNameToWidget(demo_dialog, "*label1");
    Widget label2 = XtNameToWidget(demo_dialog, "*label2");
    Dimension x=0, y=0, w=0, h=0, bw=0, w2=0;
    XtVaGetValues(subform,
		  XtNwidth, &w, XtNheight, &h, XtNborderWidth, &bw, 0);
    XtVaGetValues(box, XtNwidth, &w2, 0);
    if (w2 != w)
      XtResizeWidget(subform, w2, h, bw);

    /* Why isn't the viewport getting centered? */
    XtVaGetValues(viewport,
		  XtNx, &x, XtNy, &y, XtNheight, &h, XtNborderWidth, &bw, 0);
/*    printf("%d %d %d %d\n", x, y, w, h); */
    XtConfigureWidget(viewport, x, y, w2-x-x, h, bw);

    /* And the text line, too. */
    XtVaGetValues(text_line,
		  XtNwidth, &w, XtNheight, &h, XtNborderWidth, &bw, 0);
    XtVaGetValues(viewport, XtNwidth, &w2, 0);
    if (w2 != w)
      XtResizeWidget(text_line, w2, h, bw);

    /* And the labels too. */
    XtVaGetValues(label1,
		  XtNwidth, &w, XtNheight, &h, XtNborderWidth, &bw, 0);
    if (w2 != w)
      XtResizeWidget(label1, w2, h, bw);

    XtVaGetValues(label2,
		  XtNwidth, &w, XtNheight, &h, XtNborderWidth, &bw, 0);
    if (w2 != w)
      XtResizeWidget(label2, w2, h, bw);

  }

#endif /* HAVE_ATHENA */

  pop_up_dialog_box(demo_dialog, demo_form);

#ifdef HAVE_ATHENA
  /* For Athena, have to do this after the dialog is managed. */
  ensure_selected_item_visible (demo_list);
#endif /* HAVE_ATHENA */
}


/* the Screensaver Parameters dialog */

static struct resources {
  int timeout, cycle, secs, ticks, lock_time, passwd_time;
  int verb, cmap, fade, unfade, lock_p;
} res;


static void 
hack_time_cb (Display *dpy, char *line, int *store, Bool sec_p)
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
res_sec_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  hack_time_cb (XtDisplay (button), get_text_string (button),
		(int *) client_data, True);
}

static void
res_min_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  hack_time_cb (XtDisplay (button), get_text_string (button),
		(int *) client_data, False);
}

static void
res_int_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  char *line = get_text_string (button);
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
res_bool_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  int *store = (int *) client_data;
#ifdef HAVE_MOTIF
  *store = ((XmToggleButtonCallbackStruct *) call_data)->set;
#else /* HAVE_ATHENA */
  Boolean state = FALSE;
  XtVaGetValues (button, XtNstate, &state, 0);
  *store = state;
#endif /* HAVE_ATHENA */
}

static void
res_cancel_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  XtDestroyWidget (resources_dialog);
  resources_dialog = 0;
  XMapRaised (XtDisplay (demo_dialog), XtWindow (demo_dialog));
}


static void
res_done_cb (Widget button, XtPointer client_data, XtPointer call_data)
{
  saver_preferences *p = (saver_preferences *) client_data;

  res_cancel_cb (button, client_data, call_data);

#ifdef HAVE_ATHENA
  /* Check all text widgets, since we don't have callbacks for these. */
  res_min_cb (timeout_text,     (XtPointer) &res.timeout,     0);
  res_min_cb (cycle_text,       (XtPointer) &res.cycle,       0);
  res_sec_cb (fade_text,        (XtPointer) &res.secs,        0);
  res_int_cb (ticks_text,       (XtPointer) &res.ticks,       0);
  res_min_cb (lock_time_text,   (XtPointer) &res.lock_time,   0);
  res_sec_cb (passwd_time_text, (XtPointer) &res.passwd_time, 0);
#endif /* HAVE_ATHENA */

  /* Throttle the timeouts to minimum sane values. */
  if (res.timeout < 5) res.timeout = 5;
  if (res.cycle < 2) res.cycle = 2;
  if (res.passwd_time < 10) res.passwd_time = 10;

  p->timeout = res.timeout * 1000;
  p->cycle = res.cycle * 1000;
  p->lock_timeout = res.lock_time * 1000;
  p->passwd_timeout = res.passwd_time * 1000;
  p->fade_seconds = res.secs;
  p->fade_ticks = res.ticks;
  p->verbose_p = res.verb;
  p->install_cmap_p = res.cmap;
  p->fade_p = res.fade;
  p->unfade_p = res.unfade;
  p->lock_p = res.lock_p;

  write_init_file (p, short_version);
}


static void
make_resources_dialog (saver_preferences *p, Widget parent)
{
  Screen *screen = XtScreen (parent);
  Display *dpy = XtDisplay (parent);

  create_resources_dialog (parent,
			   DefaultVisualOfScreen (screen),
			   DefaultColormapOfScreen (screen));

  add_button_callback (res_done,   res_done_cb,   (XtPointer) p);
  add_button_callback (res_cancel, res_cancel_cb, (XtPointer) p);

#define CB(widget,type,slot) \
	add_text_callback ((widget), (type), (XtPointer) (slot))
#define CBT(widget,type,slot) \
	add_toggle_callback ((widget), (type), (XtPointer) (slot))

#ifdef HAVE_MOTIF
  /* When using Athena widgets, we can't set callbacks for these,
     so we'll check them all if "done" gets pressed.
   */
  CB (timeout_text,	res_min_cb,  &res.timeout);
  CB (cycle_text,	res_min_cb,  &res.cycle);
  CB (fade_text,	res_sec_cb,  &res.secs);
  CB (ticks_text,	res_int_cb,  &res.ticks);
  CB (lock_time_text,	res_min_cb,  &res.lock_time);
  CB (passwd_time_text,	res_sec_cb,  &res.passwd_time);
#endif /* HAVE_MOTIF */

  CBT (verbose_toggle,	res_bool_cb, &res.verb);
  CBT (cmap_toggle,	res_bool_cb, &res.cmap);
  CBT (fade_toggle,	res_bool_cb, &res.fade);
  CBT (unfade_toggle,	res_bool_cb, &res.unfade);
  CBT (lock_toggle,	res_bool_cb, &res.lock_p);
#undef CB
#undef CBT

#if 0
  /* #### have some property that says whether locking is disabled? */
  if (si->locking_disabled_p)
    {
      disable_widget (passwd_time_text);
      disable_widget (lock_time_text);
      disable_widget (lock_toggle);
    }
#endif

  {
    Bool found_any_writable_cells = False;
    int nscreens = ScreenCount(dpy);
    int i;
    for (i = 0; i < nscreens; i++)
      {
	Screen *s = ScreenOfDisplay (dpy, i);
	if (has_writable_cells (s, DefaultVisualOfScreen (s)))
	  {
	    found_any_writable_cells = True;
	    break;
	  }
      }

    if (! found_any_writable_cells)	/* fading isn't possible */
      {
	disable_widget (fade_text);
	disable_widget (ticks_text);
	disable_widget (cmap_toggle);
	disable_widget (fade_toggle);
	disable_widget (unfade_toggle);
      }
  }
}


static void
fmt_time (char *buf, unsigned int s, int min_p)
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
pop_resources_dialog (saver_preferences *p)
{
  char buf [100];

  res.timeout = p->timeout / 1000;
  res.cycle = p->cycle / 1000;
  res.lock_time = p->lock_timeout / 1000;
  res.passwd_time = p->passwd_timeout / 1000;
  res.secs = p->fade_seconds;
  res.ticks = p->fade_ticks;
  res.verb = p->verbose_p;
  res.cmap = p->install_cmap_p;
  res.fade = p->fade_p;
  res.unfade = p->unfade_p;
  res.lock_p = (p->lock_p /* #### && !si->locking_disabled_p */);

  fmt_time (buf, res.timeout, 1);     set_text_string (timeout_text, buf);
  fmt_time (buf, res.cycle, 1);       set_text_string (cycle_text, buf);
  fmt_time (buf, res.lock_time, 1);   set_text_string (lock_time_text, buf);
  fmt_time (buf, res.passwd_time, 0); set_text_string (passwd_time_text, buf);
  fmt_time (buf, res.secs, 0);        set_text_string (fade_text, buf);
  sprintf (buf, "%u", res.ticks);     set_text_string (ticks_text, buf);

  set_toggle_button_state (verbose_toggle, res.verb);
  set_toggle_button_state (cmap_toggle, res.cmap);
  set_toggle_button_state (fade_toggle, res.fade);
  set_toggle_button_state (unfade_toggle, res.unfade);
  set_toggle_button_state (lock_toggle, res.lock_p);

  pop_up_dialog_box (resources_dialog, resources_form);
}

static void
run_hack (Display *dpy, int n)
{
  if (n <= 0) abort();
  xscreensaver_command (dpy, XA_DEMO, n, False);
}


static void
warning_dialog_dismiss_cb (Widget button, XtPointer client_data,
			   XtPointer call_data)
{
  Widget shell = (Widget) client_data;
  XtDestroyWidget (shell);
}

static void
warning_dialog (Widget parent, const char *message)
{
  char *msg = strdup (message);
  char *head;

  Widget dialog = 0;
  Widget label = 0;
  Widget ok = 0;
  int i = 0;

#ifdef HAVE_MOTIF

  Widget w;
  Widget container;
  XmString xmstr;
  Arg av[10];
  int ac = 0;

  ac = 0;
  dialog = XmCreateWarningDialog (parent, "versionWarning", av, ac);

  w = XmMessageBoxGetChild (dialog, XmDIALOG_MESSAGE_LABEL);
  if (w) XtUnmanageChild (w);
  w = XmMessageBoxGetChild (dialog, XmDIALOG_CANCEL_BUTTON);
  if (w) XtUnmanageChild (w);
  w = XmMessageBoxGetChild (dialog, XmDIALOG_HELP_BUTTON);
  if (w) XtUnmanageChild (w);

  ok = XmMessageBoxGetChild (dialog, XmDIALOG_OK_BUTTON);

  ac = 0;
  XtSetArg (av[ac], XmNnumColumns, 1); ac++;
  XtSetArg (av[ac], XmNorientation, XmVERTICAL); ac++;
  XtSetArg (av[ac], XmNpacking, XmPACK_COLUMN); ac++;
  XtSetArg (av[ac], XmNrowColumnType, XmWORK_AREA); ac++;
  XtSetArg (av[ac], XmNspacing, 0); ac++;
  container = XmCreateRowColumn (dialog, "container", av, ac);

#else  /* HAVE_ATHENA */

  Widget form;
  dialog = XtVaCreatePopupShell("warning_dialog", transientShellWidgetClass,
				parent, 0);
  form = XtVaCreateManagedWidget("warning_form", formWidgetClass, dialog, 0);

#endif /* HAVE_ATHENA */

  head = msg;
  while (head)
    {
      char name[20];
      char *s = strchr (head, '\n');
      if (s) *s = 0;

      sprintf (name, "label%d", i++);

#ifdef HAVE_MOTIF
      xmstr = XmStringCreate (head, XmSTRING_DEFAULT_CHARSET);
      ac = 0;
      XtSetArg (av[ac], XmNlabelString, xmstr); ac++;
      label = XmCreateLabelGadget (container, name, av, ac);
      XtManageChild (label);
      XmStringFree (xmstr);
#else  /* HAVE_ATHENA */
      
      label = XtVaCreateManagedWidget (name, labelWidgetClass,
				       form,
				       XtNleft, XtChainLeft,
				       XtNright, XtChainRight,
				       XtNlabel, head,
				       (label ? XtNfromVert : XtNtop),
				       (label ? label : XtChainTop),
				       0);

#endif /* HAVE_ATHENA */

      if (s)
	head = s+1;
      else
	head = 0;
    }

#ifdef HAVE_MOTIF

  XtManageChild (container);
  XtRealizeWidget (dialog);
  XtManageChild (dialog);

#else  /* HAVE_ATHENA */

  ok = XtVaCreateManagedWidget ("ok", commandWidgetClass, form,
				XtNleft, XtChainLeft,
				XtNbottom, XtChainBottom,
				XtNfromVert, label,
				0);

  XtRealizeWidget (dialog);
  XtPopup (dialog, XtGrabNone);

#endif /* HAVE_ATHENA */

  add_button_callback (ok, warning_dialog_dismiss_cb, dialog);

  free (msg);
}



/* The main demo-mode command loop.
 */

#if 0
static Bool
mapper (XrmDatabase *db, XrmBindingList bindings, XrmQuarkList quarks,
	XrmRepresentation *type, XrmValue *value, XPointer closure)
{
  int i;
  for (i = 0; quarks[i]; i++)
    {
      if (bindings[i] == XrmBindTightly)
	fprintf (stderr, (i == 0 ? "" : "."));
      else if (bindings[i] == XrmBindLoosely)
	fprintf (stderr, "*");
      else
	fprintf (stderr, " ??? ");
      fprintf(stderr, "%s", XrmQuarkToString (quarks[i]));
    }

  fprintf (stderr, ": %s\n", (char *) value->addr);

  return False;
}
#endif

static void
the_network_is_not_the_computer (Widget parent)
{
  Display *dpy = XtDisplay (parent);
  char *rversion, *ruser, *rhost;
  char *luser, *lhost;
  char *msg = 0;
  struct passwd *p = getpwuid (getuid ());
  const char *d = DisplayString (dpy);

# if defined(HAVE_UNAME)
  struct utsname uts;
  if (uname (&uts) < 0)
    lhost = "<UNKNOWN>";
  else
    lhost = uts.nodename;
# elif defined(VMS)
  strcpy (lhost, getenv("SYS$NODE"));
# else  /* !HAVE_UNAME && !VMS */
  strcat (lhost, "<UNKNOWN>");
# endif /* !HAVE_UNAME && !VMS */

  if (p && p->pw_name)
    luser = p->pw_name;
  else
    luser = "???";

  server_xscreensaver_version (dpy, &rversion, &ruser, &rhost);

  /* Make a buffer that's big enough for a number of copies of all the
     strings, plus some. */
  msg = (char *) malloc (10 * ((rversion ? strlen(rversion) : 0) +
			       (ruser ? strlen(ruser) : 0) +
			       (rhost ? strlen(rhost) : 0) +
			       strlen(lhost) +
			       strlen(luser) +
			       strlen(d) +
			       30));
  *msg = 0;

  if (!rversion || !*rversion)
    {
      sprintf (msg,
	       "Warning:\n\n"
	       "xscreensaver doesn't seem to be running on display \"%s\".",
	       d);
    }
  else if (p && ruser && *ruser && !!strcmp (ruser, p->pw_name))
    {
      /* Warn that the two processes are running as different users.
       */
      sprintf(msg,
	       "Warning:\n\n"
	      "%s is running as user \"%s\" on host \"%s\".\n"
	      "But the xscreensaver managing display \"%s\"\n"
	      "is running as user \"%s\" on host \"%s\".\n"
	      "\n"
	      "Since they are different users, they won't be reading/writing\n"
	      "the same ~/.xscreensaver file, so %s isn't\n"
	      "going to work right.\n"
	      "\n"
	      "Either re-run %s as \"%s\", or re-run\n"
	      "xscreensaver as \"%s\".\n",
	      progname, luser, lhost,
	      d,
	      (ruser ? ruser : "???"), (rhost ? rhost : "???"),
	      progname,
	      progname, (ruser ? ruser : "???"),
	      luser);
    }
  else if (rhost && *rhost && !!strcmp (rhost, lhost))
    {
      /* Warn that the two processes are running on different hosts.
       */
      sprintf (msg,
	       "Warning:\n\n"
	       "%s is running as user \"%s\" on host \"%s\".\n"
	       "But the xscreensaver managing display \"%s\"\n"
	       "is running as user \"%s\" on host \"%s\".\n"
	       "\n"
	       "If those two machines don't share a file system (that is,\n"
	       "if they don't see the same ~%s/.xscreensaver file) then\n"
	       "%s won't work right.",
	       progname, luser, lhost,
	       d,
	       (ruser ? ruser : "???"), (rhost ? rhost : "???"),
	       luser,
	       progname);
    }
  else if (!!strcmp (rversion, short_version))
    {
      /* Warn that the version numbers don't match.
       */
      sprintf (msg,
	       "Warning:\n\n"
	       "This is %s version %s.\n"
	       "But the xscreensaver managing display \"%s\"\n"
	       "is version %s.  This could cause problems.",
	       progname, short_version,
	       d,
	       rversion);
    }


  if (*msg)
    warning_dialog (parent, msg);

  free (msg);
}


static char *defaults[] = {
#include "XScreenSaver_ad.h"
 0
};

int
main (int argc, char **argv)
{
  XtAppContext app;
  saver_preferences P, *p;
  Bool prefs = False;
  int i;
  Display *dpy;
  Widget toplevel_shell = XtAppInitialize (&app, progclass, 0, 0,
					   &argc, argv, defaults,
					   0, 0);
  dpy = XtDisplay (toplevel_shell);
  db = XtDatabase (dpy);
  XtGetApplicationNameAndClass (dpy, &progname, &progclass);

  for (i = 1; i < argc; i++)
    {
      char *s = argv[i];
      if (s[0] == '-' && s[1] == '-')
	s++;
      if (!strcmp (s, "-prefs"))
	prefs = True;
      else
	{
	  fprintf (stderr, "usage: %s [ -display dpy-string ] [ -prefs ]\n",
		   progname);
	  exit (1);
	}
    }

  short_version = (char *) malloc (5);
  memcpy (short_version, screensaver_id + 17, 4);
  short_version [4] = 0;

  memset (&P, 0, sizeof(P));
  p = &P;
  p->db = db;
  load_init_file (p);

  global_prefs_kludge = p;	/* I hate C so much... */

#if 0
  {
    XrmName name = { 0 };
    XrmClass class = { 0 };
    int count = 0;
    XrmEnumerateDatabase (db, &name, &class, XrmEnumAllLevels, mapper,
			  (XtPointer) &count);
  }
#endif


  XA_VROOT = XInternAtom (dpy, "__SWM_VROOT", False);
  XA_SCREENSAVER = XInternAtom (dpy, "SCREENSAVER", False);
  XA_SCREENSAVER_VERSION = XInternAtom (dpy, "_SCREENSAVER_VERSION",False);
  XA_SCREENSAVER_TIME = XInternAtom (dpy, "_SCREENSAVER_TIME", False);
  XA_SCREENSAVER_ID = XInternAtom (dpy, "_SCREENSAVER_ID", False);
  XA_SCREENSAVER_RESPONSE = XInternAtom (dpy, "_SCREENSAVER_RESPONSE", False);
  XA_SELECT = XInternAtom (dpy, "SELECT", False);
  XA_DEMO = XInternAtom (dpy, "DEMO", False);
  XA_RESTART = XInternAtom (dpy, "RESTART", False);

  make_demo_dialog (toplevel_shell, p);

  if (prefs)
    {
      make_resources_dialog (p, toplevel_shell);
      pop_resources_dialog (p);
    }

  the_network_is_not_the_computer (resources_dialog
				   ? resources_dialog
				   : demo_dialog);

  XtAppMainLoop(app);
  exit (0);
}
