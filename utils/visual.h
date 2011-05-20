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

extern Visual *get_visual P((Screen *, const char *name, Bool, Bool));
extern Visual *get_visual_resource P((Screen *, char *, char *, Bool));
extern int visual_depth P((Screen *, Visual *));
extern int visual_class P((Screen *, Visual *));
extern int visual_cells P((Screen *, Visual *));
extern int screen_number P((Screen *));
extern Visual *find_similar_visual P((Screen *, Visual *old));
extern void describe_visual P((FILE *f, Screen *, Visual *));
extern Visual *get_overlay_visual P((Screen *, unsigned long *pixel_return));

#endif /* __VISUAL_H__ */
