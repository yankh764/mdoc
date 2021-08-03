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
#include "informative.h"
#include "mdoc.h"


const char *prog_name_inf;

/* Static Functions Prototype */
static char *get_config_path();
static void missing_arg_err(const int);
static void invalid_arg_err(const int);
static int generate_opt();
static struct users_configs *get_configs();
static int count_opt(const char *, bool, bool, bool);
static void opts_cleanup(struct users_configs *, struct l_list *);
static void big_docs_num_error();
static void small_docs_num_error();
static int rearrange_if_needed(struct l_list *, bool, bool); 
static int list_opt(const char *, bool, bool, bool, bool, bool);
static int numerous_opening(struct users_configs *, struct l_list *, bool, bool, bool, bool);
static int open_opt(const char *, bool, bool, bool, bool, bool, bool);
static int open_doc_list(struct users_configs *, const struct l_list *, bool, bool);
static int details_opt(const char *, bool, bool, bool, bool, bool);


static char *get_config_path() {
    const char *config = ".config/mdoc";
    char *config_path = NULL;
    const char *home; 
    size_t len;

    if((home = getenv_inf("HOME"))) {
        len = strlen(config) + strlen(home) + 2;
        
        if((config_path = malloc_inf(sizeof(char) * len)))
            snprintf(config_path, len, "%s/%s", home, config);
    }

    return config_path;
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
        retval = generate_config(config_path);
        free(config_path);
    }

    return retval;
}


static struct users_configs *get_configs() {
    struct users_configs *configs = NULL;
    char *config_path;

    if((config_path = get_config_path())) {
        configs = read_configs(config_path);
        free(config_path);
    }

    return configs;
}


static int count_opt(const char *str, bool ignore, 
                     bool rec, bool color) {
    struct users_configs *configs;
    struct l_list *doc_list;
    int retval = -1;

    if((configs = get_configs())) {
        doc_list = search_for_doc_multi_dir(configs->docs_dir_path, str, ignore, rec);
        
        if(!prev_error) {
            print_docs_num(doc_list, color);
            retval = 0;
        }  

        opts_cleanup(configs, doc_list);
    }

    return retval;
}


static void opts_cleanup(struct users_configs *configs, struct l_list *doc_list) {
    if(doc_list)
        free_l_list(doc_list);

    free_users_configs(configs);
}


static int rearrange_if_needed(struct l_list *doc_list, 
                               bool sort, bool reverse) {
    if(sort)
        if(sort_docs_alpha(doc_list))
            return -1;
    
    if(reverse)
        reverse_l_list_obj(doc_list);

    return 0;
}


static int list_opt(const char *str, bool ignore, bool rec, 
                    bool color, bool sort, bool reverse) {
    struct users_configs *configs;
    struct l_list *doc_list;
    int retval = -1;
    
    if((configs = get_configs())) {
        if((doc_list = search_for_doc_multi_dir(configs->docs_dir_path, str, ignore, rec)))
            if(!(retval = rearrange_if_needed(doc_list, sort, reverse)))
                display_docs(doc_list, color);
        
        opts_cleanup(configs, doc_list);
    }

    return retval;
}


static int open_doc_list(struct users_configs *configs, 
                         const struct l_list *doc_list,
			         	 bool rec, bool color) {
	const struct l_list *ptr = doc_list;
	char *doc_path;
	int retval = 0;

	for(; ptr && !retval; ptr=ptr->next) {
		if((doc_path = get_doc_path_multi_dir(configs->docs_dir_path, ptr->obj, rec))) {
			if(!(retval = open_founded_doc_path(configs, doc_path)))
				print_opening_doc(ptr->obj, color);
			
			free(doc_path);
		}
		else 
			retval = -1;
	}
	
	return retval;
}


static int open_opt(const char *str, bool ignore, bool rec, bool color, 
                    bool sort, bool reverse, bool numerous) {
    struct users_configs *configs;
    struct l_list *doc_list;
    int retval = -1;
    
    if((configs = get_configs())) {
        if((doc_list = search_for_doc_multi_dir(configs->docs_dir_path, str, ignore, rec))) {
            if(numerous)
                retval = numerous_opening(configs, doc_list, rec, color, sort, reverse);
            
            else if(count_l_list_nodes(doc_list) == 1)
                retval = open_doc_list(configs, doc_list, rec, color);

            else 
                big_docs_num_error();   
        }

        else if(!prev_error)
            small_docs_num_error();
            
        opts_cleanup(configs, doc_list);
    }

    return retval;
}


static int numerous_opening(struct users_configs *configs, struct l_list *doc_list,
                            bool rec, bool color, bool sort, bool reverse) {
    if(rearrange_if_needed(doc_list, sort, reverse))
        return -1;
    
    return open_doc_list(configs, doc_list, rec, color);
}


static int details_opt(const char *str, bool ignore, bool rec, 
                       bool color, bool sort, bool reverse) {
    struct users_configs *configs;
    struct l_list *doc_list;
    int retval = -1;

    if((configs = get_configs())) {
        if((doc_list = search_for_doc_multi_dir(configs->docs_dir_path, str, ignore, rec)))
            if(!rearrange_if_needed(doc_list, sort, reverse))
                retval = print_doc_list_details(doc_list, color);

        opts_cleanup(configs, doc_list);
    }
//test
    return retval;
}


static void big_docs_num_error() {
    fprintf(stderr, "%s: can't open document: Several documents were found\n", prog_name_inf);
    fprintf(stderr, "Try '%s -h' for more information.\n", prog_name_inf);
}


static void small_docs_num_error() {
    fprintf(stderr, "%s: can't open document: No document were found\n", prog_name_inf);
    fprintf(stderr, "Try '%s -h' for more information.\n", prog_name_inf);
}

            
int main(int argc, char **argv) {
    const char *valid_opt = ":hgsrainc::l::d::o::RC";
    char *count_arg, *open_arg, *list_arg, *details_arg;
    struct users_configs *configs = NULL;
    struct l_list *doc_list = NULL;
    bool recursive = 1;
    bool generate = 0;
    bool numerous = 0;
    bool reverse = 0;
    bool details = 0;
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
            case 'n':
                numerous = 1;
                break;
            case 'c':
                count = 1;
                count_arg = argv[optind];
                break;
            case 'l':
                list = 1;
                list_arg = argv[optind];
                break;
            case 'd':
                details = 1;
                details_arg = argv[optind];
                break;
            case 'o':
                open = 1;
                open_arg = argv[optind];
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
    
    else if(details) {
        if(all)
            details_arg = NULL;
        
        else if(!details_arg) {
            missing_arg_err('d');
            goto Out;
        }

        if(details_opt(details_arg, ignore, recursive, color, sort, reverse))
            goto Out;
    }


    else if(open) {
        if(all)
            open_arg = NULL;

        else if(!open_arg) {
            missing_arg_err('o');
            goto Out;
        }

        if(open_opt(open_arg, ignore, recursive, color, sort, reverse, numerous))
            goto Out;
    }

    retval = 0;

    Out:
        if(configs)
            free_users_configs(configs);
        
        if(doc_list)
            free_l_list(doc_list);

        return retval;
}
