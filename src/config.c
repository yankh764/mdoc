/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for the config management of the rdoc program.        |
---------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include "informative.h"
#include "input.h"
#include "config.h"


/* Static Functions Prototype */
static struct users_configs *input_configs();


/*                                                                   
 * This function will ask the user 2 questions. The first one is the 
 * documents directory absolute path. Second is the pdf viewer's name.
 * Then it'll take the user's answers as an input.
 */
static struct users_configs *input_configs() { 
    struct users_configs *retval = NULL;
    struct users_configs *input;

	if((input = malloc_inf(sizeof(struct users_configs)))) {
		input->docs_dir_path = input->pdf_viewer = NULL;
	
		printf("Please enter your documents directory absolute path: ");
		if(!(input->docs_dir_path = get_line(stdin))) 
			goto Out;

		printf("\nPlease enter your pdf's viewer name: ");
		if(!(input->pdf_viewer = get_line(stdin)))
			goto Out;

		retval = input;
	}

    Out:
        if(!retval && input) {
			if(input->pdf_viewer)
				free(input->pdf_viewer);
			if(input->docs_dir_path)
				free(input->docs_dir_path);
				
			free(input);
		}
        
        return retval; 
}


/*
 * The function will generate the configurations according to the 
 * user's answers.
 */
int generate_config(const char *abs_config_path) {
	struct users_configs *configs = NULL;
	int retval = -1;
	FILE *fp;

	if((fp = fopen_inf(abs_config_path, "w")))
		if((configs = input_configs())) {
			fprintf(fp, "%s\n%s\n", configs->docs_dir_path, configs->pdf_viewer);
			printf("\nYour configurations were generated succesfully.\n");
		
			retval = 0;
		}

	if(fp)
		if(fclose_inf(fp))
			retval = -1;
	if(configs)
		free_users_configs(configs); 
		
	return retval;
}


struct users_configs *read_configs(const char *abs_config_path) {
	struct users_configs *retval = NULL;
	struct users_configs *configs = NULL;
	FILE *fp;
	
	if((fp = fopen_inf(abs_config_path, "r")))
		if((configs = malloc_inf(sizeof(struct users_configs)))) {
			configs->docs_dir_path = configs->pdf_viewer = NULL;
	
			/* If successfully read the 2 lines of configs */
			if((configs->docs_dir_path = get_line(fp)) && 
					   (configs->pdf_viewer = get_line(fp))) 
				retval = configs;
		}

	if(fp)
		if(fclose_inf(fp))
			retval = NULL;

	if(!retval && configs) {  
		if(configs->pdf_viewer)
			free(configs->pdf_viewer);
		if(configs->docs_dir_path)
			free(configs->docs_dir_path);
				
		free(configs);
	}
		
	return retval;
}


/*
 * I'm going to use this only when I'm 100% sure that everything in the struct
 * is allocated, because if not there is a potential that free() will try
 * to free unallocated memory address or a memory address that exists on the
 * stack.
 */
void free_users_configs(struct users_configs *ptr) {
	free(ptr->pdf_viewer);
	free(ptr->docs_dir_path);
	free(ptr);
}

