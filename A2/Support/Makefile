# .DEFAULT_GOAL=alil

CC=gcc
ThreadLib=pthread

test1:
	@$(CC) test1.c sut.c a1_lib.c -l $(ThreadLib)
	./a.out

test2:
	@$(CC) test2.c sut.c a1_lib.c -l $(ThreadLib)
	./a.out

test3:
	@$(CC) test3.c sut.c a1_lib.c -l $(ThreadLib)
	./a.out

test4:
	@$(CC) test4.c sut.c a1_lib.c -l $(ThreadLib)
	./a.out

test5:
	@$(CC) test5.c sut.c a1_lib.c -l $(ThreadLib)
	./a.out

clean:
	@clear
	@rm -rf a.out
