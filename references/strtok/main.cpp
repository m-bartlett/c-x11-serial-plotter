#include <string.h>
#include <vector>
#include <sstream>

using string = std::string;
using vec = std::vector<int>;

int main(int argc, char *argv[]) {

	char STR[] = "-107\n-87\n-65\n-44\n-22\n0\n22\n44\n65\n87\n107\n127\n146\n-1633";
	char str[sizeof(STR)]="";
	char tmp[sizeof(STR)]="";
	strcpy(str,STR);

  char* token; 
  char* rest = str; 

  bool keep_last = STR[strlen(STR)-1] == '\n';
  int ints[20]={0};
  int last_element;
  int i=0;
  char last_str[10];
  while ( token = strtok_r(rest, "\n", &rest) ) {
  	// strcat(tmp, ",");
  	// last_len = strlen(token);
  	last_element = atoi(token);
  	ints[i] = last_element;
  	i++;
  }
  if (!keep_last) {
  	sprintf(last_str,"%d",last_element);
  	i--;
  }
	
  for (int j=0; j<i; j++) printf("%d,",ints[j]);
  printf("\n");
	for (char c: last_str) printf("%c ",c);
  printf("\n");



  return 0;
}