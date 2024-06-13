default:
	@gcc -g -pipe main.c -o binary
	@./binary

clean:
	@rm -f binary core
