/* xlockmore.h --- xscreensaver compatibility layer for xlockmore modules.
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
 * The definitions in this file make it possible to compile an xlockmore
 * module into a standalone program, and thus use it with xscreensaver.
 * By Jamie Zawinski <jwz@netscape.com> on 10-May-97; based on the ideas
 * in the older xlock.h by Charles Hannum <mycroft@ai.mit.edu>.  (I had
 * to redo it, since xlockmore has diverged so far from xlock...)
 */

#if !defined(PROGCLASS) || !defined(HACK_INIT) || !defined(HACK_DRAW)
ERROR!  Define PROGCLASS, HACK_INIT, and HACK_DRAW before including xlockmore.h
#endif

#ifndef __STDC__
ERROR!  Sorry, xlockmore.h requires ANSI C (gcc, for example.)
  /* (The ansi dependency is that we use string concatenation,
     and cpp-based stringification of tokens.) */
#endif


#include <stdio.h>
#include <math.h>
#include "screenhack.h"

#if defined(DEF_CYCLES) && !defined(USE_CYCLES)
# define USE_CYCLES
#endif

#if defined(DEF_BATCHCOUNT) && !defined(USE_BATCHCOUNT)
# define USE_BATCHCOUNT
#endif

#if defined(DEF_DECAY) && !defined(USE_DECAY)
# define USE_DECAY
#endif

#if defined(DEF_TRAIL) && !defined(USE_TRAIL)
# define USE_TRAIL
#endif

#if defined(DEF_GROW) && !defined(USE_GROW)
# define USE_GROW
#endif

#if defined(DEF_LISS) && !defined(USE_LISS)
# define USE_LISS
#endif

#if defined(DEF_AMMANN) && !defined(USE_AMMANN)
# define USE_AMMANN
#endif

#if defined(DEF_JONG) && !defined(USE_JONG)
# define USE_JONG
#endif

#if defined(DEF_SINE) && !defined(USE_SINE)
# define USE_SINE
#endif

#if defined(DEF_SIZE) && !defined(USE_SIZE)
# define USE_SIZE
#endif

#ifndef NUMCOLORS
# ifdef DEF_NCOLORS
#  define NUMCOLORS DEF_NCOLORS
# else
#  define NUMCOLORS 64
# endif
#endif

#ifndef DEF_NCOLORS
# define DEF_NCOLORS NUMCOLORS
#endif

#ifndef DEF_DELAY
# define DEF_DELAY 50000
#endif

#ifndef DEF_CYCLES
# define DEF_CYCLES 1
#endif

#ifndef DEF_BATCHCOUNT
# define DEF_BATCHCOUNT 1
#endif

#ifndef DEF_SIZE
# define DEF_SIZE 1
#endif

#define cpp_stringify_noop_helper(x)#x
#define cpp_stringify(x) cpp_stringify_noop_helper(x)

char *defaults[] = {
  PROGCLASS ".background:	black",
  PROGCLASS ".foreground:	white",
  "*ncolors:	" cpp_stringify(DEF_NCOLORS),
  "*delay:	" cpp_stringify(DEF_DELAY),
#ifdef USE_CYCLES
  "*cycles:	" cpp_stringify(DEF_CYCLES),
#endif
#ifdef USE_BATCHCOUNT
  "*count:	" cpp_stringify(DEF_BATCHCOUNT),
#endif
#ifdef USE_SIZE
  "*size:	" cpp_stringify(DEF_SIZE),
#endif
#ifdef USE_DECAY
  "*decay:	" DEF_DECAY,
#endif
#ifdef USE_TRAIL
  "*trail:	" DEF_TRAIL,
#endif
#ifdef USE_GROW
  "*grow:	" DEF_GROW,
#endif
#ifdef USE_LISS
  "*liss:	" DEF_LISS,
#endif
#ifdef USE_AMMANN
  "*ammann:	" DEF_AMMANN,
#endif
#ifdef USE_JONG
  "*jong:	" DEF_JONG,
#endif
#ifdef USE_SINE
  "*sine:	" DEF_SINE,
#endif
#ifdef USE_3D
  "*use3d:	False",
  "*delta3d:	1.5",
  "*left3d:	Blue",
  "*right3d:	Red",
  "*both3d:	Magenta",
  "*none3d:	Black",
#endif
  0
};

XrmOptionDescRec options[] = {
  {"-ncolors",	".ncolors",		XrmoptionSepArg, 0},
  {"-delay",	".delay",		XrmoptionSepArg, 0},
#ifdef USE_CYCLES
  {"-cycles",	".cycles",		XrmoptionSepArg, 0},
#endif
#ifdef USE_BATCHCOUNT
  {"-count",	".count",		XrmoptionSepArg, 0},
#endif
#ifdef USE_SIZE							/* galaxy */
  {"-size",	".size",		XrmoptionSepArg, 0},
#endif
#ifdef USE_DECAY						/* grav */
  {"-decay",	".decay",		XrmoptionNoArg, "True"},
  {"-no-decay",	".decay",		XrmoptionNoArg, "False"},
#endif
#ifdef USE_TRAIL					/* grav, galaxy */
  {"-trail",	".trail",		XrmoptionNoArg, "True"},
  {"-no-trail",	".trail",		XrmoptionNoArg, "False"},
#endif
#ifdef USE_GROW							/* drift */
  {"-grow",	".grow",		XrmoptionNoArg, "True"},
  {"-no-grow",	".grow",		XrmoptionNoArg, "False"},
#endif
#ifdef USE_LISS							/* drift */
  {"-liss",	".liss",		XrmoptionNoArg, "True"},
  {"-no-liss",	".liss",		XrmoptionNoArg, "False"},
#endif
#ifdef USE_AMMANN						/* penrose */
  {"-ammann",	".ammann",		XrmoptionNoArg, "True"},
  {"-no-ammann",".ammann",		XrmoptionNoArg, "False"},
#endif
#ifdef USE_JONG							/* hopalong */
  {"-jong",	".jong",		XrmoptionNoArg, "True"},
  {"-no-jong",	".jong",		XrmoptionNoArg, "False"},
#endif
#ifdef USE_SINE							/* hopalong */
  {"-sine",	".sine",		XrmoptionNoArg, "True"},
  {"-no-sine",	".sine",		XrmoptionNoArg, "False"},
#endif
#ifdef USE_3D
  {"-3d",	".use3d",		XrmoptionNoArg, "True"},
  {"-no-3d",	".use3d",		XrmoptionNoArg, "False"},
  {"-delta3d",	".delta3d",		XrmoptionSepArg, 0 },
  {"-left3d",	".left3d",		XrmoptionSepArg, 0 },
  {"-right3d",	".right3d",		XrmoptionSepArg, 0 },
  {"-both3d",	".both3d",		XrmoptionSepArg, 0 },
  {"-none3d",	".none3d",		XrmoptionSepArg, 0 },
#endif
};
int options_size = (sizeof (options) / sizeof (options[0]));

char *progclass = PROGCLASS;


typedef struct ModeInfo {
  Display *dpy;
  Window window;
  int npixels;
  unsigned long *pixels;
  unsigned long white;
  unsigned long black;
  XWindowAttributes xgwa;
  GC gc;
  long pause;
  Bool fullrandom;
#ifdef USE_CYCLES
  long cycles;
#endif
#ifdef USE_BATCHCOUNT
  long batchcount;
#endif
#ifdef USE_SIZE
  long size;
#endif
#ifdef USE_3D
  Bool threed;
  long threed_left_color;
  long threed_right_color;
  long threed_both_color;
  long threed_none_color;
  long threed_delta;
#endif
} ModeInfo;

#undef MAX
#undef MIN
#undef ABS
#define MAX(a,b)((a)>(b)?(a):(b))
#define MIN(a,b)((a)<(b)?(a):(b))
#define ABS(a)((a)<0 ? -(a) : (a))

/* the xlockmore hacks expect these routines to never return negative. */
#define LRAND()			((long) (random() & 0x7fffffff))
#define NRAND(n)		((int) (LRAND() % (n)))
#define MAXRAND			(2147483648.0) /* unsigned 1<<31 as a float */
#define SRAND(n)		/* already seeded by screenhack.c */

#define SINF(n)			((float)sin((double)(n)))
#define COSF(n)			((float)cos((double)(n)))
#define FABSF(n)		((float)fabs((double)(n)))

#define MI_DISPLAY(MI)		((MI)->dpy)
#define MI_WINDOW(MI)		((MI)->window)
#define MI_NUM_SCREENS(MI)	(1)
#define MI_SCREEN(MI)		(0)
#define MI_WIN_WHITE_PIXEL(MI)	((MI)->white)
#define MI_WIN_BLACK_PIXEL(MI)	((MI)->black)
#define MI_NPIXELS(MI)		((MI)->npixels)
#define MI_PIXEL(MI,N)		((MI)->pixels[(N)])
#define MI_WIN_WIDTH(MI)	((MI)->xgwa.width)
#define MI_WIN_HEIGHT(MI)	((MI)->xgwa.height)
#define MI_WIN_DEPTH(MI)	((MI)->xgwa.depth)
#define MI_WIN_COLORMAP(MI)	((MI)->xgwa.colormap)
#define MI_VISUAL(MI)		((MI)->xgwa.visual)
#define MI_GC(MI)		((MI)->gc)
#define MI_PAUSE(MI)		((MI)->pause)
#define MI_WIN_IS_FULLRANDOM(MI)((MI)->fullrandom)
#define MI_WIN_IS_VERBOSE(MI)   (False)
#define MI_WIN_IS_INSTALL(MI)   (True)

#ifdef USE_3D
# define MI_WIN_IS_USE3D(MI)    ((MI)->threed)
# define MI_LEFT_COLOR(MI)	((MI)->threed_left_color)
# define MI_RIGHT_COLOR(MI)	((MI)->threed_right_color)
# define MI_BOTH_COLOR(MI)	((MI)->threed_both_color)
# define MI_NONE_COLOR(MI)	((MI)->threed_none_color)
# define MI_DELTA3D(MI)		((MI)->threed_delta)
#endif

#ifdef USE_CYCLES
# define MI_CYCLES(MI)		((MI)->cycles)
#endif
#ifdef USE_BATCHCOUNT
# define MI_BATCHCOUNT(MI)	((MI)->batchcount)
#endif
#ifdef USE_SIZE
# define MI_SIZE(MI)		((MI)->size)
#endif

#ifdef USE_DECAY
static Bool decay;
#endif
#ifdef USE_TRAIL
static Bool trail;
#endif
#ifdef USE_GROW
static Bool grow;
#endif
#ifdef USE_LISS
static Bool liss;
#endif
#ifdef USE_AMMANN
static Bool ammann;
#endif
#ifdef USE_JONG
static Bool jong;
#endif
#ifdef USE_SINE
static Bool sine;
#endif


void HACK_INIT(ModeInfo *);
void HACK_DRAW(ModeInfo *);

#ifdef HACK_FREE
void HACK_FREE(ModeInfo *);
#endif

void screenhack(Display *dpy, Window window)
{
  ModeInfo mi;
  XGCValues gcv;
  XColor color;
#ifdef HACK_FREE
  int i;
  time_t start, now;
#endif
  int color_lossage;

#ifdef SPREAD_COLORS
  /* let's cycle in the other direction sometimes (noticable with braid.) */
  int counterclockwise = random()&1;
#endif

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
      color_lossage = 2;
#ifdef SPREAD_COLORS
    MONO:
#endif
      mi.npixels = 2;
      mi.pixels = pixels;
      pixels[0] = mi.black;
      pixels[1] = mi.white;
    }
  else
    {
      int i = get_integer_resource ("ncolors", "Integer");
      mi.pixels = (unsigned long *) calloc (i, sizeof (unsigned long));
      color_lossage = i;
#ifdef SPREAD_COLORS
    AGAIN:
#endif
      for (mi.npixels = 0; mi.npixels < i; mi.npixels++)
	{
#ifdef SPREAD_COLORS
	  int H;
	  double S, V;
	  if (counterclockwise)
	    H = (360 * (i-mi.npixels-1)) / i;		/* range 360-0    */
	  else
	    H = (360 * mi.npixels) / i;			/* range 0-360    */
	  S = ((double) NRAND(33) + 66) / 100.0;	/* range 66%-100% */
	  V = ((double) NRAND(33) + 66) / 100.0;	/* range 66%-100% */
	  hsv_to_rgb (H, S, V, &color.red, &color.green, &color.blue);
#else
# ifdef BRIGHT_COLORS
	  int H = (360 * mi.npixels) / i;		 /* range 0-360    */
	  double S = ((double) NRAND(70) + 30) / 100.0;  /* range 30%-100% */
	  double V = ((double) NRAND(33) + 66) / 100.0;  /* range 66%-100% */
	  hsv_to_rgb (H, S, V, &color.red, &color.green, &color.blue);
# else /* linear random colors */
	  color.red   = NRAND(0xFFFF);
	  color.green = NRAND(0xFFFF);
	  color.blue  = NRAND(0xFFFF);
# endif
#endif
	  if (! XAllocColor (dpy, mi.xgwa.colormap, &color))
	    {
#ifndef SPREAD_COLORS
	      break;
#else  /* SPREAD_COLORS */
	      /* Oops, couldn't allocate all the colors we wanted.  Since
		 we're trying to make a ramp that matches up on both ends,
		 let's reduce the number of colors by 10% and try again...
	       */
	      int j = i;
	      XFreeColors(dpy, mi.xgwa.colormap, mi.pixels, mi.npixels, 0);
	      i = (i * 9) / 10;
	      if (i >= j) i--;
	      if (i <= 2) goto MONO;
	      goto AGAIN;
#endif /* SPREAD_COLORS */
	    }
	  mi.pixels[mi.npixels] = color.pixel;
	}
    }

  if (color_lossage != mi.npixels)
    fprintf(stderr, "%s: wanted %d colors, got %d.\n",
	    progname, color_lossage, mi.npixels);

  gcv.foreground = mi.white;
  gcv.background = mi.black;
  mi.gc = XCreateGC(dpy, window, GCForeground|GCBackground, &gcv);

  mi.fullrandom = True;

  mi.pause      = get_integer_resource ("delay", "Usecs");

#ifdef USE_CYCLES
  mi.cycles     = get_integer_resource ("cycles", "Int");
#endif
#ifdef USE_BATCHCOUNT
  mi.batchcount = get_integer_resource ("count", "Int");
#endif
#ifdef USE_SIZE
  mi.size	= get_integer_resource ("size", "Int");
#endif

#ifdef USE_DECAY
  decay = get_boolean_resource ("decay", "Boolean");
  if (decay) mi.fullrandom = False;
#endif
#ifdef USE_TRAIL
  trail = get_boolean_resource ("trail", "Boolean");
  if (trail) mi.fullrandom = False;
#endif
#ifdef USE_GROW
  grow = get_boolean_resource ("grow", "Boolean");
  if (grow) mi.fullrandom = False;
#endif
#ifdef USE_LISS
  liss = get_boolean_resource ("liss", "Boolean");
  if (liss) mi.fullrandom = False;
#endif
#ifdef USE_AMMANN
  ammann = get_boolean_resource ("ammann", "Boolean");
  if (ammann) mi.fullrandom = False;
#endif
#ifdef USE_JONG
  jong = get_boolean_resource ("jong", "Boolean");
  if (jong) mi.fullrandom = False;
#endif
#ifdef USE_SINE
  sine = get_boolean_resource ("sine", "Boolean");
  if (sine) mi.fullrandom = False;
#endif

#ifdef USE_3D
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
#endif

  if (mi.pause < 0)
    mi.pause = 0;
  else if (mi.pause > 100000000)
    mi.pause = 100000000;

  XClearWindow (dpy, window);

#ifdef HACK_FREE
  i = 0;
  start = time((time_t) 0);
#endif

  HACK_INIT(&mi);
  do {
    HACK_DRAW(&mi);
    if (mi.pause)
      usleep(mi.pause);
    XSync(dpy, False);

#ifdef HACK_FREE
    if (i++ > (DEF_BATCHCOUNT / 4) &&
	(start + 5) < (now = time((time_t) 0)))
      {
	i = 0;
	start = now;
	HACK_FREE(&mi);
	HACK_INIT(&mi);
      }
#endif

  } while (1);
}
