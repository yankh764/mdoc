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
#include "exec.h"
#include "input.h"
#include "informative.h"
#include "rdoc.h"

#define ANSI_COLOR_RED   "\e[0;31m"
#define ANSI_COLOR_RESET "\e[0m"


/* To indicate if an previous error occoured in a functions
   that could overwrite errno with 0 (success) before returning */
bool prev_error;

/* Static Functions Prototype */
static struct l_list *alloc_l_list_obj(size_t);
static char *small_let_copy(const char *);
static int strstr_i(const char *, const char *);
static void strstr_i_cleanup(char *, char *);
static char *get_entry_path(const char *, const char *);
static struct l_list *get_last_node(struct l_list *);
static void search_for_doc_error(char *, struct l_list *, struct l_list *);
static struct l_list *search_for_doc_retval(struct l_list *, struct l_list *,  struct l_list *);
static void get_doc_path_error(char *, char *);
static char *get_doc_path_retval(char *, char *);
static void print_colorful(struct l_list *);
static void print_no_color(struct l_list *);


/*
 * Return a pointer to allocated l_list structure that also 
 * has an allocated obj to it with the size of obj_size.
 */
static struct l_list *alloc_l_list_obj(size_t obj_size) {
	struct l_list *retval = NULL;
	struct l_list *ptr;

	if((ptr = malloc_inf(sizeof(struct l_list))) &&
	        (ptr->obj = (char *) malloc_inf(obj_size)))
		retval = ptr;

	if(!retval && ptr)
		free(ptr);

	return retval;
}


void free_l_list(struct l_list *ptr) {
	struct l_list *prev_ptr;

	while(ptr) {
		free(ptr->obj);
		
		prev_ptr = ptr;
		ptr = ptr->next;	
		
		free(prev_ptr);
	}
}


/*
 * Make a small letters copy of str.
 */
static char *small_let_copy(const char *str) {
	size_t len = strlen(str) + 1;
	unsigned int i;
	char *str_small; /* Small letter copy of str */

	if((str_small = (char *) malloc_inf(len))) {
		for(i=0; str[i]!='\0'; i++) {
			if(isupper(str[i]))
				str_small[i] = tolower(str[i]);
			else
				str_small[i] = str[i];
		}
		str_small[i] = '\0';
	}
	
	return str_small;
}


/*
 * The same as strstr() but with "ignored case distinction"
 * and diffrent return values. 0 = didn't find, 1 = found, -1 = fail.
 */
static int strstr_i(const char *haystack, const char *needle) {
	char *haystack_small = NULL; 
	char *needle_small = NULL; 
	int retval = -1;
	
	/* If made small letter copy of haystack and needle sucessfully */
	if((haystack_small = small_let_copy(haystack)) && 
	          (needle_small = small_let_copy(needle))) {
		if(strstr(haystack_small, needle_small))
			retval = 1;
		else 
			retval = 0; 
	}
	
	strstr_i_cleanup(haystack_small, needle_small);

	return retval;
}


static void strstr_i_cleanup(char *ptr1, char *ptr2) {
	if(ptr1)
		free(ptr1);
	if(ptr2)
		free(ptr2);
}


static char *get_entry_path(const char *dir_path, const char *entry_name) {
    const size_t path_len = strlen(dir_path) + strlen(entry_name) + 2;
    char *entry_path;

    if((entry_path = (char *) malloc_inf(path_len)))
        snprintf(entry_path, path_len, "%s/%s", dir_path, entry_name);

    return entry_path;
}


static struct l_list *get_last_node(struct l_list *ptr) {
	struct l_list *prev = NULL;

	for(; ptr; ptr=ptr->next)
		prev = ptr;

	return prev;
}


/*
 * The function will search the passed str sequence in all the 
 * existing documents in the documents directory. 
 */
struct l_list *search_for_doc(const char *dir_path, const char *str, 
                              bool ignore_case, bool recursive) {
	/* Note: I tried to find better variables names for doc_list_begin and 
      doc_list_rec_begin, but didn't find any. So just to make things clear
      doc_list_begin refers to the beginning of the documents list that is
      filled by the function, whereas doc_list_rec_begin refers to the begnning
      of the documents list that is created, filled and returned by the recursion. */
	struct l_list *doc_list_begin, *doc_list_rec_begin;
	struct l_list *current_node, *current_node_rec;
	struct dirent *entry;
	struct stat stbuf;
	char *new_path = NULL;
	size_t len;
	int ret;
	DIR *dp;
	
	doc_list_begin = doc_list_rec_begin = NULL;

	if((dp = opendir_inf(dir_path)))
		while((entry = readdir_inf(dp))) {
			/* Skip current and pervious directory entries */
			if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			
			if(!(new_path = get_entry_path(dir_path, entry->d_name)))
				break;
			
			if(stat_inf(new_path, &stbuf))
				break;

			if(S_ISDIR(stbuf.st_mode) && recursive) {
				/* If still not initialized try to initialize it */
				if(!doc_list_rec_begin) { 
					if((doc_list_rec_begin = search_for_doc((const char *) new_path, 
					                                        str, ignore_case, recursive)))
						current_node_rec = get_last_node(doc_list_rec_begin);
				}	
				else
					if((current_node_rec->next = search_for_doc((const char *) new_path, 
					                                            str, ignore_case, recursive)))
						current_node_rec = get_last_node(current_node_rec->next);
		
				if(prev_error)
					break;
			}

			free(new_path);
			new_path = NULL; /* Mark it as already free */

			if(S_ISREG(stbuf.st_mode)) {
				if(ignore_case) {
					if((ret = strstr_i(entry->d_name, str)) == -1)
						break;
					else if(ret == 0)
						continue;
				}
				else 
					if(!strstr(entry->d_name, str))
						continue;

				len = strlen(entry->d_name) + 1;
				
				if(!doc_list_begin) {
					if(!(doc_list_begin = alloc_l_list_obj(len)))
						break;
			
					current_node = doc_list_begin;
				}
				else
					if(!(current_node = current_node->next = alloc_l_list_obj(len)))
						break;
		
				snprintf(current_node->obj, len, "%s", entry->d_name);
				/* Mark the next node as empty in case the current one is the last */
				current_node->next = NULL;
			}
		}
	
	if(errno || prev_error)
		search_for_doc_error(new_path, doc_list_begin, doc_list_rec_begin);

	if(dp)
		/* If error occured while closing dp and no cleanup have been done already */
		if(closedir_inf(dp) && !prev_error) 
			search_for_doc_error(new_path, doc_list_begin, doc_list_rec_begin);
	
	if(prev_error)
		doc_list_begin = doc_list_rec_begin = NULL;

	return search_for_doc_retval(current_node, doc_list_begin, doc_list_rec_begin);
}


static struct l_list *search_for_doc_retval(struct l_list *current_node, 
		                                    struct l_list *doc_list_begin,
											struct l_list *doc_list_rec_begin) {
	if(doc_list_begin) {
		if(doc_list_rec_begin)
			current_node->next = doc_list_rec_begin;

		return doc_list_begin;
	}

	else if(doc_list_rec_begin)
		return doc_list_rec_begin;
	
	else
		return NULL;
}

/*
 * Function for the cleanup when error occures in search_for_doc().
 */
static void search_for_doc_error(char *char_ptr, struct l_list *l_list_ptr1, 
                                 struct l_list *l_list_ptr2) {
	if(char_ptr)
		free(char_ptr);	
	if(l_list_ptr1)
		free_l_list(l_list_ptr1);
	if(l_list_ptr2)
		free_l_list(l_list_ptr2);

	prev_error = true;
}


void display_docs(struct l_list *ptr, bool color_status) {
	if(ptr) {
		printf("These are the documents that were found:\n\n");

		if(color_status)
			print_colorful(ptr);
		else
			print_no_color(ptr);
	}
}


static void print_colorful(struct l_list *ptr) {
	for(; ptr; ptr=ptr->next)
		printf("<*>" ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", ptr->obj);

	printf("\n");
}


static void print_no_color(struct l_list *ptr) {
	for(; ptr; ptr=ptr->next)
		printf("<*> %s\n", ptr->obj);
	
	printf("\n");
}


unsigned int count_l_list_nodes(struct l_list *ptr) {
	unsigned int i;

	for(i=0; ptr; i++)
		ptr = ptr->next;

	return i;
}


/*
 * This function will be used only when count_l_list_nodes() returns 1.
 */
char *get_doc_path(const char *dir_path, const char *full_doc_name, bool recursive) {
	char *new_path, *ret_path; /* ret_path is the returned path from the recursion */
	struct dirent *entry;
	struct stat stbuf;
	DIR *dp;

	new_path = ret_path = NULL;
	
	if((dp = opendir_inf(dir_path)))
		while((entry = readdir_inf(dp))) {
			if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

			if(!(new_path = get_entry_path(dir_path, entry->d_name)))
				break;

			if(stat_inf(new_path, &stbuf))
				break;
			
			if(S_ISDIR(stbuf.st_mode) && recursive) {
				if((ret_path = get_doc_path((const char *) new_path, full_doc_name, recursive)))
					break;
				
				else if(prev_error)
					break;
			}
			else if(S_ISREG(stbuf.st_mode) && strstr(new_path, full_doc_name))
				break;
			
			free(new_path);
			new_path = NULL;
		}

	if(errno || prev_error)
		get_doc_path_error(new_path, ret_path);

	if(dp)
		if(closedir_inf(dp) && !prev_error)
			get_doc_path_error(new_path, ret_path);

	if(prev_error)
		new_path = ret_path = NULL;

	return get_doc_path_retval(new_path, ret_path);
}

/*
 * Function for the cleanup when error occures in get_doc_path().
 */
static void get_doc_path_error(char *char_ptr1, char *char_ptr2) {
	if(char_ptr1)
		free(char_ptr1);
	if(char_ptr2)
		free(char_ptr2);

	prev_error = true;
}


static char *get_doc_path_retval(char *new_path, char *ret_path) {
	if(ret_path) {
	/* If both of them are allocated ignore new_path and
	   free it because ret_path is the desired value     */
		if(new_path)
			free(new_path);

		return ret_path;
	}
	else if(new_path)
		return new_path;
	
	else 
		return NULL;
}


int open_doc(const char *pdf_viewer, const char *doc_path) {
	char *const x[] = {(char *) pdf_viewer, (char *) doc_path, NULL};
	int retval;

	if(!(retval = execvp_process(pdf_viewer, x)))
		printf("Opening: %s\n", doc_path);
	
	return retval;
}
