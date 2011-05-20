/* xscreensaver, Copyright (c) 1992, 1995, 1997
 *  Jamie Zawinski <jwz@netscape.com>
 *
 *  reaction/diffusion textures
 *  Copyright (c) 1997 Scott Draves spot@transmeta.com
 *  this code is derived from Bomb
 *  see http://www.cs.cmu.edu/~spot/bomb.html
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

#include <math.h>

#include "screenhack.h"

/* why doesn't this work??? */
#ifdef HAVE_XSHM_EXTENSION
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

#define test_pattern_hyper 0

/* costs ~6% speed */
#define dither_when_mapped 1

char *progclass = "RD";


char *defaults [] = {
  "RD.background:	black",		/* to placate SGI */
  "RD.foreground:	white",
  "*width:	100",
  "*height:	100",
  "*epoch:	40000",
  "*palette:	-1",
  "*reaction:	-1",
  "*diffusion:	-1",
  "*verbose:	off",
  "*radius:	-1",
  "*speed:	0.0",
  "*size:	0.66",
  "*delay:	1000",
  "*colors:	-1",
  0
};

XrmOptionDescRec options [] = {
  { "-width",		".width",	XrmoptionSepArg, 0 },
  { "-height",		".height",	XrmoptionSepArg, 0 },
  { "-epoch",		".epoch",	XrmoptionSepArg, 0 },
  { "-palette",		".palette",	XrmoptionSepArg, 0 },
  { "-reaction",	".reaction",	XrmoptionSepArg, 0 },
  { "-diffusion",	".diffusion",	XrmoptionSepArg, 0 },
  { "-verbose",		".verbose",	XrmoptionSepArg, 0 },
  { "-radius",		".radius",	XrmoptionSepArg, 0 },
  { "-speed",		".speed",	XrmoptionSepArg, 0 },
  { "-size",		".size",	XrmoptionSepArg, 0 },
  { "-delay",		".delay",	XrmoptionSepArg, 0 },
  { "-ncolors",		".colors",	XrmoptionSepArg, 0 },
  { 0, 0, 0, 0 }
};

#define bps 16
#define mx ((1<<16)-1)

/* you can replace integer mults wish shift/adds with these,
   but it doesn't help on my 586 */
#define x5(n) ((n<<2)+n)
#define x7(n) ((n<<3)-n)

/* why strip bit? */
#define R (ya_random()&((1<<30)-1))

/* should factor into RD-specfic and compute-every-pixel general */
void
screenhack (Display *dpy, Window win)
{
  GC gc;
  XGCValues gcv;
  XWindowAttributes xgwa;
  Colormap cmap = 0;
  XImage *image;
  int width, height, radius;
  int array_width, array_height;
  double array_x, array_y;
  double array_dx, array_dy;
  int w2;
  int frame = 0, epoch_time;
  char *p;
  int vdepth, pdepth;
  ushort *r1, *r2, *r1b, *r2b;
  int npix;
  int reaction = 0;
  int diffusion = 0;
  int verbose;
  int mapped;
  int *m = 0;
#if dither_when_mapped
  unsigned char *mc = 0;
#endif
#ifdef HAVE_XSHM_EXTENSION
  int use_shm = 0;
  XShmSegmentInfo shm_info;
#endif
  int ncolors = 0;
  XColor *colors = 0;

  int delay = get_float_resource ("delay", "Integer");

  XGetWindowAttributes (dpy, win, &xgwa);
  width = get_integer_resource ("width", "Integer");
  height = get_integer_resource ("height", "Integer");
  {
    double s = get_float_resource ("size", "Float");
    double p = get_float_resource ("speed", "Float");
    if (s < 0.0 || s > 1.0)
      s = 1.0;
    s = sqrt(s);
    array_width = xgwa.width * s;
    array_height = xgwa.height * s;
    if (s < 0.99) {
      array_width = (array_width / width) * width;
      array_height = (array_height / height) * height;
    }
    if (array_width < width) array_width = width;
    if (array_height < height) array_height = height;
    array_x = (xgwa.width - array_width)/2;
    array_y = (xgwa.height - array_height)/2;
    array_dx = p;
    array_dy = .31415926 * p;
  }
  if (width < 10) width = 10;
  if (height < 10) height = 10;
  verbose = get_boolean_resource ("verbose", "Boolean");
  npix = (width + 2) * (height + 2);
  epoch_time = get_integer_resource ("epoch", "Integer");
  w2 = width + 2;
  gcv.function = GXcopy;
  gc = XCreateGC(dpy, win, GCFunction, &gcv);
  vdepth = visual_depth(DefaultScreenOfDisplay(dpy), xgwa.visual);

  /* This code only deals with pixmap depths of 1, 8, 16, and 32.
     Therefore, we assume that those depths will be supported by the
     coresponding visual depths (that depth-24 displays accept depth-32
     pixmaps, and that depth-12 displays accept depth-16 pixmaps.) */
  pdepth = (vdepth == 1 ? 1 :
	    vdepth <= 8 ? 8 :
	    vdepth <= 16 ? 16 :
	    32);

  cmap = xgwa.colormap;
  ncolors = get_integer_resource ("colors", "Integer");

  if (ncolors <= 0) {
    if (vdepth > 8)
      ncolors = 2047;
    else
      ncolors = 255;
  }

  if (mono_p || ncolors < 2) ncolors = 2;
  if (ncolors <= 2) mono_p = True;
  colors = (XColor *) malloc(sizeof(*colors) * (ncolors+1));

  mapped = (vdepth <= 8 &&
	    has_writable_cells(xgwa.screen, xgwa.visual));

  if (!mapped)
    m = (int *) malloc(sizeof(int) * (1<<16));
#if dither_when_mapped
  else {
    int i, di;
    mc = (unsigned char *) malloc(1<<16);
    for (i = 0; i < (1<<16); i++) {
      di = (i + (ya_random()&255))>>8;
      if (di > 255) di = 255;
      mc[i] = di;
    }
  }
#endif
  p = malloc(npix * (pdepth == 1 ? 1 : (pdepth / 8)));
  r1 = (ushort *) malloc(sizeof(ushort) * npix);
  r2 = (ushort *) malloc(sizeof(ushort) * npix);
  r1b = (ushort *) malloc(sizeof(ushort) * npix);
  r2b = (ushort *) malloc(sizeof(ushort) * npix);
  if (!p || !r1 || !r2 || !r1b || !r2b) {
    fprintf(stderr, "not enough memory for %d pixels.\n", npix);
    exit(1);
  }

#ifdef HAVE_XSHM_EXTENSION
  if (use_shm) {
    printf("p=%X\n", p);
    free(p);
    image = XShmCreateImage(dpy, xgwa.visual, vdepth,
			    ZPixmap, 0, &shm_info, width, height);
    shm_info.shmid = shmget(IPC_PRIVATE,
			    image->bytes_per_line * image->height,
			    IPC_CREAT | 0777);
    if (shm_info.shmid == -1)
      printf ("shmget failed!");
    shm_info.readOnly = False;
    p = shmat(shm_info.shmid, 0, 0);
    printf("p=%X %d\n", p, image->bytes_per_line);
    XShmAttach(dpy, &shm_info);
    XSync(dpy, False);
  } else
#endif
  image = XCreateImage(dpy, xgwa.visual, vdepth,
		       ZPixmap, 0, p,
		       width, height, 8, 0);

  while (1) {
    int i, j;
    ushort *t;
#if test_pattern_hyper
    if (frame&0x100)
      sleep(1);
#endif
    if (verbose) {
      double tm = 0;
      struct timeval tp;
      if (!(frame%100)) {
	double tm2;
#ifdef GETTIMEOFDAY_TWO_ARGS
        struct timezone tzp;
	gettimeofday(&tp, &tzp);
#else
        gettimeofday(&tp);
#endif
	tm2 = tp.tv_sec + tp.tv_usec * 1e-6;
	if (frame > 0)
	  printf("fps = %2.4g\n", 100.0 / (tm2 - tm));
	tm = tm2;
      }
    }
    if (!(frame%epoch_time)) {
      int s;
      if (0 != frame) {
	int t = epoch_time / 500;
	if (t > 15)
	  t = 15;
	sleep(t);
      }
	  
      for (i = 0; i < npix; i++) {
	/* equilibrium */
	r1[i] = 65500;
	r2[i] = 11;
      }

      memset(colors, 0, ncolors*sizeof(*colors));
      make_smooth_colormap (dpy, xgwa.visual, cmap, colors, &ncolors,
			    True, 0, True);
      if (ncolors <= 2) {
	mono_p = True;
	ncolors = 2;
	colors[0].flags = DoRed|DoGreen|DoBlue;
	colors[0].red = colors[0].green = colors[0].blue = 0;
	XAllocColor(dpy, cmap, &colors[0]);
	colors[1].flags = DoRed|DoGreen|DoBlue;
	colors[1].red = colors[1].green = colors[1].blue = 0xFFFF;
	XAllocColor(dpy, cmap, &colors[1]);
      }

      /* Scale it up so that there are exactly 255 colors -- that keeps the
	 animation speed consistent, even when there aren't many allocatable
	 colors, and prevents the -mono mode from looking like static. */
      if (ncolors != 255) {
	int i, n = 255;
	double scale = (double) ncolors / (double) (n+1);
	XColor *c2 = (XColor *) malloc(sizeof(*c2) * (n+1));
	for (i = 0; i < n; i++)
	  c2[i] = colors[(int) (i * scale)];
	free(colors);
	colors = c2;
	ncolors = n;
      }


      XSetWindowBackground(dpy, win, colors[255 % ncolors].pixel);
      XClearWindow(dpy, win);

      s = w2 * height/2 + width/2;
      radius = get_integer_resource ("radius", "Integer");
      if (radius < 0)
	radius = 1 + ((R%10) ? (R%5) : (R % (width/2-2)));
      for (i = -radius; i < (radius+1); i++)
	for (j = -radius; j < (radius+1); j++)
	  r2[s + i + j*w2] = mx - (R&63);
      reaction = get_integer_resource ("reaction", "Integer");
      if (reaction < 0 || reaction > 2) reaction = R&1;
      diffusion = get_integer_resource ("diffusion", "Integer");
      if (diffusion < 0 || diffusion > 2)
	diffusion = (R%5) ? ((R%3)?0:1) : 2;
      if (2 == reaction && 2 == diffusion)
	reaction = diffusion = 0;
      
/*      if (verbose)
	printf("reaction = %d\ndiffusion = %d\n"
	       "palette = %d\nradius = %d\n",
	       reaction, diffusion, palette, radius);
*/
    }
    for (i = 0; i <= width+1; i++) {
      r1[i] = r1[i + w2 * height];
      r2[i] = r2[i + w2 * height];
      r1[i + w2 * (height + 1)] = r1[i + w2];
      r2[i + w2 * (height + 1)] = r2[i + w2];
    }
    for (i = 0; i <= height+1; i++) {
      r1[w2 * i] = r1[width + w2 * i];
      r2[w2 * i] = r2[width + w2 * i];
      r1[w2 * i + width + 1] = r1[w2 * i + 1];
      r2[w2 * i + width + 1] = r2[w2 * i + 1];
    }
    for (i = 0; i < height; i++) {
      int ii = i + 1;
      char *q = p + width * i;
      short *qq = ((short *) p) + width * i;
      long  *qqq = ((long *) p) + width * i;
      ushort *i1 = r1 + 1 + w2 * ii;
      ushort *i2 = r2 + 1 + w2 * ii;
      ushort *o1 = r1b + 1 + w2 * ii;
      ushort *o2 = r2b + 1 + w2 * ii;
      for (j = 0; j < width; j++) {
#if test_pattern_hyper
	int r1 = (i * j + (frame&127)*frame)&65535;
#else
	int uvv, r1 = 0, r2 = 0;
	switch (diffusion) {
	case 0:
	  r1 = i1[j] + i1[j+1] + i1[j-1] + i1[j+w2] + i1[j-w2];
	  r1 = r1 / 5;
	  r2 = (i2[j]<<3) + i2[j+1] + i2[j-1] + i2[j+w2] + i2[j-w2];
	  r2 = r2 / 12;
	  break;
	case 1:
	  r1 = i1[j+1] + i1[j-1] + i1[j+w2] + i1[j-w2];
	  r1 = r1 >> 2;
	  r2 = (i2[j]<<2) + i2[j+1] + i2[j-1] + i2[j+w2] + i2[j-w2];
	  r2 = r2 >> 3;
	  break;
	case 2:
	  r1 = (i1[j]<<1) + (i1[j+1]<<1) + (i1[j-1]<<1) + i1[j+w2] + i1[j-w2];
	  r1 = r1 >> 3;
	  r2 = (i2[j]<<2) + i2[j+1] + i2[j-1] + i2[j+w2] + i2[j-w2];
	  r2 = r2 >> 3;
	  break;
	}
	uvv = (((r1 * r2) >> bps) * r2) >> bps;
	switch (reaction) {  /* costs 4% */
	case 0:
	  r1 += 4 * (((28 * (mx-r1)) >> 10) - uvv);
	  r2 += 4 * (uvv - ((80 * r2) >> 10));
	  break;
	case 1:
	  r1 += 3 * (((27 * (mx-r1)) >> 10) - uvv);
	  r2 += 3 * (uvv - ((80 * r2) >> 10));
	  break;
	case 2:
	  r1 += 2 * (((28 * (mx-r1)) >> 10) - uvv);
	  r2 += 3 * (uvv - ((80 * r2) >> 10));
	  break;
	}
	if (r1 > mx) r1 = mx;
	if (r2 > mx) r2 = mx;
	if (r1 < 0) r1 = 0;
	if (r2 < 0) r2 = 0;
	o1[j] = r1;
	o2[j] = r2;
#endif

	if (mapped)
#if dither_when_mapped
	  q[j] = colors[mc[r1]  % ncolors].pixel;
#else
	  q[j] = colors[(r1>>8) % ncolors].pixel;
#endif
	else if (pdepth == 8)
	  q[j] = colors[(r1>>8) % ncolors].pixel;
	else if (pdepth == 16)
	  qq[j] = colors[(r1>>8) % ncolors].pixel;
	else if (pdepth == 32)
	  qqq[j] = colors[(r1>>8) % ncolors].pixel;
	else
	  abort();
      }
    }
    t = r1; r1 = r1b; r1b = t;
    t = r2; r2 = r2b; r2b = t;
    for (i = 0; i < array_width; i += width)
      for (j = 0; j < array_height; j += height)
#ifdef HAVE_XSHM_EXTENSION
	if (use_shm)
	  XShmPutImage(dpy, win, gc, image, 0, 0, i, j,
		       width, height, False);
	else
#endif
	  XPutImage(dpy, win, gc, image, 0, 0, i+array_x, j+array_y, width, height);

    array_x += array_dx;
    array_y += array_dy;
    if (array_x < 0) {
      array_x = 0;
      array_dx = -array_dx;
    } else if (array_x > (xgwa.width - array_width)) {
      array_x = (xgwa.width - array_width);
      array_dx = -array_dx;
    }
    if (array_y < 0) {
      array_y = 0;
      array_dy = -array_dy;
    } else if (array_y > (xgwa.height - array_height)) {
      array_y = (xgwa.height - array_height);
      array_dy = -array_dy;
    }
    frame++;

    XSync(dpy, False);
    if (delay > 0)
      usleep(delay);
  }
}