run:
	gcc -g -o myshell shell.c

runtest:
	./myshell < tests.txt > tests_output.txt

clean:
	rm myshell test  *output.txt