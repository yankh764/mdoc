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
	float size_format;
	const char *unit_name;
};


/*--------------------------------*/
/*   Static Functions Prototype   */
/*--------------------------------*/
static char *get_entry_path(const char *, const char *);
static struct doc_list *get_last_node(const struct doc_list *);
static char *prep_open_doc_argv(char **, const char *, const char *, const char *);
static void adjust_doc_list_members(struct doc_list *, const char *, const char *, const struct stat *); 
static void display_doc_name_colorful(const char *);
static bool dot_entry(const char *); 
static bool if_save_doc(const char *, const char *, bool);
static void display_doc_name_no_color(const char *);
static unsigned int get_argc_val(const char *);
static void free_and_null(void **);
static struct doc_list *get_doc_list_alpha(struct doc_list **, const unsigned int);
static void *alloc_doc_list();
static int open_doc(char *const *);
static unsigned int prep_add_args(char **, char *, unsigned int);
static struct doc_list *search_for_doc(const char *, const char *, bool, bool);
static struct doc_list *search_for_doc_retval(struct doc_list *, struct doc_list *,  struct doc_list *);
static void print_docs_num_color(const unsigned int, const char *);
static void print_docs_num_no_color(const unsigned int, const char *);
static void print_opening_doc_color(const char *);
static void print_opening_doc_no_color(const char *);
static struct doc_list *search_for_doc_multi_dir_split(char *, const char *, bool, bool);
static void print_doc_size(off_t, bool);
static struct meas_unit get_proper_size_format(off_t);
static float bytes_to_gb(off_t); 
static float bytes_to_mb(off_t);
static float bytes_to_kb(off_t);
static struct meas_unit ret_proper_size_format(float, const char *); 
static void print_doc_size_color(const struct meas_unit);
static void print_doc_size_no_color(const struct meas_unit);
static void print_doc_path(const char *, bool);
static void print_doc_path_no_color(const char *);
static void print_doc_path_color(const char *);
static void print_last_mod_time(const char *, bool);
static void print_last_mod_time_color(const char *); 
static void print_last_mod_time_no_color(const char *);
static void print_doc_modes(const mode_t, bool);
static void print_doc_modes_color(const mode_t); 
static void print_doc_modes_no_color(const mode_t); 
static int save_proper_dir_content(const char *, const char*, bool, bool, struct doc_list **, struct doc_list **);
static struct doc_list *free_doc_list_node(struct doc_list *);
static struct doc_list *save_doc(const char *, const char *, const struct stat *);
static int save_doc_to_proper_var(const char *, const char *, const struct stat *,  struct doc_list **, struct doc_list **);
static void free_and_null_doc_list(struct doc_list **); 
static void catch_readdir_inf_err();
static void print_doc_name(const char *, bool);
static void print_doc_name_color(const char *);
static void print_doc_name_no_color(const char *);
static char *get_last_mod_time(const time_t);
static void *alloc_stat_struct();
static struct stat *get_stat_dynamic(const char *);
static void save_doc_list_nodes(const struct doc_list *, struct doc_list **);
static int sort_doc_list(struct doc_list **, struct doc_list **, const unsigned int);
static long int get_smallest_doc_name_i(struct doc_list **, const unsigned int);
static void adjust_smallest_val(char **, char **, unsigned int *, unsigned int);
static char *get_add_args_cp(const char *);



static void *alloc_doc_list() 
{
	return malloc_inf(sizeof(struct doc_list));
}


void free_doc_list(struct doc_list *ptr) 
{
	while ((ptr = free_doc_list_node(ptr)))
		;
}


/*
 * Free doc_list node and return a pointer to the next one.
 */
static struct doc_list *free_doc_list_node(struct doc_list *ptr)
{
	struct doc_list *next = ptr->next;

	free(ptr->path);
	free(ptr->name);
	free(ptr->stbuf);
	free(ptr);

	return next;
}


static char *get_entry_path(const char *dir_path, const char *entry_name) 
{
    const size_t path_len = strlen(dir_path) + strlen(entry_name) + 2;
    char *entry_path;

    if ((entry_path = malloc_inf(sizeof(char) * path_len)))
        snprintf(entry_path, path_len, "%s/%s", dir_path, entry_name);

    return entry_path;
}


static struct doc_list *get_last_node(const struct doc_list *ptr) 
{
	struct doc_list *prev;

	for (prev=NULL; ptr; ptr=ptr->next)
		prev = (void *) ptr;

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


static struct doc_list *search_for_doc(const char *dir_path, const char *str, 
                                       bool ignore_case, bool recursive) 
{
	struct doc_list *doc_list_rec_begin = NULL;
	struct doc_list *current_node_rec = NULL;
	struct doc_list *doc_list_begin = NULL; 
	struct doc_list *current_node = NULL;
	struct dirent *entry;
	struct stat *stbuf;
	char *new_path;
	DIR *dp;

	if ((dp = opendir_inf(dir_path))) {
		/* 
		 * To distinguish end of stream 
		 * from an error in readdir_inf()
		 */
		errno = 0;

		while ((entry = readdir_inf(dp))) {
			if (dot_entry(entry->d_name)) 
				continue;
			
			if (!(new_path = get_entry_path(dir_path, entry->d_name)))
				/* 
				 * The not passed (to a doc_list node) new_path and stbuf would
				 * be already freed at the end of the loop. Only structs of 
				 * doc_list may be allocated, so free them.
				 */
				goto free_docs_lists;
			
			if (!(stbuf = get_stat_dynamic(new_path)))
				/* 
				 * Failed to allocate stbuf, so free the not passed to a node
				 * new_path then continue to the rest of the errors labels.
				 */
				goto free_new_path;

			if (S_ISDIR(stbuf->st_mode)) {
				if (recursive)
					if (save_proper_dir_content(new_path, str, ignore_case, recursive, 
										        &doc_list_rec_begin, &current_node_rec))
						/*
						 * Both stbuf and new_path are allocated and not passed to
						 * a doc_list node. So jump to the first label on the errors
						 * cleanup and start freeing all the allocated variables.
						 */
						goto free_stbuf;
			} else if (S_ISREG(stbuf->st_mode)) {
				if (if_save_doc(entry->d_name, str, ignore_case)) {
					if (save_doc_to_proper_var(new_path, entry->d_name, stbuf, 
											   &doc_list_begin, &current_node))
						/*
						 * Both stbuf and new_path are allocated and not passed to
						 * a doc_list node. So jump to the first label on the errors
						 * cleanup and start freeing all the allocated variables.
						 */
						goto free_stbuf;

					continue;
				} 
			} 
			free(new_path);
			free(stbuf);
		}
		catch_readdir_inf_err();

		if (closedir_inf(dp) || prev_error) {
			dp = NULL;
			/* 
			 * The not passed (to a doc_list node) new_path and stbuf would
			 * be already freed at the end of the loop. Only structs of 
			 * doc_list may be allocated, so free them.
			 */
			goto free_docs_lists;
		}
	} else {
		/* 
		 * No variables were allocated, just mark
		 * that an error occured and return NULL.
		 */
		goto error;	
	}

	return search_for_doc_retval(current_node, doc_list_begin, doc_list_rec_begin);

/* ERRORS CLEANUP */
free_stbuf:
	free(stbuf);
free_new_path:
	free(new_path);
free_docs_lists:
	if (doc_list_begin)
		free_doc_list(doc_list_begin);
	if (doc_list_rec_begin)
		free_doc_list(doc_list_rec_begin);
error:	
	if (dp)
		closedir_inf(dp);
	
	prev_error = 1;
	
	return NULL;
}


static struct stat *get_stat_dynamic(const char *path)
{
	struct stat *stbuf;

	if ((stbuf = alloc_stat_struct()))
		if (stat_inf(path, stbuf))
			free_and_null((void **) &stbuf);

	return stbuf;
}


static void *alloc_stat_struct()
{
	return malloc_inf(sizeof(struct stat));
}


static int save_doc_to_proper_var(const char *doc_path, const char *doc_name,
								  const struct stat *stbuf,
								  struct doc_list **doc_list_begin, 
								  struct doc_list **current_node)
{
	if (!(*doc_list_begin)) {
		if (!(*doc_list_begin = save_doc(doc_path, doc_name, stbuf)))
			return -1;

		*current_node = *doc_list_begin;
	} else {
		if (!((*current_node)->next = save_doc(doc_path, doc_name, stbuf)))
			return -1;

		*current_node = (*current_node)->next;
	}

	return 0;
}


static void adjust_doc_list_members(struct doc_list *current_node, 
                                    const char *doc_path, 
									const char *doc_name, 
								    const struct stat *statbuf) 
{
	current_node->path = (char *) doc_path;
	current_node->name = (char *) doc_name;
	current_node->stbuf = (void *) statbuf;
	/* 
	 * Mark the next node as empty in case 
	 * the current one is the last one 
	 */
	current_node->next = NULL;
}


static struct doc_list *save_doc(const char *doc_path, 
								 const char *doc_name, 
								 const struct stat *stbuf)
{
	struct doc_list *node;
	char *doc_name_cp;

	if ((node = alloc_doc_list())) {
		if((doc_name_cp = strcpy_dynamic(doc_name)))
			adjust_doc_list_members(node, doc_path, doc_name_cp, stbuf);
		else
			free_and_null((void **) &node);
	}

	return node;
}


/* Check if to save the document name to the document list.
 * Return 1 for yes, 0 for no and -1 for error.
 */
static bool if_save_doc(const char *doc_name, 
                        const char *str, bool ignore) 
{
	/* If str is NULL, save every 
	   file name to the linked list */
	if (!str)
		return 1;
	
	if (ignore)
		return strstr_i(doc_name, str);

	return strstr(doc_name, str) ? 
		1 : 0;
}


static struct doc_list *search_for_doc_retval(struct doc_list *current_node, 
                                              struct doc_list *doc_list_begin,
                                              struct doc_list *doc_list_rec_begin) 
{
	if (doc_list_begin) {
		if (doc_list_rec_begin)
			current_node->next = doc_list_rec_begin;
		return doc_list_begin;
	
	} else if (doc_list_rec_begin) {
		return doc_list_rec_begin;
	
	} else {
		return NULL;
	}
}


static void catch_readdir_inf_err()
{
	if (errno)
		prev_error = 1;
}


void display_doc_name(const char *name, bool color_status) 
{
		if (color_status)
			display_doc_name_colorful(name);
		else
			display_doc_name_no_color(name);

}


static void display_doc_name_colorful(const char *name) 
{
	printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "+" ANSI_COLOR_BLUE "]" 
		   ANSI_COLOR_RED " %s" ANSI_COLOR_RESET "\n", 
		   name);
}


static void display_doc_name_no_color(const char *name) 
{
	printf("[+] %s\n", name);
}


unsigned int count_doc_list_nodes(const struct doc_list *ptr) 
{
	unsigned int i;

	for (i=0; ptr; i++)
		ptr = ptr->next;

	return i;
}


static void free_and_null(void **ptr) 
{
	free(*ptr);
	*ptr = NULL;
}


/*
 * The funtion will return the address of the dynamic copy of add_args
 * (if there was any) so open_founded_doc_path() could free it after 
 * finishing from the argv.
 */
static char *prep_open_doc_argv(char **argv, const char *pdf_viewer, 
                                const char *add_args, const char *doc_path) 
{
	char *add_args_cp = NULL;
	unsigned int i = 0;

	argv[i++] = (char *) pdf_viewer;

	if (add_args)
		if ((add_args_cp = get_add_args_cp(add_args)))
			i = prep_add_args(argv, add_args_cp, i);

	argv[i++] = (char *) doc_path;
	argv[i] = NULL;

	return add_args_cp;
}


static char *get_add_args_cp(const char *add_args)
{
	char *add_args_cp;

	if (!(add_args_cp = strcpy_dynamic(add_args)))
		/* To distinguish between an error and empty add_args */
		prev_error = 1;

	return add_args_cp;
}


/*
 * Prepare the additional arguments in the argv and return the index
 * of the next i.
 */
static unsigned int prep_add_args(char **argv, char *add_args, unsigned int i) 
{
	unsigned int ret;

	for (; (ret = space_to_null(add_args)); add_args+=ret)
		/* In case someone wrote more than one space;
		   which will lead to a segmentation fault */
		if (*add_args != '\0')
			argv[i++] = add_args;

	return i;
}


static int open_doc(char *const *argv) 
{
	return execvp_process(argv[0], argv);
}


int open_doc_path(const struct users_configs *configs, const char *doc_path) 
{
    const unsigned int argc = get_argc_val(configs->add_args);
    char *argv[argc+1];
	char *add_args_cp;
	int retval = -1;

	add_args_cp = prep_open_doc_argv(argv, configs->pdf_viewer, 
									 configs->add_args, doc_path);
	if (!prev_error) {
		retval = open_doc(argv);

		if (add_args_cp)
			free(add_args_cp);
	}

    return retval;
}


static unsigned int get_argc_val(const char *add_args) 
{
	/* The 2 stands for doc_path and pdf_viewer */
	unsigned int val = 2;

	if (add_args)
		val += count_words(add_args);

	return val;
}


void print_opening_doc(const char *doc_name, bool color) 
{
	if (color)
		print_opening_doc_color(doc_name);
	else 
		print_opening_doc_no_color(doc_name);
}


/*
 * The main reason I chose to follow this method (using another functions)
 * for printing colored outputs is to decrease the confusion that
 * may occur while seeing a printf() call with lots of macros.
 */
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


void print_docs_num(const unsigned int docs_num, bool color) 
{
	const char *file = (docs_num == 1) ? 
        "File" : "Files";

	if (color)
		print_docs_num_color(docs_num, file);
	else
		print_docs_num_no_color(docs_num, file);
}


static void print_docs_num_color(const unsigned int num, 
		                         const char *file_word)
{
	printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "COUNTED" ANSI_COLOR_BLUE "]"
		   ANSI_COLOR_RED " %d %s\n" ANSI_COLOR_RESET,
		   num, file_word);
}


static void print_docs_num_no_color(const unsigned int num, 
		                            const char *file_word) 
{
	printf("[COUNTED] %d %s\n", num, file_word);
}


/*
 * Return a pointer to a rearranged ptr
 */
struct doc_list *sort_docs_names_alpha(const struct doc_list *ptr) 
{
	const unsigned int nodes_num = count_doc_list_nodes(ptr);
	struct doc_list *unsorted_array[nodes_num];
	struct doc_list *sorted_array[nodes_num];

	save_doc_list_nodes(ptr, unsorted_array);

	if (sort_doc_list(unsorted_array, sorted_array, nodes_num))
		return NULL;		

	return get_doc_list_alpha(sorted_array, nodes_num);
}


static int sort_doc_list(struct doc_list **unsorted_array, 
						 struct doc_list **sorted_array, 
						 const unsigned int nodes_num)
{
	unsigned int i;
	long int ret;

	for (i=0; i<nodes_num; i++) {
		if ((ret = get_smallest_doc_name_i(unsorted_array, nodes_num)) == -1)
			return -1;

		sorted_array[i] = unsorted_array[ret];
		unsorted_array[ret] = NULL;
	}

	return 0;
}


static long int get_smallest_doc_name_i(struct doc_list **array, 
										const unsigned int nodes_num)
{
	/* 
     * I initialized smallest_i to zero get rid of the 
	 * unaccurate -Wmaybe-uninitialized warning when 
	 * compiling with GCC.
     */
	unsigned int smallest_i = 0;
	char *smallest = NULL;
	char *current;
	unsigned int i;
	
	for (i=0; i<nodes_num; i++) {
		if (!array[i])
			continue;

		if (smallest) {
			if (!(current = small_let_copy(array[i]->name)))
				/* 
				 * smallest is allocated, so goto 
				 * fail_alloc_current and free it
				 */
				goto fail_alloc_current;
			/* If current word needs to come before smallest word */
			if(alpha_cmp(smallest, current))
				adjust_smallest_val(&smallest, &current, &smallest_i, i);
			else
				free(current);
		} else {
			if (!(smallest = small_let_copy(array[i]->name)))
				goto error;

			smallest_i = i;
		}
	}
	free(smallest);

	return smallest_i;

fail_alloc_current:
	free(smallest);
error:
	return -1;
}


static void adjust_smallest_val(char **smallest, char **current, 
								unsigned int *smallest_i, unsigned int current_i)
{
	free(*smallest);
	*smallest = *current;
	*smallest_i = current_i;
}


static struct doc_list *get_doc_list_alpha(struct doc_list **sorted_array, 
										   const unsigned int nodes_num) 
{
	struct doc_list *sorted = NULL;
	struct doc_list *current_node;
	unsigned int i;

	for (i=0; i<nodes_num; i++) {
		if (!sorted)
			current_node = sorted = sorted_array[i];
		else
			current_node = current_node->next = sorted_array[i];
		/* In case this node is the last one */
		current_node->next = NULL;
	}

	return sorted;
}


/*
 * Return a pointer to a reversed ptr
 */
struct doc_list *reverse_doc_list(const struct doc_list *ptr) 
{
	const unsigned int nodes_num = count_doc_list_nodes(ptr);
	struct doc_list *nodes_array[nodes_num];
	struct doc_list *reversed = NULL;
	struct doc_list *current_node;
	long int i;

	save_doc_list_nodes(ptr, nodes_array);
	
	for (i=nodes_num-1; i>-1; i--) {
		if (!reversed)
			current_node = reversed = nodes_array[i];
		else 
			current_node = current_node->next = nodes_array[i];
		/* In case this node is the last one */
		current_node->next = NULL;
	}

	return reversed;
}


static void save_doc_list_nodes(const struct doc_list *ptr, 
								struct doc_list **nodes_array)
{
	unsigned int i;

	for (i=0; ptr; ptr=ptr->next)
		nodes_array[i++] = (void *) ptr;
}


struct doc_list *search_for_doc_multi_dir(const char *dirs_path, const char *str, 
                                          bool ignore_case, bool rec) 
{
	struct doc_list *list = NULL;
	char *dirs_path_cp; 

	if ((dirs_path_cp = strcpy_dynamic(dirs_path))) {
		list = search_for_doc_multi_dir_split(dirs_path_cp, str, ignore_case, rec);	
		free(dirs_path_cp);
	}

	return list;
}


/* Split dirs_path into one dir path at a time by converting
 * each space with a null byte, then check for documents with 
 * the sequence str in them one path at a time.
 */
static struct doc_list *search_for_doc_multi_dir_split(char *dirs_path, 
													   const char *str,
													   bool ignore_case, 
													   bool rec)
{
	struct doc_list *doc_list_begin = NULL;
	struct doc_list *current_node;
	unsigned int ret;

	for (; (ret = space_to_null(dirs_path)); dirs_path+=ret)
		if (*dirs_path != '\0') 
			if (save_proper_dir_content(dirs_path, str, ignore_case, rec, 
			 					        &doc_list_begin, &current_node)) {
				if (doc_list_begin)
					free_and_null_doc_list(&doc_list_begin);

				break;
			}

	return doc_list_begin;
}


static void free_and_null_doc_list(struct doc_list **ptr) 
{
	free_doc_list(*ptr);
	*ptr = NULL;
}


static struct meas_unit get_proper_size_format(off_t bytes) 
{
	const off_t gb = 1000000000;
	const off_t mb = 1000000;
	const off_t kb = 1000;

	if (bytes >= gb) 
		return ret_proper_size_format(bytes_to_gb(bytes), "GB");

	else if (bytes >= mb)
		return ret_proper_size_format(bytes_to_mb(bytes), "MB");

	else if (bytes >= kb)
		return ret_proper_size_format(bytes_to_kb(bytes), "KB");

	else 
		return ret_proper_size_format(bytes, "B");
}


static struct meas_unit ret_proper_size_format(float size_format, 
	                                           const char *unit_name) 
{
	struct meas_unit retval;

	retval.size_format = size_format;
	retval.unit_name = unit_name;

	return retval; 
}


static float bytes_to_gb(off_t bytes) 
{
	return (float) bytes / 1000000000.0; 
}


static float bytes_to_mb(off_t bytes) 
{
	return (float) bytes / 1000000.0;
}


static float bytes_to_kb(off_t bytes) 
{ 
	return (float) bytes / 1000.0;
}


static void print_doc_size(off_t bytes, bool color)  
{
	const struct meas_unit format = get_proper_size_format(bytes);

	if (color)
		print_doc_size_color(format);
	else
		print_doc_size_no_color(format);
}


/* REMINDER:
 *
 * The main reason I chose to follow this method (using another functions)
 * for printing colored outputs is because to decrease the confusion that
 * may occur while seeing a printf() call with lots of macros.
 */
static void print_doc_size_color(const struct meas_unit format) 
{
	printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "SIZE" ANSI_COLOR_BLUE "]"
		   ANSI_COLOR_RED " %0.1f %s\n" ANSI_COLOR_RESET, 
		   format.size_format, format.unit_name);
}


static void print_doc_size_no_color(const struct meas_unit format) 
{
	printf("[SIZE] %0.1f %s\n", format.size_format, format.unit_name);	
} 


static void print_doc_path(const char *doc_path, bool color)
{
	if (color)
		print_doc_path_color(doc_path);	
	else
		print_doc_path_no_color(doc_path);
}


static void print_doc_path_color(const char *doc_path)
{
	printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "PATH" ANSI_COLOR_BLUE "]"
		   ANSI_COLOR_RED " %s\n" ANSI_COLOR_RESET, doc_path);
}


static void print_doc_path_no_color(const char *doc_path)
{
	printf("[PATH] %s\n", doc_path);
}


int print_doc_details(const struct doc_list *list, bool color) 
{
	char *time_buf;

	if(!(time_buf = get_last_mod_time(list->stbuf->st_mtime)))
		return -1;

	print_doc_path(list->path, color);
	print_doc_name(list->name, color);
	print_last_mod_time(time_buf, color);
	print_doc_modes(list->stbuf->st_mode, color);
	print_doc_size(list->stbuf->st_size, color);
	free(time_buf);

	return 0;
}


static void print_last_mod_time(const char *buffer, bool color) 
{
	if (color)
		print_last_mod_time_color(buffer);
	else 
		print_last_mod_time_no_color(buffer);
}


static char *get_last_mod_time(const time_t timep)
{
	/*
	 * According to the Linux Man pages (man ctime(3), line 82) when using
	 * using the function ctime_r(), the char buffer must have room for
	 * at least 26 bytes. I decided I'll give it room for 35 bytes.
	 */
	const unsigned int size = 35;
	char *buffer;

	if ((buffer = malloc_inf(sizeof(char) * size)))
		if (!ctime_r_inf(&timep, buffer))
			free_and_null((void **) &buffer);

	return buffer;
}


static void print_last_mod_time_color(const char *buffer) 
{
	printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "TIME" ANSI_COLOR_BLUE "]"
		   ANSI_COLOR_RED " %s" ANSI_COLOR_RESET, 
		   buffer);	
}


static void print_last_mod_time_no_color(const char *buffer) 
{
	printf("[TIME] %s", buffer);	
}

static void print_doc_modes(const mode_t mode, bool color) 
{
	if (color)
		print_doc_modes_color(mode);
	else
		print_doc_modes_no_color(mode);
}


static void print_doc_name(const char *name, bool color)
{
	if (color)
		print_doc_name_color(name);
	else
		print_doc_name_no_color(name);
}


static void print_doc_name_color(const char *name)
{
	printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "NAME" ANSI_COLOR_BLUE "]"
		   ANSI_COLOR_RED " %s\n" ANSI_COLOR_RESET, 
		   name);	
}


static void print_doc_name_no_color(const char *name)
{
	printf("[NAME] %s\n", name);	
}


static void print_doc_modes_color(const mode_t mode) 
{
	printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "MODE" ANSI_COLOR_BLUE "]"
		   ANSI_COLOR_RED " -%c%c%c%c%c%c%c%c%c\n" ANSI_COLOR_RESET, 
		   ((mode & S_IRUSR) ? 'r' : '-'), 
		   ((mode & S_IWUSR) ? 'w' : '-'),
		   ((mode & S_IXUSR) ? 'x' : '-'),
		   ((mode & S_IRGRP) ? 'r' : '-'),
		   ((mode & S_IWGRP) ? 'w' : '-'),
		   ((mode & S_IXGRP) ? 'x' : '-'), 
		   ((mode & S_IROTH) ? 'r' : '-'), 
		   ((mode & S_IWOTH) ? 'w' : '-'), 
		   ((mode & S_IXOTH) ? 'x' : '-')
		  );
}


static void print_doc_modes_no_color(const mode_t mode) 
{
	printf("[MODE] -%c%c%c%c%c%c%c%c%c\n", 
		   ((mode & S_IRUSR) ? 'r' : '-'), 
		   ((mode & S_IWUSR) ? 'w' : '-'),
		   ((mode & S_IXUSR) ? 'x' : '-'),
		   ((mode & S_IRGRP) ? 'r' : '-'),
		   ((mode & S_IWGRP) ? 'w' : '-'),
		   ((mode & S_IXGRP) ? 'x' : '-'), 
		   ((mode & S_IROTH) ? 'r' : '-'), 
		   ((mode & S_IWOTH) ? 'w' : '-'), 
		   ((mode & S_IXOTH) ? 'x' : '-')
		  );
}


static int save_proper_dir_content(const char *dir_path,
								   const char *str,
								   bool ignore, bool rec,
								   struct doc_list **beginning,
								   struct doc_list **current_node)
{
	if (!(*beginning)) {
		if ((*beginning = search_for_doc(dir_path, str, ignore, rec)))
			*current_node = get_last_node(*beginning);
	} else {
		if (((*current_node)->next = search_for_doc(dir_path, str, ignore, rec)))
			*current_node = get_last_node(*current_node);
	}

	return prev_error ?
		-1 : 0;
}


void display_help(const char *name) 
{
	printf("Usage: %s [OPTIONS]... ARGUMENT\n", name);
	printf(
	       "A command-line tool for managing your documents and easing your life.\n"
	       
		   "\n"
		   
		   "Available options:\n"
	       " -h \t\t Display this help message\n"
	       " -g \t\t Generate new configurations file\n"
	       " -s \t\t Sort the founded documents alphabetically\n"
	       " -r \t\t Reverse the order of the founded documents\n"
	       " -a \t\t Include all documents\n"
	       " -i \t\t Ignore case distinctions while searching for the documents\n"
	       " -n \t\t Allow numerous documents opening (execution)\n"
	       " -c \t\t Count the existing documents with the passed string sequence in their names\n"
	       " -l \t\t List the existing documents with the passed string sequence in their names\n"
	       " -d \t\t Display details on the documents with the passed string sequence in their names\n"
		   " -o \t\t Open the founded document with the passed string sequence in it's name\n"
	       " -R \t\t Disable recursive searching for the documents\n"
	       " -C \t\t Disable colorful output\n"
           
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
		   "     a space. Example: --arg1 --arg2 --arg3...\n"
		   
		   "\n"

		   "  3. By default when using the -o option you can't open more than a document\n"
		   "     in a run, but you can use the -n option with it to give the program the\n"
		   "     approval to open more than one document in a run.\n"
		   
		   "\n"
		   
		   "  4. You can use the -a optoin with the -c, -l, -d and -o options instead\n"
	       "     of passing an actual argument.\n" 

		   "\n"

		   "  5. The [TIME] section in the -d option stands for the last modification\n"
		   "     time, or if the document haven't been modified once, it'll stand for\n"
		   "     the creation time of the document.\n"

		   "\n\n"

		   "EXIT CODES:\n"
		   " 0   Success\n"
		   " 1   Error in the command line syntax\n"
		   " 2   General error in the program\n"
		  );
}
