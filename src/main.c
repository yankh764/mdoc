/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for the mdoc program.                                 |
---------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "informative.h"
#include "mdoc.h"


const char *prog_name_inf;

/* Static Functions Prototype */
static char *get_config_path();


static char *get_config_path() {
    const char *config_path = ".config/mdoc";
    char *abs_config_path;
    char *retval = NULL;
    char *home; 
    size_t len;

    if((home = getenv_inf("HOME"))) {
        len = strlen(config_path) + strlen(home) + 2;
        
        if((abs_config_path = malloc_inf(len)))
            snprintf(abs_config_path, len, "%s/%s", home, config_path);

        retval = abs_config_path;
    }
    
    return retval;
}


int main(int argc, char **argv) {
    const char *valid_opt = ":hgsraic:l:o:RC";
    char *config_path = NULL;
    bool recursive = 1;
    bool reverse = 0;
    bool ignore = 0;
    bool color = 1;
    bool count = 0;
    bool list = 0;
    bool open = 0;
    bool help = 1;
    bool sort = 0;
    bool all = 0;
    int retval, opt;

    prog_name_inf = argv[0];

    while((opt = getopt(argc, argv, valid_opt)) != EOF) {
        switch(opt) {
            case 's':
                sort = 1;
                break;
            case 'r':
                reverse = 1;
                break;
            case 'a':
                all = 1;
                break;
            case 'i':
                ignore = 1;
                break;
            case 'C':
                color = 0;
                break;
            case 'R':
                recursive = 0;
                break;
            case 'c':
                
            /* Accept no other options if -h or -g is passed */ 
            case 'h':
                display_help(prog_name_inf);
                goto Out;
            
            case 'g':
                if((config_path = get_config_path())) {
                    if(generate_config(config_path))
                        goto CleanUp;
                }
                else 
                    retval = 1; 
                
                goto Out;    
        }
    }
    
    if(help)
        display_help(prog_name_inf);

    Out:
        return retval;

    CleanUp:
        if(config_path)
            free(config_path);

        goto Out;
}
