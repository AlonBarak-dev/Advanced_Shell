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
char last_command[1024];
char result[1024];
int used_last_command;
char *token;
char* prompt_name;
int i;
char *outfile;
int fd, amper, override_stdout_redirect, append_stdout_redirect, stderr_redirect, piping, retid, status, argc1;
int fildes[2];
char *argv1[10], *argv2[10];
char path[256];


char* replaceWord(const char* s, const char* oldW,
                const char* newW)
{
    
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
 
    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;
 
            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }
 
    // Making new string of enough length 
    i = 0;
    while (*s) {
        // compare the substring with the result
        if (strstr(s, oldW) == s) {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }

    result[i] = '\0';
    return result;
}

int save_last_command(){

    if (override_stdout_redirect)
    {
        argv1[argc1 - 2] = ">";
    }
    else if(append_stdout_redirect){
        argv1[argc1 - 2] = ">>";
    }
    else if(stderr_redirect){
        argv1[argc1 - 2] = "2>";
    }
    
    strcpy(last_command, "");
    int i = 0;
    while (argv1[i] != NULL)
    {
        strcat(last_command, " ");
        strcat(last_command, argv1[i]);
        i++;
    }
    if(piping){
        i = 0;
        strcat(last_command, " ");
        strcat(last_command, "|");
        while (argv2[i] != NULL)
        {
            strcat(last_command, " ");
            strcat(last_command, argv2[i]);
            i++;
        }
    }
    
}


int parse_first_part(){
    /*
        This Function parse the first part of the given command from the user.
        first part == the command before a |.
        it fills the argv1 array with tokenized commands.
        It return 0 if the command is empty, 
        else it returns 1.
    */
    used_last_command = 0;
    // An indicator when the user use a pipe in his command
    piping = 0;
    /* parse command line */
    i = 0;
    // tokenize the command string
    char copy_command[1024];
    strcpy(copy_command, command);
    token = strtok (command," ");
    while (token != NULL)
    {
        if(i >= 10){
            printf("Entered too much arguments, can only use 10 arguments! \n");
            break;
        }
        if (!strcmp(token, "!!"))
        {
            // want to perform the last command, replace the !! with last command
            token = strtok(replaceWord(copy_command, token, last_command), " ");
            used_last_command = 1;
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

int perform_cd(){

    if (argc1 > 0 && ! strcmp(argv1[0], "cd"))
    {
        getcwd(path, 256);
        printf("Before: %s\n", path);       // Delete before submitting
        if (argc1 == 1 || ! strcmp(argv1[1], ".."))
        {
            // Go back to parent
            chdir("..");
        }
        else {
            // Go to the desired path
            strcat(path, "/");
            strcat(path, argv1[1]);
            chdir(path);
        }
        getcwd(path, 256);
        printf("After: %s\n", path);    // Delete before submitting
        return 1;
    }
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
        
        /* CD command */
        if (perform_cd())
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
        
        // save the last command

        save_last_command();
        
        
    }
}
