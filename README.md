# proc_maps_parser
a lightweight library to parse Linux's /proc/[pid]/maps file, which contains the memory map of a process

# /proc/[pid]/maps
A file containing the currently mapped memory regions and
their access permissions.

# memory map region
proc_maps_parser represents a memory region with this C structure.
```C
typedef struct procmaps_struct
{
	void *addr_start; //< start address of the area
	void *addr_end;	  //< end address
	size_t length;	  //< size of the range
	short is_r;
	short is_w;
	short is_x;
	short is_p;
	size_t offset; //< offset
	unsigned int dev_major;
	unsigned int dev_minor;
	unsigned long long inode; //< inode of the file that backs the area
	char *pathname;			  //< the path of the file that backs the area ( dynamically allocated)
	procmaps_map_type map_type;
	char map_anon_name[MAPPING_ANON_NAME_MAX_LEN + 1]; //< name of the anonymous mapping
	short file_deleted;	//< whether the file backing the mapping was deleted
	// chained list
	struct procmaps_struct *next;
} procmaps_struct;

```
# Building
After cloning, building this project is possible as follows:
```
$ cd [proc_maps_parser checkout]
$ make # to build it as a static library
$ make examples # to build both the static library and the example program
```
Optionally, the BUILD_DIR environment variable can be specified when running make to build the static library somewhere else. By default, this is set to a build/ directory in the root of the source tree.

# Usage
from ./examples/map.c
```C
	int pid = 512; // Change this to the target PID
	procmaps_iterator maps = {0};
	procmaps_error_t parser_err = PROCMAPS_SUCCESS;

	parser_err = pmparser_parse(pid, &maps_iter);
	if (parser_err)
	{
		printf("[proc_maps_parser]: failure to parse the memory map of process %d (error=%d)\n", pid, (int)parser_err);
		return -1;
	}

	// iterate over areas
	procmaps_struct *mem_region = NULL;

	while ((mem_region = pmparser_next(&maps_iter)) != NULL)
	{
		pmparser_print(mem_region);
	}
	
	// mandatory: should free the list
	pmparser_free(&maps_iter);
```

