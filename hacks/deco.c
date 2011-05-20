/* xscreensaver, Copyright (c) 1997 Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * Concept snarfed from Michael D. Bayne in
 * http://www.go2net.com/internet/deep/1997/04/16/body.html
 */

#include "screenhack.h"
#include <stdio.h>

static unsigned long pixels[512];
static int npixels = 0;
static int max_depth = 0;
static int min_height = 0;
static int min_width = 0;

static void
#ifdef __STDC__
deco (Display *dpy,
      Window window,
      Colormap cmap,
      GC fgc, GC bgc,
      int x, int y, int w, int h, int depth)
#else /* !__STDC__ */
deco (dpy, window, cmap, fgc, bgc, x, y, w, h, depth)
  Display *dpy;
  Window window;
  Colormap cmap;
  GC fgc, bgc;
  int x, y, w, h, depth;
#endif /* !__STDC__ */
{
  if (((random() % max_depth) < depth) || (w < min_width) || (h < min_height))
    {
      if (!mono_p)
	{
	  XColor color;
	  XGCValues gcv;
	  if (npixels == sizeof (pixels) / sizeof (unsigned long))
	    goto REUSE;
	  color.flags = DoRed|DoGreen|DoBlue;
	  color.red = random ();
	  color.green = random ();
	  color.blue = random ();
	  if (! XAllocColor (dpy, cmap, &color))
	    goto REUSE;
	  pixels [npixels++] = color.pixel;
	  gcv.foreground = color.pixel;
	  goto DONE;
	REUSE:
	  gcv.foreground = pixels [random () % npixels];
	DONE:
	  XChangeGC (dpy, bgc, GCForeground, &gcv);
	}
      XFillRectangle (dpy, window, bgc, x, y, w, h);
      XDrawRectangle (dpy, window, fgc, x, y, w, h);
    }
  else
    {
      if (random() & 1)
	{
	  deco (dpy, window, cmap, fgc, bgc, x, y, w/2, h, depth+1);
	  deco (dpy, window, cmap, fgc, bgc, x+w/2, y, w/2, h, depth+1);
	}
      else
	{
	  deco (dpy, window, cmap, fgc, bgc, x, y, w, h/2, depth+1);
	  deco (dpy, window, cmap, fgc, bgc, x, y+h/2, w, h/2, depth+1);
	}
    }
}


char *progclass = "Deco";

char *defaults [] = {
  "Deco.background:	black",		/* to placate SGI */
  "Deco.foreground:	white",
  "*maxDepth:		12",
  "*minWidth:		20",
  "*minHeight:		20",
  "*delay:		5",
  0
};

XrmOptionDescRec options [] = {
  { "-max-depth",	".maxDepth",	XrmoptionSepArg, 0 },
  { "-min-width",	".minWidth",	XrmoptionSepArg, 0 },
  { "-min-height",	".minHeight",	XrmoptionSepArg, 0 },
  { "-delay",		".delay",	XrmoptionSepArg, 0 },
};
int options_size = (sizeof (options) / sizeof (options[0]));

void
#ifdef __STDC__
screenhack (Display *dpy, Window window)
#else /* ! __STDC__ */
screenhack (dpy, window) Display *dpy; Window window;
#endif /* ! __STDC__ */
{
  GC fgc, bgc;
  XGCValues gcv;
  Colormap cmap;
  XWindowAttributes xgwa;
  int delay = get_integer_resource ("delay", "Integer");

  max_depth = get_integer_resource ("maxDepth", "Integer");
  if (max_depth < 1) max_depth = 1;
  else if (max_depth > 1000) max_depth = 1000;

  min_width = get_integer_resource ("minWidth", "Integer");
  if (min_width < 2) min_width = 2;
  min_height = get_integer_resource ("minHeight", "Integer");
  if (min_height < 2) min_height = 2;

  XGetWindowAttributes (dpy, window, &xgwa);

  gcv.foreground = get_pixel_resource("foreground", "Foreground",
				      dpy, xgwa.colormap);
  fgc = XCreateGC (dpy, window, GCForeground, &gcv);

  gcv.foreground = get_pixel_resource("background", "Background",
				      dpy, xgwa.colormap);
  bgc = XCreateGC (dpy, window, GCForeground, &gcv);

  if (!mono_p)
    {
      GC tmp = fgc;
      fgc = bgc;
      bgc = tmp;
    }

  while (1)
    {
      XGetWindowAttributes (dpy, window, &xgwa);
      XFillRectangle(dpy, window, bgc, 0, 0, xgwa.width, xgwa.height);
      deco (dpy, window, xgwa.colormap, fgc, bgc,
	    0, 0, xgwa.width, xgwa.height, 0);
      XSync (dpy, True);
      if (delay)
	sleep(delay);
    }
}
