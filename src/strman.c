/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains necessary functions for st- |
| ring management.                                      |
---------------------------------------------------------
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "informative.h"
#include "strman.h"


/* Static Functions Prototype */
static void convert_to_lower(char *);


/*
 * Make a small letters copy of str.
 */
char *small_let_copy(const char *str) 
{
	const size_t len = strlen(str) + 1;
	char *str_small; 
	size_t i;

	if ((str_small = malloc_inf(sizeof(char) * len))) {
		for (i=0; str[i]!='\0'; i++) {
			if (isupper(str[i]))
				str_small[i] = tolower(str[i]);
			else
				str_small[i] = str[i];
		}
		str_small[i] = '\0';
	}
	
	return str_small;
}


static void convert_to_lower(char *str) 
{
	unsigned int i;

	for (i=0; str[i]!='\0'; i++)
		if (isupper(str[i]))
			str[i] = tolower(str[i]);
}


/*
 * The same as strstr() but with "ignored case distinction"
 * and diffrent return values. 0 = didn't find, 1 = found.
 */
bool strstr_i(const char *haystack, const char *needle) 
{
	const size_t haystack_len = strlen(haystack) + 1;
	const size_t needle_len = strlen(needle) + 1;
	char haystack_cp[haystack_len]; 
	char needle_cp[needle_len]; 
	
	memcpy(haystack_cp, haystack, haystack_len);
	memcpy(needle_cp, needle, needle_len);

	convert_to_lower(haystack_cp);
	convert_to_lower(needle_cp);

	return strstr(haystack_cp, needle_cp) ? 
		1 : 0;
}


/*
 * Return 1 if word_to_check should come before assumed_smaller
 * alphabetically, otherwise return 0.
 */
bool alpha_cmp(const char *assumed_smaller, const char *word_to_check) 
{
	const size_t assumed_len = strlen(assumed_smaller);
	const size_t check_len = strlen(word_to_check);
	size_t min_len, i;

	min_len = (assumed_len < check_len) ? assumed_len : check_len; 
	
	for (i=0; i<min_len; i++) {
	/* If both chars are alphabetical characters check 
	   their values.                                   */
		if (isalpha(word_to_check[i])
		 && isalpha(assumed_smaller[i])) {
			if (word_to_check[i] < assumed_smaller[i])
				return 1;
			else if (assumed_smaller[i] < word_to_check[i])
				return 0;
		/* If assumed_smaller[i] isn't alphabetical char whereas word_to_check[i] is */
		} else if (!isalpha(assumed_smaller[i]) 
				&& isalpha(word_to_check[i])) {
			return 1;
		/* If word_to_check[i] isn't alphabetical char whereas assumed_smaller[i] is */
		} else if (!isalpha(word_to_check[i]) 
				&& isalpha(assumed_smaller[i])) {
			return 0;
		}
	}
	
	/* If haven't returned yet, return 1 if  
	   check_len < assumed_len, otherwise return 0 */	
	return (check_len < assumed_len); 
}


unsigned int count_words(const char *line) 
{
	unsigned int words;
	unsigned int i;
	bool inside = 0;

	for (i=0, words=0; line[i]!='\0'; i++) {
		if (isspace(line[i]))
			inside = 0;
		else {
 			if (!inside) 
				words++;
			
			inside = 1;
		}
	}
	
	return words;
}


/*
 * Replace each space (white character) with a null byte 
 * and return the index of the next char.
 */
unsigned int space_to_null(char *text) 
{
	unsigned int i;

	for (i=0; text[i]!='\0'; i++)
		if (isspace(text[i])) {
			text[i++] = '\0';
			break;
		}
			
	return i;
}


char *strcpy_dynamic(const char *src) 
{
	const size_t len = strlen(src) + 1;
	char *src_cpy;

	if ((src_cpy = malloc_inf(sizeof(char) * len)))
		/*
		 * I decided to use memcpy() instead of strncpy()
		 * just because I like it's behaviour more.
		 */
		memcpy(src_cpy, src, len);

	return src_cpy;
}
