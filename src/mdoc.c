/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for the mdoc program.                                 |
---------------------------------------------------------
*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "exec.h"
#include "input.h"
#include "strman.h"
#include "informative.h"
#include "mdoc.h"


/* To indicate if an previous error occoured in a functions
   that could overwrite errno with 0 (success) before returning */
bool prev_error;

/* Static Functions Prototype */
static struct l_list *alloc_l_list_obj(const size_t);
static char *get_entry_path(const char *, const char *);
static struct l_list *get_last_node(struct l_list *);
static void search_for_doc_error(char *, struct l_list **, struct l_list **);
static void get_doc_path_error(char **, char **);
static char *get_doc_path_retval(char *, char *);
static void print_docs_colorful(const struct l_list *);
static void print_docs_no_color(const struct l_list *);
static void save_l_list_obj(const struct l_list *, char **);
static unsigned int get_argc_val(const char *);
static void free_and_null(void **);
static void reorganize_l_list_alpha(struct l_list *, char *const *);
static void *alloc_l_list();
static int open_founded_doc_path(struct users_configs *, char *);
static int open_doc(char *const *);
static void print_opening_doc(const char *, const bool);
static struct l_list *search_for_doc_retval(struct l_list *, struct l_list *,  
                                            struct l_list *);


/*
 * Return a pointer to allocated l_list structure that also 
 * has an allocated obj to it with the size of obj_size.
 */
static struct l_list *alloc_l_list_obj(const size_t obj_size) {
	struct l_list *retval = NULL;
	struct l_list *ptr;

	if((ptr = alloc_l_list()) 
	 && (ptr->obj = malloc_inf(sizeof(char) * obj_size)))
		retval = ptr;

	if(!retval && ptr)
		free(ptr);

	return retval;
}


static void *alloc_l_list() 
{
	return malloc_inf(sizeof(struct l_list));
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


static char *get_entry_path(const char *dir_path, const char *entry_name) {
    const size_t path_len = strlen(dir_path) + strlen(entry_name) + 2;
    char *entry_path;

    if((entry_path = malloc_inf(sizeof(char) * path_len)))
        snprintf(entry_path, path_len, "%s/%s", dir_path, entry_name);

    return entry_path;
}


static struct l_list *get_last_node(struct l_list *ptr) {
	struct l_list *prev;

	for(; ptr; ptr=ptr->next)
		prev = ptr;

	return prev;
}


/*
 * The function will search dor the passed str sequence in dir_path
 * then it will save the founded ones in the linked list. If str is NULL
 * it'll save all the docs to the linked list.
 */
struct l_list *search_for_doc(const char *dir_path, const char *str, 
                              const bool ignore_case, const bool recursive) {
	struct l_list *current_node_rec, *current_node;;
	/* The begnning of the documents list that is 
	   created, filled and returned by the recursion */
	struct l_list *doc_list_rec_begin;
	/* The beginning of the documents list that is filled by the function */	
	struct l_list *doc_list_begin; 
	struct dirent *entry;
	struct stat stbuf;
	char *new_path;
	size_t len;
	int ret;
	DIR *dp;

	/* Initialize pointers */
	doc_list_rec_begin = doc_list_begin = current_node_rec = current_node = NULL;
	new_path = NULL;

	if((dp = opendir_inf(dir_path)))
		while((entry = readdir_inf(dp))) {
			/* Skip current and pervious directory entries */
			if(strcmp(entry->d_name, ".") == 0 
			 || strcmp(entry->d_name, "..") == 0)
				continue;
			
			if(!(new_path = get_entry_path(dir_path, entry->d_name)))
				break;
			
			if(stat_inf(new_path, &stbuf))
				break;

			if(S_ISDIR(stbuf.st_mode) && recursive) {
				/* If still not initialized try to initialize it */
				if(!doc_list_rec_begin) { 
					if((doc_list_rec_begin = search_for_doc(new_path, str, 
					                                        ignore_case, recursive)))
						current_node_rec = get_last_node(doc_list_rec_begin);
				}	
				else
					if((current_node_rec->next = search_for_doc(new_path, str, 
					                                            ignore_case, recursive)))
						current_node_rec = get_last_node(current_node_rec->next);
		
				if(prev_error)
					break;
			}
			free_and_null((void **) &new_path);

			if(S_ISREG(stbuf.st_mode)) {
			/* If str is NULL, save every file name to the linked list */
				if(str) {
					if(ignore_case) {
						if((ret = strstr_i(entry->d_name, str)) == -1)
							break;
						else if(ret == 0)
							continue;
					}
					else 
						if(!strstr(entry->d_name, str))
							continue;
				}

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
		search_for_doc_error(new_path, &doc_list_begin, &doc_list_rec_begin);

	if(dp)
		/* If error occured while closing dp and no cleanup have been done already */
		if(closedir_inf(dp) && !prev_error) 
			search_for_doc_error(new_path, &doc_list_begin, &doc_list_rec_begin);
	
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
static void search_for_doc_error(char *char_ptr, 
                                 struct l_list **l_list_ptr1, 
                                 struct l_list **l_list_ptr2) {
	if(char_ptr)
		free(char_ptr);	
	if(*l_list_ptr1)
		free_l_list(*l_list_ptr1);
	if(*l_list_ptr2)
		free_l_list(*l_list_ptr2);

	*l_list_ptr1 = *l_list_ptr2 = NULL;
	prev_error = true;
}


void display_docs(const struct l_list *ptr, const bool color_status) {
		if(color_status)
			print_docs_colorful(ptr);
		else
			print_docs_no_color(ptr);

}


static void print_docs_colorful(const struct l_list *ptr) {
	for(; ptr; ptr=ptr->next)
		printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "+" ANSI_COLOR_BLUE "]" 
		       ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", 
		       ptr->obj);
}


static void print_docs_no_color(const struct l_list *ptr) {
	for(; ptr; ptr=ptr->next)
		printf("[+] %s\n", ptr->obj);
}


unsigned int count_l_list_nodes(const struct l_list *ptr) {
	unsigned int i;

	for(i=0; ptr; i++)
		ptr = ptr->next;

	return i;
}


/*
 * This function will be used only when count_l_list_nodes() returns 1.
 */
char *get_doc_path(const char *dir_path, 
                   const char *full_doc_name, 
                   const bool recursive) {
	char *new_path, *ret_path; /* ret_path is the returned path from the recursion */
	struct dirent *entry;
	struct stat stbuf;
	DIR *dp;

	new_path = ret_path = NULL;
	
	if((dp = opendir_inf(dir_path)))
		while((entry = readdir_inf(dp))) {
			if(strcmp(entry->d_name, ".") == 0 
			 || strcmp(entry->d_name, "..") == 0)
				continue;

			if(!(new_path = get_entry_path(dir_path, entry->d_name)))
				break;

			if(stat_inf(new_path, &stbuf))
				break;
			
			if(S_ISDIR(stbuf.st_mode) && recursive) {
				if((ret_path = get_doc_path(new_path, full_doc_name, recursive)))
					break;
				
				else if(prev_error)
					break;
			}
			else if(S_ISREG(stbuf.st_mode) 
			 && strstr(new_path, full_doc_name))
				break;
		
			free_and_null((void **) &new_path);
		}
	/* Catch errors */
	if(errno || prev_error)
		get_doc_path_error(&new_path, &ret_path);

	if(dp)
		if(closedir_inf(dp) && !prev_error)
			get_doc_path_error(&new_path, &ret_path);

	return get_doc_path_retval(new_path, ret_path);
}


static void free_and_null(void **ptr) {
	free(*ptr);
	*ptr = NULL;
}


/*
 * Function for the cleanup when error occures in get_doc_path().
 */
static void get_doc_path_error(char **char_ptr1, char **char_ptr2) {
	if(*char_ptr1)
		free_and_null((void **) char_ptr1);
	if(*char_ptr2)
		free_and_null((void **) char_ptr2);

	prev_error = 1;
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


void prep_open_doc_argv(char **argv, char *pdf_viewer, 
                        char *add_args, char *doc_path) {
	unsigned int i = 0;
	unsigned int ret;

	argv[i++] = pdf_viewer;

	if(add_args)
		for(; (ret = space_to_null(add_args)); add_args+=ret)
			argv[i++] = add_args;
	
	argv[i++] = doc_path;
	argv[i] = NULL;
}


static int open_doc(char *const *argv) 
{
	return execvp_process(argv[0], argv);
}


int open_doc_list(struct users_configs *configs, 
                  const struct l_list *doc_list,
				  const bool rec, const bool color) {
	const struct l_list *ptr = doc_list;
	char *doc_path;
	int retval = 0;

	for(; !retval && ptr; ptr=ptr->next) {
		if((doc_path = get_doc_path(configs->docs_dir_path, ptr->obj, rec))) {
			if(!(retval = open_founded_doc_path(configs, doc_path)))
				print_opening_doc(ptr->obj, color);
			
			free(doc_path);
		}
		else 
			retval = -1;
	}
	
	return retval;
}


static int open_founded_doc_path(struct users_configs *configs, char *doc_path) {
    unsigned int argc = get_argc_val(configs->add_args);
    char *argv[argc+1];

    prep_open_doc_argv(argv, configs->pdf_viewer, configs->add_args, doc_path);

    return open_doc(argv);
}


static unsigned int get_argc_val(const char *add_args) {
	/* The 2 stands for doc_path and pdf_viewer */
	unsigned int val = 2;

	if(add_args)
		val += count_words(add_args);

	return val;
}


static void print_opening_doc(const char *doc_name, const bool color) {
	if(color)
		printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "OPENING" ANSI_COLOR_BLUE "]" 
               ANSI_COLOR_RED " %s\n" ANSI_COLOR_RESET, 
               doc_name);
	else 
		printf("[OPENING] %s\n", doc_name);
}


void print_docs_num(const struct l_list *doc_list, const bool color) {
    if(color)
        printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "COUNTED" ANSI_COLOR_BLUE "]"               
               ANSI_COLOR_RED " %d Documents\n" ANSI_COLOR_RESET, 
               count_l_list_nodes(doc_list));
    else
        printf("[COUNTED] %d Documents\n", count_l_list_nodes(doc_list));
}


/*
 * Sort the linked list of docs alphabetically.
 */
int sort_docs_alpha(struct l_list *unsorted_l_list) {
	const unsigned int obj_num = count_l_list_nodes(unsorted_l_list);	
	char *unsorted_array[obj_num+1];
	char *sorted_array[obj_num+1];
	int retval;

	save_l_list_obj(unsorted_l_list, unsorted_array);
	
	if(!(retval = strsort_alpha(unsorted_array, sorted_array, obj_num)))
		reorganize_l_list_alpha(unsorted_l_list, sorted_array);
	
	return retval;
}


/*
 * Save every obj address in the linked list to the array of pointers.
 */
static void save_l_list_obj(const struct l_list *ptr, char **array) {
	unsigned int i;

	for(i=0; ptr; ptr=ptr->next)
		array[i++] = ptr->obj;
	
	array[i] = NULL;
}


/*
 * Reorganize the linked list's objects alphabetically.
 */
static void reorganize_l_list_alpha(struct l_list *unsorted_l_list, 
                                    char *const *sorted_array) {
	struct l_list *ptr = unsorted_l_list;
	unsigned int i;

	for(i=0; ptr; ptr=ptr->next)
		ptr->obj = sorted_array[i++];
}


void reverse_l_list_obj(struct l_list *ptr) {
	const unsigned int obj_num = count_l_list_nodes(ptr);
	char *objs_array[obj_num+1];
	long int i;

	save_l_list_obj(ptr, objs_array);
	
	for(i=obj_num-1; i>-1; ptr=ptr->next)
		ptr->obj = objs_array[i--];
}


void display_help(const char *name) {
	printf("Usage: %s  <options>  [argument]\n", name);
	printf(
	       "A command-line tool for managing your documents and easing your life."
	       
		   "\n\n"
		   
		   "Available options:\n"

	       " -h \t\t Display this help message.\n"
	       " -g \t\t Generate new configurations file.\n"
	       " -s \t\t Sort the founded documents alphabetically.\n"
	       " -r \t\t Reverse the order of the founded documents.\n"
	       " -a \t\t Include all documents.\n"
	       " -i \t\t Ignore case distinctions while searching for the documents.\n"
	       " -n \t\t Allow numerous documents opening (execution).\n"
		   " -c \t\t Count the existing documents with the passed string sequence.\n"
	       " -l \t\t List the existing documents with the passed string sequence.\n"
	       " -o \t\t Open the founded document with the passed string sequence.\n"
	       " -R \t\t Disable recursive searching for the documents.\n"
	       " -C \t\t Disable colorful output.\n"
           
		   "\n\n"
	       
		   "NOTES:\n"

	       "  1. You can use the -a optoin with the -c, -l and -o options instead\n"
	       "     of passing an actual argument.\n"
		   
		   "\n"

	       "  2. You can use the -n option with the -o option to give the program\n"
		   "     the approval to open more than one document in a run.\n"
	      );
}
