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


enum EXIT_CODES { 
    PROG_ERROR = 2,
    CLI_ERROR = 1,
    SUCCES = 0
};

char *prog_name_inf;


/* Static Functions Prototype */
static char *get_config_path();
static int missing_arg_err(const int);
static int invalid_arg_err(const int);
static int generate_opt();
static struct users_configs *get_configs();
static int count_opt(const char *, bool, bool, bool);
static void opts_cleanup(struct users_configs *, struct doc_list *);
static void big_docs_num_error();
static struct doc_list *rearrange_if_needed(struct doc_list *, bool, bool); 
static int list_opt(const char *, bool, bool, bool, bool, bool);
static int numerous_opening(const struct users_configs *, struct doc_list *, bool, bool, bool);
static int open_opt(const char *, bool, bool, bool, bool, bool, bool);
static int open_doc_list(const struct users_configs *, const struct doc_list *, bool);
static int details_opt(const char *, bool, bool, bool, bool, bool);
static char *get_opt_arg(const char *);
static void display_docs_names(const struct doc_list *, bool);
static int print_docs_details(const struct doc_list *, bool);
static void separate_if_needed(const struct doc_list *);



static char *get_config_path() 
{
    const char config[] = ".config/mdoc";
    char *config_path = NULL;
    char *home; 
    size_t len;

    if ((home = getenv_inf("HOME"))) {
        len = strlen(config) + strlen(home) + 2;
        
        if ((config_path = malloc_inf(sizeof(char) * len)))
            snprintf(config_path, len, "%s/%s", home, config);
    }

    return config_path;
}


static int generate_opt() 
{
    char *config_path; 
    int retval = -1;

    if ((config_path = get_config_path())) {
        retval = generate_config(config_path);
        free(config_path);
    }

    return retval;
}


static struct users_configs *get_configs() 
{
    struct users_configs *configs = NULL;
    char *config_path;

    if ((config_path = get_config_path())) {
        configs = read_configs(config_path);
        free(config_path);
    }

    return configs;
}


static int count_opt(const char *str, bool ignore, bool rec, bool color) 
{
    struct users_configs *configs;
    struct doc_list *list;
    int retval = -1;

    if ((configs = get_configs())) {
        list = search_for_doc_multi_dir(configs->docs_dir_path, str, ignore, rec);
        
        if (!prev_error) {
            print_docs_num(count_doc_list_nodes(list), color);
            retval = 0;
        }  
        opts_cleanup(configs, list);
    }

    return retval;
}


static void opts_cleanup(struct users_configs *configs, struct doc_list *ptr) 
{
    if (ptr)
        free_doc_list(ptr);

    free_users_configs(configs);
}


static struct doc_list *rearrange_if_needed(struct doc_list *list, 
                                            bool sort, bool reverse) 
{
    struct doc_list *rearranged = list;

    if (sort)
        if (!(rearranged = sort_docs_names_alpha(rearranged)))
            return NULL;
    
    if (reverse)
        rearranged = reverse_doc_list(rearranged);

    return rearranged;
}


static int list_opt(const char *str, bool ignore, bool rec, 
                    bool color, bool sort, bool reverse) 
{
    struct doc_list *list, *rearranged;
    struct users_configs *configs;
    int retval = -1;
    
    if ((configs = get_configs())) {
        if ((list = search_for_doc_multi_dir(configs->docs_dir_path, str, ignore, rec)))
            if ((rearranged = rearrange_if_needed(list, sort, reverse))) {
                display_docs_names(rearranged, color);
                retval = 0;
            }
        opts_cleanup(configs, list);
    }

    return retval;
}


static void display_docs_names(const struct doc_list *ptr, bool color)
{
    for(; ptr; ptr=ptr->next)
        display_doc_name(ptr->name, color);
}


static int open_doc_list(const struct users_configs *configs, 
                         const struct doc_list *ptr, bool color) 
{
	int retval;

	for (; ptr; ptr=ptr->next) {
		if ((retval = open_doc_path(configs, ptr->path)))
            break;
        
		print_opening_doc(ptr->name, color);
	}
	
	return retval;
}


static int open_opt(const char *str, bool ignore, bool rec, bool color, 
                    bool sort, bool reverse, bool numerous) 
{
    struct users_configs *configs;
    struct doc_list *list;
    int retval = -1;
    
    if ((configs = get_configs())) {
        if ((list = search_for_doc_multi_dir(configs->docs_dir_path, 
                                             str, ignore, rec))) {
            if (numerous)
                retval = numerous_opening(configs, list, color, sort, reverse);
            else if (count_doc_list_nodes(list) == 1)
                retval = open_doc_list(configs, list, color);
            else 
                big_docs_num_error();   
        }
        opts_cleanup(configs, list);
    }

    return retval;
}


static void big_docs_num_error() {
    fprintf(stderr, "%s: can't open document: Several documents were found\n", prog_name_inf);
    fprintf(stderr, "Try '%s -h' for more information.\n", prog_name_inf);
}


static int numerous_opening(const struct users_configs *configs, 
                            struct doc_list *list,
                            bool color, bool sort, bool reverse) 
{
    struct doc_list *rearranged;

    if (!(rearranged = rearrange_if_needed(list, sort, reverse)))
        return -1;
    
    return open_doc_list(configs, rearranged, color);
}


static int details_opt(const char *str, bool ignore, bool rec, 
                       bool color, bool sort, bool reverse) 
{
    struct doc_list *list, *rearranged;
    struct users_configs *configs;
    int retval = -1;

    if ((configs = get_configs())) {
        if ((list = search_for_doc_multi_dir(configs->docs_dir_path, str, ignore, rec)))
            if ((rearranged = rearrange_if_needed(list, sort, reverse)))
                retval = print_docs_details(rearranged, color);

        opts_cleanup(configs, list);
    }
    
    return retval;
}


static int print_docs_details(const struct doc_list *ptr, bool color)
{
	int retval;

	for (;ptr; ptr=ptr->next) {
		if ((retval = print_doc_details(ptr, color)))
			break;

        separate_if_needed(ptr->next);
    }

	return retval;
}


/*
 * Separate the details on each document if needed (if there's 
 * another document after it).
 */
static void separate_if_needed(const struct doc_list *ptr) 
{
    /* const char separator[] = 
        "-----------------------------"
        "-----------------------------"
        "-----------------------------"
        "-----------------------------"
        "-----------------------------"
        "--\n"; 
    */
    
    /* For now the separator will be a new line */
    const char separator[] = "\n";

    if (ptr)
        printf("%s", separator);
}


static char *get_opt_arg(const char *last_argv) 
{
    char *optarg = NULL;
    
    if (*last_argv != '-')
        optarg = (char *) last_argv;

    return optarg;
}


static int missing_arg_err(const int opt) 
{
    fprintf(stderr, "%s: missing argument for the '-%c' option\n", prog_name_inf, opt);
    fprintf(stderr, "Try '%s -h' for more information.\n", prog_name_inf);

    return CLI_ERROR;
}


static int invalid_arg_err(const int opt) 
{
    fprintf(stderr, "%s: invalid option '-%c'\n", prog_name_inf, opt);
    fprintf(stderr, "Try '%s -h' for more information.\n", prog_name_inf);

    return CLI_ERROR;
}


int main(int argc, char **argv) 
{
    const char valid_opt[] = ":hgsraincldoRC";
    /* 
     * I initialized the options argument pointers
     * to NULL to get rid of the annoying unaccurate 
     * -Wmaybe-uninitialized warning when compiling
     * with GCC.
     */
    char *details_arg = NULL;
    char *count_arg = NULL; 
    char *open_arg = NULL;
    char *list_arg = NULL; 
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
    int opt;

    prog_name_inf = argv[0];
  
    if (argc == 1)
        display_help(prog_name_inf);

    while ((opt = getopt(argc, argv, valid_opt)) != EOF) 
        switch (opt) {
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
            count_arg = get_opt_arg(argv[argc-1]);
            break;
        case 'l':
            list = 1;
            list_arg = get_opt_arg(argv[argc-1]);
            break;
        case 'd':
            details = 1;
            details_arg = get_opt_arg(argv[argc-1]);
            break;
        case 'o':
            open = 1;
            open_arg = get_opt_arg(argv[argc-1]);
            break;
        case 'R':
            recursive = 0;
            break;
        case 'C':
            color = 0;
            break;
        default:
            return invalid_arg_err(optopt);
        }

    if (help) {
        display_help(prog_name_inf);
    
    } else if (generate) {
        if (generate_opt())
            return PROG_ERROR;
    
    } else if (count) {
        if (all) 
            count_arg = NULL;
        else if (!count_arg)
            return missing_arg_err('c');
        
        if (count_opt(count_arg, ignore, recursive, color))
            return PROG_ERROR;
    
    } else if (list) {
        if (all)
            list_arg = NULL;
        else if (!list_arg)
            return missing_arg_err('l');
        
        if (list_opt(list_arg, ignore, recursive, color, sort, reverse))
            return PROG_ERROR;
    
    } else if (details) {
        if (all)
            details_arg = NULL;
        else if (!details_arg)
            return missing_arg_err('d');

        if (details_opt(details_arg, ignore, recursive, color, sort, reverse))
            return PROG_ERROR;
    
    } else if (open) {
        if (all)
            open_arg = NULL;
        else if (!open_arg)
            return missing_arg_err('o');

        if (open_opt(open_arg, ignore, recursive, color, sort, reverse, numerous))
            return PROG_ERROR;
    }

    return SUCCES;
}
