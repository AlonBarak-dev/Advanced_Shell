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
> This command allows thw user to re-execute the last command.

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



