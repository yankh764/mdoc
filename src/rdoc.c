
/*
---------------------------------------------------------
| License: GPL-3.0                                      |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for the rdoc program.                                 |
---------------------------------------------------------
*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "rdoc.h"


enum magic_numbers {
	doc_name_len = 300, 
};

/* Functions Prototype */
static struct users_configs *input_configs();
static struct l_list *alloc_l_list_obj(unsigned int);
static struct l_list *search(const char *, const char *);
//static struct l_list *search_recursively(DIR *, struct l_list *, const char *);

/*                                                                   
* This function will ask the user 2 questions. The first one is the 
* documents directory absolute path. Second is the pdf viewer's name.
* Then it'll take the user's answers as an input.
*/
static struct users_configs *input_configs() { 
	struct users_configs *input; 

	if(!(input = malloc(sizeof(struct users_configs))))
		goto Error;

	printf("Please enter your documents directory absolute path: ");
	if(!(fgets(input->docs_dir_path, IN_LEN, stdin))) 
		goto Error;

	printf("\nPlease enter your pdf's viewer name: ");
	if(!(fgets(input->pdf_viewer, IN_LEN, stdin)))
		goto Error;

	return input;

	Error:
		printf("Error\n");
		if(input)
			free(input);
		return NULL;
}

/*
* The function will generate the configurations according to the 
* user's answers.
*/
int generate_config(const char *abs_config_path) {
	struct users_configs *configs;
	int retval = -1;
	FILE *fp;

	if(!(fp = fopen(abs_config_path, "w")))
		goto Out;

	if(!(configs = input_configs()))
		goto Out;

	fprintf(fp, "%s%s", configs->docs_dir_path, configs->pdf_viewer);
	printf("\nYour configurations were generated succesfully.\n");

	retval = 0;
	
	Out:
		if(configs)
			free(configs);
		if(fp)
			if(fclose(fp))
				retval = -1; 

		return retval;
}


int read_configs(const char *abs_config_path, struct users_configs *configs) {
	FILE *fp;
	int retval = -1;

	if(!(fp = fopen(abs_config_path, "r"))) 
		goto Out;

	if(!fgets(configs->docs_dir_path, IN_LEN, fp)) 
		goto Out;

	if(!fgets(configs->pdf_viewer, IN_LEN, fp)) 
		goto Out;

	retval = 0;

	Out:
		if(fp)
			if(fclose(fp))
				retval = -1;

		return retval;
}

/*
* Return a pointer to allocated l_list structure that also 
* has an allocated obj to it with the size of obj_size.
*/
static struct l_list *alloc_l_list_obj(unsigned int obj_size) {
	struct l_list *ptr;

	if(!(ptr = malloc(sizeof(struct l_list))))
		goto Error;
	
	if(!(ptr->obj = malloc(obj_size)))
		goto Error;

	return ptr;

	Error:
		if(ptr)
			free(ptr);

		return NULL;
}

void free_l_list(struct l_list *ptr) {
	struct l_list *current_ptr = ptr;
	struct l_list *prev_ptr;

	while(current_ptr) {
		if(current_ptr->obj)
			free(current_ptr->obj);
		
		prev_ptr = current_ptr;
		current_ptr = current_ptr->next;
		
		free(prev_ptr);
	}
}

static struct l_list *search(const char *dir_path, const char *str) {
	const unsigned int dir_path_len = strlen(dir_path);
	unsigned int new_path_len;
	struct l_list *retval = NULL;
	struct l_list *doc_list, *ptr;
	struct dirent *entry;
	struct stat stbuf;
	char *new_path;
	DIR *dp;

	if(!(dp = opendir(dir_path)))
		goto Out;

	if(!(doc_list = alloc_l_list_obj(doc_name_len)))
		goto Out;
	
	ptr = doc_list;

	while((entry = readdir(dp))) {
		/* Skip current and pervious directory entries and files
		   that don't have an substring occurrence of str        */ 
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") 
			|| !strstr(entry->d_name, str))
			continue;
		
		new_path_len = dir_path_len + strlen(entry->d_name) + 2;
		if(!(new_path = (char *) malloc(new_path_len)))
			goto CleanUp;

		snprintf(new_path, new_path_len, "%s/%s", dir_path, entry->d_name);
		if(!(stat(new_path, &stbuf)))
			goto CleanUp;
		
		free(new_path);
		new_path = NULL;

		if(!S_ISREG(stbuf.st_mode))
			continue; 

		if(!ptr)
			if(!(ptr = alloc_l_list_obj(doc_name_len)))
				goto CleanUp;

		snprintf(ptr->obj, doc_name_len, "%s", entry->d_name);
		ptr = ptr->next;
	}
	if(errno)
		goto CleanUp;

	retval = doc_list;

	Out:
		if(dp)
			if(closedir(dp))
				retval = NULL;

		return retval;

	CleanUp:
		if(new_path)
			free(new_path);
		free_l_list(doc_list);

		goto Out;
}

/*
* The function will search the passed str sequence in all the 
* existing documents in the documents directory. 
*/
int search_for_doc(const char *docs_dir_path, const char *str, unsigned int recursive) {

	if(recursive) {
		;
	}
	else {
		search(docs_dir_path, doc, str);
	}
	return 0;
}