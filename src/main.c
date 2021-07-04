#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mdoc.h"


const char *prog_name_inf;

int main(int argc, char **argv) {
    int option; 

    prog_name_inf = argv[0];

//char *const argvs[] = {"firefox", "/home/yan/Documents/CS/Linux/LFS_Book.pdf", NULL};
//open_doc(argvs, "/home/yan/Documents/CS/Linux/LFS_Book.pdf", 1);
struct l_list *x = search_for_doc("/home/yan/Documents", NULL, 1, 1);
display_docs(x, 1);

printf("\n");

reverse_l_list_obj(x);
display_docs(x, 1);
    //while((option = getopt(argc, argv, ":h")))
    return 0;
}
