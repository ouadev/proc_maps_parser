#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "pmparser.h"

/**
 * pmparser_print
 * @param mem_reg memory region to display
 */
void pmparser_print(procmaps_struct *mem_reg);

const char *map_type_to_string(procmaps_map_type type);
/**
 *
 * Usage: map [pid]
 */

int main(int argc, char *argv[])
{
	int pid = -1;
	procmaps_iterator maps_iter = {0};
	procmaps_error_t parser_err = PROCMAPS_SUCCESS;

	if (argc == 2)
		pid = atoi(argv[1]);
	else
		pid = -1;

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

	return 0;
}

void pmparser_print(procmaps_struct *mem_reg)
{
	printf("0x%.12lx-0x%.12lx\t", (size_t)mem_reg->addr_start, (size_t)mem_reg->addr_end);
	printf("%c%c%c%c ", mem_reg->is_r ? 'r' : '-', mem_reg->is_w ? 'w' : '-', mem_reg->is_x ? 'x' : '-', mem_reg->is_p ? 'p' : 's');
	printf("%ld\n", mem_reg->length);
	printf("%s\t", map_type_to_string(mem_reg->map_type));
	if (mem_reg->map_type == PROCMAPS_MAP_FILE)
	{
		printf("Offset:%ld ", mem_reg->offset);
		printf("%s", mem_reg->pathname);
	}
	else if (mem_reg->map_type == PROCMAPS_MAP_ANON_PRIV || mem_reg->map_type == PROCMAPS_MAP_ANON_SHMEM)
	{
		printf("%s", mem_reg->map_anon_name);
	}
	else if (mem_reg->map_type == PROCMAPS_MAP_OTHER)
	{
		printf("%s", mem_reg->pathname);
	}
	printf("\n");
	printf("inode :%llu\ndevice:%u:%u", mem_reg->inode, mem_reg->dev_major, mem_reg->dev_minor);
	printf("\n\n");
}

const char *map_type_to_string(procmaps_map_type type)
{
	switch (type)
	{
	case PROCMAPS_MAP_FILE:
		return "file";
	case PROCMAPS_MAP_STACK:
		return "process_stack";
	case PROCMAPS_MAP_STACK_TID:
		return "thread_stack";
	case PROCMAPS_MAP_VDSO:
		return "VDSO";
	case PROCMAPS_MAP_HEAP:
		return "heap";
	case PROCMAPS_MAP_ANON_PRIV:
		return "anon_private";
	case PROCMAPS_MAP_ANON_SHMEM:
		return "anon_shared";
	case PROCMAPS_MAP_ANON_MMAPS:
		return "anonymous";
	case PROCMAPS_MAP_VVAR:
		return "vvar";
	case PROCMAPS_MAP_VSYSCALL:
		return "vsyscall";
	case PROCMAPS_MAP_OTHER:
		return "other";
	default:
		return "unknown";
	}
}
