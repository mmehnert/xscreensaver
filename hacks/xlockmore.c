/* xlockmore.c --- xscreensaver compatibility layer for xlockmore modules.
 * xscreensaver, Copyright (c) 1997 Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * This file, along with xlockmore.h, make it possible to compile an xlockmore
 * module into a standalone program, and thus use it with xscreensaver.
 * By Jamie Zawinski <jwz@netscape.com> on 10-May-97; based on the ideas
 * in the older xlock.h by Charles Hannum <mycroft@ai.mit.edu>.  (I had
 * to redo it, since xlockmore has diverged so far from xlock...)
 */

#include <stdio.h>
#include <math.h>
#include "screenhack.h"
#include "xlockmoreI.h"

/* Storage for the parameters that only *some* xlockmore hacks use...
 */
Bool decay  = False;	/* grav */
Bool trail  = False;	/* grav */
Bool grow   = False;	/* drift */
Bool liss   = False;	/* drift */
Bool ammann = False;	/* penrose */
Bool jong   = False;	/* hopalong */
Bool sine   = False;	/* hopalong */

void 
#ifdef __STDC__
xlockmore_screenhack (Display *dpy, Window window,
		      Bool want_writable_colors,
		      Bool want_uniform_colors,
		      Bool want_smooth_colors,
		      Bool want_bright_colors,
		      void (*hack_init) (ModeInfo *),
		      void (*hack_draw) (ModeInfo *),
		      void (*hack_free) (ModeInfo *))
#else  /*! __STDC__ */
xlockmore_screenhack (dpy, window,
		      want_writable_colors,
		      want_uniform_colors,
		      want_smooth_colors,
		      want_bright_colors,
		      hack_init, hack_draw, hack_free)
	Display *dpy;
	Window window;
	Bool want_writable_colors;
	Bool want_uniform_colors;
	Bool want_smooth_colors;
	Bool want_bright_colors;
	void (*hack_init) ();
	void (*hack_draw) ();
	void (*hack_free) ();
#endif /* !__STDC__ */
{
  ModeInfo mi;
  XGCValues gcv;
  XColor color;
  int i;
  time_t start, now;

  memset(&mi, 0, sizeof(mi));
  mi.dpy = dpy;
  mi.window = window;
  XGetWindowAttributes (dpy, window, &mi.xgwa);

  color.flags = DoRed|DoGreen|DoBlue;
  color.red = color.green = color.blue = 0;
  if (!XAllocColor(dpy, mi.xgwa.colormap, &color))
    abort();
  mi.black = color.pixel;
  color.red = color.green = color.blue = 0xFFFF;
  if (!XAllocColor(dpy, mi.xgwa.colormap, &color))
    abort();
  mi.white = color.pixel;

  if (mono_p)
    {
      static unsigned long pixels[2];
      static XColor colors[2];
    MONO:
      mi.npixels = 2;
      mi.pixels = pixels;
      mi.colors = colors;
      pixels[0] = mi.black;
      pixels[1] = mi.white;
      colors[0].flags = DoRed|DoGreen|DoBlue;
      colors[1].flags = DoRed|DoGreen|DoBlue;
      colors[0].red = colors[0].green = colors[0].blue = 0;
      colors[1].red = colors[1].green = colors[1].blue = 0xFFFF;
    }
  else
    {
      mi.npixels = get_integer_resource ("ncolors", "Integer");
      mi.colors = (XColor *) calloc (mi.npixels, sizeof (*mi.colors));

      mi.writable_p = want_writable_colors;

      if (want_uniform_colors)
	make_uniform_colormap (dpy, mi.xgwa.visual, mi.xgwa.colormap,
			       mi.colors, &mi.npixels,
			       True, &mi.writable_p, True);
      else if (want_smooth_colors)
	make_smooth_colormap (dpy, mi.xgwa.visual, mi.xgwa.colormap,
			      mi.colors, &mi.npixels,
			      True, &mi.writable_p, True);
      else
	make_random_colormap (dpy, mi.xgwa.visual, mi.xgwa.colormap,
			      mi.colors, &mi.npixels,
			      want_bright_colors,
			      True, &mi.writable_p, True);

      if (mi.npixels <= 2)
	goto MONO;
      else
	{
	  int i;
	  mi.pixels = (unsigned long *)
	    calloc (mi.npixels, sizeof (*mi.pixels));
	  for (i = 0; i < mi.npixels; i++)
	    mi.pixels[i] = mi.colors[i].pixel;
	}
    }

  gcv.foreground = mi.white;
  gcv.background = mi.black;
  mi.gc = XCreateGC(dpy, window, GCForeground|GCBackground, &gcv);

  mi.fullrandom = True;

  mi.pause      = get_integer_resource ("delay", "Usecs");

  mi.cycles     = get_integer_resource ("cycles", "Int");
  mi.batchcount = get_integer_resource ("count", "Int");
  mi.size	= get_integer_resource ("size", "Int");

  decay = get_boolean_resource ("decay", "Boolean");
  if (decay) mi.fullrandom = False;

  trail = get_boolean_resource ("trail", "Boolean");
  if (trail) mi.fullrandom = False;

  grow = get_boolean_resource ("grow", "Boolean");
  if (grow) mi.fullrandom = False;

  liss = get_boolean_resource ("liss", "Boolean");
  if (liss) mi.fullrandom = False;

  ammann = get_boolean_resource ("ammann", "Boolean");
  if (ammann) mi.fullrandom = False;

  jong = get_boolean_resource ("jong", "Boolean");
  if (jong) mi.fullrandom = False;

  sine = get_boolean_resource ("sine", "Boolean");
  if (sine) mi.fullrandom = False;

  mi.threed = get_boolean_resource ("use3d", "Boolean");
  mi.threed_delta = get_float_resource ("delta3d", "Boolean");
  mi.threed_right_color = get_pixel_resource ("right3d", "Color", dpy,
					      mi.xgwa.colormap);
  mi.threed_left_color = get_pixel_resource ("left3d", "Color", dpy,
					     mi.xgwa.colormap);
  mi.threed_both_color = get_pixel_resource ("both3d", "Color", dpy,
					     mi.xgwa.colormap);
  mi.threed_none_color = get_pixel_resource ("none3d", "Color", dpy,
					     mi.xgwa.colormap);

  if (mi.pause < 0)
    mi.pause = 0;
  else if (mi.pause > 100000000)
    mi.pause = 100000000;

  XClearWindow (dpy, window);

  i = 0;
  start = time((time_t) 0);

  hack_init (&mi);
  do {
    hack_draw (&mi);
    if (mi.pause)
      usleep(mi.pause);
    XSync(dpy, False);

    if (hack_free)
      {
	if (i++ > (mi.batchcount / 4) &&
	    (start + 5) < (now = time((time_t) 0)))
	  {
	    i = 0;
	    start = now;
	    hack_free (&mi);
	    hack_init (&mi);
	  }
      }

  } while (1);
}
