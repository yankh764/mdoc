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
 * Get a line of input from stdin without limiting it's size.
 */
char *get_line() {
	char *retval, *line, *line_addr;
	unsigned int i;
	int c;

	retval = line_addr = NULL;

	for(i=0; (c = getchar()); i++) {
		if(c == EOF)
			goto Out;

		else if(c == '\n')
			break;
		
		if(!(line = (char *) realloc_inf(line, i+1)))
			goto Out;
		
		/* Save the address of line in case realloc_inf() overwrote it with NULL, 
		   so cleanup can be done.                                               */
		if(!line_addr)
			line_addr = line;
		
		line[i] = c;
	}	
	
	if(line) {
		if(!(line = (char *) realloc_inf(line, i+1)))
			goto Out;
		
		line[i] = '\0';
		retval = line;
	}
	else 
		if(line_addr)
			free(line_addr);

	return retval;
	
	Out:
		if(!retval)
			if(line)
				free(line);

		return retval;
}


/*
 * The same as get_line but you have control over the stream 
 * and EOF is allowed.
 */
char *fget_line(FILE *stream) {
	char *retval = NULL;
	char *line = NULL;
	unsigned int i;
	int c;

	for(i=0; (c = getc(stream)) != EOF; i++) {
		if(c == '\n')
			break;
		
		if(!(line = (char *) realloc_inf(line, i+1)))
			goto Out;

		line[i] = c;
	}
	/* If line wasn't allocated and first input char was '\n' */
	if(!line) 
		goto Out;

	if(!(line = (char *) realloc_inf(line, i+1)))
		goto Out;
	
	line[i] = '\0';
	retval = line;
	
	Out:
		if(!retval)
			if(line)
				free(line);

		return retval;
}
