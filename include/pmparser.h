/*
 @Author	: ouadev
 @date		: December 2015

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.  No representations are made about the suitability of this
software for any purpose.  It is provided "as is" without express or
implied warranty.

 */

#ifndef H_PMPARSER
#define H_PMPARSER

// Documentation link: https://man7.org/linux/man-pages/man5/proc_pid_maps.5.html

// maximum length of the name of an anonymous mapping
#define MAPPING_ANON_NAME_MAX_LEN 80

/**
 * @brief Type of a memory's region mapping.
 *
 */
typedef enum
{
	PROCMAPS_MAP_FILE,
	PROCMAPS_MAP_STACK,
	PROCMAPS_MAP_STACK_TID,
	PROCMAPS_MAP_VDSO,
	PROCMAPS_MAP_VVAR,
	PROCMAPS_MAP_VSYSCALL,
	PROCMAPS_MAP_HEAP,
	PROCMAPS_MAP_ANON_PRIV,
	PROCMAPS_MAP_ANON_SHMEM,
	PROCMAPS_MAP_ANON_MMAPS,
	PROCMAPS_MAP_OTHER,
} procmaps_map_type;

/**
 * procmaps_struct
 * @desc hold all the information about an area in the process's  VM
 */
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
	char map_anon_name[MAPPING_ANON_NAME_MAX_LEN + 1]; //< name of the anonymous mapping in case map_type is an anon mapping
	short file_deleted;								   //< whether the file backing the mapping was deleted
	// chained list
	struct procmaps_struct *next; //<handler of the chained list
} procmaps_struct;

/**
 * @brief procmaps error type
 *
 */
typedef enum procmaps_error
{
	PROCMAPS_SUCCESS = 0,
	PROCMAPS_ERROR_OPEN_MAPS_FILE,
	PROCMAPS_ERROR_READ_MAPS_FILE,
	PROCMAPS_ERROR_MALLOC_FAIL,
} procmaps_error_t;

/**
 * procmaps_iterator
 * @desc holds iterating information
 */
typedef struct procmaps_iterator
{
	procmaps_struct *head;
	procmaps_struct *current;
	size_t count;
} procmaps_iterator;

/**
 * @brief Main function to parse process memory
 * @param pid process ID
 * @param maps_it output : the memory region iterator over the chained list, t should only be read when return is 0.
 * @return procmaps_error_t outcome of the function
 */
procmaps_error_t pmparser_parse(int pid, procmaps_iterator *maps_it);

/**
 * pmparser_next
 * @description move between areas
 * @param p_procmaps_it the iterator to move on step in the chained list
 * @return a procmaps structure filled with information about this VM area
 */
procmaps_struct *pmparser_next(procmaps_iterator *p_procmaps_it);
/**
 * pmparser_free
 * @description should be called at the end to free the resources
 * @param p_procmaps_it the iterator structure returned by pmparser_parse
 */
void pmparser_free(procmaps_iterator *p_procmaps_it);

#endif
