#include <X11/Xlib.h>
//https://www.x.org/releases/X11R7.7/doc/libX11/libX11/libX11.html
//https://github.com/QMonkey/Xlib-demo/blob/master/src/double_win.c has button press and event wait

#include <stdio.h>
#include <stdlib.h>		/* getenv(), etc. */
#include <unistd.h>		/* sleep(), etc.  */


Window create_simple_window(Display* display, int width, int height, int x, int y) {
  int screen_num = DefaultScreen(display);
  Window win;
  win = XCreateSimpleWindow(
    display,
    RootWindow(display, screen_num),
    x, y,
    width, height,
    0, // border width
    BlackPixel(display, screen_num),
    WhitePixel(display, screen_num)
  );
  XMapWindow(display, win); /* make the window actually appear on the screen. */
  XFlush(display); /* flush all pending requests to the X server. */
  return win;
}


GC create_gc(Display* display, Window win, int reverse_video) {
  GC gc;
  XGCValues values;
  int screen_num = DefaultScreen(display);
  gc = XCreateGC(display, win, 0 /*mask*/, &values);
  if (gc < 0) fprintf(stderr, "XCreateGC: \n");
  XSetForeground(display, gc, BlackPixel(display, screen_num));
  XSetBackground(display, gc, WhitePixel(display, screen_num));
  XSetLineAttributes(
    display,
    gc,
    2, // line width
    LineSolid, // line style -- LineSolid, LineOnOffDash, LineDoubleDash
    CapButt, // cap style -- CapNotLast, CapButt, CapRound, CapProjecting
    JoinBevel // join style --  JoinMiter, JoinRound, or JoinBevel --- http://developer.classpath.org/doc/java/awt/doc-files/capjoin.png
  );
  XSetFillStyle(display, gc, FillSolid); // FillSolid, FillTiled, FillStippled, or FillOpaqueStippled.
  return gc;
}