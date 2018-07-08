/*
Simple keyboard layout indicator
Andrey Shashanov (2018)

gcc -O3 -s -lX11 -lXft `pkg-config --cflags freetype2` -o kblayout kblayout.c
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
#define KBLAYOUT_NUM_CHARS 2
#define WIDTH 30U
#define HEIGHT 17U
#define POS_X -1 /* -1 on the center */
#define POS_Y -1 /* -1 on the center */

static char *program_name(char *s);

int main(int argc __attribute__((unused)), char *argv[])
{
    Display *dpy;
    int scr;
    Drawable rwin;
    Visual *vsl;
    Colormap cmap;
    XftFont *xftfont;
    XSetWindowAttributes xwa;
    Window win;
    XftColor xftbgcolor, xftfgcolor;
    ssize_t pos_x, pos_y;
    ssize_t prevgroup = -1;

    if ((dpy = XOpenDisplay(NULL)) == NULL)
    {
        fprintf(stderr, "%s: Error open DISPLAY\n", program_name(argv[0]));
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
        pos_x = WIDTH / 2 - xftfont->max_advance_width * KBLAYOUT_NUM_CHARS / 2;
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

    for (;;)
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

                i = KBLAYOUT_NUM_CHARS;
                do
                {
                    --i;
                    name[i] = (char)toupper(name[i]);
                } while (i);

                xftdraw = XftDrawCreate(dpy, win, vsl, cmap);
                XClearWindow(dpy, win);
                XftDrawString8(xftdraw, &xftfgcolor, xftfont, pos_x, pos_y,
                               (const FcChar8 *)name, KBLAYOUT_NUM_CHARS);
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

    /* code will never be executed
    XDestroyWindow(dpy, win);
    XftColorFree(dpy, vsl, cmap, &xftfgcolor);
    XftColorFree(dpy, vsl, cmap, &xftbgcolor);
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
    */
}

/* get short program name */
char *program_name(char *s)
{
    char *p;
    if (s == NULL)
        return NULL;
    for (p = s; *s; ++s)
        if (*s == '/')
            p = s + 1U;
    return p;
}
