#ifndef MDOC_H
#define MDOC_H

#define ANSI_COLOR_RED    "\x1b[31m"
#define ANSI_COLOR_BLUE   "\x1b[34m"
#define ANSI_COLOR_GREEN  "\x1b[32m"
#define ANSI_COLOR_RESET  "\x1b[0m"

#include <stdbool.h>

/* To indicate if an previous error eccoured in a functions
   that could overwrite errno with 0 (success) before returning */
extern bool prev_error; 

struct l_list {
	char *obj;
	struct l_list *next;
};

struct l_list *search_for_doc(const char *, const char *, const bool, const bool);
char *get_doc_path(const char *, const char *, const bool);
void prep_open_doc_argv(char **, char *, char *, char *);
int open_doc(char *const *);
int sort_docs_alpha(struct l_list *);
void free_l_list(struct l_list *);
void display_docs(const struct l_list *, const bool);
unsigned int count_l_list_nodes(const struct l_list *);
void reverse_l_list_obj(struct l_list *);
void display_help(const char *);

#endif
