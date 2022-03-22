#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define BUFSIZE 8
#define LISTLEN 16

char **lines;
int line_count, line_array_size;

void init_lines(void)
{
    lines = malloc(sizeof(char *) * LISTLEN);
    line_count = 0;
    line_array_size = LISTLEN;
}

void add_line(char *p)
{
    if (DEBUG) printf("Adding |%s|\n", p);
    if (line_count == line_array_size) {
	line_array_size *= 2;
	lines = realloc(lines, line_array_size * sizeof(char *));
	// TODO: check whether lines is NULL
    }

    lines[line_count] = p;
    line_count++;
}

// char wrap[5]  = {'w','r','a','p','\0'};

int main(int argc, char** argv)
{
    int fd, bytes;                                          // for reading the file
    int file_width;                                         // this will be the width of the output file
    char buf[BUFSIZE];                                      // our buffer
    char* crnt;                                             // our output file, wrap+argv[1]
    char* crnt_temp;
    int crntlen;    
    int buf_pos, buf_start;                                 // current position in buffer buf
    int line_len;                                           // keep track of how many chars are 
    int whitespace_count;                                   // keep track of whitespace
    int newline_count;                                      // keep track of new lines

    init_lines();


    if(argc > 2) {
        file_width = atoi(argv[1]);
        if (file_width < 1) {
            if (DEBUG) printf("Please supply a positive file width");
        }
        // output = argv[2];
        //if (DEBUG) puts(output);
        fd = open(argv[2],O_RDONLY);
        if (fd == -1) {
            perror(argv[2]);
            return EXIT_FAILURE;
        }
    }
    else {
        fd = 0;
        if (DEBUG) printf("Please supply file width and file name");
        return EXIT_FAILURE;
    }


    crnt = NULL;
    crntlen = 0;


    // This is for reading from a file and printing out words
    while ((bytes = read(fd,buf,BUFSIZE)) > 0) {
        buf_start = 0;
        whitespace_count = 0;
        newline_count = 0;
        for (buf_pos = 0; buf_pos < bytes; buf_pos++) {

            if (buf[buf_pos] == '\n') {
                newline_count++;
                if (newline_count > 1) {                            // if we have a paragraph
                    printf("%s",crnt);
                    printf("\n\n");
                    line_len = 0;
                    crnt = NULL;
                    crntlen = 0;
                }
            }
            if (buf[buf_pos] == ' ') {
                whitespace_count++;
                if((whitespace_count > 1)) continue;                // ignore whitespaces
                int word_len = buf_pos - buf_start;                 // get length of word

                if ((line_len + word_len) > file_width) {           // word or the continuation of the word doesn't fit into the line
                   printf("\n");                                    // print new line
                   line_len = word_len + crntlen;                   // line len is length of current word and what was left in the crnt
                }
                else {                                              // we still have space on the line
                    line_len += word_len;                           // update line_len and add the length of current word
                }


                crnt = realloc(crnt, crntlen + word_len + 1);       // realloc crnt so we can copy word into it
                memcpy(&crnt[crntlen],&buf[buf_start],word_len);    // copy word into crnt
                crnt[crntlen+word_len] = '\0';                      // add null character in order to avoid memory leak
                printf("%s",crnt);                                  // print out word
                crnt = NULL;                                        
                crntlen = 0;
                buf_start = buf_pos + 1;                            // update buf_start

               
            }
            whitespace_count = 0;                                   // restart counter if it doesn't enter if statement
            newline_count = 0;                                      // restart counter if it doesn't enter if statement
        }

        if (buf_pos < buf_start) {                                  // if we have characters leftover in the buffer
            int add_on = buf_pos - buf_start;
            crnt = realloc(crnt,crntlen+add_on);
            memcpy(&crnt[crntlen],&buf[buf_start], add_on);
            crntlen += add_on;
        }

    }









    return EXIT_SUCCESS;
}

