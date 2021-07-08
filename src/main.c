/*
----------------------------------------------------------
| License: GNU GPL-3.0                                   |
----------------------------------------------------------
| This file contains the main functions for the mdoc pr- |
| ogram.                                                 |
| This program's purposes are to help you managing your  |
| documents and especially while opening them, somehow   |
| like xdg-open.                                         |
----------------------------------------------------------
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
static int count_docs();
static void missing_arg_err(const int);
static void invalid_arg_err(const int);
static int generate_config_file();


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


static void missing_arg_err(const int opt) {
    fprintf(stderr, "%s: missing argument for the '-%c' option\n", prog_name_inf, opt);
    fprintf(stderr, "Try '%s -h' for more information.\n", prog_name_inf);
}


static void invalid_arg_err(const int opt) {
    fprintf(stderr, "%s: invalid option '-%c'\n", prog_name_inf, opt);
    fprintf(stderr, "Try '%s -h' for more information.\n", prog_name_inf);
}


static int generate_config_file() {
    char *config_path; 
    int retval = -1;

    if((config_path = get_config_path())) {
        if(!generate_config(config_path))
            retval = 0;

        free(config_path);
    }

    return retval;
}


int main(int argc, char **argv) {
    const char *valid_opt = ":hgsraic::l::o:RC";
    char *count_arg, *open_arg, *list_arg;
    struct users_configs *configs = NULL;
    struct l_list *doc_list = NULL;
    char *config_path = NULL;
    bool recursive = 1;
    bool reverse = 0;
    bool ignore = 0;
    bool color = 1;
    bool count = 0;
    bool list = 0;
    bool open = 0;
    bool sort = 0;
    bool all = 0;
    int retval = 0;
    int opt;

    prog_name_inf = argv[0];
   
    if(argc == 1)
        display_help(prog_name_inf);

    while((opt = getopt(argc, argv, valid_opt)) != EOF) {
        switch(opt) {
            case 'h':
                display_help(prog_name_inf);
                return retval;
            case 'g':
                retval = generate_config_file();
                return retval;
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
            case 'c':
                count = 1;
                count_arg = optarg;
                break;
            case 'l':
                list = 1;
                list_arg = optarg;
                break;
            case 'o':
                open = 1;
                open_arg = optarg;
                break;
            case 'R':
                recursive = 0;
                break;
            case 'C':
                color = 0;
                break;
            case ':':
                missing_arg_err(optopt);
                //goto Error;
            default:
                invalid_arg_err(optopt) ;
                //goto Error;
        }
    }

    if(count) {
        ;
    }

    CleanUp:
        if(config_path)
            free(config_path);
    
        if(configs)
            free_users_configs(configs);

    return retval;
}
