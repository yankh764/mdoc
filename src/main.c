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
static void open_opt_docs_num_error(const unsigned int);
static void print_opening_doc(const char *, const bool);
static void print_docs_num(const struct l_list *, const bool);
static int count_opt(const char *, const bool, const bool, const bool);
static void opt_cleanup(struct users_configs *, struct l_list *);
static int open_opt(const char *, const bool, const bool, const bool);
static int open_founded_doc_path(struct users_configs *, char *);
static void open_opt_cleanup(struct users_configs *, struct l_list *, char *);
static int list_opt(const char *, const bool, const bool, 
                    const bool, const bool, const bool);



static char *get_config_path() {
    const char *config = ".config/mdoc";
    char *retval = NULL;
    char *config_path;
    const char *home; 
    size_t len;

    if((home = getenv_inf("HOME"))) {
        len = strlen(config) + strlen(home) + 2;
        
        if((config_path = malloc_inf(len)))
            snprintf(config_path, len, "%s/%s", home, config);

        retval = config_path;
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


static int count_opt(const char *str, const bool ignore, 
                     const bool rec, const bool color) {
    struct l_list *doc_list = NULL;
    struct users_configs *configs;
    int retval = -1;

    if((configs = get_configs())) {
        doc_list = search_for_doc(configs->docs_dir_path, str, ignore, rec);
        
        if(!prev_error) {
            print_docs_num(doc_list, color);
            retval = 0;
        }  

        opt_cleanup(configs, doc_list);
    }

    return retval;
}


/*
 * A cleanup for the functions list_opt() and count_opt()
 */
static void opt_cleanup(struct users_configs *configs, struct l_list *doc_list) {
    if(doc_list)
        free_l_list(doc_list); 

    free_users_configs(configs);
}


static void print_docs_num(const struct l_list *doc_list, const bool color) {
    if(color)
        printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "%d" ANSI_COLOR_BLUE "]"               
               ANSI_COLOR_RED " documents were found\n" ANSI_COLOR_RESET, 
               count_l_list_nodes(doc_list));
    else
        printf("[%d] documents were found\n", count_l_list_nodes(doc_list));
}


static int list_opt(const char *str, const bool ignore, const bool rec, 
                    const bool color, const bool sort, 
                    const bool reverse) {
    struct l_list *doc_list = NULL;
    struct users_configs *configs;
    int retval = -1;
    
    if((configs = get_configs())) {
        doc_list = search_for_doc(configs->docs_dir_path, str, ignore, rec);

        if(!prev_error) {
            if(doc_list && sort)
                sort_docs_alpha(doc_list);
                
            if(doc_list && reverse)
                reverse_l_list_obj(doc_list); 
            
            display_docs(doc_list, color);
            
            retval = 0;
        }
    
        opt_cleanup(configs, doc_list);
    }

    return retval;
}


static int open_opt(const char *str, const bool ignore, 
                    const bool rec, const bool color) {
    struct l_list *doc_list = NULL;
    struct users_configs *configs;
    char *doc_path = NULL;
    unsigned int doc_num;
    int retval = -1;
    
    if((configs = get_configs())) {
        doc_list = search_for_doc(configs->docs_dir_path, str, ignore, rec);
    
        if(!prev_error) {
            if((doc_num = count_l_list_nodes(doc_list)) == 1) {
                if((doc_path = get_doc_path(configs->docs_dir_path, doc_list->obj, rec))
                 && !(retval = open_founded_doc_path(configs, doc_path)))
                        print_opening_doc(doc_path, color);
            }
            else 
                open_opt_docs_num_error(doc_num);
        }

        open_opt_cleanup(configs, doc_list, doc_path);
    }
    
    return retval;
}


static void open_opt_cleanup(struct users_configs *configs, 
                             struct l_list *doc_list, char *doc_path) {
    if(doc_path)
        free(doc_path);

    if(doc_list)
        free_l_list(doc_list);

    free_users_configs(configs);
}


static int open_founded_doc_path(struct users_configs *configs, char *doc_path) {
    const unsigned int argc = count_words(configs->docs_dir_path) + 2;
    char *argv[argc+1];

    prep_open_doc_argv(argv, configs->pdf_viewer, doc_path, configs->add_args);

    return open_doc(argv);
}


static void print_opening_doc(const char *doc_path, const bool color) {
	if(color)
		printf(ANSI_COLOR_BLUE "[" ANSI_COLOR_GREEN "Opening" ANSI_COLOR_BLUE "] " 
               ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, 
               doc_path);
	else 
		printf("Opening: %s\n", doc_path);
}


static void open_opt_docs_num_error(const unsigned int docs_num) {
    if(docs_num == 0)
        fprintf(stderr, "No document were found\n");
    else
        fprintf(stderr, "More than 1 document were found\n");
}

            
int main(int argc, char **argv) {
    const char *valid_opt = ":hgsraic::l::o:RC";
    char *count_arg, *open_arg, *list_arg;
    struct users_configs *configs = NULL;
    struct l_list *doc_list = NULL;
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
                count_arg = argv[optind];
                break;
            case 'l':
                list = 1;
                list_arg = argv[optind];
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

        if(count_opt(count_arg, ignore, recursive, color))
            goto Out;
    }

    else if(list) {
        if(all)
            list_arg = NULL;

        else if(!list_arg) {
            missing_arg_err('l');
            goto Out;
        }
        
        if(list_opt(list_arg, ignore, recursive, color, sort, reverse))
            goto Out;
    }

    else if(open)
        if(open_opt(open_arg, ignore, recursive, color))
            goto Out;

    retval = 0;

    Out:
        if(configs)
            free_users_configs(configs);
        
        if(doc_list)
            free_l_list(doc_list);

        return retval;
}
