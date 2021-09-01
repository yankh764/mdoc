/*
----------------------------------------------------------
| License: GNU GPL-3.0                                   |
----------------------------------------------------------
| This source file contains all functions for input man- |
| agement                                                |
----------------------------------------------------------
*/

#include <stdlib.h>
#include <stdbool.h>
#include "informative.h"
#include "input.h"


/* Static Functions Prototype */
static bool end_of_input(const int);


/*
 * Return true if the passed char indicates the
 * end of the input (if the char is '\n' or EOF).
 */
static bool end_of_input(const int c) 
{
	return (c == '\n' || c == EOF);
}


/*
 * Get a line of input from stream without limiting it's size.
 */
char *get_line(FILE *stream) 
{
	char *line_address = NULL;
	char *line = NULL;
	unsigned int i;
	int c;

	for (i=0; (c = getc(stream)); i++) {
		if (end_of_input(c))
			break;
		
		if (!(line = realloc_inf(line, sizeof(char) * (i+1))))
			goto error;

		line[i] = c;
		/* 
		 * Save the address of line in case realloc_inf() overwrote it 
		 * with NULL, so cleanup can be done.                                               
		 */
		line_address = line;
	}	
	if (line)
		if (!(line = realloc_inf(line, sizeof(char) * (i+1))))	
			goto error;

	line[i] = '\0';	

	return line;

error:
	if (line_address)
		free(line_address);

	return NULL;
}
