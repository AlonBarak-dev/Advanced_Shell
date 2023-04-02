# Advanced Programming - Shell Programming

## Redirection
> To perform Redirection using the '>' , '>>' , '2>' operators you can use the below example:

```C 
hello: ls -l >> log.txt      // will append the output of the 'ls -l' command to the EOF of log.txt
```

```C
hello: ls -l > log.txt      // will overwrite the content of log.txt with the output of 'ls -l'
```

```C
hello: ls -l 2> err.txt     // will redirect the stderr output to err.txt
```

> Multiple redirections
```C
hello: ls -l > log.txt 2> err.txt       // will redirect both stdout and stderr to log.txt and err.txt, will work for '>>' too.
```

## Renaming the Prompt
> To rename the prompt, please execute the below command:

```C
hello: prompt = new_prompt_name
```

> This command results in a new prompt name:
```C 
new_prompt_name |
```

## Echo & its extensions
> 'Echo' consists of many features in this Shell, follow the below steps to use the command correctly.

### 'Echo' for basic printing:
```C
hello: echo hello world
hello world
```

```C
hello: echo Hi $env_var how are you?
Hi $env_var_content how are you?
```

### 'Echo $?'
> This command will print the execution status of the last command.

```C
hello: ls
shell.c myshell
hello: echo $?
0       // means success, other codes mean failure
```

## 'CD' command
> 'CD' command allows the user to navigate between folders easily. follow the below instructions:

```C
hello: pwd
/home/alon/Documents/Advanced_Programming/Ex1/task_shell
hello: cd test_folder       // navigating to a folder
hello: pwd
/home/alon/Documents/Advanced_Programming/Ex1/task_shell/test_folder
```

```C
hello: pwd
/home/alon/Documents/Advanced_Programming/Ex1/task_shell
hello: cd ..
hello: pwd
/home/alon/Documents/Advanced_Programming/Ex1
```

```C
hello: pwd
/home/alon/Documents/Advanced_Programming/Ex1/task_shell
hello: cd
/home/alon/Documents/Advanced_Programming/Ex1
```

## '!!' command
> This command allows the user to re-execute the last command.

```C
hello: ls
shell.c myshell
hello: !!
shell.c myshell
```

```C
hello: ls
shell.c myshell
hello: !! > log.txt     // will execute 'ls > log.txt'
hello:
```

## Exiting from the shell
> If you wish to exit the shell program, type the next command:

```C 
hello: quit
```

## Killing child processes 
> if you executed a process from within the shell and you wish to kill/stop it, 
> follow the below instructions:

```C
hello: python3 test.py      // this will run a python script that sleeps for 10s
// Now press CTRL-C to kill the above process
^C You typed Control-C!
hello:      // Back to the shell
```

> p.s this will not help in case of using '&'


## Multi-Piping
> To use multi-piping for executing multiple commands in a pipline use the '|' symbol

This command will execute the 'ls -l' and then 'grep user_name' and finally 'wc -l'.
Eventually the final output is redirected into the log.txt file.

```C
hello: ls -l | grep user_name | wc -l > log.txt
hello:
```

This command will result in an empty output since the input for the 'grep' command 
is being redirected to 'log.txt'.

```C
hello: ls -l > log.txt | grep user_name
hello:
```

