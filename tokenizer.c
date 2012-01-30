#include <errno.h>
#include <ctype.h>

#include "tokenizer.h"

//The maximum length of any single token
#define MAX_TOKEN_LENGTH 255

///
/// Private functions and data.
///

//Functions used to parse tokens.
static parse_token* parse_funcs = NULL;
static size_t func_count = 0;


/*	Parses a whitespace delimited string from the stream.
	f = stream to be read from.
	buf = buffer string is copied to.
	returns the number of characters in the string.
*/
size_t read_str(FILE* f, char* buf);

///
/// Function implementations
///


/* 	Parses the given file for tokens, returning 0 on success or non-zero on failure.  
	file_name = file to be parsed.
	tokens = out parameter of tokens parsed.
	token_count = the number of tokens parsed.
*/
uint8_t parse_file (const char* file_name, void* tokens, size_t* token_count) {
	//Pointer to our file descriptor, and temporary buffer for reading.
	FILE* f = NULL;
	char buf[MAX_TOKEN_LENGTH];
	size_t bindex=0;

	//1.  Open file, prepare for reading
	if((f = fopen(file_name, "r")) == NULL) {
		printf("Error opening file! %i\n", errno);
		exit(0);
	}

	//2.  Read each contiguous string, one at a time
	
	while(read_str(f, buf))
		for(size_t i = 0; i < func_count; i++) {
			size_t count; 
			
			(*parse_funcs[i]) (buf, &count, tokens);
						
			//append the token to the stream
			/*tokens->next = malloc(sizeof(token));
			//tokens = tokens->next;

			if(tokens == NULL) { //Malloc failure!
				printf("malloc failed!\n");
				exit(0);
			}*/
			
			//If the token didn't take the entire string.. keep searching.
			if(count < strlen(buf)) {
				memmove(buf, buf+count, MAX_TOKEN_LENGTH-count);
				i = 0;			
			}
		}

	if(fclose(f) != 0) {
		printf("fclose failed! %i\n", errno);
		return errno;
	}

	//3.  Return success.
	return 0;
}

/*	Parses a whitespace delimited string from the stream.
	f = stream to be read from.
	buf = buffer string is copied to.
	returns the number of characters in the string.
*/
size_t read_str(FILE* f, char* buf) {
	char c;
	//Skip preceeding whitespace
	while(isspace(c = fgetc(f)) && c != EOF);
	ungetc(c, f);

	//clear buffer
	memset(buf, 0, MAX_TOKEN_LENGTH);
	
	//Grab the string
	for(size_t index = 0; index < MAX_TOKEN_LENGTH; index++) {
		buf[index] = fgetc(f);
		if(buf[index] == EOF || isspace(buf[index])) {
			buf[index] = '\0';
			return index;
		}
	}

	return MAX_TOKEN_LENGTH;
}

/*	Registers a set of token parsing functions which fullfill the contract
	specified by parse_token.
	functions = set of functions to be registered.
	func_count = number of functions to be registered.
*/
void register_token_parsers (parse_token* functions, size_t count) {
	parse_funcs = functions;
	func_count = count;
}