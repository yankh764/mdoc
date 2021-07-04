/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for the config management of the mdoc program.        |
---------------------------------------------------------
*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "informative.h"
#include "input.h"
#include "config.h"


/* Static Functions Prototype */
static struct users_configs *input_configs();
static void write_configs(FILE *, const struct users_configs *);
static void *alloc_users_configs();


/*                                                                   
 * This function will ask the user 2 questions. The first one is the 
 * documents directory absolute path. Second is the pdf viewer's name.
 * Then it'll take the user's answers as an input.
 */
static struct users_configs *input_configs() { 
    struct users_configs *retval = NULL;
    struct users_configs *input;

	if((input = alloc_users_configs())) {
		input->docs_dir_path = input->pdf_viewer = input->add_args = NULL;
	
		printf("Please enter your documents directory absolute path: ");
		if(!(input->docs_dir_path = get_line(stdin))) 
			goto Out;

		printf("\nPlease enter your pdf's viewer absolute path: ");
		if(!(input->pdf_viewer = get_line(stdin)))
			goto Out;
		
		printf("\nPlease enter additional arguments for your pdf viewer (optional): ");
		/* Only fail if error detected since it's optional secition */
		if(!(input->add_args = get_line(stdin)) && errno)
			goto Out;

		retval = input;
	}

    Out:
        if(!retval && input)
			free_users_configs(input);

        return retval; 
}


static void *alloc_users_configs() 
{
	return malloc_inf(sizeof(struct users_configs));
}


/*
 * The function will generate the configurations according to the 
 * user's answers.
 */
int generate_config(const char *abs_config_path) {
	struct users_configs *configs = NULL;
	int retval = -1;
	FILE *fp;

	if((fp = fopen_inf(abs_config_path, "w"))) {
		if((configs = input_configs())) {
			
			write_configs(fp, configs);			
			free_users_configs(configs); 
			
			retval = 0;
		}
		else
			retval = -1;
		
		if(fclose_inf(fp))
			retval = -1;
	}	

	return retval;
}


static void write_configs(FILE *fp, const struct users_configs *configs) {
	fprintf(fp, "%s\n%s\n", configs->docs_dir_path, configs->pdf_viewer);
	
	if(configs->add_args)
		fprintf(fp, "%s\n", configs->add_args);
	
	printf("\nYour configurations were generated succesfully.\n");
}


struct users_configs *read_configs(const char *abs_config_path) {
	struct users_configs *retval = NULL;
	struct users_configs *configs = NULL;
	FILE *fp;
	
	if((fp = fopen_inf(abs_config_path, "r"))) {
		if((configs = alloc_users_configs())) {
			configs->docs_dir_path = configs->pdf_viewer = configs->add_args = NULL;

			if((configs->docs_dir_path = get_line(fp)) 
			 && (configs->pdf_viewer = get_line(fp))
			 && ((configs->add_args = get_line(fp))
			 || !errno))
				retval = configs;
		}
		else
			retval = NULL;

		if(fclose_inf(fp))
			retval = NULL;
	}

	if(!retval && configs)
		free_users_configs(configs);

	return retval;
}


void free_users_configs(struct users_configs *ptr) {
	if(ptr->docs_dir_path)
		free(ptr->docs_dir_path);
	if(ptr->pdf_viewer)
		free(ptr->pdf_viewer);
	if(ptr->add_args)
		free(ptr->add_args);
	
	free(ptr);
}

