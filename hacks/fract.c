/* -*- Mode: C; tab-width: 4 -*-
   Ported from xlockmore 4.03a10 to be a standalone program and thus usable
   with xscreensaver by Jamie Zawinski <jwz@netscape.com> on 10-May-97.

   Original copyright notice from xlock.c:

    * Copyright (c) 1988-91 by Patrick J. Naughton.
    *
    * Permission to use, copy, modify, and distribute this software and its
    * documentation for any purpose and without fee is hereby granted,
    * provided that the above copyright notice appear in all copies and that
    * both that copyright notice and this permission notice appear in
    * supporting documentation.
    *
    * This file is provided AS IS with no warranties of any kind.  The author
    * shall have no liability with respect to the infringement of copyrights,
    * trade secrets or any patents by this file or any part thereof.  In no
    * event will the author be liable for any lost revenue or profits or
    * other special, indirect and consequential damages.
 */

#if !defined( lint ) && !defined( SABER )
static const char sccsid[] = "@(#)fract.c	4.02 97/04/01 xlockmore";

#endif

/*-
 * xlockmore mode written by Tracy Camp
 * campt@hurrah.com 1997
 * released to the public domain
 *
 * This was modifed from a 'screen saver' that a friend and I
 * wrote on our TI-8x calculators in high school physics one day
 * Basically another geometric pattern generator, this ones claim
 * to fame is a pseudo-fractal looking vine like pattern that creates
 * nifty whorls and loops.
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 *
 * 10-May-97: jwz@netscape.com: turned into a standalone program.
 * 21-MAR-97:  David Hansen <dhansen@metapath.com>
 *             Updated mode to draw complete patterns on every
 *             iteration instead of growing the vine.  Also made
 *             adjustments to randomization and changed variable
 *             names to make logic easier to follow.
 * 
 */

#ifndef STANDALONE
# include "xlock.h"
#else  /* STANDALONE */

# define PROGCLASS	"Fract"
# define HACK_INIT	init_fract
# define HACK_DRAW	draw_fract
# define DEF_DELAY	200000
# include "xlockmore.h"

#endif /* STANDALONE */


#ifndef STANDALONE
ModeSpecOpt fract_opts =
{0, NULL, 0, NULL, NULL};
#endif /* !STANDALONE */

typedef struct {
	int         a;
	int         x1;
	int         y1;
	int         x2;
	int         y2;
	int         length;
	int         iterations;
	int         constant;
	int         ang;
	int         centerx;
	int         centery;
} fractstruct;

static fractstruct *fracts = NULL;

void
refresh_fract(ModeInfo * mi)
{
}

void
init_fract(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	fractstruct *fp;

	if (fracts == NULL) {
		if ((fracts = (fractstruct *) calloc(MI_NUM_SCREENS(mi), sizeof (fractstruct))) == NULL) {
			return;
		}
	}
	fp = &fracts[MI_SCREEN(mi)];

	fp->iterations = 30 + NRAND(100);

	XClearWindow(display, MI_WINDOW(mi));
}

void
draw_fract(ModeInfo * mi)
{
	fractstruct *fp = &fracts[MI_SCREEN(mi)];
	Display    *display = MI_DISPLAY(mi);
	GC          gc = MI_GC(mi);
	int         i;

	if (--(fp->iterations) == 0)
		init_fract(mi);

	fp->centerx = NRAND(MI_WIN_WIDTH(mi));
	fp->centery = NRAND(MI_WIN_HEIGHT(mi));

	fp->ang = 60 + NRAND(720);
	fp->length = 100 + NRAND(3000);
	fp->constant = fp->length * (10 + NRAND(10));

	fp->a = 0;
	fp->x1 = 0;
	fp->y1 = 0;
	fp->x2 = 1;
	fp->y2 = 0;

	if (MI_NPIXELS(mi) > 2)
		XSetForeground(display, gc, MI_PIXEL(mi, NRAND(MI_NPIXELS(mi))));
	else
		XSetForeground(display, gc, MI_WIN_WHITE_PIXEL(mi));


	for (i = 0; i < fp->length; i++) {
		XDrawLine(display, MI_WINDOW(mi), gc,
			  fp->centerx + (fp->x1 / fp->constant),
			  fp->centery - (fp->y1 / fp->constant),
			  fp->centerx + (fp->x2 / fp->constant),
			  fp->centery - (fp->y2 / fp->constant));

		fp->a += (fp->ang * i);

		fp->x1 = fp->x2;
		fp->y1 = fp->y2;

		fp->x2 += (int) (i * ((cos(fp->a) * 360) / (2 * M_PI)));
		fp->y2 += (int) (i * ((sin(fp->a) * 360) / (2 * M_PI)));
	}
}

void
release_fract(ModeInfo * mi)
{
	if (fracts != NULL) {
		(void) free((void *) fracts);
		fracts = NULL;
	}
}
