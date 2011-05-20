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
 * See xlockmore.h and xlockmore.c.
 */

#ifndef __XLOCKMORE_INTERNAL_H__
#define __XLOCKMORE_INTERNAL_H__

#include "screenhack.h"


typedef struct ModeInfo {
  Display *dpy;
  Window window;
  int npixels;
  unsigned long *pixels;
  XColor *colors;
  Bool writable_p;
  unsigned long white;
  unsigned long black;
  XWindowAttributes xgwa;
  GC gc;
  long pause;
  Bool fullrandom;
  long cycles;
  long batchcount;
  long size;
  Bool threed;
  long threed_left_color;
  long threed_right_color;
  long threed_both_color;
  long threed_none_color;
  long threed_delta;
} ModeInfo;

extern void xlockmore_screenhack P((Display *dpy, Window window,
				    Bool want_writable_colors,
				    Bool want_uniform_colors,
				    Bool want_smooth_colors,
				    Bool want_bright_colors,
				    void (*hack_init) (ModeInfo *),
				    void (*hack_draw) (ModeInfo *),
				    void (*hack_free) (ModeInfo *)));

#endif /* __XLOCKMORE_INTERNAL_H__ */
