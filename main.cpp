#include "ring.hpp"
#include "serial.hpp"
#include "X11.hpp"
#include "clock.hpp"
#include <signal.h>

int fd;
Display* display;

void sighandler(int signum) {
   printf("Caught signal %d, cleaning up...\n", signum);
   XFlush(display);
   XCloseDisplay(display);
   close(fd);
   exit(1);
}

int main(int argc, char *argv[]) {

	string portname = "/dev/ttyUSB0";
	speed_t baudrate = B115200;
	unsigned int sample_num = 300;
	unsigned int sample_max = 0;
	int option;

	while((option = getopt(argc, argv, "p:b:x:y:")) != -1){ //get option from the getopt() method
	  switch(option){
		  case 'p':
				portname=optarg;
				break;
			case 'b':
				baudrate = str2baud(optarg);
				break;
	 		case 'x':
				sample_num = atoi(optarg);
				break;
			case 'y':
				sample_max = atoi(optarg);
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
	width = DisplayWidth(display, screen_num);
	height = DisplayHeight(display, screen_num);

	win = create_simple_window(display, width>>2, height>>2, 0, 0);
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


	Ring ring(sample_num);
	char buf[256];
  char token_remainder[10];
  XWindowAttributes winatt;
	start = system_clock::now(); 
	unsigned int frames=0;

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
			for (int j = 0; j < i; ++j) ring.insert(ints[j]);


			// Plot line
			XGetWindowAttributes(display, win, &winatt);
			int values[sample_num];
			ring.get_scaled_buffer(values, winatt.height, sample_max);
			// ring.memcpy(values);
			float x_interval = (float(winatt.width) / sample_num);
			float x_position = 0;
			// unsigned short y_center = winatt.height / 2;
			// unsigned short y_center = winatt.height / 4;
			unsigned short y_center = 0;
	    XPoint points[sample_num];

			for (int i = 0; i < sample_num; i++, x_position += x_interval)
				points[i] = { (unsigned short)(x_position), winatt.height - (values[i] + y_center)};
			XClearWindow(display, win);
	    XDrawLines(display, win, gc, points, sample_num, CoordModeOrigin /* CoordModeOrigin / CoordModePrevious */);

			end = system_clock::now(); 

			duration elapsed_seconds = end - start; 
			frames++;

			if( elapsed_seconds.count() >= 1){ //every second
          printf("FPS: %d\n", frames);
	        frames = 0;
					start = end;	        
	    }
	    usleep(100);                        

		}
		else if (rdlen < 0) printf("Error from read: %d: %s\n", rdlen, strerror(errno));
		else   /* rdlen == 0 */ printf("Timeout from read\n");

	} while (1);
}
