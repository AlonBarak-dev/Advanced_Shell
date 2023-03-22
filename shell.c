#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

// init params
char command[1024];
char *token;
char* prompt_name;
int i;
char *outfile;
int fd, amper, override_stdout_redirect, append_stdout_redirect, stderr_redirect, piping, retid, status, argc1;
int fildes[2];
char *argv1[10], *argv2[10];
char *history[20];



int parse_first_part(){
    /*
        This Function parse the first part of the given command from the user.
        first part == the command before a |.
        it fills the argv1 array with tokenized commands.
        It return 0 if the command is empty, 
        else it returns 1.
    */

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

void parse_second_part(){
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

int check_amper(){
    if (! strcmp(argv1[argc1 - 1], "&")) {
        argv1[argc1 - 1] = NULL;
        return 1;
    }
    else 
        return 0; 
}

int check_override_stdout_redirection(){
    /* Does command contains a '>'*/
    if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], ">")) {
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
        return 1;
    }
    else 
        return 0;
}

int check_append_stdout_redirection(){
    /* Does command contains a '>>'*/
    if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], ">>")) {
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
        return 1;
    }
    else 
        return 0;
}

int check_stderr_redirection(){
    /* Does command contains a '2>'*/
    if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], "2>")) {
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
        return 1;
    }
    else 
        return 0;
}

int replace_prompt_name(){
    /* Does the command look like: prompt = new_prompt */
    if (argc1 > 2 && ! strcmp(argv1[0] ,"prompt") && ! strcmp(argv1[1], "="))
    {
        free(prompt_name);
        prompt_name = (char*) malloc(sizeof(char)* sizeof(argv1[2]) + 1);
        strcpy(prompt_name, argv1[2]);
        strcat(prompt_name, " ");
        return 1;
    }
    else
        return 0;
    
}

int echo(){
    /* Does the command look like: echo strings.. */
    if (argc1 > 1 && ! strcmp(argv1[0], "echo"))
    {
        if (! strcmp(argv1[1], "$?"))
        {
            printf("%d \n", status);
            return 1;
        }
        
        for (size_t i = 1; i < argc1; i++)
        {
            printf("%s ", argv1[i]);
        }
        printf("\n");
        return 1;
    }
    else
        return 0;
}

int main() {

    prompt_name = (char*)malloc(sizeof(char)*8);
    strcpy(prompt_name, "hello: ");

    while (1)
    {
        printf("%s", prompt_name);
        // wait for input from the user
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
        
        // parse the given command, if 0 -> an empty command!
        if (!parse_first_part()){
            continue;
        }

        // Does command contain pipe -> parse the second part of the command
        if (piping) {
            parse_second_part();
        }

        /* Does command line end with & */ 
        amper = check_amper();

        /* Does command contains a '>' | '>>' | '2>' */
        override_stdout_redirect = 0;
        append_stdout_redirect = 0;
        stderr_redirect = 0;

        if (!(override_stdout_redirect = check_override_stdout_redirection()))
        {
            if (!(append_stdout_redirect = check_append_stdout_redirection()))
            {
                stderr_redirect = check_stderr_redirection();
            }
            
        }

        /* Replace prompt name ? */
        if (replace_prompt_name())
        {
            continue;
        }

        /* Echo command */
        if (echo())
        {
            continue;
        }
        
        

        if (fork() == 0) { 
            /* redirection of Stdout: : */
            if (override_stdout_redirect) {
                fd = creat(outfile, 0660); 
                close (STDOUT_FILENO); 
                dup(fd); 
                close(fd); 
                /* stdout is now redirected */
            } 
            if (append_stdout_redirect){
                fd = open(outfile, O_RDWR | O_CREAT, 0666);
                lseek(fd, -1, SEEK_END);
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
            }
            /* redirection of Stderr: */
            if (stderr_redirect){
                fd = creat(outfile, 0660); 
                close (STDERR_FILENO); 
                dup(fd); 
                close(fd); 
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
