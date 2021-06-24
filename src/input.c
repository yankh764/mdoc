/*
----------------------------------------------------------
| License: GNU GPL-3.0                                   |
----------------------------------------------------------
| This source file contains all functions for input man- |
| agement                                                |
----------------------------------------------------------
*/

#include <stdlib.h>
#include "informative.h"
#include "input.h"


/*
 * Get a line of input from stream without limiting it's size.
 */
char *get_line(FILE *stream) {
	char *retval, *line, *line_address;
	unsigned int i;
	int c;

	retval = line_address = line = NULL;

	for(i=0; (c = getc(stream)); i++) {
		if(c == '\n' || c == EOF)
			break;
		
		if(!(line = (char *) realloc_inf(line, i+1)))
			break;
		/* Save the address of line in case realloc_inf() overwrote it with NULL, 
		   so cleanup can be done.                                               */
		if(!line_address)
			line_address = line;
		
		line[i] = c;
	}	

	if(line) 
		if((line = (char *) realloc_inf(line, i+1)))	
			line[i] = '\0';	
	
	retval = line;

	if(!retval && line_address)
		free(line_address);

	return retval;
}

