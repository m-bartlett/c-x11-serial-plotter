#include "ring.hpp"
#include "serial.hpp"
#include "X11.hpp"
#include <signal.h>

int fd;
Display* display;

void sighandler(int signum) {
   printf("Caught signal %d, cleaning up...\n", signum);
   close(fd);
   XFlush(display);
   XCloseDisplay(display);
   exit(1);
}

int main(int argc, char *argv[]) {

	string portname = "/dev/ttyUSB0";
	speed_t baudrate = B115200;
	int option;

	while((option = getopt(argc, argv, "p:b:")) != -1){ //get option from the getopt() method
	   switch(option){
		  case 'p':
			 portname=optarg;
			 break;
		 case 'b':
			 baudrate = str2baud(optarg);
			 break;
		  case ':':
			 printf("option needs a value\n");
			 break;
		  case '?': //used for some unknown options
			 printf("unknown option: %c\n", optopt);
			 break;
	   }
	}
	for(; optind < argc; optind++){ printf("Given extra arguments: %s\n", argv[optind]); }

	printf("Using port: %s\n", portname.c_str());

	fd = open(portname.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	signal(SIGINT, sighandler);

	if (fd < 0) {printf("Error opening %s: %s\n", portname, strerror(errno)); return -1; }
	set_interface_attribs(fd, baudrate);
	tcdrain(fd);    /* delay for output */


	///////////////////////////////////////////////////////////////////////// X11 init


	int screen_num;		/* number of screen to place the window on.  */
	Window win;			/* pointer to the newly created window.      */
	unsigned int display_width, display_height, width, height;				
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

  XSetForeground(display, gc,  BlackPixel(display, screen_num));
  XSetBackground(display, gc,  WhitePixel(display, screen_num));

	/////////////////////////////////////////////////////////////////////////


	Ring ring(1024);
	char buf[256];
  char token_remainder[10];
  XWindowAttributes winatt;

	do {
		uint8_t rdlen = read(fd, buf, sizeof(buf) - 1);
		if (rdlen > 0) {
			char tmp[rdlen + sizeof(token_remainder)]="";
			buf[rdlen] = 0; // terminate at end of relevant bytes for strcat, else copy garbage
			strcat(tmp, token_remainder);
			memset(token_remainder,0, sizeof(token_remainder));
			strcat(tmp, buf);

			char* token; 
			char* rest = tmp; 

			int ints[20]={0}; int i=0;
			while ( token = strtok_r(rest, "\r\n", &rest) ) {
				strcpy(token_remainder, token);
				ints[i] = atoi(token);
				i++;
			}
			if (buf[rdlen-1] != '\n') i--; // TO-DO ring.pop(), remove ints[]
			else memset(token_remainder,0, sizeof(token_remainder));
			for (int j = 0; j < i; ++j) ring.insert(abs(ints[j]));



			// Plot line
			XGetWindowAttributes(display, win, &winatt);
			int values[ring.size];
			// ring.memcpy(values);
			ring.get_normalized_buffer(values, winatt.height);
			int tick_width = (winatt.width / ring.size);
			tick_width = tick_width ? tick_width : 1;
			// int tick_width = 20;
			int tick = 0;
	    XPoint points[ring.size]; // TO-DO: replace ring.size with cli arg

			for (int i = 0; i < ring.size; i++, tick += tick_width) points[i] = { tick, values[i] };
			XClearWindow(display, win);
	    XDrawLines(display, win, gc, points, ring.size, CoordModeOrigin /* CoordModeOrigin / CoordModePrevious */);

			// Clear screen
			// XFillRectangle(display, win, gc, 0, 0, width, height);
		}
		else if (rdlen < 0) printf("Error from read: %d: %s\n", rdlen, strerror(errno));
		else   /* rdlen == 0 */ printf("Timeout from read\n");

	} while (1);
}
