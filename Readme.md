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