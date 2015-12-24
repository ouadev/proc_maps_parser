# proc_maps_parser
a lightweight library to parse Linux's /proc/[pid]/maps file, which contains the memory map of a process

# /proc/[pid]/maps
A file containing the currently mapped memory regions and
their access permissions.  See mmap(2) for some further
information about memory mappings.

# Usage
from ./examples/map.c
```C
  int pid=-1; //-1 to use the running process id, use pid>0 to list the map of another process
  procmaps_struct* maps=pmparser_parse(pid);
	if(maps==NULL){
		printf ("[map]: cannot parse the memory map of %d\n",pid);
		return -1;
	}

	//iterate over areas
	procmaps_struct* maps_tmp=NULL;
	while( (maps_tmp=pmparser_next())!=NULL){
		pmparser_print(maps_tmp,0);
		printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~\n"); 
	}

	//mandatory: should free the list
	pmparser_free(maps);
```

