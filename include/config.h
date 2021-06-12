#ifndef CONFIG_H
#define CONFIG_H

struct users_configs {
	char *docs_dir_path; 
	char *pdf_viewer;
};

int generate_config(const char *);
struct users_configs *read_configs(const char *);
void free_users_configs(struct users_configs *);

#endif
