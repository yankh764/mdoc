#ifndef CONFIG_H
#define CONFIG_H

#define IN_LEN 600 /* Max input length */

struct users_configs {
	char docs_dir_path[IN_LEN]; // Users Document's directory absolute path
	char pdf_viewer[IN_LEN];
};

int generate_config(const char *);
struct users_configs *read_configs(const char *);

#endif
