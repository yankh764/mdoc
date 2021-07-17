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
static void missing_arg_err(const int);
static void invalid_arg_err(const int);
static int generate_opt();
static struct users_configs *get_configs();
static int count_opt(const char *, const char *, 
                     const bool, const bool);
static int list_opt(const char *, const char *,
                    const bool, const bool, const bool);


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


static int generate_opt() {
    char *config_path; 
    int retval = -1;

    if((config_path = get_config_path())) {
        if(!generate_config(config_path))
            retval = 0;

        free(config_path);
    }

    return retval;
}


static struct users_configs *get_configs() {
    struct users_configs *retval = NULL;
    char config_path;

    if((config_path = get_config_path())) {
        retval = read_configs(config_path);
        free(config_path);
    }

    return retval;
}


static int count_opt(const char *dir_path, const char *arg, 
                     const bool ignore, const bool rec) {
    struct l_list *doc_list = NULL;
    unsigned int doc_num;
    int retval = -1;

    doc_list = search_for_doc(dir_path, arg, ignore, rec);
    
    if(!prev_error) {
        doc_num = count_l_list_nodes(doc_list);

        printf("%d documents were found.\n", doc_num);

        if(doc_list)
            free_l_list(doc_list);

        retval = 0;
    }

    return retval;
}


static int list_opt(const char *dir_path, const char *arg,
                    const bool ignore, const bool rec, 
                    const bool color) {
    struct l_list *doc_list = NULL;
    int retval = -1;

    doc_list = search_for_doc(dir_path, arg, ignore, rec);
    
    if(!prev_error) {
        if(doc_list) 
            display_docs(doc_list, color);
    }
}


int main(int argc, char **argv) {
    const char *valid_opt = ":hgsraic::l::o:RC";
    char *count_arg, *open_arg, *list_arg;
    struct users_configs *configs = NULL;
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
    int retval = 1;
    int opt;

    prog_name_inf = argv[0];
   
    if(argc == 1)
        display_help(prog_name_inf);

    while((opt = getopt(argc, argv, valid_opt)) != EOF) {
        switch(opt) {
            case 'h':
                display_help(prog_name_inf);
                retval = 0;
                goto Out;
            case 'g':
                if(!generate_opt())
                    retval = 0;

                goto Out;
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
            case 'o':+
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
                goto Out;
            default:
                invalid_arg_err(optopt) ;
                goto Out;
        }
    }
        
    if(count) {
        if(!configs)
            if(!((configs = get_configs())))
                goto Out;

        if(all)
            count_arg = NULL;
        else if(!count_arg)
            goto Out;

        if(count_docs)
            goto Out;
    }

    if(list) {
        if(!configs)
            if(!(configs = get_configs()))
                goto Out;

        if(list_docs)
            goto Out;
    }

    if(open) {
        if(!configs)
            if(!(configs = get_configs()))
                goto Out;

        if(open_doc)
            goto Out;
    }

    retval = 0;

    Out:
        if(config_path)
            free(config_path);
    
        if(configs)
            free_users_configs(configs);

        return retval;
}
