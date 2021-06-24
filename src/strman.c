/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains necessary functions for st- |
| ring management.                                      |
---------------------------------------------------------
*/

#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "informative.h"
#include "strman.h"


/* Static Functions Prototype */
static void cleanup(char *, char *);
static long int get_smallest_word_i(char **, const unsigned int);
static bool alpha_cmp(const char *, const char *);
static void adjust_val_after_alpha_cmp(char *, char *, unsigned int *, unsigned int);

/*
 * Make a small letters copy of str.
 */
char *small_let_copy(const char *str) {
	size_t len = strlen(str) + 1;
	unsigned int i;
	char *str_small; /* Small letter copy of str */

	if((str_small = (char *) malloc_inf(len))) {
		for(i=0; str[i]!='\0'; i++) {
			if(isupper(str[i]))
				str_small[i] = tolower(str[i]);
			else
				str_small[i] = str[i];
		}
		str_small[i] = '\0';
	}
	
	return str_small;
}


/*
 * The same as strstr() but with "ignored case distinction"
 * and diffrent return values. 0 = didn't find, 1 = found, -1 = fail.
 */
int strstr_i(const char *haystack, const char *needle) {
	char *haystack_small = NULL; 
	char *needle_small = NULL; 
	int retval = -1;
	
	/* If made small letter copy of haystack and needle sucessfully */
	if((haystack_small = small_let_copy(haystack)) 
	 && (needle_small = small_let_copy(needle))) {
		
		if(strstr(haystack_small, needle_small))
			retval = 1;
		else 
			retval = 0; 
	}
	
	cleanup(haystack_small, needle_small);

	return retval;
}


static void cleanup(char *ptr1, char *ptr2) {
	if(ptr1)
		free(ptr1);
	if(ptr2)
		free(ptr2);
}


/*
 * Sort the array of pointers alphabetically and save it to sorted.
 */
int strsort_alpha(char **unsorted, char **sorted, 
                  const unsigned int size) {
	unsigned int i;
	long int ret_i;

	for(i=0; i<size; i++) {
		if((ret_i = get_smallest_word_i(unsorted, size)) == -1)
			return -1;
		
		sorted[i] = unsorted[ret_i];
		unsorted[ret_i] = NULL;
	}
	sorted[i] = NULL;
	
	return 0;
}
	

/*
 * Return the index of the word that has letters with a 
 * lowest value in the array of pointers. 
 */
static long int get_smallest_word_i(char **array, const unsigned int size) {
	unsigned int i, smallest_word_i;
	char *smallest_word = NULL;
	char *current_word = NULL;
	long int retval = -1; 

	for(i=0, smallest_word_i=0; i<size; i++) {
		if(array[i] == NULL)
			continue;
			
		if(smallest_word) {
			if(!(current_word = small_let_copy((const char *) array[i])))
				break;

			if(alpha_cmp((const char *) smallest_word, (const char *) current_word)) {
				adjust_val_after_alpha_cmp(smallest_word, current_word, &smallest_word_i, i);
				//smallest_word = current_word;
				//smallest_word_i = i;
				current_word = NULL;
			}
			else {
				free(current_word);
				current_word = NULL;	
			}
		}
		else {
			if(!(smallest_word = small_let_copy((const char *) array[i])))
				break;
			
			smallest_word_i = i;
		}
	}
	if(!errno)
		retval = smallest_word_i;

	cleanup(smallest_word, current_word);

	return retval;
}


/*
 * Adjust the passed variables values from get_smallest_word_i()
 * to the right values if alpha_cmp() returns 1.
 */
static void adjust_val_after_alpha_cmp(char *smallest_word, 
                                       char *current_word, 
                                       unsigned int *smallest_word_i, 
									   unsigned int current_word_i) {
	free(smallest_word);
	smallest_word = current_word;
	*smallest_word_i = current_word_i;
	current_word = NULL;
} 


/*
 * Return 1 if word_to_check should come before assumed_smaller
 * alphabetically, otherwise return 0.
 */
static bool alpha_cmp(const char *assumed_smaller, const char *word_to_check) {
	size_t assumed_len = strlen(assumed_smaller);
	size_t check_len = strlen(word_to_check);
	size_t min_len, i;
	
	min_len = (assumed_len > check_len) ? assumed_len : check_len; 
	
	for(i=0; i<min_len; i++)
	/* If both chars are alphabetical characters check 
	   their values.                                   */
		if(isalpha(word_to_check[i]) 
		 && isalpha(assumed_smaller[i]))
			if(word_to_check[i] < assumed_smaller[i])
				return 1;
	
	/* If haven't returned yet, return 1 if  
	   check_len > assumed_len, otherwise return 0 */	
	return (check_len > assumed_len); 
}
