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
#include <sys/types.h>
#include "exec.h"
#include "input.h"
#include "strman.h"
#include "informative.h"
#include "mdoc.h"

#define ANSI_COLOR_RED    "\x1b[31m"
#define ANSI_COLOR_BLUE   "\x1b[34m"
#define ANSI_COLOR_GREEN  "\x1b[32m"
#define ANSI_COLOR_RESET  "\x1b[0m"


/* To indicate if an previous error occoured in a functions
   that could overwrite errno with 0 (success) before returning */
bool prev_error = 0;

/* 
 * A struct for printing the properiet size format 
 * and unit name for the document size.
 */
struct meas_unit {
	off_t size_format;
	const char *unit_name;
};


/* Static Functions Prototype */
static char *get_doc_path(const char *, const char *, const bool);
static struct l_list *alloc_l_list_obj(const size_t);
static char *get_entry_path(const char *, const char *);
static struct l_list *get_last_node(const struct l_list *);
static void search_for_doc_error(char *, struct l_list **, struct l_list **);
static void get_doc_path_error(char **, char **);
static char *get_doc_path_retval(char *, char *);
static void adj_search_for_doc_node_val(struct l_list *, const char *, const size_t); 
static void print_docs_colorful(const struct l_list *);
static bool dot_entry(const char *); 
static int check_save_doc_name(const char *, const char *, const bool);
static void print_docs_no_color(const struct l_list *);
static void save_l_list_obj(const struct l_list *, char **);
static unsigned int get_argc_val(const char *);
static void free_and_null(void **);
static void reorganize_l_list_alpha(struct l_list *, char **);
static void *alloc_l_list();
static int open_doc(char *const *);
static unsigned int prep_add_args(char **, char *, unsigned int);
static struct l_list *search_for_doc(const char *, const char *, 
                                     const bool, const bool);
static struct l_list *search_for_doc_retval(struct l_list *, struct l_list *,  
                                            struct l_list *);
static void init_search_for_doc_ptrs(struct l_list **, struct l_list **,
                                     struct l_list **, struct l_list **, char **);
static void search_for_doc_multi_dir_err(struct l_list **);
static void free_and_null_l_list(struct l_list **ptr);
static void *alloc_stat();
static struct stat *get_stat(const char *);
static void print_docs_num_color(const struct l_list *);
static void print_docs_num_no_color(const struct l_list *);
static void print_opening_doc_color(const char *);
static void print_opening_doc_no_color(const char *);
static struct l_list *search_for_doc_multi_dir_split(char *, const char *,
		                                             const bool, const bool);
static char *get_doc_path_multi_dir_split(char *, const char *, const bool);
static void print_doc_size(const off_t);
static struct meas_unit get_proper_size_format(const off_t);


/*
 * Return a pointer to allocated l_list structure that also 
 * has an allocated obj to it with the size of obj_size.
 */
static struct l_list *alloc_l_list_obj(const size_t obj_size) {
	struct l_list *retval = NULL;
	struct l_list *ptr;

	if((ptr = alloc_l_list()))
		if((ptr->obj = malloc_inf(sizeof(char) * obj_size)))
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


static struct l_list *get_last_node(const struct l_list *ptr) {
	struct l_list *prev;

	for(; ptr; ptr=ptr->next)
		prev = (struct l_list *) ptr;

	return prev;
}


/*
 * Return 1 if the entry is a dot directory 
 * (aka: ".." and "."), otherwise 0.
 */
static bool dot_entry(const char *entry_name) 
{
	return (strcmp(entry_name, ".") == 0 ||
		    strcmp(entry_name, "..") == 0);
}


static void init_search_for_doc_ptrs(struct l_list **ptr1, struct l_list **ptr2,
                                     struct l_list **ptr3, struct l_list **ptr4,
									 char **char_ptr) {
	*ptr1 = NULL;
	*ptr2 = NULL;
	*ptr3 = NULL;
	*ptr4 = NULL;
	*char_ptr = NULL;
}


/*
 * The function will search dor the passed str sequence in dir_path
 * then it will save the founded ones in the linked list. If str is NULL
 * it'll save all the docs to the linked list.
 */
static struct l_list *search_for_doc(const char *dir_path, const char *str, 
                                     const bool ignore_case, const bool recursive) {
	struct l_list *current_node_rec, *current_node;
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

	init_search_for_doc_ptrs(&doc_list_rec_begin, &doc_list_begin,
	                         &current_node_rec, &current_node, &new_path);
	
	if((dp = opendir_inf(dir_path)))
		while((entry = readdir_inf(dp))) {
			if(dot_entry(entry->d_name)) 
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
				if((ret = check_save_doc_name(entry->d_name, 
			                                  str, ignore_case)) == 1) {
					len = strlen(entry->d_name) + 1;
				
					if(!doc_list_begin) {
						if(!(doc_list_begin = alloc_l_list_obj(len)))
							break;
			
						current_node = doc_list_begin;
					}
					else
						if(!(current_node = current_node->next = alloc_l_list_obj(len)))
							break;
				
					adj_search_for_doc_node_val(current_node, entry->d_name, len);
				}
				
				else if(ret == -1)
					break;
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


/*
 * Pass doc_name to current_node->obj and terminate current_node->next.
 */
static void adj_search_for_doc_node_val(struct l_list *current_node, 
                                        const char *doc_name, const size_t len) {
	snprintf(current_node->obj, len, "%s", doc_name);
	/* Mark the next node as empty in case the current one is the last */
	current_node->next = NULL;
}


/* Check if to save the document name to the document list.
 * Return 1 for yes, 0 for no and -1 for error.
 */
static int check_save_doc_name(const char *doc_name, 
                               const char *str, const bool ignore_case) {
	/* If str is NULL, save every 
	   file name to the linked list */
	if(!str)
		return 1;
	
	if(ignore_case)
		return strstr_i(doc_name, str);

	else
		return (strstr(doc_name, str) != NULL) ? 
			1 : 0;
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

	*l_list_ptr1 = NULL;
	*l_list_ptr2 = NULL;

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


static char *get_doc_path(const char *dir_path, 
                          const char *full_doc_name, 
				          const bool recursive) {
	char *new_path, *ret_path; /* ret_path is the returned path from the recursion */
	struct dirent *entry;
	struct stat stbuf;
	DIR *dp;

	new_path = ret_path = NULL;
	
	if((dp = opendir_inf(dir_path)))
		/* 
		 * I decided to use goto here so it'll be easier to distinguish
		 * between the errors (which don't use goto) and the succes when a 
		 * document path is founded (whiich uses goto).
		 */
		while((entry = readdir_inf(dp))) {
			if(dot_entry(entry->d_name))
				continue;

			if(!(new_path = get_entry_path(dir_path, entry->d_name)))
				break;

			if(stat_inf(new_path, &stbuf))
				break;
			
			if(S_ISDIR(stbuf.st_mode) && recursive) {
				if((ret_path = get_doc_path(new_path, full_doc_name, recursive)))
					goto Out;
				
				else if(prev_error)
					break;
			}
			else if(S_ISREG(stbuf.st_mode) 
			 && strstr(new_path, full_doc_name))
				goto Out;
		
			free_and_null((void **) &new_path);
		}
	
	if(errno || prev_error)
		get_doc_path_error(&new_path, &ret_path);


	Out:
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


void prep_open_doc_argv(char **argv, const char *pdf_viewer, 
                        char *add_args, const char *doc_path) {
	unsigned int i = 0;

	argv[i++] = (char *) pdf_viewer;

	if(add_args)
		i = prep_add_args(argv, add_args, i);

	argv[i++] = (char *) doc_path;
	argv[i] = NULL;
}


/*
 * Prepare the additional arguments in the argv and return the index
 * of the next i.
 */
static unsigned int prep_add_args(char **argv, char *add_args, unsigned int i) {
	unsigned int ret;

	for(; (ret = space_to_null(add_args)); add_args+=ret)
		/* In case someone wrote more than one space;
		   which will lead to a segmentation fault */
		if(*add_args != '\0')
			argv[i++] = add_args;

	return i;
}


static int open_doc(char *const *argv) 
{
	return execvp_process(argv[0], argv);
}


int open_founded_doc_path(struct users_configs *configs, const char *doc_path) {
    const unsigned int argc = get_argc_val(configs->add_args);
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


void print_opening_doc(const char *doc_name, const bool color) {
	if(color)
		print_opening_doc_color(doc_name);
	else 
		print_opening_doc_no_color(doc_name);
}


static void print_opening_doc_color(const char *doc_name)
{
	printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "OPENING" ANSI_COLOR_BLUE "]" 
           ANSI_COLOR_RED " %s\n" ANSI_COLOR_RESET, 
           doc_name);
}


static void print_opening_doc_no_color(const char *doc_name)
{
	printf("[OPENING] %s\n", doc_name);
}


void print_docs_num(const struct l_list *doc_list, const bool color) {
    if(color)
		print_docs_num_color(doc_list);
	else
		print_docs_num_no_color(doc_list);
}


static void print_docs_num_color(const struct l_list *doc_list)
{
	printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "COUNTED" ANSI_COLOR_BLUE "]"
		   ANSI_COLOR_RED " %d Documents\n" ANSI_COLOR_RESET,
		   count_l_list_nodes(doc_list));
}


static void print_docs_num_no_color(const struct l_list *doc_list)
{
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
static void reorganize_l_list_alpha(struct l_list *unsorted_l_list, char **sorted_array) {
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


/*
 * The same as search_for_doc() but with multiple documents directory support.
 */
struct l_list *search_for_doc_multi_dir(const char *dirs_path, const char *str, 
                                        const bool ignore_case, const bool rec) {
	struct l_list *retval = NULL;
	char *dirs_path_cp; 

	if((dirs_path_cp = strcpy_dynamic(dirs_path))) {
		retval = search_for_doc_multi_dir_split(dirs_path_cp, str, 
				                                ignore_case, rec);	
		free(dirs_path_cp);
	}

	return retval;
}


/* Split dirs_path into one dir path at a time by converting
 * each space with a null byte, then check for documents with 
 * the sequence str in them one path at a time.
 */
static struct l_list *
search_for_doc_multi_dir_split(char *dirs_path, const char *str,
		                       const bool ignore_case, const bool rec) {
	struct l_list *doc_list_begin = NULL;
	struct l_list *current_node;
	unsigned int ret;

	for(; (ret = space_to_null(dirs_path)); dirs_path+=ret)
		if(*dirs_path != '\0') {
			if(!doc_list_begin) {
				if((doc_list_begin = search_for_doc(dirs_path, str, ignore_case, rec)))
					current_node = get_last_node(doc_list_begin);
			}
			else
				if((current_node->next = search_for_doc(dirs_path, str, ignore_case, rec)))
					current_node = get_last_node(current_node->next);

			if(prev_error) {
				search_for_doc_multi_dir_err(&doc_list_begin);
				break;
			}
		}

	return doc_list_begin;
}


static void free_and_null_l_list(struct l_list **ptr) {
	free_l_list(*ptr);
	*ptr = NULL;
}


static void search_for_doc_multi_dir_err(struct l_list **doc_list) {
	if(doc_list)
		free_and_null_l_list(doc_list);
}


char *get_doc_path_multi_dir(const char *dirs_path, 
                             const char *full_doc_name, 
                             const bool rec) {
	char *retval = NULL;
	char *dirs_path_cp;
	
	if((dirs_path_cp = strcpy_dynamic(dirs_path))) {
		retval = get_doc_path_multi_dir_split(dirs_path_cp, full_doc_name, rec)	;
		free(dirs_path_cp);
	}

	return retval;
}


/* Split dirs_path into one dir path at a time by converting
 * each space with a null byte, then search for the document
 * in the dirs paths.
 */
static char *get_doc_path_multi_dir_split(char *dirs_path_cp, 
		                                  const char *full_doc_name,
										  const bool rec) {
	char *doc_path = NULL;
	unsigned int ret;
	
	for(; (ret = space_to_null(dirs_path_cp)); dirs_path_cp+=ret)
		if(*dirs_path_cp != '\0')
			if((doc_path = get_doc_path(dirs_path_cp, full_doc_name, rec)) || prev_error)
				break;

	return doc_path;
}


static void *alloc_stat() 
{
	return malloc_inf(sizeof(struct stat));
}


static struct stat *get_stat(const char *path) {
	struct stat *statbuf;

	if((statbuf = alloc_stat()))
		if(!stat_inf(path, statbuf))
			free_and_null((void **) statbuf);

	return statbuf;
}


static struct meas_unit get_proper_size_format(const off_t bytes) {

}


static void print_doc_size(const off_t bytes) {
	const off_t gb = 1000000000;
	const off_t mb = 1000000;
	const off_t kb = 1000;

	if(bytes )
		;//Print in kb
	else if(bytes >= 100000)
		;//print in mb
	else 

}


void display_help(const char *name) {
	printf("Usage: %s  <options>  [argument]\n", name);
	printf(
	       "A command-line tool for managing your documents and easing your life.\n"
	       
		   "\n"
		   
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
	       " -d \t\t Display details on the documents with the passed string sequence.\n"
		   " -o \t\t Open the founded document with the passed string sequence.\n"
	       " -R \t\t Disable recursive searching for the documents.\n"
	       " -C \t\t Disable colorful output.\n"
           
		   "\n\n"
	       
		   "NOTES:\n"
		   "  1. It's good to note that the program has multiple directories support when\n"
           "     searching for a document. So when generating the configurations you can pass\n"
		   "     more than one directory absolute path which the program will search for\n"
		   "     documents in it at a run time. Please separate the paths with a space.\n"
		   "     Example: /path/to/dir1 /path/to/dir2 /path/to/dir3...\n"

           "\n"

	       "  2. When generating the configurations, if it's desired to pass additional\n"
		   "     arguments for the documents execution command, please separate them with\n"
		   "     a space. Example: --agr1 --arg2 --arg3...\n"
		   
		   "\n"

		   "  3. By default when using the -o option you can't open more than a document\n"
		   "     in a run, but you can use the -n option with it to give the program the\n"
		   "     approval to open more than one document in a run.\n"
		   
		   "\n"
		   
		   "  4. You can use the -a optoin with the -c, -d, -l and -o options instead\n"
	       "     of passing an actual argument.\n" 
		  );
}
