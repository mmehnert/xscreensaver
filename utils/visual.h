/* xscreensaver, Copyright (c) 1997 by Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#ifndef __VISUAL_H__
#define __VISUAL_H__

extern Visual *get_visual_resource P((Display *, char *, char *, Bool));
extern int visual_depth P((Display *, Visual *));
extern int visual_class P((Display *dpy, Visual *visual));
extern int visual_cells P((Display *dpy, Visual *visual));
extern void describe_visual P((FILE *f, Display *dpy, Visual *visual));

#endif /* __VISUAL_H__ */
