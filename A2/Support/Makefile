# .DEFAULT_GOAL=alil

CC=gcc

sut:
	@clear
	@rm -rf a.out
	@$(CC) sut.c a1_lib.c -l pthread && ./a.out

sut_clean:
	@clear
	@rm -rf a.out
	@rm -rf sut.c
	@vim sut.c
	@$(CC) sut.c a1_lib.c -l pthread && ./a.out

test1:
	@clear
	@rm -rf a.out
	@gcc test1.c sut.c -l pthread

test2:
	@clear
	@rm -rf a.out
	@gcc test2.c sut.c -l pthread

test3:
	@clear
	@rm -rf a.out
	@gcc test3.c sut.c -l pthread

test4:
	@clear
	@rm -rf a.out
	@gcc test4.c sut.c a1_lib.c -l pthread

test5:
	@clear
	@rm -rf a.out
	@gcc test5.c sut.c -l pthread
