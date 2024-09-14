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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pmparser.h"

// maximum line length in a procmaps file
#define PROCMAPS_LINE_INIT_LENGTH (300)
// maximum length of the path of a maps file : /proc/[pid]/maps
#define PROCMAPS_MAPS_FILE_PATH_MAX_LENGTH 30
// maximum token size while parsing the proc/pid/maps
#define PROCMAPS_LINE_TOKEN_MAX_LEN 100

/**
 * pmparser_parse_line
 * @description internal usage
 */
static void pmparser_parse_line(char *buf, procmaps_struct *mem_reg);

/**
 * @brief Copy into dest_ptr the string from src_ptr to the first occurence of delimiter
 */
static char *pmparser_helper_extract(char *src_ptr, const char *delimiter, char *dest_ptr);

/**
 * @brief Main function to parse process memory
 *
 * @param pid process ID
 * @param maps_it output : the memory region iterator over the chained list, it should only be read when return is 0
 * @return procmaps_error_t outcome of the function
 */
procmaps_error_t pmparser_parse(int pid, procmaps_iterator *maps_it)
{
	char maps_path[PROCMAPS_MAPS_FILE_PATH_MAX_LENGTH];
	size_t line_len = 0;
	char *line_ptr = NULL;
	procmaps_struct *mem_reg = NULL;
	procmaps_struct *tail_node = NULL;
	procmaps_struct *head_node = NULL;
	size_t node_count = 0;

	if (pid >= 0)
	{
		snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);
	}
	else
	{
		sprintf(maps_path, "/proc/self/maps");
	}

	FILE *file = fopen(maps_path, "r");
	if (!file)
	{
		return PROCMAPS_ERROR_OPEN_MAPS_FILE;
	}

	// scan maps file line by line
	line_len = PROCMAPS_LINE_INIT_LENGTH;
	if ((line_ptr = (char *)malloc(line_len)) == NULL)
	{
		fclose(file);
		return PROCMAPS_ERROR_MALLOC_FAIL;
	}

	while (1)
	{
		int read = getline(&line_ptr, &line_len, file);
		if (read == -1)
		{
			if (!feof(file))
			{
				return PROCMAPS_ERROR_READ_MAPS_FILE;
			}
			else
			{
				// end of file occured while no characters have been read
				break;
			}
		}

		// allocate a node
		mem_reg = (procmaps_struct *)malloc(sizeof(procmaps_struct));
		// fill the node
		pmparser_parse_line(line_ptr, mem_reg);
		mem_reg->next = NULL;

		// Attach the node
		if (tail_node == NULL)
		{
			head_node = tail_node = mem_reg;
		}
		else
		{
			tail_node->next = mem_reg;
			tail_node = mem_reg;
		}
		node_count++;
	}

	// close file
	fclose(file);
	free(line_ptr);

	// set iterator
	maps_it->head = maps_it->current = head_node;
	maps_it->count = node_count;

	return PROCMAPS_SUCCESS;
}

/**
 * @brief move the iterator to the next memory region
 *
 * @param p_procmaps_it
 * @return procmaps_struct*
 */
procmaps_struct *pmparser_next(procmaps_iterator *p_procmaps_it)
{
	if (p_procmaps_it->current == NULL)
		return NULL;
	procmaps_struct *p_current = p_procmaps_it->current;
	p_procmaps_it->current = p_procmaps_it->current->next;
	return p_current;
}

/**
 * @brief free the parser data
 *
 * @param p_procmaps_it
 */
void pmparser_free(procmaps_iterator *p_procmaps_it)
{
	procmaps_struct *cursor = p_procmaps_it->head;
	procmaps_struct *next = NULL;

	if (p_procmaps_it->head == NULL)
		return;

	while (cursor != NULL)
	{
		next = cursor->next;
		free(cursor->pathname);
		free(cursor);
		cursor = next;
	}
	memset(p_procmaps_it, 0x00, sizeof(procmaps_iterator));
}

static char *pmparser_helper_extract(char *src_ptr, const char *delimiter, char *dest_ptr)
{
	char *p_separator = NULL;
	size_t copy_len = 0;

	p_separator = strstr(src_ptr, delimiter);
	copy_len = (p_separator - src_ptr);
	memcpy(dest_ptr, src_ptr, copy_len);
	dest_ptr[copy_len] = 0x00;

	return p_separator;
}

static void pmparser_parse_line(char *buf, procmaps_struct *mem_reg)
{
	char token[PROCMAPS_LINE_TOKEN_MAX_LEN];
	size_t pathname_len = 0;
	char *p_cursor = buf;

	// addr1
	p_cursor = pmparser_helper_extract(p_cursor, "-", token);
	p_cursor++;

	sscanf(token, "%lx", (long unsigned *)&mem_reg->addr_start);

	// addr2
	p_cursor = pmparser_helper_extract(p_cursor, " ", token);
	p_cursor++;

	sscanf(token, "%lx", (long unsigned *)&mem_reg->addr_end);

	// region size
	mem_reg->length = (unsigned long)((char*)mem_reg->addr_end - (char*)mem_reg->addr_start);

	// perm
	p_cursor = pmparser_helper_extract(p_cursor, " ", token);
	p_cursor++;

	mem_reg->is_r = (token[0] == 'r');
	mem_reg->is_w = (token[1] == 'w');
	mem_reg->is_x = (token[2] == 'x');
	mem_reg->is_p = (token[3] == 'p');

	// offset
	p_cursor = pmparser_helper_extract(p_cursor, " ", token);
	p_cursor++;

	sscanf(token, "%lx", &mem_reg->offset);

	// dev
	p_cursor = pmparser_helper_extract(p_cursor, " ", token);
	p_cursor++;

	sscanf(token, "%u:%u", &mem_reg->dev_major, &mem_reg->dev_minor);

	// inode
	p_cursor = pmparser_helper_extract(p_cursor, " ", token);
	p_cursor++;

	sscanf(token, "%llu", &mem_reg->inode);

	// pathname
	// find the start of the pathname
	while (*p_cursor == '\t' || *p_cursor == ' ')
		p_cursor++;
	// calculate its size
	char *ptr_sz = p_cursor;
	while (*ptr_sz != '\n')
	{
		ptr_sz++;
	}
	pathname_len = (ptr_sz - p_cursor);
	// copy it
	mem_reg->pathname = (char *)malloc(pathname_len * sizeof(char) + 1);
	memcpy(mem_reg->pathname, p_cursor, pathname_len);
	mem_reg->pathname[pathname_len] = 0x00;

	// Pathname decoding
	if (mem_reg->pathname[0] == 0x00)
	{
		// empty path name
		mem_reg->map_type = PROCMAPS_MAP_ANON_MMAPS;
	}
	else if (strncmp(mem_reg->pathname, "[stack]", 7) == 0)
	{
		// mapping backed by main thread stack
		mem_reg->map_type = PROCMAPS_MAP_STACK;
	}
	else if (strncmp(mem_reg->pathname, "[stack:", 7) == 0)
	{
		mem_reg->map_type = PROCMAPS_MAP_STACK_TID;
	}
	else if (strncmp(mem_reg->pathname, "[vdso]", 6) == 0)
	{
		mem_reg->map_type = PROCMAPS_MAP_VDSO;
	}
	else if (strncmp(mem_reg->pathname, "[heap]", 6) == 0)
	{
		mem_reg->map_type = PROCMAPS_MAP_HEAP;
	}
	else if (strncmp(mem_reg->pathname, "[anon:", 6) == 0)
	{
		mem_reg->map_type = PROCMAPS_MAP_ANON_PRIV;
		pmparser_helper_extract(mem_reg->pathname + 6, "]", token);
		strncpy(mem_reg->map_anon_name, token, MAPPING_ANON_NAME_MAX_LEN);
	}
	else if (strncmp(mem_reg->pathname, "[anon_shmem:", 12) == 0)
	{
		mem_reg->map_type = PROCMAPS_MAP_ANON_SHMEM;
		pmparser_helper_extract(mem_reg->pathname + 12, "]", token);
		strncpy(mem_reg->map_anon_name, token, MAPPING_ANON_NAME_MAX_LEN);
	}
	else if (strncmp(mem_reg->pathname, "[vvar]", 6) == 0)
	{
		mem_reg->map_type = PROCMAPS_MAP_VVAR;
	}
	else if (strncmp(mem_reg->pathname, "[vsyscall]", 10) == 0)
	{
		mem_reg->map_type = PROCMAPS_MAP_VSYSCALL;
	}
	else if (strncmp(mem_reg->pathname, "[", 1) == 0)
	{
		mem_reg->map_type = PROCMAPS_MAP_OTHER;
	}
	else
	{
		// file backed mapping then
		mem_reg->map_type = PROCMAPS_MAP_FILE;

		// is the file deleted ?
		// file_deleted
		if (memcmp(mem_reg->pathname + strlen(mem_reg->pathname) - 9, "(deleted)", 9) == 0)
		{
			mem_reg->file_deleted = 1;
		}
		else
		{
			mem_reg->file_deleted = 0;
		}
	}
}
