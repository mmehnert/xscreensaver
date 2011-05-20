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
#include "xlockmoreI.h"

/* Accessor macros for the ModeInfo structure
 */

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

#ifdef DEF_3D
# define MI_WIN_IS_USE3D(MI)    ((MI)->threed)
# define MI_LEFT_COLOR(MI)	((MI)->threed_left_color)
# define MI_RIGHT_COLOR(MI)	((MI)->threed_right_color)
# define MI_BOTH_COLOR(MI)	((MI)->threed_both_color)
# define MI_NONE_COLOR(MI)	((MI)->threed_none_color)
# define MI_DELTA3D(MI)		((MI)->threed_delta)
#endif

#ifdef DEF_CYCLES
# define MI_CYCLES(MI)		((MI)->cycles)
#endif
#ifdef DEF_BATCHCOUNT
# define MI_BATCHCOUNT(MI)	((MI)->batchcount)
#endif
#ifdef DEF_SIZE
# define MI_SIZE(MI)		((MI)->size)
#endif


/* Compatibility with the xlockmore RNG API
   (note that the xlockmore hacks never expect negative numbers.)
 */
#define LRAND()			((long) (random() & 0x7fffffff))
#define NRAND(n)		((int) (LRAND() % (n)))
#define MAXRAND			(2147483648.0) /* unsigned 1<<31 as a float */
#define SRAND(n)		/* already seeded by screenhack.c */

/* Some other utility macros.
 */
#define SINF(n)			((float)sin((double)(n)))
#define COSF(n)			((float)cos((double)(n)))
#define FABSF(n)		((float)fabs((double)(n)))

#undef MAX
#undef MIN
#undef ABS
#define MAX(a,b)((a)>(b)?(a):(b))
#define MIN(a,b)((a)<(b)?(a):(b))
#define ABS(a)((a)<0 ? -(a) : (a))


/* Setting defaults for the parameters that all xlockmore hacks use.
 */

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


/* Declare storage for the parameters that only some xlockmore hacks use...
 */
#ifdef DEF_DECAY
extern Bool decay;
#endif
#ifdef DEF_TRAIL
extern Bool trail;
#endif
#ifdef DEF_GROW
extern Bool grow;
#endif
#ifdef DEF_LISS
extern Bool liss;
#endif
#ifdef DEF_AMMANN
extern Bool ammann;
#endif
#ifdef DEF_JONG
extern Bool jong;
#endif
#ifdef DEF_SINE
extern Bool sine;
#endif


/* Generate the default resources.
 */

#define cpp_stringify_noop_helper(x)#x
#define cpp_stringify(x) cpp_stringify_noop_helper(x)

char *defaults[] = {
  PROGCLASS ".background:	black",
  PROGCLASS ".foreground:	white",
  "*ncolors:	" cpp_stringify(DEF_NCOLORS),
  "*delay:	" cpp_stringify(DEF_DELAY),
#ifdef DEF_CYCLES
  "*cycles:	" cpp_stringify(DEF_CYCLES),
#endif
#ifdef DEF_BATCHCOUNT
  "*count:	" cpp_stringify(DEF_BATCHCOUNT),
#endif
#ifdef DEF_SIZE
  "*size:	" cpp_stringify(DEF_SIZE),
#endif
#ifdef DEF_DECAY
  "*decay:	" DEF_DECAY,
#endif
#ifdef DEF_TRAIL
  "*trail:	" DEF_TRAIL,
#endif
#ifdef DEF_GROW
  "*grow:	" DEF_GROW,
#endif
#ifdef DEF_LISS
  "*liss:	" DEF_LISS,
#endif
#ifdef DEF_AMMANN
  "*ammann:	" DEF_AMMANN,
#endif
#ifdef DEF_JONG
  "*jong:	" DEF_JONG,
#endif
#ifdef DEF_SINE
  "*sine:	" DEF_SINE,
#endif
#ifdef DEF_3D
  "*use3d:	False",
  "*delta3d:	1.5",
  "*left3d:	Blue",
  "*right3d:	Red",
  "*both3d:	Magenta",
  "*none3d:	Black",
#endif
#ifdef DEF_TEXT
  "*text:	" DEF_TEXT,
#endif
#ifdef DEF_FONT
  "*font:	" DEF_FONT,
#endif
#ifdef DEF_BITMAP
  "*bitmap:	" DEF_BITMAP,
#endif
#ifdef DEF_MOUSE
  "*mouse:	" DEF_MOUSE,
#endif
  0
};


/* Generate the command-line arguments.
 */

XrmOptionDescRec options[] = {
  {"-ncolors",	".ncolors",		XrmoptionSepArg, 0},
  {"-delay",	".delay",		XrmoptionSepArg, 0},
#ifdef DEF_CYCLES
  {"-cycles",	".cycles",		XrmoptionSepArg, 0},
#endif
#ifdef DEF_BATCHCOUNT
  {"-count",	".count",		XrmoptionSepArg, 0},
#endif
#ifdef DEF_SIZE							/* galaxy */
  {"-size",	".size",		XrmoptionSepArg, 0},
#endif
#ifdef DEF_DECAY						/* grav */
  {"-decay",	".decay",		XrmoptionNoArg, "True"},
  {"-no-decay",	".decay",		XrmoptionNoArg, "False"},
#endif
#ifdef DEF_TRAIL					/* grav, galaxy */
  {"-trail",	".trail",		XrmoptionNoArg, "True"},
  {"-no-trail",	".trail",		XrmoptionNoArg, "False"},
#endif
#ifdef DEF_GROW							/* drift */
  {"-grow",	".grow",		XrmoptionNoArg, "True"},
  {"-no-grow",	".grow",		XrmoptionNoArg, "False"},
#endif
#ifdef DEF_LISS							/* drift */
  {"-liss",	".liss",		XrmoptionNoArg, "True"},
  {"-no-liss",	".liss",		XrmoptionNoArg, "False"},
#endif
#ifdef DEF_AMMANN						/* penrose */
  {"-ammann",	".ammann",		XrmoptionNoArg, "True"},
  {"-no-ammann",".ammann",		XrmoptionNoArg, "False"},
#endif
#ifdef DEF_JONG							/* hopalong */
  {"-jong",	".jong",		XrmoptionNoArg, "True"},
  {"-no-jong",	".jong",		XrmoptionNoArg, "False"},
#endif
#ifdef DEF_SINE							/* hopalong */
  {"-sine",	".sine",		XrmoptionNoArg, "True"},
  {"-no-sine",	".sine",		XrmoptionNoArg, "False"},
#endif
#ifdef DEF_3D
  {"-3d",	".use3d",		XrmoptionNoArg, "True"},
  {"-no-3d",	".use3d",		XrmoptionNoArg, "False"},
  {"-delta3d",	".delta3d",		XrmoptionSepArg, 0 },
  {"-left3d",	".left3d",		XrmoptionSepArg, 0 },
  {"-right3d",	".right3d",		XrmoptionSepArg, 0 },
  {"-both3d",	".both3d",		XrmoptionSepArg, 0 },
  {"-none3d",	".none3d",		XrmoptionSepArg, 0 },
#endif
#ifdef DEF_TEXT
  {"-text",	".text",		XrmoptionSepArg, 0 },
#endif
#ifdef DEF_FONT
  {"-font",	".font",		XrmoptionSepArg, 0 },
#endif
#ifdef DEF_BITMAP
  {"-bitmap",	".bitmap",		XrmoptionSepArg, 0 },
#endif
#ifdef DEF_MOUSE
  {"-mouse",	".mouse",		XrmoptionNoArg, "True" },
  {"-nomouse",	".mouse",		XrmoptionNoArg, "False" },
#endif
};
int options_size = (sizeof (options) / sizeof (options[0]));


/* Prototypes for the actual drawing routines...
 */
extern void HACK_INIT(ModeInfo *);
extern void HACK_DRAW(ModeInfo *);

#ifdef HACK_FREE
  extern void HACK_FREE(ModeInfo *);
#else
# define HACK_FREE 0
#endif


/* Emit code for the entrypoint used by screenhack.c, and pass control
   down into xlockmore.c with the appropriate parameters.
 */

char *progclass = PROGCLASS;

void screenhack (Display *dpy, Window window)
{
  xlockmore_screenhack (dpy, window,

#ifdef WRITABLE_COLORS
			True,
#else
			False,
#endif

#ifdef UNIFORM_COLORS
			True,
#else
			False,
#endif

#ifdef SMOOTH_COLORS
			True,
#else
			False,
#endif

#ifdef BRIGHT_COLORS
			True,
#else
			False,
#endif

			HACK_INIT,
			HACK_DRAW,
			HACK_FREE);
}
