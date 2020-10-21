#include <X11/Xlib.h>

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
    1, // line width
    LineSolid, // line style -- LineSolid, LineOnOffDash, LineDoubleDash
    CapButt, // cap style -- CapNotLast, CapButt, CapRound, CapProjecting
    JoinBevel // join style --  JoinMiter, JoinRound, or JoinBevel --- http://developer.classpath.org/doc/java/awt/doc-files/capjoin.png
  );
  XSetFillStyle(display, gc, FillSolid); // FillSolid, FillTiled, FillStippled, or FillOpaqueStippled.
  return gc;
}

int main(int argc, char* argv[]) {
  Display* display;
  int screen_num;		/* number of screen to place the window on.  */
  Window win;			/* pointer to the newly created window.      */
  unsigned int display_width, display_height;
  unsigned int width, height;
  char *display_name = getenv("DISPLAY");
  GC gc;			
  Colormap screen_colormap;     /* color map to use for allocating colors.   */
  XColor red, brown, blue, yellow, green;
  Status rc;			/* return status of various Xlib functions.  */

  /* open connection with the X server. */
  display = XOpenDisplay(display_name);
  if (display == NULL) {
    fprintf(stderr, "%s: cannot connect to X server '%s'\n", argv[0], display_name);
    exit(1);
  }

  /* get the geometry of the default screen for our display. */
  screen_num = DefaultScreen(display);
  display_width = DisplayWidth(display, screen_num);
  display_height = DisplayHeight(display, screen_num);

  /* make the new window occupy 1/9 of the screen's size. */
  width = (display_width / 3);
  height = (display_height / 3);

  win = create_simple_window(display, width, height, 0, 0);
  gc = create_gc(display, win, 0);
  XSync(display, False);

  screen_colormap = DefaultColormap(display, DefaultScreen(display));
  bool failure = false;
  if (!XAllocNamedColor(display, screen_colormap, "red",    &red,    &red))    failure = true;
  if (!XAllocNamedColor(display, screen_colormap, "brown",  &brown,  &brown))  failure = true;
  if (!XAllocNamedColor(display, screen_colormap, "blue",   &blue,   &blue))   failure = true;
  if (!XAllocNamedColor(display, screen_colormap, "yellow", &yellow, &yellow)) failure = true;
  if (!XAllocNamedColor(display, screen_colormap, "green",  &green,  &green))  failure = true;
  if (failure) fprintf(stderr, "XAllocNamedColor - failed to allocated color.\n");

  while (1) {
    XSetForeground(display, gc, red.pixel);
    XDrawPoint(display, win, gc, 5, 5);
    XDrawPoint(display, win, gc, 5, height-5);
    XDrawPoint(display, win, gc, width-5, 5);
    XDrawPoint(display, win, gc, width-5, height-5);
    XSetForeground(display, gc,  BlackPixel(display, screen_num));

    XPoint points[] = {
      {rand() % width, rand() % height},
      {rand() % width, rand() % height},
      {rand() % width, rand() % height},
      {rand() % width, rand() % height}
      // {0, 0},
      // {15, 15},
      // {0, 15},
      // {0, 0}
    };
    int npoints = sizeof(points)/sizeof(XPoint);
    XSetForeground(display, gc, blue.pixel);
    XDrawLines(display, win, gc, points, npoints, CoordModeOrigin /* CoordModeOrigin / CoordModePrevious */);

    XFlush(display);
    sleep(1);
  }

  /* close the connection to the X server. */
  XCloseDisplay(display);
  return 0;
}
