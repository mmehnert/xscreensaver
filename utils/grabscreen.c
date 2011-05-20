/* xscreensaver, Copyright (c) 1992, 1993, 1994, 1997
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

/* This file contains code for grabbing an image of the screen to hack its
   bits.  This is a little tricky, since doing this involves the need to tell
   the difference between drawing on the actual root window, and on the fake
   root window used by the screensaver, since at this level the illusion 
   breaks down...
 */

#ifdef __STDC__
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#undef usleep
# define usleep screenhack_usleep

#ifdef __STDC__
extern void screenhack_usleep (unsigned long usecs);
#endif

#ifdef HAVE_READ_DISPLAY_EXTENSION
# include <X11/extensions/readdisplay.h>
# ifdef __STDC__
  static Bool read_display (Display *, Window, Pixmap, Bool);
# else /* !__STDC__ */
  static Bool read_display ();
# endif /* !__STDC__ */
#endif /* HAVE_READ_DISPLAY_EXTENSION */


static Bool
#ifdef __STDC__
MapNotify_event_p (Display *dpy, XEvent *event, XPointer window)
#else /* !__STDC__ */
MapNotify_event_p (dpy, event, window)
     Display *dpy;
     XEvent *event;
     XPointer window;
#endif /* !__STDC__ */
{
  return (event->xany.type == MapNotify &&
	  event->xvisibility.window == (Window) window);
}

static void
#ifdef __STDC__
raise_window(Display *dpy, Window window, Bool dont_wait)
#else  /* !__STDC__ */
raise_window(dpy, window, dont_wait)
  Display *dpy;
  Window window;
  Bool dont_wait;
#endif /* !__STDC__ */
{
  if (! dont_wait)
    {
      XWindowAttributes xgwa;
      XSizeHints hints;
      long supplied = 0;
      memset(&hints, 0, sizeof(hints));
      XGetWMNormalHints(dpy, window, &hints, &supplied);
      XGetWindowAttributes (dpy, window, &xgwa);
      hints.x = xgwa.x;
      hints.y = xgwa.y;
      hints.width = xgwa.width;
      hints.height = xgwa.height;
      hints.flags |= (PPosition|USPosition|PSize|USSize);
      XSetWMNormalHints(dpy, window, &hints);
    }

  XMapRaised(dpy, window);

  if (! dont_wait)
    {
      XEvent event;
      XIfEvent (dpy, &event, MapNotify_event_p, (XPointer) window);
      XSync (dpy, True);
    }
}



#ifdef __STDC__
static Bool screensaver_window_p (Display *, Window);
#endif

static Bool
screensaver_window_p (dpy, window)
     Display *dpy;
     Window window;
{
  Atom type;
  int format;
  unsigned long nitems, bytesafter;
  char *version;
  if (XGetWindowProperty (dpy, window,
			  XInternAtom (dpy, "_SCREENSAVER_VERSION", False),
			  0, 1, False, XA_STRING,
			  &type, &format, &nitems, &bytesafter,
			  (unsigned char **) &version)
      == Success
      && type != None)
    return True;
  return False;
}

Pixmap
#ifdef __STDC__
grab_screen_image (Display *dpy, Window window)
#else /* !__STDC__ */
grab_screen_image (dpy, window)
     Display *dpy;
     Window window;
#endif /* !__STDC__ */
{
  /* note: this assumes vroot.h didn't encapsulate the XRootWindowOfScreen
     function, only the RootWindowOfScreen macro... */
  Window real_root = XRootWindowOfScreen (DefaultScreenOfDisplay (dpy));
  Bool root_p = (window == real_root);
  Bool saver_p = screensaver_window_p (dpy, window);
  Bool grab_mouse_p = False;
  int unmap_time = 0;
  Pixmap pixmap = 0;

  if (saver_p)
    /* I think this is redundant, but just to be safe... */
    root_p = False;

  if (saver_p)
    /* The only time grabbing the mouse is important is if this program
       is being run while the screen is locked. */
    grab_mouse_p = True;

  if (!root_p)
    {
      if (saver_p)
	unmap_time = 5000000;  /* 5 seconds */
      else
	unmap_time =  660000;  /* 2/3rd second */
    }

  if (!root_p)
    XSetWindowBackgroundPixmap (dpy, window, None);

  if (grab_mouse_p)
    {
      /* prevent random viewer of the screen saver (locker) from messing
	 with windows.   We don't check whether it succeeded, because what
	 are our options, really... */
      XGrabPointer (dpy, real_root, True, ButtonPressMask|ButtonReleaseMask,
		    GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
      XGrabKeyboard (dpy, real_root, True, GrabModeSync, GrabModeAsync,
		     CurrentTime);
    }

  if (unmap_time > 0)
    {
      XUnmapWindow (dpy, window);
      XSync (dpy, True);
      usleep(unmap_time); /* wait for everyone to swap in and handle exposes */
    }

  if (!root_p)
    {
#ifdef HAVE_READ_DISPLAY_EXTENSION
      if (! read_display(dpy, window, 0, saver_p))
#endif /* HAVE_READ_DISPLAY_EXTENSION */
	raise_window(dpy, window, saver_p);
    }
  else  /* root_p */
    {
      XWindowAttributes xgwa;
      XGetWindowAttributes(dpy, window, &xgwa);
      pixmap = XCreatePixmap(dpy, window, xgwa.width, xgwa.height, xgwa.depth);
#ifdef HAVE_READ_DISPLAY_EXTENSION
      if (! read_display(dpy, window, pixmap, True))
#endif
	{
	  XGCValues gcv;
	  GC gc;
	  gcv.function = GXcopy;
	  gcv.subwindow_mode = IncludeInferiors;
	  gc = XCreateGC (dpy, window, GCFunction | GCSubwindowMode, &gcv);
	  XCopyArea (dpy, RootWindowOfScreen (xgwa.screen), pixmap, gc,
		     xgwa.x, xgwa.y, xgwa.width, xgwa.height, 0, 0);
	  XFreeGC (dpy, gc);
	}
      XSetWindowBackgroundPixmap (dpy, window, pixmap);
    }

  if (grab_mouse_p)
    {
      XUngrabPointer (dpy, CurrentTime);
      XUngrabKeyboard (dpy, CurrentTime);
    }

  XSync (dpy, True);
  return pixmap;
}


/* When we are grabbing and manipulating a screen image, it's important that
   we use the same colormap it originally had.  So, if the screensaver was
   started with -install, we need to copy the contents of the default colormap
   into the screensaver's colormap.
 */
void
#ifdef __STDC__
copy_default_colormap_contents (Display *dpy,
				Colormap to_cmap,
				Visual *to_visual)
#else /* !__STDC__ */
copy_default_colormap_contents (dpy, to_cmap, to_visual)
     Display *dpy;
     Colormap to_cmap;
     Visual *to_visual;
#endif /* !__STDC__ */
{
  Screen *screen = DefaultScreenOfDisplay (dpy);
  Visual *from_visual = DefaultVisualOfScreen (screen);
  Colormap from_cmap = XDefaultColormapOfScreen (screen);

  XColor *old_colors, *new_colors;
  unsigned long *pixels;
  XVisualInfo vi_in, *vi_out;
  int out_count;
  int from_cells, to_cells, max_cells;
  int requested;
  int i;

  if (from_cmap == to_cmap)
    return;

  vi_in.screen = XScreenNumberOfScreen (screen);
  vi_in.visualid = XVisualIDFromVisual (from_visual);
  vi_out = XGetVisualInfo (dpy, VisualScreenMask|VisualIDMask,
			   &vi_in, &out_count);
  if (! vi_out) abort ();
  from_cells = vi_out [0].colormap_size;
  XFree ((char *) vi_out);

  vi_in.screen = XScreenNumberOfScreen (screen);
  vi_in.visualid = XVisualIDFromVisual (to_visual);
  vi_out = XGetVisualInfo (dpy, VisualScreenMask|VisualIDMask,
			   &vi_in, &out_count);
  if (! vi_out) abort ();
  to_cells = vi_out [0].colormap_size;
  XFree ((char *) vi_out);

  max_cells = (from_cells > to_cells ? to_cells : from_cells);

  old_colors = (XColor *) calloc (sizeof (XColor), max_cells);
  new_colors = (XColor *) calloc (sizeof (XColor), max_cells);
  pixels = (unsigned long *) calloc (sizeof (unsigned long), max_cells);
  for (i = 0; i < max_cells; i++)
    old_colors[i].pixel = i;
  XQueryColors (dpy, from_cmap, old_colors, max_cells);

  requested = max_cells;
  while (requested > 0)
    {
      if (XAllocColorCells (dpy, to_cmap, False, 0, 0, pixels, requested))
	{
	  /* Got all the pixels we asked for. */
	  for (i = 0; i < requested; i++)
	    new_colors[i] = old_colors [pixels[i]];
	  XStoreColors (dpy, to_cmap, new_colors, requested);
	}
      else
	{
	  /* We didn't get all/any of the pixels we asked for.  This time, ask
	     for half as many.  (If we do get all that we ask for, we ask for
	     the same number again next time, so we only do O(log(n)) server
	     roundtrips.) */
	  requested = requested / 2;
	}
    }

  free (old_colors);
  free (new_colors);
  free (pixels);
}



/* The SGI ReadDisplay extension.
   This extension lets you get back a 24-bit image of the screen, taking into
   account the colors with which all windows are *currently* displayed, even
   if those windows have different visuals.  Without this extension, presence
   of windows with different visuals or colormaps will result in technicolor
   when one tries to grab the screen image.
 */

#ifdef HAVE_READ_DISPLAY_EXTENSION

static Bool
#ifdef __STDC__
read_display (Display *dpy, Window window, Pixmap into_pixmap, Bool dont_wait)
#else /* !__STDC__ */
read_display (dpy, window, into_pixmap, dont_wait)
  Display *dpy;
  Window window;
  Pixmap into_pixmap;
  Bool dont_wait;
#endif /* !__STDC__ */
{
  XWindowAttributes xgwa;
  int rd_event_base = 0;
  int rd_error_base = 0;
  unsigned long hints = 0;
  XImage *image = 0;
  XGCValues gcv;
  GC gc;

  /* Check to see if the window is >= 24 bits deep; if not, we can't make use
     of the pixmap returned by XReadDisplay anyway.
   */
  XGetWindowAttributes (dpy, window, &xgwa);
  if (xgwa.depth < 24)
    return False;

  /* Check to see if the server supports the extension, and bug out if not.
   */
  if (! XReadDisplayQueryExtension (dpy, &rd_event_base, &rd_error_base))
    return False;

  /* Finally, try and read the screen.
   */
  hints = (XRD_TRANSPARENT | XRD_READ_POINTER);
  image = XReadDisplay (dpy, window, xgwa.x, xgwa.y, xgwa.width, xgwa.height,
			hints, &hints);
  if (!image)
    return False;
  if (!image->data)
    {
      XDestroyImage(image);
      return False;
    }

  /* Uh, this can't be right, can it?  But it's necessary.  X sucks. */
  if (image->depth == 32)
    image->depth = xgwa.depth;

  gcv.function = GXcopy;
  gc = XCreateGC (dpy, window, GCFunction, &gcv);

  if (into_pixmap)
    {
      gcv.function = GXcopy;
      gc = XCreateGC (dpy, into_pixmap, GCFunction, &gcv);
      XPutImage (dpy, into_pixmap, gc, image, 0, 0, 0, 0,
		 xgwa.width, xgwa.height);
    }
  else
    {
      gcv.function = GXcopy;
      gc = XCreateGC (dpy, window, GCFunction, &gcv);

      /* Ok, now we'll be needing that window on the screen... */
      raise_window(dpy, window, dont_wait);

      /* Plop down the bits... */
      XPutImage (dpy, window, gc, image, 0, 0, 0, 0, xgwa.width, xgwa.height);
    }

  XFreeGC (dpy, gc);

  if (image->data)
    {
      free(image->data);
      image->data = 0;
    }
  XDestroyImage(image);
  return True;
}

#endif /* HAVE_READ_DISPLAY_EXTENSION */
