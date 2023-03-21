#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

// init params
char command[1024];
char *token;
int i;
char *outfile;
int fd, amper, redirect, piping, retid, status, argc1;
int fildes[2];
char *argv1[10], *argv2[10];


int parse_first_part(){
    /*
        This Function parse the first part of the given command from the user.
        first part == the command before a |.
        it fills the argv1 array with tokenized commands.
        It return 0 if the command is empty, 
        else it returns 1.
    */

    command[strlen(command) - 1] = '\0';
    // An indicator when the user use a pipe in his command
    piping = 0;
    /* parse command line */
    i = 0;
    // tokenize the command string
    token = strtok (command," ");
    while (token != NULL)
    {
        if(i >= 10){
            printf("Entered too much arguments, can only use 10 arguments! \n");
            break;
        }
        argv1[i] = token;
        token = strtok (NULL, " ");
        i++;
        // check whether the user want to use piping, if so break!
        if (token && ! strcmp(token, "|")) {
            piping = 1;
            break;
        }
    }
    argv1[i] = NULL;
    argc1 = i;

    /* Is command empty */
    if (argv1[0] == NULL)
        return 0;
    else
        return 1;
}

int main() {

    while (1)
    {
        printf("hello: ");
        // wait for input from the user
        fgets(command, 1024, stdin);
        
        // parse the given command, if 0 -> an empty command!
        if (!parse_first_part())
        {
            continue;
        }

        /* Does command contain pipe */
        if (piping) {
            i = 0;
            while (token!= NULL)
            {
                if(i >= 10){
                    printf("Entered too much arguments, can only use 10 arguments! \n");
                    break;
                }
                token = strtok (NULL, " ");
                argv2[i] = token;
                i++;
            }
            argv2[i] = NULL;
        }

        /* Does command line end with & */ 
        if (! strcmp(argv1[argc1 - 1], "&")) {
            amper = 1;
            argv1[argc1 - 1] = NULL;
            }
        else 
            amper = 0; 

        if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], ">")) {
            redirect = 1;
            argv1[argc1 - 2] = NULL;
            outfile = argv1[argc1 - 1];
            }
        else 
            redirect = 0; 

        /* for commands not part of the shell command language */ 

        if (fork() == 0) { 
            /* redirection of IO ? */
            if (redirect) {
                fd = creat(outfile, 0660); 
                close (STDOUT_FILENO); 
                dup(fd); 
                close(fd); 
                /* stdout is now redirected */
            } 
            if (piping) {
                pipe (fildes);
                if (fork() == 0) { 
                    /* first component of command line */ 
                    close(STDOUT_FILENO); 
                    dup(fildes[1]); 
                    close(fildes[1]); 
                    close(fildes[0]); 
                    /* stdout now goes to pipe */ 
                    /* child process does command */ 
                    execvp(argv1[0], argv1);
                } 
                /* 2nd command component of command line */ 
                close(STDIN_FILENO);
                dup(fildes[0]);
                close(fildes[0]); 
                close(fildes[1]); 
                /* standard input now comes from pipe */ 
                execvp(argv2[0], argv2);
            } 
            else
                execvp(argv1[0], argv1);
        }
        /* parent continues over here... */
        /* waits for child to exit if required */
        if (amper == 0)
            retid = wait(&status);
    }
}
