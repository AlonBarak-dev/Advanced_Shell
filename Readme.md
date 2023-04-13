# Advanced Programming - Shell Programming

## How to run?
> First, you need to compile the program using the makefile provided in this repo.
run:
```
make
```
later, run:
```
./myshell
```

## How to run the Test?
> First, make sure the program is compiled:
run:
```
make
```
> Later, you can use the Makefile command to run the tests:
```
make runtest
```
> Please noitce that the tests located in the tests.txt file.
> To clean you can run:
```
make clean
```

> The output of the test is located in the tests_output.txt file.

## Redirection
#### This Assignment does not support the stdin redirection as it was not mentioned in the paper!
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

## Setting & getting Environment variables
> To set a new Environment variable you should use the next commands:

Method 1.

```C
hello: $env_var = value
```

Method 2.

```C
hello: echo Enter a string
read env_var
value
hello:
```

> To get the value of an environment variable use:

```C
hello: echo $env_var
```


## Navigate between past commands
> If you wish to navigate/execute past commands follow the below instructions:

1. Once you executed some commands you will be able to go back up to 20 commands.
2. Use the up/down arrows to navigate.
3. To start navigating press the Up arrow and then press Enter.
4. Then, you can go back and forward using the arrows and Enter.

```C 
hello: ls
shell.c myshell
hello: // press the Up arrow and then press Enter
hello: ls
```

5. To execute the command you on, press Enter.

```C 
hello: ls
shell.c myshell
hello: // press the Up arrow and then press Enter
hello: ls   // press another Enter
shell.c myshell
```

6. To exit History mode, press Q and then Enter or execute a history command.

```C 
hello: ls
shell.c myshell
hello: // press the Up arrow and then press Enter
hello: ls   // press Q and then Enter -> hello: lsQ & Enter
```

## If-Else commands
> In this shell you can also write a complete If-Else component.
> The shell will execute the IF statement, if its status is 0 it will execute all commands 
> that appear in between the 'then' and 'else'/'fi' statements.
> If the status isn't 0, it will execute the commands after the 'else' (if appear).

```C
hello: if ls -l     // assume 'ls -l' status is 0.
then
echo hello world
hello world     // preform the 'then' commands
ls 
shell.c myshell
.
.
.
else
echo bye world
ls -l
echo Those commands are being ignored!
fi
```


```C
hello: if ls -l     // assume 'ls -l' status isn't 0.
then
echo hello world
echo Those commands are being ignored!
ls 
else
echo bye world
bye world
ls
shell.c myshell
echo Those commands are being executed!
Those commands are being executed!
fi
```