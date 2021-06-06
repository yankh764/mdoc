#ifndef RDOC_H
#define RDOC_H

struct l_list {
	char *obj;
	struct l_list *next;
};

extern const char *_prog_name_rdoc_;

void free_l_list(struct l_list *);
struct l_list *search_for_doc(const char *, const char *, unsigned int, unsigned int);
void print_l_list(struct l_list *);

#endif
