/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for the rdoc program.                                 |
---------------------------------------------------------
*/

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "informative.h"
#include "rdoc.h"


/* Give the right program name for the error messages */
const char *_prog_name_inf_ = "rdoc";

/* Static Functions Prototype */
static struct l_list *alloc_l_list_obj(size_t);
static char *convert_to_lower(char *);
static int strstr_i(const char *, const char *);
static char *get_entry_path(const char *, const char *);


/*
 * Return a pointer to allocated l_list structure that also 
 * has an allocated obj to it with the size of obj_size.
 */
static struct l_list *alloc_l_list_obj(size_t obj_size) {
	struct l_list *ptr;

	if(!(ptr = malloc_inf(sizeof(struct l_list))))
		goto Error;
	
	if(!(ptr->obj = (char *) malloc_inf(obj_size)))
		goto Error;

	return ptr;

	Error:	
		if(ptr)
			free(ptr);
	
		return NULL;
}


void free_l_list(struct l_list *ptr) {
	struct l_list *prev_ptr;

	while(ptr) {
		if(ptr->obj)
			free(ptr->obj);
		
		prev_ptr = ptr;
		ptr = ptr->next;
		
		free(prev_ptr);
	}
}


/*
 * Make a small letters copy of str.
 */
static char *convert_to_lower(char *str) {
	size_t len = strlen(str) + 1;
	unsigned int i;
	char *str_c; // Copy of str

	if((str_c = (char *) malloc_inf(len))) {
		for(i=0; str[i]!='\0'; i++) {
			if(isupper(str[i]))
				str_c[i] = tolower(str[i]);
			else
				str_c[i] = str[i];
		}
		str_c[i] = '\0';
	}
	
	return str_c;
}

/*
 * The same as strstr() but with ignored case distinction
 * and diffrent return values. 0 = didn't find, 1 = found, -1 = fail.
 */
static int strstr_i(const char *haystack, const char *needle) {
	char *haystack_l = NULL; 
	char *needle_l = NULL; 
	int retval = -1;
	
	if(!(haystack_l = convert_to_lower((char *) haystack)))
		goto Out;

	if(!(needle_l = convert_to_lower((char *) needle)))
		goto Out;
	
	if(strstr(haystack_l, needle_l))
		retval = 1;
	else 
		retval = 0;

	Out:
		if(haystack_l)
			free(haystack_l);	
		if(needle_l)
			free(needle_l);

		return retval;
}

static char *get_entry_path(const char *dir_path, const char *entry_name) {
    const size_t path_len = strlen(dir_path) + strlen(entry_name) + 2;
    char *entry_path;

    if((entry_path = (char *) malloc_inf(path_len)))
        snprintf(entry_path, path_len, "%s/%s", dir_path, entry_name);

    return entry_path;
}

/*
 * The function will search the passed str sequence in all the 
 * existing documents in the documents directory. 
 */
struct l_list *search_for_doc(const char *docs_dir_path, const char *str, 
                              unsigned int ignore_case, unsigned int recursive) {
	struct l_list *doc_list_begin, *doc_list_rec_begin, *retval;
	struct l_list *ptr, *ptr_rec;
	struct dirent *entry;
	struct stat stbuf;
	size_t obj_len;
	char *new_path;
	int status;
	DIR *dp;
     
	doc_list_begin = doc_list_rec_begin = retval = NULL;
    if(!(dp = opendir_inf(docs_dir_path))) 
		goto Out;

	while((entry = readdir_inf(dp))) {
		/* Skip current and pervious directory entries. */ 
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		
		if(!(new_path = get_entry_path(docs_dir_path, entry->d_name))) 
			goto CleanUp;

		if(stat_inf(new_path, &stbuf))
			goto CleanUp;
		
		obj_len = strlen(entry->d_name) + 1;
		if(recursive) {
			if(S_ISDIR(stbuf.st_mode)) {	
				if(!doc_list_rec_begin) {
                    doc_list_rec_begin = search_for_doc(new_path, str, ignore_case, recursive);
					if(errno)
                       goto CleanUp;
					
                    ptr_rec = doc_list_rec_begin;
                }
                else {
                    ptr_rec = ptr_rec->next = search_for_doc(new_path, str, ignore_case, recursive);
                    if(!ptr_rec)
						goto CleanUp;
                }
			
			free(new_path);
		    new_path = NULL;
			continue;
            }
        }
		free(new_path);
		new_path = NULL;

		if(!S_ISREG(stbuf.st_mode))
			continue; 
		
		if(ignore_case) {
			status = strstr_i(entry->d_name, str);	
			
			if(status == -1)
				goto CleanUp;
			else if(status == 0)
				continue;
		}
		else 
			if(!strstr(entry->d_name, str))
				continue;
        
		if(!doc_list_begin) {
			if(!(doc_list_begin = alloc_l_list_obj(obj_len)))
				goto CleanUp;
			
			snprintf(doc_list_begin->obj, obj_len, "%s", entry->d_name);
			
			ptr = doc_list_begin;	
			continue;
		}

		ptr = ptr->next = alloc_l_list_obj(obj_len);
		if(!ptr)
			goto CleanUp;

		snprintf(ptr->obj, obj_len, "%s", entry->d_name);
	}

	if(errno) 
		goto CleanUp;

	/* Determine the right retval */
	if(doc_list_begin) {
		if(doc_list_rec_begin)
			ptr->next = doc_list_rec_begin;
		
		retval = doc_list_begin;
	} 
	
	else if(doc_list_rec_begin)
		retval = doc_list_rec_begin;
	
	else 
		goto CleanUp;


	Out:
		if(dp)
			if(closedir_inf(dp))
				// If cleanup haven't been already done jump to CleanUp and do it.
				if(retval) {  
					dp = NULL;
					retval = NULL;
					goto CleanUp;
				}
		
		return retval;

	CleanUp:
		if(new_path)
			free(new_path);
		if(doc_list_begin)
			free_l_list(doc_list_begin);
		if(doc_list_rec_begin)
			free_l_list(doc_list_rec_begin);

		goto Out;
}

void print_l_list(struct l_list *ptr) {
	while(ptr) {
		if(ptr->obj)
			printf("%s\n", ptr->obj);

		ptr = ptr->next;
	}
}
