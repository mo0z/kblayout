/*
 * kblayout.c
 * Simple keyboard layout indicator
 *
 * Andrey Shashanov (2018)
 *
 * Compile with:
 * gcc -O3 -lX11 -s -lXft `pkg-config --cflags freetype2` kblayout.c -o kblayout
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xft/Xft.h>

#define POSITION_X 1500
#define POSITION_Y 0
#define BG_COLOR "#222222"
#define FG_COLOR "#bbbbbb"
#define FONT "monospace:size=10"
#define LANG_OUTPUT_LENGTH 2
#define WIDTH 30
#define HEIGHT 17
#define POS_X -1
#define POS_Y -1

int main(void)
{
    XSetWindowAttributes xwa;
    XEvent evnt;
    XftColor xftbgcolor, xftfgcolor;
    XftDraw *xftdraw;
    XkbDescPtr kb;
    XkbStateRec state;
    char *name;
    int pos_x, pos_y;
    int prevgroup = -1;

    Display *dpy = XOpenDisplay(NULL);
    int scr = DefaultScreen(dpy);
    Drawable rwin = RootWindow(dpy, scr);
    Visual *vsl = DefaultVisual(dpy, scr);
    Colormap cmap = DefaultColormap(dpy, scr);

    XftColorAllocName(dpy, vsl, cmap, BG_COLOR, &xftbgcolor);
    XftColorAllocName(dpy, vsl, cmap, FG_COLOR, &xftfgcolor);

    XftFont *xftfont = XftFontOpenName(dpy, scr, FONT);

    if (POS_X == -1)
        pos_x = WIDTH / 2 - xftfont->max_advance_width * LANG_OUTPUT_LENGTH / 2;
    else
        pos_x = POS_X;

    if (POS_Y == -1)
        pos_y = HEIGHT / 2 + xftfont->ascent / 2 - 1;
    else
        pos_y = POS_Y;

    xwa.override_redirect = 1;
    xwa.background_pixel = xftbgcolor.pixel;
    xwa.event_mask = ExposureMask | VisibilityChangeMask;
    Window win = XCreateWindow(dpy,
                               rwin,
                               POSITION_X, POSITION_Y,
                               WIDTH, HEIGHT,
                               0,
                               CopyFromParent,
                               InputOutput,
                               vsl,
                               CWBackPixel | CWOverrideRedirect | CWEventMask,
                               &xwa);
    XMapWindow(dpy, win);
    XFlush(dpy);

    XkbSelectEvents(dpy, XkbUseCoreKbd, XkbAllEventsMask, XkbAllEventsMask);

    while (1)
    {
        XkbGetState(dpy, XkbUseCoreKbd, &state);
        while (XPending(dpy))
        {
            XNextEvent(dpy, &evnt);
            if ((state.group != prevgroup) ||
                (evnt.type == Expose) ||
                (evnt.type == VisibilityNotify))
            {
                kb = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);
                name = XGetAtomName(dpy, kb->names->groups[state.group]);
                for (int i = 0; i < LANG_OUTPUT_LENGTH; i++)
                {
                    name[i] = (char)toupper(name[i]);
                }

                xftdraw = XftDrawCreate(dpy, win, vsl, cmap);
                XClearWindow(dpy, win);
                XftDrawString8(xftdraw, &xftfgcolor, xftfont, pos_x, pos_y,
                               name, LANG_OUTPUT_LENGTH);
                XftDrawDestroy(xftdraw);
                XRaiseWindow(dpy, win);
                XFlush(dpy);

                prevgroup = state.group;
                XFree(name);
                XkbFreeKeyboard(kb, XkbAllComponentsMask, 1);
            }
        }
        XNextEvent(dpy, &evnt);
    }

    XDestroyWindow(dpy, win);
    XftColorFree(dpy, vsl, cmap, &xftfgcolor);
    XftColorFree(dpy, vsl, cmap, &xftbgcolor);
    XCloseDisplay(dpy);
    exit(EXIT_SUCCESS);
}
