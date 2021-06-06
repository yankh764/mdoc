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

	if(!(input = malloc_inf(sizeof(struct users_configs))))
		goto Out;

	printf("Please enter your documents directory absolute path: ");
	if(!fgets_inf(input->docs_dir_path, IN_LEN, stdin)) 
		goto Out;

	printf("\nPlease enter your pdf's viewer name: ");
	if(!fgets_inf(input->pdf_viewer, IN_LEN, stdin))
		goto Out;

    retval = input;

    Out:
        if(!retval)
            if(input)
                free(input);
        
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

	if(!(fp = fopen_inf(abs_config_path, "w")))
		goto Out;

	if(!(configs = input_configs()))
		goto Out;

	fprintf(fp, "%s%s", configs->docs_dir_path, configs->pdf_viewer);
	printf("\nYour configurations were generated succesfully.\n");

	retval = 0;
	
	Out:
		if(fp)
			if(fclose_inf(fp))
				retval = -1;
		if(configs)
			free(configs); 

		return retval;
}


struct users_configs *read_configs(const char *abs_config_path) {
	struct users_configs *retval = NULL;
	struct users_configs *configs = NULL;
	FILE *fp;
	
	if(!(fp = fopen_inf(abs_config_path, "r"))) 
		goto Out;
	
	if(!(configs = malloc_inf(sizeof(struct users_configs))))
		goto Out;

	if(!fgets_inf(configs->docs_dir_path, IN_LEN, fp)) 
		goto Out;

	if(!fgets_inf(configs->pdf_viewer, IN_LEN, fp)) 
		goto Out;

	retval = configs;

	Out:
		if(fp)
			if(fclose_inf(fp))
				retval = NULL;
		/* If some error occured */
		if(!retval)
			if(configs) 
				free(configs);

		return retval;
}
