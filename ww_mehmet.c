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

int fd, bytes;                                          // for reading the file
int file_width;                                         // this will be the width of the output file
char buf[BUFSIZE];                                      // our buffer
char* crnt;                                             // our output file, wrap+argv[1]
int crntlen;    
int buf_pos, buf_start;                                 // current position in buffer buf
int line_len;                                           // keep track of how many chars are 
int whitespace_count;                                   // keep track of whitespace
int newline_count;                                      // keep track of new lines
char last_char;                                         // keep track of last character


void init_lines(void)
{
    lines = malloc(sizeof(char *) * LISTLEN);
    line_count = 0;
    line_array_size = LISTLEN;
}

// void add_line(char *p)
// {
//     if (DEBUG) printf("Adding |%s|\n", p);
//     if (line_count == line_array_size) {
// 	line_array_size *= 2;
// 	lines = realloc(lines, line_array_size * sizeof(char *));
// 	// TODO: check whether lines is NULL
//     }

//     lines[line_count] = p;
//     line_count++;
// }

void add_word()
{
    int word_len = buf_pos - buf_start + 1;                 // get length of word

    if ((line_len + word_len) > file_width) {               // word or the continuation of the word doesn't fit into the line
        printf("\n");                                       // print new line
        line_len = word_len;                                // line len is length of current word and what was left in the crnt
    }
    else {                                                  // we still have space on the line
        line_len += word_len;                               // update line_len and add the length of current word
    }

    crnt = realloc(crnt, crntlen + word_len + 1);           // realloc crnt so we can copy word into it
    memcpy(&crnt[crntlen],&buf[buf_start],word_len);        // copy word into crnt
    crnt[crntlen+word_len] = '\0';                          // add null character in order to avoid memory leak
    printf("%s",crnt);                                      // print out word
    fflush(stdout);
    free(crnt);
    crntlen = 0;
    crnt = NULL;
    buf_start = buf_pos + 1;                                // update buf_start
 
}



// char wrap[5]  = {'w','r','a','p','\0'};

int main(int argc, char** argv)
{
    //init_lines();
    if(argc  == 3) {
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
    else if (argc > 3) {
        if (DEBUG) printf("Too many arguments");
        return EXIT_FAILURE;
    }
    else if (argc == 2) {
        if (DEBUG) printf("Please supply file/directory name");
        return EXIT_FAILURE;
    }
    else {
        fd = 0;
        if (DEBUG) printf("Please supply file width and file name");
        return EXIT_FAILURE;
    }


    crnt = NULL;
    crntlen = 0;
    line_len = 0;

    // This is for reading from a file and printing out words
    while ((bytes = read(fd,buf,BUFSIZE)) > 0) {
        buf_start = 0;
        for (buf_pos = 0; buf_pos < bytes; buf_pos++) {
            // statements for updating whitespace and newline count
            if(buf[buf_pos] != ' ') whitespace_count = 0;
            if (buf[buf_pos] != '\n') newline_count = 0;
            
            if (buf[buf_pos] == '\n') {
                newline_count++;
                if (newline_count > 1) {                            // if we have a paragraph
                    printf("%s",crnt);
                    fflush(stdout);
                    printf("\n\n");
                    line_len = 0;
                    crnt = NULL;
                    crntlen = 0;
                }
                if (buf_pos < bytes && buf[buf_pos+1] != '\n') {
                    add_word();
                }
            }
            if (buf[buf_pos] == ' ') {
                whitespace_count++;
                if((buf_pos == 0 && crntlen == 0)){           
                    buf_start = buf_pos + 1;
                    continue;
                }
                if (last_char == ' ' && whitespace_count > 1) {
                    while(buf_pos+1 < bytes && buf[buf_pos] == ' ') {
                        buf_pos++;
                        buf_start = buf_pos;
                        last_char = buf[buf_pos];
                    }
                    continue;
                }

                add_word();
            }
            last_char = buf[buf_pos];
        }

        if (buf_start < buf_pos) {                                  // if we have characters leftover in the buffer
            if (buf[buf_start] == ' ') continue;
            int add_on = buf_pos - buf_start;
            crnt = realloc(crnt,crntlen+add_on+1);
            memcpy(&crnt[crntlen],&buf[buf_start], add_on);
            crnt[crntlen+add_on] = '\0';
            crntlen += add_on;
            line_len += add_on;
        }

    }





    return EXIT_SUCCESS;
}

