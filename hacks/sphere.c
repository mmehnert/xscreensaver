/* -*- Mode: C; tab-width: 4 -*-
 * sphere.c --- draw a bunch of shaded spheres
 */
#if !defined( lint ) && !defined( SABER )
static const char sccsid[] = "@(#)sphere.c	4.00 97/01/01 xlockmore";
#endif

/* Copyright 1988 by Sun Microsystems, Inc. Mountain View, CA.
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Sun or MIT not be used in advertising
 * or publicity pertaining to distribution of the software without specific
 * prior written permission. Sun and M.I.T. make no representations about the
 * suitability of this software for any purpose. It is provided "as is"
 * without any express or implied warranty.
 *
 * SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL SUN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ***************************************************************************
 *
 * Revision History:
 * 30-May-97: jwz@netscape.com: made it go vertically as well as horizontally.
 * 27-May-97: jwz@netscape.com: turned into a standalone program.
 * 2-Sep-93: xlock version (David Bagley bagleyd@bigfoot.com)
 * 1988: Revised to use SunView canvas instead of gfxsw Sun Microsystems
 * 1982: Orignal Algorithm  Tom Duff  Lucasfilm Ltd.
 */

#ifdef STANDALONE
# define PROGCLASS					"Sphere"
# define HACK_INIT					init_sphere
# define HACK_DRAW					draw_sphere
# define DEF_DELAY					1000
# define BRIGHT_COLORS
# include "xlockmore.h"				/* from the xscreensaver distribution */
#else  /* !STANDALONE */
# include "xlock.h"					/* from the xlockmore distribution */
  ModeSpecOpt sphere_opts = {
	0, NULL, 0, NULL, NULL };
#endif /* !STANDALONE */

/* 
 * (NX, NY, NZ) is the light source vector -- length should be 100
 */
#define NX 48
#define NY (-36)
#define NZ 80
#define NR 100
#define SQRT(a) ((int)sqrt((double)(a)))

typedef struct {
	int         width, height;
	int         radius;
	int         x0;		/* x center */
	int         y0;		/* y center */
	int         color;
	int         x, y;
	int         dirx, diry;
	int         maxx, maxy;
	XPoint     *points;
} spherestruct;

static spherestruct *spheres = NULL;

void
init_sphere(ModeInfo * mi)
{
	spherestruct *sp;

	if (spheres == NULL) {
		if ((spheres = (spherestruct *) calloc(MI_NUM_SCREENS(mi),
					     sizeof (spherestruct))) == NULL)
			return;
	}
	sp = &spheres[MI_SCREEN(mi)];

	if (sp->points) {
		(void) free((void *) sp->points);
		sp->points = NULL;
	}
	sp->width = MI_WIN_WIDTH(mi);
	sp->height = MI_WIN_HEIGHT(mi);
	sp->points = (XPoint *) malloc(sp->height * sizeof (XPoint));

	XClearWindow(MI_DISPLAY(mi), MI_WINDOW(mi));

	sp->x = sp->radius = 0;
}

void
draw_sphere(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	GC          gc = MI_GC(mi);
	spherestruct *sp = &spheres[MI_SCREEN(mi)];
	register    y, miny, maxy, npts = 0;

	if (ABS(sp->x) >= sp->radius) {
	  sp->radius = NRAND(MIN(sp->width / 2, sp->height / 2) - 1) + 1;

	  sp->dirx = (LRAND() & 1) * 2 - 1;
	  sp->diry = (LRAND() & 1) * 2 - 1;

	  if (sp->diry == 1)
		{
		  sp->x0 = NRAND(sp->width);
		  sp->y0 = NRAND(sp->height);
		}
	  else
		{
		  sp->y0 = NRAND(sp->width);
		  sp->x0 = NRAND(sp->height);
		}

		sp->x = -sp->radius * sp->dirx;

		if (MI_NPIXELS(mi) > 2)
			sp->color = NRAND(MI_NPIXELS(mi));
	}
	if (sp->dirx == 1) {
		if (sp->x0 + sp->x < 0)
			sp->x = -sp->x0;
	} else {
	  if (sp->diry == 1) {
		  if (sp->x0 + sp->x >= sp->width)
			sp->x = sp->width - sp->x0 - 1;
	  } else {
		  if (sp->x0 + sp->x >= sp->height)
			sp->x = sp->height - sp->x0 - 1;
		}
	}

	sp->maxy = SQRT(sp->radius * sp->radius - sp->x * sp->x);
	miny = -sp->maxy;
	if (sp->y0 - sp->maxy < 0)
		miny = -sp->y0;
	maxy = sp->maxy;

	if (sp->diry == 1) {
	  if (sp->y0 + sp->maxy >= sp->height)
		maxy = sp->height - sp->y0;
	} else {
	  if (sp->y0 + sp->maxy >= sp->height)
		maxy = sp->width - sp->y0;
	}

	XSetForeground(display, gc, MI_WIN_BLACK_PIXEL(mi));

	if (sp->diry == 1)
	  XDrawLine(display, MI_WINDOW(mi), gc,
				sp->x0 + sp->x, sp->y0 + miny,
				sp->x0 + sp->x, sp->y0 + maxy);
	else
	  XDrawLine(display, MI_WINDOW(mi), gc,
				sp->y0 + miny, sp->x0 + sp->x,
				sp->y0 + maxy, sp->x0 + sp->x);

	if (MI_NPIXELS(mi) > 2)
		XSetForeground(display, gc, MI_PIXEL(mi, sp->color));
	else
		XSetForeground(display, gc, MI_WIN_WHITE_PIXEL(mi));
	for (y = miny; y <= maxy; y++)
		if ((NRAND(sp->radius * NR)) <=
		    (NX * sp->x + NY * y + NZ *
		     SQRT(sp->radius * sp->radius - sp->x * sp->x - y * y))) {
		  if (sp->diry == 1)
			{
			  sp->points[npts].x = sp->x + sp->x0;
			  sp->points[npts].y = y + sp->y0;
			}
		  else
			{
			  sp->points[npts].y = sp->x + sp->x0;
			  sp->points[npts].x = y + sp->y0;
			}
			npts++;
		}
	XDrawPoints(display, MI_WINDOW(mi), gc, sp->points, npts, CoordModeOrigin);
	if (sp->dirx == 1) {
		sp->x++;
		if (sp->diry == 1) {
		  if (sp->x0 + sp->x >= sp->width)
			sp->x = sp->radius;
		} else {
		  if (sp->x0 + sp->x >= sp->height)
			sp->x = sp->radius;
		}
	} else {
		sp->x--;
		if (sp->x0 + sp->x < 0)
			sp->x = -sp->radius;
	}
}

void
release_sphere(ModeInfo * mi)
{
	if (spheres != NULL) {
		int         screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++) {
			spherestruct *sp = &spheres[screen];

			if (sp->points) {
				(void) free((void *) sp->points);
				sp->points = NULL;
			}
		}
		(void) free((void *) spheres);
		spheres = NULL;
	}
}

void
refresh_sphere(ModeInfo * mi)
{
	spherestruct *sp = &spheres[MI_SCREEN(mi)];

	XClearWindow(MI_DISPLAY(mi), MI_WINDOW(mi));
	sp->x = -sp->radius;
}
