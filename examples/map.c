#include "../pmparser.h"

/**
* 
* Usage: map [pid]
*/

int main(int argc, char* argv[]){
	int pid;
	if(argc==2) pid=atoi(argv[1]);
	else pid=-1;

	procmaps_iterator* maps = pmparser_parse(pid);
	if(maps==NULL){
		printf ("[map]: cannot parse the memory map of %d\n",pid);
		return -1;
	}

	//iterate over areas
	procmaps_struct* maps_tmp=NULL;
	
	while( (maps_tmp = pmparser_next(maps)) != NULL){
		pmparser_print(maps_tmp,0);
		printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~\n"); 
	}

	//mandatory: should free the list
	pmparser_free(maps);

	return 0;

}
