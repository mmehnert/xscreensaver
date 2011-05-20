/* xscreensaver, Copyright (c) 1992, 1995, 1996, 1997
 *  Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include "screenhack.h"
#include <stdio.h>

#define MAXPOLY	16
#define SCALE	6

struct qpoint {
  long x, y;
  long dx, dy;
};

struct qline {
  struct qpoint *p;
  XColor color;
  Bool dead;
};

struct qix {
  int id;
  int fp;
  int nlines;
  int npoly;
  struct qline *lines;
};

static GC draw_gc, erase_gc;
static unsigned int default_fg_pixel;
static long maxx, maxy, max_spread, max_size;
static int color_shift;
static Bool random_p, solid_p, xor_p, transparent_p, gravity_p;
static int delay;
static int count;
static Colormap cmap;
static unsigned long base_pixel;
static int npoly;

static GC *gcs[2];

static void
#ifdef __STDC__
get_geom (Display *dpy, Window window)
#else /* ! __STDC__ */
get_geom (dpy, window)
     Display *dpy;
     Window window;
#endif /* ! __STDC__ */
{
  XWindowAttributes xgwa;
  XGetWindowAttributes (dpy, window, &xgwa);
  maxx = ((long)(xgwa.width+1)<<SCALE)  - 1;
  maxy = ((long)(xgwa.height+1)<<SCALE) - 1;
}

static struct qix *
#ifdef __STDC__
init_one_qix (Display *dpy, Window window, int nlines, int npoly)
#else /* ! __STDC__ */
init_one_qix (dpy, window, nlines, npoly)
     Display *dpy;
     Window window;
     int nlines;
     int npoly;
#endif /* ! __STDC__ */
{
  int i, j;
  struct qix *qix = (struct qix *) calloc (1, sizeof (struct qix));
  qix->nlines = nlines;
  qix->lines = (struct qline *) calloc (qix->nlines, sizeof (struct qline));
  qix->npoly = npoly;
  for (i = 0; i < qix->nlines; i++)
    qix->lines[i].p = (struct qpoint *)
      calloc(qix->npoly, sizeof(struct qpoint));

  if (!mono_p && !transparent_p)
    {
      hsv_to_rgb (random () % 360, frand (1.0), frand (0.5) + 0.5,
		  &qix->lines[0].color.red, &qix->lines[0].color.green,
		  &qix->lines[0].color.blue);
      if (!XAllocColor (dpy, cmap, &qix->lines[0].color))
	{
	  qix->lines[0].color.pixel = default_fg_pixel;
	  XQueryColor (dpy, cmap, &qix->lines[0].color);
	  if (!XAllocColor (dpy, cmap, &qix->lines[0].color))
	    abort ();
	}
    }

  if (max_size == 0)
    {
      for (i = 0; i < qix->npoly; i++)
	{
	  qix->lines[0].p[i].x = random () % maxx;
	  qix->lines[0].p[i].y = random () % maxy;
	}
    }
  else
    {
      /*assert(qix->npoly == 2);*/
      qix->lines[0].p[0].x = random () % maxx;
      qix->lines[0].p[0].y = random () % maxy;
      qix->lines[0].p[1].x = qix->lines[0].p[0].x + (random () % (max_size/2));
      qix->lines[0].p[1].y = qix->lines[0].p[0].y + (random () % (max_size/2));
      if (qix->lines[0].p[1].x > maxx) qix->lines[0].p[1].x = maxx;
      if (qix->lines[0].p[1].y > maxy) qix->lines[0].p[1].y = maxy;
    }

  for (i = 0; i < qix->npoly; i++)
    {
      qix->lines[0].p[i].dx = (random () % (max_spread + 1)) - (max_spread /2);
      qix->lines[0].p[i].dy = (random () % (max_spread + 1)) - (max_spread /2);
    }
  qix->lines[0].dead = True;

  for (i = 1; i < qix->nlines; i++)
    {
      for(j=0; j<qix->npoly; j++)
	qix->lines[i].p[j] = qix->lines[0].p[j];
      qix->lines[i].color = qix->lines[0].color;
      qix->lines[i].dead = qix->lines[0].dead;
  
      if (!mono_p && !transparent_p)
	if (!XAllocColor (dpy, cmap, &qix->lines[i].color))
	  abort ();
    }
  return qix;
}

/* I don't believe this fucking language doesn't have builtin exponentiation.
   I further can't believe that the fucking ^ character means fucking XOR!! */
static int 
#ifdef __STDC__
i_exp (int i, int j)
#else /* ! __STDC__ */
i_exp (i,j) int i, j;
#endif /* ! __STDC__ */
{
  int k = 1;
  while (j--) k *= i;
  return k;
}


static void
#ifdef __STDC__
merge_colors (int argc, XColor **argv, XColor *into_color, int mask,
	      Bool increment_p)
#else /* ! __STDC__ */
merge_colors (argc, argv, into_color, mask, increment_p)
     int argc;
     XColor **argv;
     XColor *into_color;
     int mask;
     Bool increment_p;
#endif /* ! __STDC__ */
{
  int j;
  *into_color = *argv [0];
  into_color->pixel |= mask;
#define SHORT_INC(x,y) (x = ((((x)+(y)) > 0xFFFF) ? 0xFFFF : ((x)+(y))))
#define SHORT_DEC(x,y) (x = ((((x)-(y)) < 0)      ? 0      : ((x)-(y))))
  for (j = 1; j < argc; j++)
    if (increment_p)
      {
	SHORT_INC (into_color->red,   argv[j]->red);
	SHORT_INC (into_color->green, argv[j]->green);
	SHORT_INC (into_color->blue,  argv[j]->blue);
      }
    else
      {
	SHORT_DEC (into_color->red,   argv[j]->red);
	SHORT_DEC (into_color->green, argv[j]->green);
	SHORT_DEC (into_color->blue,  argv[j]->blue);
      }
#undef SHORT_INC
#undef SHORT_DEC
}

/* fill in all the permutations of colors that XAllocColorCells() has 
   allocated for us.  Thanks Ron, you're an additive kind of guy. */
static void
#ifdef __STDC__
permute_colors (XColor *pcolors, XColor *colors,
		int count,
		unsigned long *plane_masks,
		Bool increment_p)
#else /* ! __STDC__ */
permute_colors (pcolors, colors, count, plane_masks, increment_p)
     XColor *pcolors, *colors;
     int count;
     unsigned long *plane_masks;
     Bool increment_p;
#endif /* ! __STDC__ */
{
  int out = 0;
  int max = i_exp (2, count);
  if (count > 31) abort ();
  for (out = 1; out < max; out++)
    {
      XColor *argv [32];
      int this_mask = 0;
      int argc = 0;
      int bit;
      for (bit = 0; bit < 32; bit++)
	if (out & (1<<bit))
	  {
	    argv [argc++] = &pcolors [bit];
	    this_mask |= plane_masks [bit];
	  }
      merge_colors (argc, argv, &colors [out-1], this_mask, increment_p);
    }
}


static struct qix **
#ifdef __STDC__
init_qix (Display *dpy, Window window)
#else /* ! __STDC__ */
init_qix (dpy, window)
     Display *dpy;
     Window window;
#endif /* ! __STDC__ */
{
  int nlines;
  struct qix **qixes;
  XGCValues gcv;
  XWindowAttributes xgwa;
  XGetWindowAttributes (dpy, window, &xgwa);
  cmap = xgwa.colormap;
  count = get_integer_resource ("count", "Integer");
  if (count <= 0) count = 1;
  nlines = get_integer_resource ("segments", "Integer");
  if (nlines <= 0) nlines = 20;
  npoly = get_integer_resource("poly", "Integer");
  if (npoly <= 2) npoly = 2;
  if (npoly > MAXPOLY) npoly = MAXPOLY;
  get_geom (dpy, window);
  max_spread = get_integer_resource ("spread", "Integer");
  if (max_spread <= 0) max_spread = 10;
  max_spread <<= SCALE;
  max_size = get_integer_resource ("size", "Integer");
  if (max_size < 0) max_size = 0;
  max_size <<= SCALE;
  random_p = get_boolean_resource ("random", "Boolean");
  solid_p = get_boolean_resource ("solid", "Boolean");
  xor_p = get_boolean_resource ("xor", "Boolean");
  transparent_p = get_boolean_resource ("transparent", "Boolean");
  gravity_p = get_boolean_resource("gravity", "Boolean");
  delay = get_integer_resource ("delay", "Integer");
  color_shift = get_integer_resource ("colorShift", "Integer");
  if (color_shift < 0 || color_shift >= 360) color_shift = 5;
  if (delay < 0) delay = 0;

  /* Clear up ambiguities regarding npoly */
  if (solid_p) 
    {
      if (npoly != 2)
	fprintf(stderr, "%s: Can't have -solid and -poly; using -poly 2\n",
		progname);
      npoly = 2;
    }      
  if (npoly > 2)
    {
      if (max_size)
	fprintf(stderr, "%s: Can't have -poly and -size; using -size 0\n",
		progname);
      max_size = 0;
    }

  if (count == 1 && transparent_p)
    transparent_p = False; /* it's a no-op */

  if (transparent_p && CellsOfScreen (DefaultScreenOfDisplay (dpy)) <= 2)
    {
      fprintf (stderr, "%s: -transparent only works on color displays.\n",
	       progname);
      transparent_p = False;
    }

  if (xor_p && !transparent_p)
    mono_p = True;

  gcs[0] = gcs[1] = 0;
  gcv.foreground = default_fg_pixel =
    get_pixel_resource ("foreground", "Foreground", dpy, cmap);

  if (transparent_p)
    {
      Bool increment_p = get_boolean_resource ("additive", "Boolean");
      unsigned long plane_masks [32];
      XColor *pcolors, *colors;
      int nplanes = count;
      int i, total_colors;

      /* permutations would be harder if the number of planes didn't fit
	 in an int.  Who has >32-bit displays anyway... */
      if (nplanes > 31) nplanes = 31;

      while (nplanes > 1 &&
	     !XAllocColorCells (dpy, cmap, False, plane_masks, nplanes,
				&base_pixel, 1))
	nplanes--;

      if (nplanes <= 1)
	{
	  fprintf (stderr,
         "%s: couldn't allocate any color planes; turning -transparent off.\n",
		   progname);
	  transparent_p = False;
	  if (xor_p)
	    goto NON_TRANSPARENT_XOR;
	  else
	    goto NON_TRANSPARENT;
	}
      else if (nplanes != count)
	{
	  fprintf (stderr,
		   "%s: only allocated %d color planes (instead of %d).\n",
		   progname, nplanes, count);
	  count = nplanes;
	}

      gcs[0] = (GC *) malloc (count * sizeof (GC));
      gcs[1] = xor_p ? gcs[0] : (GC *) malloc (count * sizeof (GC));
      total_colors = i_exp (2, count);
      pcolors = (XColor *) calloc (count, sizeof (XColor));
      colors = (XColor *) calloc (total_colors, sizeof (XColor));
      for (i = 0; i < count; i++)
	{
	  gcv.plane_mask = plane_masks [i];
	  gcv.foreground = ~0;
	  if (xor_p)
	    {
	      gcv.function = GXxor;
	      gcs [0][i] = XCreateGC (dpy, window,
				      GCForeground|GCFunction|GCPlaneMask,
				      &gcv);
	    }
	  else
	    {
	      gcs [0][i] = XCreateGC (dpy, window, GCForeground|GCPlaneMask,
				      &gcv);
	      gcv.foreground = 0;
	      gcs [1][i] = XCreateGC (dpy, window, GCForeground|GCPlaneMask,
				      &gcv);
	    }

	  /* pick the "primary" (not in that sense) colors.
	     If we are in subtractive mode, pick higher intensities. */
	  hsv_to_rgb (random () % 360, frand (1.0),
		      frand (0.5) + (increment_p ? 0.2 : 0.5),
		      &pcolors[i].red, &pcolors[i].green, &pcolors[i].blue);

	  pcolors [i].flags = DoRed | DoGreen | DoBlue;
	  pcolors [i].pixel = base_pixel | plane_masks [i];
	}
      permute_colors (pcolors, colors, count, plane_masks, increment_p);
      /* clone the default background of the window into our "base" pixel */
      colors [total_colors - 1].pixel =
	get_pixel_resource ("background", "Background", dpy, cmap);
      XQueryColor (dpy, cmap, &colors [total_colors - 1]);
      colors [total_colors - 1].pixel = base_pixel;
      XStoreColors (dpy, cmap, colors, total_colors);
      XSetWindowBackground (dpy, window, base_pixel);
      XClearWindow (dpy, window);
    }
  else if (xor_p)
    {
    NON_TRANSPARENT_XOR:
      gcv.function = GXxor;
      gcv.foreground =
	(default_fg_pixel ^ get_pixel_resource ("background", "Background",
						dpy, cmap));
      draw_gc = erase_gc = XCreateGC(dpy,window,GCForeground|GCFunction,&gcv);
    }
  else
    {
    NON_TRANSPARENT:
      draw_gc = XCreateGC (dpy, window, GCForeground, &gcv);
      gcv.foreground = get_pixel_resource ("background", "Background",
					   dpy, cmap);
      erase_gc = XCreateGC (dpy, window, GCForeground, &gcv);
    }

  qixes = (struct qix **) malloc ((count + 1) * sizeof (struct qix *));
  qixes [count] = 0;
  while (count--)
    {
      qixes [count] = init_one_qix (dpy, window, nlines, npoly);
      qixes [count]->id = count;
    }
  return qixes;
}

static void
#ifdef __STDC__
free_qline (Display *dpy, Window window, Colormap cmap,
	    struct qline *qline,
	    struct qline *prev,
	    struct qix *qix)
#else /* ! __STDC__ */
free_qline (dpy, window, cmap, qline, prev, qix)
     Display *dpy;
     Window window;
     Colormap cmap;
     struct qline *qline, *prev;
     struct qix *qix;
#endif /* ! __STDC__ */
{
  int i;
  if (qline->dead || !prev)
    ;
  else if (solid_p)
    {
      XPoint points [4];
      /*assert(qix->npoly == 2);*/
      points [0].x = qline->p[0].x >> SCALE; 
      points [0].y = qline->p[0].y >> SCALE;
      points [1].x = qline->p[1].x >> SCALE;
      points [1].y = qline->p[1].y >> SCALE;
      points [2].x = prev->p[1].x >> SCALE;
      points [2].y = prev->p[1].y >> SCALE;
      points [3].x = prev->p[0].x >> SCALE;
      points [3].y = prev->p[0].y >> SCALE;
      XFillPolygon (dpy, window, (transparent_p ? gcs[1][qix->id] : erase_gc),
		    points, 4, Complex, CoordModeOrigin);
    }
  else
    {
      /*  XDrawLine (dpy, window, (transparent_p ? gcs[1][qix->id] : erase_gc),
	             qline->p1.x, qline->p1.y, qline->p2.x, qline->p2.y);*/
      static XPoint points[MAXPOLY+1];
      for(i = 0; i < qix->npoly; i++)
	{
	  points[i].x = qline->p[i].x >> SCALE;
	  points[i].y = qline->p[i].y >> SCALE;
	}
      points[qix->npoly] = points[0];
      XDrawLines(dpy, window, (transparent_p ? gcs[1][qix->id] : erase_gc),
		 points, qix->npoly+1, CoordModeOrigin);
    }

  if (!mono_p && !transparent_p)
    XFreeColors (dpy, cmap, &qline->color.pixel, 1, 0);

  qline->dead = True;
}

static void
#ifdef __STDC__
add_qline (Display *dpy, Window window, Colormap cmap,
	   struct qline *qline,
	   struct qline *prev_qline,
	   struct qix *qix)
#else /* ! __STDC__ */
add_qline (dpy, window, cmap, qline, prev_qline, qix)
     Display *dpy;
     Window window;
     Colormap cmap;
     struct qline *qline, *prev_qline;
     struct qix *qix;
#endif /* ! __STDC__ */
{
  int i;

  for(i=0; i<qix->npoly; i++)
    qline->p[i] = prev_qline->p[i];
  qline->color = prev_qline->color;
  qline->dead = prev_qline->dead;

#define wiggle(point,delta,max)						\
  if (random_p) delta += (random () % (1 << (SCALE+1))) - (1 << SCALE);	\
  if (delta > max_spread) delta = max_spread;				\
  else if (delta < -max_spread) delta = -max_spread;			\
  point += delta;							\
  if (point < 0) point = 0, delta = -delta, point += delta<<1;		\
  else if (point > max) point = max, delta = -delta, point += delta<<1;

  if (gravity_p)
    for(i=0; i<qix->npoly; i++)
      qline->p[i].dy += 3;

  for (i = 0; i < qix->npoly; i++)
    {
      wiggle (qline->p[i].x, qline->p[i].dx, maxx);
      wiggle (qline->p[i].y, qline->p[i].dy, maxy);
    }

  if (max_size)
    {
      /*assert(qix->npoly == 2);*/
      if (qline->p[0].x - qline->p[1].x > max_size)
	qline->p[0].x = qline->p[1].x + max_size
	  - (random_p ? random() % max_spread : 0);
      else if (qline->p[1].x - qline->p[0].x > max_size)
	qline->p[1].x = qline->p[0].x + max_size
	  - (random_p ? random() % max_spread : 0);
      if (qline->p[0].y - qline->p[1].y > max_size)
	qline->p[0].y = qline->p[1].y + max_size
	  - (random_p ? random() % max_spread : 0);
      else if (qline->p[1].y - qline->p[0].y > max_size)
	qline->p[1].y = qline->p[0].y + max_size
	  - (random_p ? random() % max_spread : 0);
    }

  if (!mono_p && !transparent_p)
    {
      XColor desired;
      cycle_hue (&qline->color, color_shift);
      qline->color.flags = DoRed | DoGreen | DoBlue;
      desired = qline->color;
      if (XAllocColor (dpy, cmap, &qline->color))
	{
	  /* XAllocColor returns the actual RGB that the hardware let us
	     allocate.  Restore the requested values into the XColor struct
	     so that limited-resolution hardware doesn't cause cycle_hue to
	     get "stuck". */
	  qline->color.red = desired.red;
	  qline->color.green = desired.green;
	  qline->color.blue = desired.blue;
	}
      else
	{
	  qline->color = prev_qline->color;
	  if (!XAllocColor (dpy, cmap, &qline->color))
	    abort (); /* same color should work */
	}
      XSetForeground (dpy, draw_gc, qline->color.pixel);
    }
  if (! solid_p)
    {
      /*  XDrawLine (dpy, window, (transparent_p ? gcs[0][qix->id] : draw_gc),
	             qline->p1.x, qline->p1.y, qline->p2.x, qline->p2.y);*/
      static XPoint points[MAXPOLY+1];
      for (i = 0; i < qix->npoly; i++)
	{
	  points[i].x = qline->p[i].x >> SCALE;
	  points[i].y = qline->p[i].y >> SCALE;
	}
      points[qix->npoly] = points[0];
      XDrawLines(dpy, window, (transparent_p ? gcs[0][qix->id] : draw_gc),
		 points, qix->npoly+1, CoordModeOrigin);
    }
  else if (!prev_qline->dead)
    {
      XPoint points [4];
      points [0].x = qline->p[0].x >> SCALE;
      points [0].y = qline->p[0].y >> SCALE;
      points [1].x = qline->p[1].x >> SCALE;
      points [1].y = qline->p[1].y >> SCALE;
      points [2].x = prev_qline->p[1].x >> SCALE;
      points [2].y = prev_qline->p[1].y >> SCALE;
      points [3].x = prev_qline->p[0].x >> SCALE;
      points [3].y = prev_qline->p[0].y >> SCALE;
      XFillPolygon (dpy, window, (transparent_p ? gcs[0][qix->id] : draw_gc),
		    points, 4, Complex, CoordModeOrigin);
    }

  qline->dead = False;
}

static void
#ifdef __STDC__
qix1 (Display *dpy, Window window, struct qix *qix)
#else /* ! __STDC__ */
qix1 (dpy, window, qix)
     Display *dpy;
     Window window;
     struct qix *qix;
#endif /* ! __STDC__ */
{
  int ofp = qix->fp - 1;
  static int gtick = 0;
  if (ofp < 0) ofp = qix->nlines - 1;
  if (gtick++ == 500)
    get_geom (dpy, window), gtick = 0;
  free_qline (dpy, window, cmap, &qix->lines [qix->fp],
	      &qix->lines[(qix->fp + 1) % qix->nlines], qix);
  add_qline (dpy, window, cmap, &qix->lines[qix->fp], &qix->lines[ofp], qix);
  if ((++qix->fp) >= qix->nlines)
    qix->fp = 0;
}


char *progclass = "Qix";

char *defaults [] = {
  "Qix.background:	black",		/* to placate SGI */
  "Qix.foreground:	white",
  "*count:	1",
  "*segments:	50",
  "*poly:	2",
  "*spread:	8",
  "*size:	0",
  "*colorShift:	3",
  "*solid:	false",
  "*delay:	10000",
  "*random:	true",
  "*xor:	false",
  "*transparent:false",
  "*gravity:	false",
  "*additive:	true",
  0
};

XrmOptionDescRec options [] = {
  { "-count",		".count",	XrmoptionSepArg, 0 },
  { "-segments",	".segments",	XrmoptionSepArg, 0 },
  { "-poly",		".poly",	XrmoptionSepArg, 0 },
  { "-spread",		".spread",	XrmoptionSepArg, 0 },
  { "-size",		".size",	XrmoptionSepArg, 0 },
  { "-delay",		".delay",	XrmoptionSepArg, 0 },
  { "-color-shift",	".colorShift",	XrmoptionSepArg, 0 },
  { "-random",		".random",	XrmoptionNoArg, "true" },
  { "-linear",		".random",	XrmoptionNoArg, "false" },
  { "-solid",		".solid",	XrmoptionNoArg, "true" },
  { "-hollow",		".solid",	XrmoptionNoArg, "false" },
  { "-xor",		".xor",		XrmoptionNoArg, "true" },
  { "-no-xor",		".xor",		XrmoptionNoArg, "false" },
  { "-transparent",	".transparent",	XrmoptionNoArg, "true" },
  { "-non-transparent",	".transparent",	XrmoptionNoArg, "false" },
  { "-gravity",		".gravity",	XrmoptionNoArg, "true" },
  { "-no-gravity",	".gravity",	XrmoptionNoArg, "false" },
  { "-additive",	".additive",	XrmoptionNoArg, "true" },
  { "-subtractive",	".additive",	XrmoptionNoArg, "false" },
};
int options_size = (sizeof (options) / sizeof (options[0]));

void
#ifdef __STDC__
screenhack (Display *dpy, Window window)
#else /* ! __STDC__ */
screenhack (dpy, window) Display *dpy; Window window;
#endif /* ! __STDC__ */
{
  struct qix **q1 = init_qix (dpy, window);
  struct qix **qn;
  while (1)
    for (qn = q1; *qn; qn++)
      {
	qix1 (dpy, window, *qn);
	XSync (dpy, True);
	if (delay) usleep (delay);
      }
}
