/* xscreensaver, Copyright (c) 1992, 1995, 1997
 *  Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * And remember: X Windows is to graphics hacking as roman numerals are to
 * the square root of pi.
 */

/* This file contains simple code to open a window or draw on the root.
   The idea being that, when writing a graphics hack, you can just link
   with this .o to get all of the uninteresting junk out of the way.

   -  create a procedure `screenhack(dpy, window)'

   -  create a variable `char *progclass' which names this program's
      resource class.

   -  create a variable `char defaults []' for the default resources.

   -  create a variable `XrmOptionDescRec options []' for the command-line,
      and `int options_size' which is `XtNumber (options)'.

   And that's it...
 */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Error.h>
#include "screenhack.h"
#include "version.h"
#include "vroot.h"

char *progname;
XrmDatabase db;
Bool mono_p;

static XrmOptionDescRec default_options [] = {
  { "-root",	".root",		XrmoptionNoArg, "True" },
  { "-window",	".root",		XrmoptionNoArg, "False" },
  { "-mono",	".mono",		XrmoptionNoArg, "True" },
  { "-install",	".installColormap",	XrmoptionNoArg, "True" },
  { "-noinstall",".installColormap",	XrmoptionNoArg, "False" },
  { "-visual",	".visualID",		XrmoptionSepArg, 0 }
};

static char *default_defaults[] = {
  "*root:		false",
  "*geometry:		600x480", /* this should be .geometry, but nooooo... */
  "*mono:		false",
  "*installColormap:	false",
  "*visualID:		default",
  0
};

static XrmOptionDescRec *merged_options;
static int merged_options_size;
static char **merged_defaults;

static void
merge_options P((void))
{
  int options_sizeof = options_size * sizeof (options[0]);
  int defaults_size;
  merged_options_size = XtNumber (default_options) + options_size;
  merged_options = (XrmOptionDescRec *)
    malloc (sizeof (default_options) + options_sizeof);
  memcpy (merged_options, options, options_sizeof);
  memcpy (merged_options + options_size, default_options,
	  sizeof (default_options));

  for (defaults_size = 0; defaults [defaults_size]; defaults_size++);
  merged_defaults = (char **)
    malloc (sizeof (default_defaults) + (defaults_size * sizeof (char *)));
  memcpy (merged_defaults, default_defaults, sizeof (default_defaults));
  memcpy ((merged_defaults - 1 +
	   (sizeof (default_defaults) / sizeof (default_defaults[0]))),
	  defaults, ((defaults_size + 1) * sizeof (defaults[0])));
}


/* Make the X errors print out the name of this program, so we have some
   clue which one has a bug when they die under the screensaver.
 */

static int
#ifdef __STDC__
screenhack_ehandler (Display *dpy, XErrorEvent *error)
#else /* !__STDC__ */
screenhack_ehandler (dpy, error)
     Display *dpy;
     XErrorEvent *error;
#endif /* !__STDC__ */
{
  fprintf (stderr, "\nX error in %s:\n", progname);
  if (XmuPrintDefaultErrorMessage (dpy, error, stderr))
    exit (-1);
  else
    fprintf (stderr, " (nonfatal.)\n");
  return 0;
}

static Bool
#ifdef __STDC__
MapNotify_event_p (Display *dpy, XEvent *event, XPointer window)
#else /* !__STDC__ */
MapNotify_event_p (dpy, event, window)
     Display *dpy;
     XEvent *event;
     XPointer window;
#endif /* !__STDC__ */
{
  return (event->xany.type == MapNotify &&
	  event->xvisibility.window == (Window) window);
}


void
#ifdef __STDC__
main (int argc, char **argv)
#else /* !__STDC__ */
main (argc, argv)
     int argc;
     char **argv;
#endif /* !__STDC__ */
{
  XtAppContext app;
  Widget toplevel;
  Display *dpy;
  Window window;
  Visual *visual;
  Colormap cmap;
  Bool root_p;
  XEvent event;
  Boolean dont_clear /*, dont_map */;
  char version[255];

  merge_options ();
  toplevel = XtAppInitialize (&app, progclass, merged_options,
			      merged_options_size, &argc, argv,
			      merged_defaults, 0, 0);
  dpy = XtDisplay (toplevel);
  db = XtDatabase (dpy);
  XtGetApplicationNameAndClass (dpy, &progname, &progclass);
  XSetErrorHandler (screenhack_ehandler);

  {
    char *v = (char *) strdup(strchr(screensaver_id, ' '));
    char *s = (char *) strchr(v, ',');
    *s = 0;
    sprintf (version, "%s: from the XScreenSaver%s distribution.",
	     progclass, v);
    free(v);
  }

  if (argc > 1)
    {
      const char *s;
      int i;
      int x = 18;
      int end = 78;
      Bool help_p = !strcmp(argv[1], "-help");
      fprintf (stderr, "%s\n", version);
      for (s = progclass; *s; s++) fprintf(stderr, " ");
      fprintf (stderr,
	       "  http://www.netscape.com/people/jwz/xscreensaver/\n\n");

      if (!help_p)
	fprintf(stderr, "Unrecognised option: %s\n", argv[1]);
      fprintf (stderr, "Options include: ");
      for (i = 0; i < merged_options_size; i++)
	{
	  char *sw = merged_options [i].option;
	  Bool argp = (merged_options [i].argKind == XrmoptionSepArg);
	  int size = strlen (sw) + (argp ? 6 : 0) + 2;
	  if (x + size >= end)
	    {
	      fprintf (stderr, "\n\t\t ");
	      x = 18;
	    }
	  x += size;
	  fprintf (stderr, "%s", sw);
	  if (argp) fprintf (stderr, " <arg>");
	  if (i != merged_options_size - 1) fprintf (stderr, ", ");
	}
      fprintf (stderr, ".\n");
      exit (help_p ? 0 : 1);
    }

  dont_clear = get_boolean_resource ("dontClearRoot", "Boolean");
/*dont_map = get_boolean_resource ("dontMapWindow", "Boolean"); */
  mono_p = get_boolean_resource ("mono", "Boolean");
  if (CellsOfScreen (DefaultScreenOfDisplay (dpy)) <= 2)
    mono_p = True;

  root_p = get_boolean_resource ("root", "Boolean");
  if (root_p)
    {
      XWindowAttributes xgwa;
      window = RootWindowOfScreen (XtScreen (toplevel));
      XtDestroyWidget (toplevel);
      XGetWindowAttributes (dpy, window, &xgwa);
      cmap = xgwa.colormap;
      visual = xgwa.visual;
    }
  else
    {
      Boolean def_visual_p;
      Screen *screen = XtScreen (toplevel);
      visual = get_visual_resource (screen, "visualID", "VisualID", False);

      if (toplevel->core.width <= 0)
	toplevel->core.width = 600;
      if (toplevel->core.height <= 0)
	toplevel->core.height = 480;

      def_visual_p = (visual == DefaultVisualOfScreen (screen));

      if (!def_visual_p)
	{
	  unsigned int bg, bd;
	  Widget new;

	  cmap = XCreateColormap (dpy, RootWindowOfScreen(screen),
				  visual, AllocNone);
	  bg = get_pixel_resource ("background", "Background", dpy, cmap);
	  bd = get_pixel_resource ("borderColor", "Foreground", dpy, cmap);

	  new = XtVaAppCreateShell (progname, progclass,
				    topLevelShellWidgetClass, dpy,
				    XtNmappedWhenManaged, False,
				    XtNvisual, visual,
				    XtNdepth, visual_depth (screen, visual),
				    XtNwidth, toplevel->core.width,
				    XtNheight, toplevel->core.height,
				    XtNcolormap, cmap,
				    XtNbackground, (Pixel) bg,
				    XtNborderColor, (Pixel) bd,
				    0);
	  XtDestroyWidget (toplevel);
	  toplevel = new;
	  XtRealizeWidget (toplevel);
	  window = XtWindow (toplevel);
	}
      else
	{
	  XtVaSetValues (toplevel, XtNmappedWhenManaged, False, 0);
	  XtRealizeWidget (toplevel);
	  window = XtWindow (toplevel);

	  if (get_boolean_resource ("installColormap", "InstallColormap"))
	    {
	      cmap = XCreateColormap (dpy, window,
				   DefaultVisualOfScreen (XtScreen (toplevel)),
				      AllocNone);
	      XSetWindowColormap (dpy, window, cmap);
	    }
	  else
	    {
	      cmap = DefaultColormap (dpy, DefaultScreen (dpy));
	    }
	}

/*
      if (dont_map)
	{
	  XtVaSetValues (toplevel, XtNmappedWhenManaged, False, 0);
	  XtRealizeWidget (toplevel);
	}
      else
*/
	{
	  XtPopup (toplevel, XtGrabNone);
	}

      XtVaSetValues(toplevel, XtNtitle, version, 0);
    }

  if (!dont_clear)
    {
      XSetWindowBackground (dpy, window,
			    get_pixel_resource ("background", "Background",
						dpy, cmap));
      XClearWindow (dpy, window);
    }

  if (!root_p)
    /* wait for it to be mapped */
    XIfEvent (dpy, &event, MapNotify_event_p, (XPointer) window);

  XSync (dpy, False);
  srandom ((int) time ((time_t *) 0));
  screenhack (dpy, window);
}
