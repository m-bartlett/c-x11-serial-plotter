#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <string>
#include <vector>
#include <map>


int fd;


void sighandler(int signum) {
   printf("Caught signal %d, coming out...\n", signum);
   close(fd);
   exit(1);
}


using string = std::string;
using vec = std::vector<int>;


speed_t str2baud(string baudstring) {
	std::map<string, speed_t> baudmap = {
		{"115200", B115200} , {"230400", B230400}, {"57600", B57600}, {"38400", B38400}, {"19200", B19200}, {"9600", B9600}, {"4800", B4800}, {"2400", B2400}, {"1800", B1800}, {"1200", B1200}, {"600", B600}, {"300", B300}, {"200", B200}, {"150", B150}, {"134", B134}, {"110", B110}, {"75", B75}, {"50", B50}
	};
	printf("Map: %d\n", baudmap[baudstring]);
	return baudmap[baudstring];
}


int set_interface_attribs(int fd, int speed) {
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error from tcgetattr: %s\n", strerror(errno));
		return -1;
	}

	cfsetospeed(&tty, (speed_t)speed);
	cfsetispeed(&tty, (speed_t)speed);

	tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;         /* 8-bit characters */
	tty.c_cflag &= ~PARENB;     /* no parity bit */
	tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
	tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

	/* fetch bytes as they become available */
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		printf("Error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

void set_mincount(int fd, int mcount) {
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error tcgetattr: %s\n", strerror(errno));
		return;
	}

	tty.c_cc[VMIN] = mcount ? 1 : 0;
	tty.c_cc[VTIME] = 5;        /* half second timer */

	if (tcsetattr(fd, TCSANOW, &tty) < 0)
		printf("Error tcsetattr: %s\n", strerror(errno));
}


#define DEFAULT_RING_SIZE 40
struct Ring {
	int *data;
	size_t size;
	size_t rindex=0;
	int mindex=0;
	int maxdex=0;

	Ring() { data = new int[DEFAULT_RING_SIZE]; this->size = DEFAULT_RING_SIZE; };
	Ring(int sz) { data = new int[sz]; this->size = sz; };
	~Ring() { delete []data; }
	int data_min() { return data[mindex]; }
	int data_max() { return data[maxdex]; }

	void insert(int v) {
		this->data[rindex] = v;
		if (rindex == mindex) {
			int min = 0xFFFFFFFF;
			for (int i=0; i < this->size; i++) {
				if (min > this->data[i]) {
					mindex = i;
					min = this->data[i];
				}
			}
		}
		if (rindex == maxdex) {
			int max = 0;
			for (int i=0; i < this->size; i++) {
				if (max < this->data[i]) {
					maxdex = i;
					max = this->data[i];
				}
			}
		}
		if (v < this->data[mindex]) mindex = rindex;
		else if (v > this->data[maxdex]) maxdex = rindex;
		rindex = (++rindex) >= this->size ? 0 : rindex;
	}

	void memcpy(int *buff) { // Assumes buff is the same size as ring.size
		for (int i=this->rindex; i < this->size; i++) buff[i] = this->data[i];
		for (int i=0; i < this->rindex; i++) buff[i] = this->data[i];
	}

	void zero() { memset(this->data, 0, this->size); }

};


int main(int argc, char *argv[]) {

	string portname = "/dev/ttyUSB0";
	speed_t baudrate = B115200;
	int wlen;

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
	for(; optind < argc; optind++){ //when some extra arguments are passed
	   printf("Given extra arguments: %s\n", argv[optind]);
	}

	printf("Using port: %s\n", portname.c_str());


	fd = open(portname.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	signal(SIGINT, sighandler);

	if (fd < 0) {
		printf("Error opening %s: %s\n", portname, strerror(errno));
		return -1;
	}
	set_interface_attribs(fd, baudrate);
	// set_mincount(fd, 50);                /* set to pure timed read */

	tcdrain(fd);    /* delay for output */


	Ring ring(300);

	char buf[256];
  char token_remainder[10];


	do {
		uint8_t rdlen = read(fd, buf, sizeof(buf) - 1);
		if (rdlen > 0) {
			if (rdlen > 64) {
				rdlen = 64;
				memset(buf, 0, sizeof(buf));
				continue;
			}
			char tmp[rdlen + sizeof(token_remainder)]="";
			strcat(tmp, token_remainder);
			memset(token_remainder,0, sizeof(token_remainder));
			strcat(tmp, buf);

			char* token; 
			char* rest = tmp; 

			printf("dicks: %d\n", rdlen);
			bool keep_last = buf[rdlen-1] == '\n';
			int ints[20]={0};
			int last_element;
			int i=0;

			while ( token = strtok_r(rest, "\n", &rest) ) {
				last_element = atoi(token);
				ints[i] = last_element;
				i++;
			}

			if (!keep_last) {
				sprintf(token_remainder,"%d",last_element);
				i--;
			}

			// for (int i=0; i < rdlen; i++) ring.insert(buf[i]);
			for (int j=0; j<i; j++) printf("%d,",ints[j]);
			printf("\n");
			printf("Raw: %s", buf);
			printf("\n");
		}
		else if (rdlen < 0) printf("Error from read: %d: %s\n", rdlen, strerror(errno));
		else   /* rdlen == 0 */ printf("Timeout from read\n");

	} while (1);
}
