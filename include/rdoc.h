#ifndef RDOC_H
#define RDOC_H

#define IN_LEN 600

struct users_configs {
	char docs_dir_path[IN_LEN]; // Users Document's directory absolute path
	char pdf_viewer[IN_LEN];
};

struct l_list {
	char *obj;
	struct l_list *next;
};

int generate_config(const char *);
int read_configs(const char *, struct users_configs *);
void free_l_list(struct l_list *);
struct l_list *search_for_doc(const char *, const char *, unsigned int);

#endif