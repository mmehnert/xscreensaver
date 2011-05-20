/* coral, by "Frederick G.M. Roeber" <roeber@netscape.com>, 15-jul-97.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include "screenhack.h"
#include "colors.h"

static GC draw_gc;
static unsigned int default_fg_pixel;
#define NCOLORSMAX 200
static XColor colors[NCOLORSMAX];
static int ncolors = 0;
static int colorindex = 0;
static int colorsloth;

static XPoint *walkers;
static int nwalkers;
static int width;
static int height;
static unsigned char *board;
#define dot(x, y) (board[(y*width)+x])

static void
init_coral(Display *dpy, Window window)
{
    XGCValues gcv;
    Colormap cmap;
    XWindowAttributes xgwa;
    Bool writeable = False;
    int seeds;
    int density;
    int i;

    srand(time(0));
    XClearWindow(dpy, window);
    XGetWindowAttributes(dpy, window, &xgwa);
    width = xgwa.width;
    height = xgwa.height;
    board = (unsigned char *)calloc(width*xgwa.height, sizeof(unsigned char));
    if( (unsigned char *)0 == board ) exit(1);
    cmap = xgwa.colormap;
    if( ncolors ) {
        free_colors(dpy, cmap, colors, ncolors);
        ncolors = 0;
    }
    gcv.foreground = default_fg_pixel = get_pixel_resource("foreground", "Foreground", dpy, cmap);
    draw_gc = XCreateGC(dpy, window, GCForeground, &gcv);
    ncolors = NCOLORSMAX;
    make_uniform_colormap(dpy, xgwa.visual, cmap, colors, &ncolors, True, &writeable, False);
    colorindex = rand()%ncolors;
    
    density = get_integer_resource("density", "Integer");
    if( density < 1 ) density = 1;
    if( density > 100 ) density = 90; /* more like mold than coral */
    nwalkers = (width*height*density)/100;
    walkers = (XPoint *)calloc(nwalkers, sizeof(XPoint));
    if( (XPoint *)0 == walkers ) exit(1);

    seeds = get_integer_resource("seeds", "Integer");
    if( seeds < 1 ) seeds = 1;
    if( seeds > 1000 ) seeds = 1000;

    colorsloth = nwalkers*2/ncolors;
    XSetForeground(dpy, draw_gc, colors[colorindex].pixel);

    for( i = 0; i < seeds; i++ ) {
        int x, y;
        do {
            x = rand() % width;
            y = rand() % height;
        } while( dot(x, y) );

        dot((x-1), (y-1)) = dot(x, (y-1)) = dot((x+1), (y-1)) =
        dot((x-1),  y   ) = dot(x,  y   ) = dot((x+1),  y   ) =
        dot((x-1), (y+1)) = dot(x, (y+1)) = dot((x+1), (y+1)) = 1;
        XDrawPoint(dpy, window, draw_gc, x, y);
    }

    for( i = 0; i < nwalkers; i++ ) {
        walkers[i].x = (rand() % (width-2)) + 1;
        walkers[i].y = (rand() % (height-2)) + 1;
    }
}

static void
coral(Display *dpy, Window window)
{
    int delay2 = get_integer_resource ("delay2", "Integer");
    while( 1 ) {
        int i;

        for( i = 0; i < nwalkers; i++ ) {
            int x = walkers[i].x;
            int y = walkers[i].y;

            if( dot(x, y) ) {
                XDrawPoint(dpy, window, draw_gc, x, y);
                /* Mark the surrounding area as "sticky" */
                dot((x-1), (y-1)) = dot(x, (y-1)) = dot((x+1), (y-1)) =
                    dot((x-1),  y   ) =                 dot((x+1),  y   ) =
                    dot((x-1), (y+1)) = dot(x, (y+1)) = dot((x+1), (y+1)) = 1;
                nwalkers--;
                walkers[i].x = walkers[nwalkers].x;
                walkers[i].y = walkers[nwalkers].y;
                if( 0 == (nwalkers%colorsloth) ) {
                    colorindex++;
                    if( colorindex == ncolors )
                        colorindex = 0;
                    XSetForeground(dpy, draw_gc, colors[colorindex].pixel);
                }

                if( 0 == nwalkers ) {
                    XSync(dpy, True);
                    return;
                }
            } else {
                /* move it a notch */
                do {
                    switch( rand() % 4 ) {
                    case 0:
                        if( 1 == x ) continue;
                        walkers[i].x--;
                        break;
                    case 1:
                        if( width-2 == x ) continue;
                        walkers[i].x++;
                        break;
                    case 2:
                        if( 1 == y ) continue;
                        walkers[i].y--;
                        break;
                    case 3:
                        if( height-2 == y ) continue;
                        walkers[i].y++;
                        break;
                    }
                } while(0);
            }
        }

        XSync(dpy, True);
	if (delay2 > 0)
	  usleep(delay2);
    }
}

char *progclass = "Coral";

char *defaults[] = {
    "Coral.background: black",
    "Coral.foreground: white",
    "*density: 25",
    "*seeds: 20", /* too many for 640x480, too few for 1280x1024 */
    "*delay: 5",
    "*delay2: 1000",
    0
};

XrmOptionDescRec options[] = {
    { "-density", ".density", XrmoptionSepArg, 0 },
    { "-seeds", ".seeds", XrmoptionSepArg, 0 },
    { "-delay", ".delay", XrmoptionSepArg, 0 },
    { "-delay2", ".delay2", XrmoptionSepArg, 0 },
    { 0, 0, 0, 0 }
};

void
screenhack(dpy, window)
Display *dpy;
Window window;
{
    int delay = get_integer_resource ("delay", "Integer");

    while( 1 ) {
        init_coral(dpy, window);
        coral(dpy, window);
        if( delay ) sleep(delay);
    }
}
