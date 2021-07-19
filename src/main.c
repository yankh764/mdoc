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
#include "strman.h"
#include "config.h"
#include "informative.h"
#include "mdoc.h"


const char *prog_name_inf;

/* Static Functions Prototype */
static char *get_config_path();
static void missing_arg_err(const int);
static void invalid_arg_err(const int);
static int generate_opt();
static struct users_configs *get_configs();
static int count_opt(const char *, const char *, const bool, const bool);
static int list_opt(const char *, const char *, const bool,
                    const bool, const bool, const bool, const bool);
static int open_opt(const struct users_configs, const char *, 
                    const bool, const bool, const bool);


static char *get_config_path() {
    const char *config_path = ".config/mdoc";
    char *abs_config_path;
    char *retval = NULL;
    const char *home; 
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
    char *config_path;

    if((config_path = get_config_path())) {
        retval = read_configs(config_path);
        free(config_path);
    }

    return retval;
}


static int count_opt(const char *dir_path, const char *str, 
                     const bool ignore, const bool rec) {
    struct l_list *doc_list;
    int retval = -1;

    doc_list = search_for_doc(dir_path, str, ignore, rec);
    
    if(!prev_error) {
        printf("%d documents were found\n", count_l_list_nodes(doc_list));

        if(doc_list)
            free_l_list(doc_list);

        retval = 0;
    }

    return retval;
}


static int list_opt(const char *dir_path, const char *str, 
                    const bool ignore, const bool rec, 
                    const bool color, const bool sort, 
                    const bool reverse) {
    struct l_list *doc_list;
    int retval = 0;
    
    if((doc_list = search_for_doc(dir_path, str, ignore, rec))) {
        if(sort)
            sort_docs_alpha(doc_list);

        if(reverse)
            reverse_l_list_obj(doc_list);
        
        printf("These are the documents that were found:\n\n");
        display_docs(doc_list, color);

        free_l_list(doc_list);
    }
    else if(prev_error)
        retval = -1;

    return retval;
}


static int open_opt(const struct users_configs configs, const char *str, 
                    const bool ignore, const bool rec, const bool color) {
   /* The 2 stands for pdf_viewer and doc_path */
    const char *dir_path = configs.docs_dir_path;
    const unsigned int argc = count_words(dir_path) + 2; 
    struct l_list *doc_list;
    unsigned int docs_num;
    char *argv[argc+1];
    char *doc_path;
    int retval = -1;
    
    if((doc_list = search_for_doc(dir_path, str, ignore, rec))) {
        docs_num = count_l_list_nodes(doc_list);
        
        if(docs_num == 1) {
            prep_open_doc_argv(argv, pdf_viewer, doc_path, add_args);
            
        }
    }
    
    else if(!prev_error)
        fprintf(stderr, "No document were found\n");
    
    //prep_open_doc_argv(argv, pdf_viewer, doc_path, add_args);
    
    //return open_doc(argv, doc_path, color);  
}


int main(int argc, char **argv) {
    const char *valid_opt = ":hgsraic::l::o:RC";
    char *count_arg, *open_arg, *list_arg;
    struct users_configs *configs = NULL;
    struct l_list *doc_list = NULL;
    bool need_configs = 0;
    bool recursive = 1;
    bool generate = 0;
    bool reverse = 0;
    bool ignore = 0;
    bool color = 1;
    bool count = 0;
    bool help = 0;
    bool list = 0;
    bool open = 0;
    bool sort = 0;
    bool all = 0;
    int retval = 1;
    int opt;

    prog_name_inf = argv[0];
   
    if(argc == 1)
        display_help(prog_name_inf);

    while((opt = getopt(argc, argv, valid_opt)) != EOF)
        switch(opt) {
            case 'h':
                help = 1;
                break;
            case 'g':
                generate = 1;
                break;
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
                need_configs = 1;
                count_arg = argv[optind];
                break;
            case 'l':
                list = 1;
                need_configs = 1;
                list_arg = argv[optind];
                break;
            case 'o':
                open = 1;
                need_configs = 1;
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

    if(help)
        display_help(prog_name_inf);
   
    else if(generate) { 
        if(generate_opt())
            goto Out;
    }

    else if(count) {
        if(all) 
            count_arg = NULL;
           
        else if(!count_arg) {
            missing_arg_err('c');
            goto Out;
        }

        if(count_opt(configs->docs_dir_path, count_arg, ignore, recursive))
            goto Out;
    }

    else if(list) {
        if(all)
            list_arg = NULL;

        else if(!list_arg) {
            missing_arg_err('l');
            goto Out;
        }
            
        if(list_opt(configs->docs_dir_path, list_arg, ignore, 
                    recursive, color, sort, reverse))
            goto Out;
    }

    else if(open)
        if(open_opt(*configs, open_arg, ignore, recursive, color))
            goto Out;

    retval = 0;

    Out:
        if(configs)
            free_users_configs(configs);
        
        if(doc_list)
            free_l_list(doc_list);

        return retval;
}
