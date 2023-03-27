#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>
#include <envz.h>

// init params
char command[1024];
char last_command[1024];
char result[1024];
int used_last_command;
int number_of_pipes;
char *token;
char* prompt_name;
int i;
char *stdout_outfile;
char* stderr_outfile;
int red_std_idx, red_err_idx, red_std_in_arg_idx, red_err_in_arg_idx;
int fd, amper, override_stdout_redirect, append_stdout_redirect, stderr_redirect, piping, retid, status, argc1;
int fildes[2];
int *pipes_fd;
char *argv1[10], *argv2[10];
char path[256];
pid_t pid_1;
struct args** arguments;
int number_of_arguments;
struct stack** command_history;
int history_ptr;

/* Command Stack area */
typedef struct stack{
    char* commands[20];
    int first_idx, last_idx;
}stack;


void create_history(){
    command_history = (stack**)malloc(sizeof(stack*));
    *command_history = (stack*)malloc(sizeof(stack));
    (*command_history)->first_idx = 0;
    (*command_history)->last_idx = 0;
}

void push(char* command){

    int first = (*command_history)->first_idx % 20;
    int last = (*command_history)->last_idx % 20;
    
    if (first == last)
    {
        // first push
        (*command_history)->commands[last] = (char*)malloc(sizeof(char)*30);
        strcpy((*command_history)->commands[last], command);
        (*command_history)->last_idx++;
    }
    else if(last == first - 1){
        // stack is full
        (*command_history)->first_idx++;
        (*command_history)->last_idx++;
        strcpy((*command_history)->commands[last], command);
    }
    else{
        (*command_history)->commands[last] = (char*)malloc(sizeof(char)*30);
        strcpy((*command_history)->commands[last], command);
        (*command_history)->last_idx++;
    }
}


char* get(int idx){
    return (*command_history)->commands[idx];
}


/* Command Stack area */



/* Args struct area */

typedef struct arg
{
    char* arg[10];
    int argc;
} arg;

typedef struct args 
{
    arg argument;
    struct args* next;
} args;

args* create_args(){
    // allocate memory for the first argument
    args* argv = (struct args*)malloc(sizeof(args));
    return argv;
}

args* get_last_argument(){
    args* copy = *arguments;
    while (copy->next != NULL)
    {
        copy = copy->next;
    }
    return copy;
    
}

void add_argument(char* arg[10]){
    // get last argument
    args* last_argument = get_last_argument();
    // allocate memory for a new argument
    if (number_of_arguments > 0)
    {
        last_argument->next = (args*)malloc(sizeof(args));
        last_argument = last_argument->next;    
    }
    
    // define args 
    last_argument->argument.argc = 0;
    for (size_t i = 0; i < 10; i++)
    {
        if (arg[i] != NULL)
        {
            last_argument->argument.arg[i] = (char*)malloc(sizeof(char)*96);
            strcpy(last_argument->argument.arg[i], arg[i]);
            last_argument->argument.argc++;
        }
        else{
            break;
        }
    }
    number_of_arguments++;
}


/* Args struct area */



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

    args* arg_ptr;
    int argc;

    if (override_stdout_redirect)
    {
        arg_ptr = *arguments;
        int idx = 0;
        while (idx < red_std_idx)
        {
            arg_ptr = arg_ptr->next;
        }
        argc = arg_ptr->argument.argc;
        arg_ptr->argument.arg[red_std_in_arg_idx] = ">";
    }
    else if(append_stdout_redirect){
        arg_ptr = *arguments;
        int idx = 0;
        while (idx < red_std_idx)
        {
            arg_ptr = arg_ptr->next;
        }
        argc = arg_ptr->argument.argc;
        arg_ptr->argument.arg[red_std_in_arg_idx] = ">>";
    }
    if(stderr_redirect){
        arg_ptr = *arguments;
        int idx = 0;
        while (idx < red_std_idx)
        {
            arg_ptr = arg_ptr->next;
        }
        argc = arg_ptr->argument.argc;
        arg_ptr->argument.arg[red_err_in_arg_idx] = "2>";
    }
    
    strcpy(last_command, "");

    arg_ptr = *arguments;
    while (arg_ptr != NULL)
    {
        for(size_t i = 0; i < arg_ptr->argument.argc; i++)
        {
            strcat(last_command, " ");
            strcat(last_command, arg_ptr->argument.arg[i]);
        }
        arg_ptr = arg_ptr->next;
        if (arg_ptr != NULL)
        {
            strcat(last_command, " |");
        }
    }
    push(last_command);
    
}



int parse_command(){

    // init arguments struct
    arguments = (args**)malloc(sizeof(args*));
    *arguments = create_args();
    number_of_arguments = 0;
    // An indicator when the user use a pipe in his command
    piping = 0;
    // reset
    number_of_pipes = 0;
    /* parse command line */
    int empty_command = 0;
    // deep copy of the command in case the user typed '!!'
    char copy_command[1024];
    strcpy(copy_command, command);  
    // tokenize the command string
    token = strtok (command," ");
    // Dynamic allocation for argv
    char* argv[10];
    for (size_t i = 0; i < 10; i++)
    {
        argv[i] = NULL;
    }
    
    int i = 0;
    while (token != NULL)
    {
        empty_command = 1;
        if (!strcmp(token, "!!"))
        {
            // want to perform the last command, replace the !! with last command
            token = strtok(replaceWord(copy_command, token, last_command), " ");
            used_last_command = 1;
        }
        argv[i] = token;
        token = strtok (NULL, " ");
        i++;
        // check for piping
        if (token && ! strcmp(token, "|"))
        {
            add_argument(argv);
            // reset argv
            for (size_t i = 0; i < 10; i++)
            {
                argv[i] = NULL;
            }
            number_of_pipes++;
            i = 0;
            token = strtok (NULL, " ");
            piping = 1;
        }
        
    }
    // add the last argument
    add_argument(argv);
    return empty_command;
}



int check_amper(){

    args* last_arg = get_last_argument();
    int last_argc = last_arg->argument.argc;
    if (! strcmp(last_arg->argument.arg[last_argc - 1], "&"))
    {
        last_arg->argument.arg[last_argc - 2] = NULL;
        return 1;
    }
    return 0; 
}

int check_override_stdout_redirection(){
    /* Does command contains a '>'*/

    args* arg_ptr = *arguments;
    int argc = arg_ptr->argument.argc;
    red_std_idx = 0;
    while (arg_ptr)
    {
        argc = arg_ptr->argument.argc;
        for (size_t i = 1; i < argc - 1; i++)
        {
            if (argc > 1 && ! strcmp(arg_ptr->argument.arg[i], ">"))
            {
                arg_ptr->argument.arg[i] = NULL;
                stdout_outfile = arg_ptr->argument.arg[i + 1];
                red_std_in_arg_idx = i;
                return 1;
            }   
        }
        arg_ptr = arg_ptr->next;
        red_std_idx++;
    }
    red_std_idx = -1;
    return 0;
}

int check_append_stdout_redirection(){
    /* Does command contains a '>>'*/
    
    args* arg_ptr = *arguments;
    int argc = arg_ptr->argument.argc;
    red_std_idx = 0;
    while (arg_ptr)
    {
        argc = arg_ptr->argument.argc;
        for (size_t i = 1; i < argc - 1; i++)
        {
            if (argc > 1 && ! strcmp(arg_ptr->argument.arg[i], ">>"))
            {
                arg_ptr->argument.arg[i] = NULL;
                stdout_outfile = arg_ptr->argument.arg[i + 1];
                red_std_in_arg_idx = i;
                return 1;
            }   
        }
        arg_ptr = arg_ptr->next;
        red_std_idx++;
    }
    red_std_idx = -1;
    return 0;
}

int check_stderr_redirection(){
    /* Does command contains a '2>'*/
    args* arg_ptr = *arguments;
    int argc = arg_ptr->argument.argc;
    red_err_idx = 0;
    while (arg_ptr)
    {
        argc = arg_ptr->argument.argc;
        for (size_t i = 1; i < argc - 1; i++)
        {
            if (argc > 1 && arg_ptr->argument.arg[i] != NULL && ! strcmp(arg_ptr->argument.arg[i], "2>"))
            {
                arg_ptr->argument.arg[i] = NULL;
                stderr_outfile = arg_ptr->argument.arg[i + 1];
                red_err_in_arg_idx = i;
                return 1;
            }   
        }
        arg_ptr = arg_ptr->next;
        red_err_idx++;
    }
    return 0;
}

int replace_prompt_name(){
    /* Does the command look like: prompt = new_prompt */
    args* last_arg = get_last_argument();
    int last_argc = last_arg->argument.argc;
    if (last_argc > 2 && ! strcmp(last_arg->argument.arg[0] ,"prompt") && ! strcmp(last_arg->argument.arg[1], "="))
    {
        free(prompt_name);
        prompt_name = (char*) malloc(sizeof(char)* sizeof(last_arg->argument.arg[2]) + 1);
        strcpy(prompt_name, last_arg->argument.arg[2]);
        strcat(prompt_name, " ");
        return 1;
    }
    else
        return 0;
    
}

int echo(){
    /* Does the command look like: echo strings.. */
    args* ptr_arg = *arguments;
    int ptr_argc = ptr_arg->argument.argc;

    while (ptr_arg)
    {
        if (ptr_argc > 1 && ! strcmp(ptr_arg->argument.arg[0], "echo"))
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                if (override_stdout_redirect) {
                    fd = creat(stdout_outfile, 0660); 
                    close (STDOUT_FILENO); 
                    dup(fd); 
                    close(fd); 
                    /* stdout is now redirected */
                } 
                if (append_stdout_redirect){
                    fd = open(stdout_outfile, O_RDWR | O_CREAT, 0666);
                    lseek(fd, -1, SEEK_END);
                    close(STDOUT_FILENO);
                    dup(fd);
                    close(fd);
                }
                /* redirection of Stderr: */
                if (stderr_redirect){
                    fd = creat(stderr_outfile, 0660); 
                    close (STDERR_FILENO); 
                    dup(fd); 
                    close(fd); 
                }
                if (! strcmp(ptr_arg->argument.arg[1], "$?"))
                {
                    printf("%d \n", status);
                    exit(0);
                }

                if (ptr_arg->argument.arg[1][0] == '$')
                {
                    // get Environment variable
                    printf("%s \n", getenv(ptr_arg->argument.arg[1]+1));
                    exit(0);
                }
                
                int i = 1;
                while (ptr_arg->argument.arg[i])
                {
                    printf("%s ", ptr_arg->argument.arg[i]);
                    i++;
                }
                printf("\n");
                exit(0);
            }
            else{
                wait(&status);
                return 1;
            }
        }
        if (ptr_arg->next)
        {
            // move to the next argument
            ptr_arg = ptr_arg->next;
            ptr_argc = ptr_arg->argument.argc;
        }
        else{
            return 0;
        }
    }
    
}

int perform_cd(){

    args* last_arg = get_last_argument();
    int last_argc = last_arg->argument.argc;

    if (last_argc > 0 && ! strcmp(last_arg->argument.arg[0], "cd"))
    {
        getcwd(path, 256);
        printf("Before: %s\n", path);       // Delete before submitting
        if (last_argc == 1 || ! strcmp(last_arg->argument.arg[1], ".."))
        {
            // Go back to parent
            chdir("..");
        }
        else {
            // Go to the desired path
            strcat(path, "/");
            strcat(path, last_arg->argument.arg[1]);
            chdir(path);
        }
        getcwd(path, 256);
        printf("After: %s\n", path);    // Delete before submitting
        return 1;
    }
    return 0;
    
}

int quit(){
    args* last_arg = get_last_argument();
    int last_argc = last_arg->argument.argc;
    if (last_argc > 0 && ! strcmp(last_arg->argument.arg[0], "quit"))
    {
        return 1;
    }
    return 0;
    
}

void int_handler(int signal){
    printf("You typed Control-C!\n");
    if(pid_1 == 0){
        // kill(pid_1, SIGINT);
        printf("kill%d\n", pid_1);
        exit(0);
    }
    
}

void execute_pipes(){   //TODO 

    // allocate file discriptors for piping
    pipes_fd = (int*)malloc(sizeof(int) * 2 * number_of_pipes);
    // create pipes
    for (size_t i = 0; i < number_of_pipes; i++){
        if(pipe(pipes_fd + i*2) < 0) {
            perror("couldn't pipe");
            exit(EXIT_FAILURE);
        }
    }
    int idx = 0;
    int arg_idx = 0;
    args* ptr_arg = *arguments;
    pid_t pid;
    while (ptr_arg)
    {
        pid = fork();
        if (pid == 0)
        {
            // if not last argument
            if (ptr_arg->next)
            {
                if (dup2(pipes_fd[idx+1], 1) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            if (arg_idx == red_std_idx)
            {
                if (override_stdout_redirect) {
                    fd = creat(stdout_outfile, 0660); 
                    close (STDOUT_FILENO); 
                    dup(fd); 
                    close(fd); 
                    /* stdout is now redirected */
                } 
                if (append_stdout_redirect){
                    fd = open(stdout_outfile, O_RDWR | O_CREAT, 0666);
                    lseek(fd, -1, SEEK_END);
                    close(STDOUT_FILENO);
                    dup(fd);
                    close(fd);
                }
            }
            if (arg_idx == red_err_idx)
            {
                /* redirection of Stderr: */
                if (stderr_redirect){
                    fd = creat(stderr_outfile, 0660); 
                    close (STDERR_FILENO); 
                    dup(fd); 
                    close(fd); 
                }
            }
            
            // if not first command && j!= 2*number_of_pipes
            if (idx != 0)
            {
                if (dup2(pipes_fd[idx-2], 0) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            for (size_t i = 0; i < 2*number_of_pipes; i++)
            {
                close(pipes_fd[i]);
            }
            if (execvp(*(ptr_arg->argument.arg), ptr_arg->argument.arg) < 0)
            {
                perror(*ptr_arg->argument.arg);
                exit(EXIT_FAILURE);
            } 
        }
        else if (pid < 0)
        {
            perror("error");
            exit(EXIT_FAILURE);
        }
        ptr_arg = ptr_arg->next;
        idx += 2;
        arg_idx++;
    }

    for (size_t i = 0; i < 2*number_of_pipes; i++)
    {
        close(pipes_fd[i]);
    }

    for(i = 0; i < number_of_pipes + 1; i++)
        wait(&status);
    

}

int set_env(){
    args* last_arg = get_last_argument();
    int last_argc = last_arg->argument.argc;

    if (last_argc > 2 && last_arg->argument.arg[0][0] == '$' && ! strcmp(last_arg->argument.arg[1], "="))
    {
        // set new environment variable
        char* new_var = (char*) malloc(sizeof(char)*sizeof(last_arg->argument.arg));
        strcpy(new_var, "");
        int i = 2;
        while (last_arg->argument.arg[i])
        {
            strcat(new_var, last_arg->argument.arg[i]);
            strcat(new_var, " ");
            i++;
        }
        
        setenv((last_arg->argument.arg[0] + 1), new_var, 1);
        return 1;
    }
    return 0;
}

int read_var(){
    args* last_arg = get_last_argument();
    int last_argc = last_arg->argument.argc;

    if (last_argc > 3 && !strcmp(last_arg->argument.arg[0], "echo") && !strcmp(last_arg->argument.arg[1], "Enter") && !strcmp(last_arg->argument.arg[2], "a") && !strcmp(last_arg->argument.arg[3], "string"))
    {
        // the user wants to define a new env variable
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
        // parse the given command, if 0 -> an empty command!
        if (!parse_command()){
            return 0;
        }
        args* last_arg = get_last_argument();
        int last_argc = last_arg->argument.argc;
        if (last_argc > 1 && ! strcmp(last_arg->argument.arg[0], "read"))
        {
            // save the variable name
            char* var_name = (char*)malloc(sizeof(char)*sizeof(last_arg->argument.arg[1]));
            strcpy(var_name, last_arg->argument.arg[1]);

            // read the variable content
            fgets(command, 1024, stdin);
            command[strlen(command) - 1] = '\0';
            // parse the given command, if 0 -> an empty command!
            if (!parse_command()){
                return 0;
            }
            args* last_arg = get_last_argument();
            int last_argc = last_arg->argument.argc;
            // set new environment variable
            char* new_var = (char*) malloc(sizeof(char)*sizeof(last_arg->argument.arg));
            strcpy(new_var, "");
            int i = 0;
            while (last_arg->argument.arg[i])
            {
                strcat(new_var, last_arg->argument.arg[i]);
                strcat(new_var, " ");
                i++;
            }
            
            setenv(var_name, new_var, 1);
            return 1;
        }
        return 0;

    }
    return 0;
    
}

int main() {

    prompt_name = (char*)malloc(sizeof(char)*8);
    strcpy(prompt_name, "hello: ");
    signal(SIGINT, int_handler);
    pid_1 = -1;
    // create the history stack
    history_ptr = 0;
    create_history();

    while (1)
    {
        printf("%s", prompt_name);
        // wait for input from the user
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';

        // parse the given command, if 0 -> an empty command!
        if (!parse_command()){
            continue;
        }

        /* Check if the user wants to QUIT */
        if (quit())
        {
            // exit with code 0
            exit(0);
        }
        
        /* Does command line end with & */ 
        amper = check_amper();

        /* Does command contains a '>' | '>>' | '2>' */
        override_stdout_redirect = 0;
        append_stdout_redirect = 0;
        stderr_redirect = 0;

        if (!(override_stdout_redirect = check_override_stdout_redirection())){
            append_stdout_redirect = check_append_stdout_redirection();
        }
        stderr_redirect = check_stderr_redirection();

        /* Replace prompt name ? */
        if (replace_prompt_name())
        {
            continue;
        }

        if (read_var())
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

        /* New Env var */
        if (set_env())
        {
            continue;
        }
        

        execute_pipes();
        /* parent continues over here... */
        /* waits for child to exit if required */
        if (amper == 0)
            retid = wait(&status);
        
        // save the last command

        save_last_command();
        
        
    }
}
