
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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "rdoc.h"

/* Functions Prototype */
static struct users_configs *input_configs();
static struct l_list *alloc_l_list_obj(size_t);
static struct l_list *search(const char *, const char *, unsigned int);
static int strstr_i(const char *, const char *);
static char *convert_to_lower(char *);
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
		if(fp)
			if(fclose(fp))
				retval = -1;
		if(configs)
			free(configs); 

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
static struct l_list *alloc_l_list_obj(size_t obj_size) {
	struct l_list *ptr;

	if(!(ptr = malloc(sizeof(struct l_list))))
		goto Error;
	
	if(!(ptr->obj = (char *) malloc(obj_size)))
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

static struct l_list *search(const char *dir_path, const char *str,
							unsigned int ignore_case) {
	const unsigned int dir_path_len = strlen(dir_path);
	unsigned int new_path_len;
	struct l_list *doc_list = NULL;
	struct l_list *retval = NULL;
	struct l_list *ptr;
	struct dirent *entry;
	struct stat stbuf;
	char *new_path;
	int status;
	size_t obj_len;
	DIR *dp;

	if(!(dp = opendir(dir_path))) 
		goto Out;

	while((entry = readdir(dp))) {
		/* Skip current and pervious directory entries. */ 
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
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

		new_path_len = dir_path_len + strlen(entry->d_name) + 2;
		if(!(new_path = (char *) malloc(new_path_len)))
			goto CleanUp;

		snprintf(new_path, new_path_len, "%s/%s", dir_path, entry->d_name);

		if((stat(new_path, &stbuf)))
			goto CleanUp;
		
		free(new_path);
		new_path = NULL;

		if(!S_ISREG(stbuf.st_mode))
			continue; 

		obj_len = strlen(entry->d_name) + 1;
		if(!doc_list) {
			if(!(doc_list = alloc_l_list_obj(obj_len)))
				goto Out;
			
			snprintf(doc_list->obj, obj_len, "%s", entry->d_name);
			ptr = doc_list;
			continue;
		}

		ptr = ptr->next = alloc_l_list_obj(obj_len);
		if(!ptr)
			goto CleanUp;

		snprintf(ptr->obj, obj_len, "%s", entry->d_name);
	}

	if(errno) 
		goto CleanUp;

	retval = doc_list;

	Out:
		if(dp)
			if(closedir(dp))
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

		if(doc_list)
			free_l_list(doc_list);

		goto Out;
}

static char *convert_to_lower(char *str) {
	size_t len = strlen(str) + 1;
	unsigned int i;
	char *str_c; // Copy of str

	if((str_c = (char *) malloc(len))) {
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
	int retval = -1;
	char *haystack_c, *needle_c; 
	
	if(!(haystack_c = convert_to_lower((char *) haystack)))
		goto Out;

	if(!(needle_c = convert_to_lower((char *) needle)))
		goto Out;
	
	if(strstr(haystack_c, needle_c))
		retval = 1;
	else 
		retval = 0;

	Out:
		if(haystack_c)
			free(haystack_c);	
		if(needle_c)
			free(needle_c);

		return retval;
}

/*
* The function will search the passed str sequence in all the 
* existing documents in the documents directory. 
*/
struct l_list *search_for_doc(const char *docs_dir_path, const char *str,
							  unsigned int ignore_case, unsigned int recursive) {

	if(recursive) {
		;
	}
	else {
		;
	}

	return search(docs_dir_path, str, ignore_case);
}