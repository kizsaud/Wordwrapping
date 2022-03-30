#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#define BUFSIZE 3


int fd_in, bytes, fd_out;                               // for reading, writing files
int file_width;                                         // this will be the width of the output file
char buf[BUFSIZE];                                      // our buffer
char* crnt;                                             // this will hold our leftover characters from the buffer
int crntlen;                                            // length of variable char* crnt
int buf_pos, buf_start;                                 // buf_pos = current position in buffer buf, buf_start = where we will start copying in to fd_out 
int line_len;                                           // keep track of how many chars are currently on the output file line 
int whitespace_count;                                   // keep track of whitespace
int newline_count;                                      // keep track of new lines
char last_char;                                         // keep track of last character
int success = 0;                                        // checks if program ran successfully. for handling "word > file_width" error
struct stat statbuff;                                   // stat structure to check incoming pathname's type
DIR* dir;                                               // variables to handle directory inputs 
struct dirent* de; 




/**
 * @brief This function prints out a word from the given file argv[2]
 * if there is enough space for it on the current line. If the the length
 * of the current word exceeds the file width, then we print out the word
 * on a new line and reset line_len. 
 * @param fd_out: This is the file descriptor that we will be writing our
 * bytes to.
 */
void add_word(int fd_out)
{
    int written = 0;
    int word_len = buf_pos - buf_start;                             // get length of word
    if (word_len > file_width) {
        success = -1;
        written = write(fd_out,"\n",1);
        if (written == -1 ) {
            if (DEBUG) printf("Problem occured while writing to fd_out\n");
            return;
        }
    }
    if ((line_len + word_len + 1) > file_width) {                   // word or the continuation of the word doesn't fit into the line                      
        written = write(fd_out,"\n",1);
        if (written == -1) return;
        line_len = word_len + 1 + crntlen;
    }
    else {                                                          // we still have space on the line
        line_len += word_len + 1;                                   // update line_len and add the length of current word
    }

    crnt = realloc(crnt, crntlen + word_len + 2);                   // realloc crnt so we can copy word into it
    memcpy(&crnt[crntlen],&buf[buf_start],word_len);                // copy word into crnt
    crnt[crntlen + word_len] = ' ';                                 // replace '\n' with a space
    crnt[crntlen+word_len + 1] = '\0';                              // add null character 
    written = write(fd_out,crnt,crntlen+word_len+1);
    if (written == -1) return;
    // fflush(stdout);
    free(crnt);
    crntlen = 0;
    crnt = NULL;
    buf_start = buf_pos + 1;                                        // update buf_start

    if (success == -1) {
        printf("\n");                                               // print another new line if word is too long
        line_len = 0;
    }
}

/**
 * @brief This function prints out a paragraph if we have 
 * met the right conditions for it. It will first print out
 * whatever is left in crnt and then print 2 newline characters.
 * 
 * @param fd_out: This is the place we are going to be writing our
 * paragraphs into.
 */
void add_paragraph(int fd_out)
{
    int written = 0;
    if(crnt != NULL){
        written = write(fd_out, &crnt[0],crntlen);
        if (written < 0) return;
    }
    written = write(fd_out,"\n\n",2);
    if (written  == -1) return;
    free(crnt);
    line_len = 0;
    crnt = NULL;
    crntlen = 0;
    newline_count = 0;
}

/**
 * @brief This function reads from the set file descriptor fd into the 
 * buffer buff. From there, we iterate through each buffer buff we get
 * and print out/write words through the add_word() and add_paragraph()
 * functions.
 * 
 * @param fd_in: This is the file descriptor from which we will be going
 * to read our bytes from.
 * 
 * @param fd_out: This is the file descriptor to which we will be going
 * to write our bytes to by calling add_word(fd_out) and add_paragraph(fd_out)
 */
void word_wrap(int fd_in, int fd_out)
{
    crnt = NULL;
    crntlen = 0;
    line_len = 0;

    // This is for reading from a file and printing out words
    while ((bytes = read(fd_in,buf,BUFSIZE)) > 0) {                     // iterate through given file
        buf_start = 0;
        for (buf_pos = 0; buf_pos < bytes; buf_pos++) {                 // iterate through buffer
            if (buf[buf_pos] == ' ' || buf[buf_pos] == '\n') {          // current char is a newline or a whitespace
                if(buf[buf_pos] == ' ') whitespace_count++;             // update counters
                if(buf[buf_pos] == '\n') newline_count++;
                if ((buf_pos == 0 && crntlen == 0) || (last_char == ' ' || last_char == '\n')) {        // check for last character. update buf_start depending on it
                    last_char = buf[buf_pos];
                    buf_start = buf_pos + 1;
                }
                else {                                                  // if last_char wasn't a new line or a whitespace, then add the word
                    add_word(fd_out);
                }
            }
            else {                                                      // current char is anything but a newline or a whitespace
                if (last_char == ' ' || last_char == '\n') {            // check last character
                    if (newline_count >= 2) add_paragraph(fd_out);      // check how many new lines we saw till now     
                }
                else {
                    newline_count = 0;
                    whitespace_count = 0;
                }
                
            }
            last_char = buf[buf_pos];
        }
        if (buf_start < buf_pos) {                                      // if we have characters leftover in the buffer
            int add_on = buf_pos - buf_start;                           // get the leftover's length
            crnt = realloc(crnt,crntlen+add_on+1);
            memcpy(&crnt[crntlen],&buf[buf_start], add_on);
            crnt[crntlen+add_on] = '\0';
            crntlen += add_on;                                          // update crntlen
            line_len += add_on;                                         // add what was left to line_len
        }
    }

    buf_pos = buf_start = 0;
    if (crntlen != 0) add_word(fd_out);                                 // if we still have something left in crnt

    return;
}


int main(int argc, char** argv)
{
    if(argc  == 3) {
        file_width = atoi(argv[1]);
        if (file_width < 1) {                                           // incorrect file width
            if (DEBUG) printf("Please supply a positive integer for the file width\n");
        }
        int x = stat(argv[2],&statbuff);
        if (x == -1) {
            perror(argv[2]);
            return EXIT_FAILURE;
        }
        if (S_ISREG(statbuff.st_mode)) {                                // given input is a regular file, works
            fd_in = open(argv[2],O_RDONLY);
            if (fd_in == -1) {
                perror(argv[2]);
                return EXIT_FAILURE;
            }
            word_wrap(fd_in,1);
            int temp = write(1,"\n",1);                                 // end of "file" with new line
            if (temp == -1) return EXIT_FAILURE;
            close(fd_in);
        }
        else if (S_ISDIR(statbuff.st_mode)) {                                                   // given input is a directory
            dir = opendir(argv[2]);
            while ((de = readdir(dir))) {
                if(de->d_name[0] == '.' || (strncmp(de->d_name,"wrap",4) == 0)) {
                    if (DEBUG) printf("File ignored.\n");
                    continue;                                                                   // ignore (.) and already wrapped files
                }
                char s[270] = "wrap.";                                                          // temporary string for output file
                // char* output_file = malloc(sizeof(char) * (5+strlen(de->d_name)));
                // output_file = strcat("wrap.",de->d_name);
                // while ((de = readdir(dir))) {
                // }
                chdir(argv[2]);
                fd_in = open(de->d_name, O_RDONLY);                                             // read current file in the directory
                fd_out = open(strcat(s,de->d_name),O_RDWR|O_CREAT|O_APPEND, 0600);              // create output file
                if (DEBUG) printf("Current file name is: %s\n", de->d_name);
                
                // if (DEBUG) printf("%s\n", getcwd(s, 100));
                word_wrap(fd_in,fd_out);
                int temp = write(fd_out,"\n",1);                                                // end of file with new line
                if (temp == -1) return EXIT_FAILURE;
                close(fd_in);
                close(fd_out);
            }
        }
    }
  

    // works
    else if (argc == 2) {                                               // we will read from stdin and write to stdout
        // FIXME: check if second argument is actually an integer

        file_width = atoi(argv[1]);
        word_wrap(0,1);                                                 // fd_in = stdin = 0, fd_out = stdout = 1
        int temp = write(fd_out,"\n",1);                                // end of file with new line
        if (temp == -1) return EXIT_FAILURE;
        close(0);
        close(1);
    }
    else {                                                              // need two more arguments
        fd_in = 0;
        if (DEBUG) printf("Please supply file width and file name\n");
        return EXIT_FAILURE;
    }

    if (success == -1) {                                                // if we were unsuccessful/ if one word is longer than fiel width
        if (DEBUG) printf("One word in the file was longer than the given file width!");
        return EXIT_FAILURE;
    }

    if (DEBUG) printf("\nProgram completed\n");


    return EXIT_SUCCESS;
}




