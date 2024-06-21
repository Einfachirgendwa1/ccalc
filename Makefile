default: main.c lib.c
	@clang -g -pipe main.c lib.c -o binary
	@./binary

tests: tests.c lib.c
	@clang -g -pipe tests.c lib.c -o tests
	@./tests

clean:
	@rm -f binary core tests
