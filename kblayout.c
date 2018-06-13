/*
 * kblayout.c
 * Simple keyboard layout indicator
 *
 * Andrey Shashanov (2018)
 *
 * Build:
 * gcc -Wall -pedantic -std=c99 -O3 -s -lX11 -lXft `pkg-config --cflags freetype2` kblayout.c -o kblayout
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xft/Xft.h>

#define POSITION_X 600
#define POSITION_Y 0
#define BG_COLOR "#222222"
#define FG_COLOR "#bbbbbb"
#define FONT "monospace:size=10"
#define LANG_OUTPUT_LENGTH 2
#define WIDTH 30
#define HEIGHT 17
#define POS_X -1
#define POS_Y -1

static char *getprogname_of_argv(char *argv_zero_tzs);

/* get current process name without path */
char *getprogname_of_argv(char *argv_zero_tzs)
{
    char *p;
    if (argv_zero_tzs == NULL)
        return NULL;
    for (p = argv_zero_tzs; *argv_zero_tzs; ++argv_zero_tzs)
        if (*argv_zero_tzs == '/')
            p = argv_zero_tzs + 1;
    return p;
}

int main(int argc __attribute__((unused)), char *argv[])
{
    Display *dpy;
    int scr;
    Drawable rwin;
    Visual *vsl;
    Colormap cmap;
    XftFont *xftfont;
    Window win;
    XSetWindowAttributes xwa;
    XftColor xftbgcolor, xftfgcolor;
    ssize_t pos_x, pos_y;
    ssize_t prevgroup = -1;

    if ((dpy = XOpenDisplay(NULL)) == NULL)
    {
        fprintf(stderr, "%s: Error open DISPLAY\n",
                getprogname_of_argv(argv[0]));
        return EXIT_FAILURE;
    }

    scr = DefaultScreen(dpy);
    rwin = RootWindow(dpy, scr);
    vsl = DefaultVisual(dpy, scr);
    cmap = DefaultColormap(dpy, scr);

    XftColorAllocName(dpy, vsl, cmap, BG_COLOR, &xftbgcolor);
    XftColorAllocName(dpy, vsl, cmap, FG_COLOR, &xftfgcolor);

    xftfont = XftFontOpenName(dpy, scr, FONT);

    if (POS_X == -1)
        pos_x = WIDTH / 2 - xftfont->max_advance_width * LANG_OUTPUT_LENGTH / 2;
    else
        pos_x = POS_X;

    if (POS_Y == -1)
        pos_y = HEIGHT / 2 + xftfont->ascent / 2 - 1;
    else
        pos_y = POS_Y;

    xwa.background_pixel = xftbgcolor.pixel;
    xwa.override_redirect = 1;
    xwa.event_mask = ExposureMask | VisibilityChangeMask;
    win = XCreateWindow(dpy,
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
        XkbStateRec state;
        XEvent evnt;

        XkbGetState(dpy, XkbUseCoreKbd, &state);
        while (XPending(dpy))
        {
            XNextEvent(dpy, &evnt);
            if (state.group != prevgroup ||
                evnt.type == Expose ||
                evnt.type == VisibilityNotify)
            {
                XkbDescPtr kb;
                char *name;
                XftDraw *xftdraw;
                size_t i;

                kb = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);
                name = XGetAtomName(dpy, kb->names->groups[state.group]);

                for (i = LANG_OUTPUT_LENGTH; i--;)
                    name[i] = (char)toupper(name[i]);

                xftdraw = XftDrawCreate(dpy, win, vsl, cmap);
                XClearWindow(dpy, win);
                XftDrawString8(xftdraw, &xftfgcolor, xftfont, pos_x, pos_y,
                               (const FcChar8 *)name, LANG_OUTPUT_LENGTH);
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
}
