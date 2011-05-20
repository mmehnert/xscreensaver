/* -*- mode: C; tab-width: 4 -*-
 * xscreensaver, Copyright (c) 1992, 1993, 1994, 1996, 1997, 1998
 * Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

/* distort
 * by Jonas Munsin (jmunsin@iki.fi) and Jamie Zawinski <jwz@jwz.org>
 * it's a bit of a resource hog at the moment
 * TODO:
 *	-mutiple spheres/lenses (with bounces/layering)
 *	-different distortion matrices (and realtime changing matrices)
 *	-randomize movement a bit
 *	-speedup :)
 * program idea borrowed from a screensaver on a non-*NIX OS,
 */

#include <math.h>
#include "screenhack.h"
#include <X11/Xutil.h>

#ifdef HAVE_XSHM_EXTENSION
# include "xshm.h"
static Bool use_shm;
static XShmSegmentInfo shm_info;
#endif /* HAVE_XSHM_EXTENSION */

static int delay, radius, speed;
static XWindowAttributes xgwa;
static GC gc;

static XImage *orig_map, *buffer_map;

static int ***from;

static void init_distort (Display *dpy, Window window) 
{
	XGCValues gcv;
	long gcflags;
	int i, j;
    
	delay = get_integer_resource ("delay", "Integer");
	radius = get_integer_resource ("radius", "Integer");
	speed = get_integer_resource ("speed", "Integer");

#ifdef HAVE_XSHM_EXTENSION
	use_shm = get_boolean_resource("useSHM", "Boolean");
#endif /* HAVE_XSHM_EXTENSION */

	if (delay < 0)
		delay = 0;
	if (radius <= 0)
		radius = 60;
	if (speed <= 0) 
		speed = 2;

	XGetWindowAttributes (dpy, window, &xgwa);

	gcv.function = GXcopy;
	gcv.subwindow_mode = IncludeInferiors;
	gcflags = GCForeground |GCFunction;
	if (use_subwindow_mode_p(xgwa.screen, window)) /* see grabscreen.c */
		gcflags |= GCSubwindowMode;
	gc = XCreateGC (dpy, window, gcflags, &gcv);

	grab_screen_image (xgwa.screen, window);

	buffer_map = 0;
	orig_map = XGetImage(dpy, window, 0, 0, xgwa.width, xgwa.height,
						 ~0L, ZPixmap);


# ifdef HAVE_XSHM_EXTENSION
	if (use_shm)
	  {
		buffer_map = create_xshm_image(dpy, xgwa.visual, orig_map->depth,
									   ZPixmap, 0, &shm_info,
									   2*radius + speed + 2,
									   2*radius + speed + 2);
		if (!buffer_map)
		  use_shm = False;
	  }
# endif /* HAVE_XSHM_EXTENSION */

	if (!buffer_map)
	  {
		buffer_map = XCreateImage(dpy, xgwa.visual,
								  orig_map->depth, ZPixmap, 0, 0,
								  2*radius + speed + 2, 2*radius + speed + 2,
								  8, 0);
		buffer_map->data = (unsigned char *)
		  calloc(buffer_map->height, buffer_map->bytes_per_line);
	  }

	from = (int ***)malloc ((2*radius+speed+2) * sizeof(int **));
	for(i = 0; i < 2*radius+speed+2; i++) {
		from[i] = (int **)malloc((2*radius+speed+2) * sizeof(int *));
		for (j = 0; j < 2*radius+speed+2; j++)
			from[i][j] = (int *)malloc(2*sizeof(int));
	}

	/* initialize a "see-trough" matrix */
	for (i = 0; i < 2*radius+speed+2; i++) {
		for (j = 0 ; j < 2*radius+speed+2 ; j++) {
			from[i][j][0]=i;
			from[i][j][1]=j;
		}
	}

	/* initialize the distort matrix */
	for (i = 0; i < 2*radius; i++) {
		for(j = 0; j < 2*radius; j++) {
			double r;
			r = sqrt ((i-radius)*(i-radius)+(j-radius)*(j-radius));
			if (r < radius-1) {
				r = sin(r*(M_PI_2)/radius);
				if (i < radius)
					from[i][j][0] = radius + (i-radius)*r;
				else
					from[i][j][0] = radius + (i-radius)*r;
				if (j < radius)
					from[i][j][1] = radius + (j-radius)*r;
				else
					from[i][j][1] = radius + (j-radius)*r;
			}
		}
	}

	XSetGraphicsExposures(dpy, gc, False); /* stop events from XCopyArea */
}

static void
move_lens (int *x, int *y, int *xmove, int *ymove) {
	if (*xmove==0)
		*xmove=speed;
	if (*ymove==0)
		*ymove=speed;
	if (*x==0)
		*x = (random() % (xgwa.width-2*radius));
	if (*y==0)
		*y = (random() % (xgwa.height-2*radius));
	if (*x + 4*radius/2 >= xgwa.width)
	  *xmove = -abs(*xmove);
	if (*x <= speed) 
	  *xmove = abs(*xmove);
	if (*y + 4*radius/2 >= xgwa.height)
	  *ymove = -abs(*ymove);
	if (*y <= speed)
	  *ymove = abs(*ymove);

	*x = *x + *xmove;
	*y = *y + *ymove;
}

static void distort (Display *dpy, Window window)
{
	static int x, y, xmove=0, ymove=0;
	int i,j;

	move_lens (&x, &y, &xmove, &ymove);

	for(i = 0 ; i < 2*radius+speed+2; i++) {
	  for(j = 0 ; j < 2*radius+speed+2 ; j++) {
		if (x+from[i][j][0] >= 0 &&
			x+from[i][j][0] < xgwa.width &&
			y+from[i][j][1] >= 0 &&
			y+from[i][j][1] < xgwa.height)
		  XPutPixel(buffer_map, i, j,
					XGetPixel(orig_map,
							  x+from[i][j][0],
							  y+from[i][j][1]));
	  }
	}

	XPutImage(dpy, window, gc, buffer_map, 0, 0, x, y,
			  2*radius+speed+2, 2*radius+speed+2);
}



char *progclass = "Distort";

char *defaults [] = {
	"*dontClearRoot:		True",
#ifdef __sgi    /* really, HAVE_READ_DISPLAY_EXTENSION */
	"*visualID:			Best",
#endif

	"*delay:			10000",
	"*radius:			60",
	"*speed:			2",
#ifdef HAVE_XSHM_EXTENSION
    "*useSHM:			False",		/* xshm turns out not to help. */
#endif /* HAVE_XSHM_EXTENSION */
	0
};

XrmOptionDescRec options [] = {
	{ "-delay",	".delay",	XrmoptionSepArg, 0 },
	{ "-radius",	".radius",	XrmoptionSepArg, 0 },
	{ "-speed",	".speed",	XrmoptionSepArg, 0 },
#ifdef HAVE_XSHM_EXTENSION
	{ "-shm",		".useSHM",	XrmoptionNoArg, "True" },
	{ "-no-shm",	".useSHM",	XrmoptionNoArg, "False" },
#endif /* HAVE_XSHM_EXTENSION */
	{ 0, 0, 0, 0 }
};
		

void screenhack (Display *dpy, Window window) {
	init_distort (dpy, window);
	while (1) {
		distort (dpy, window);
		XSync (dpy, True);
		if (delay) usleep (delay);
	}
}
