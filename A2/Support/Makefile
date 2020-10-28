# .DEFAULT_GOAL=alil

CC=gcc


test:
	@clear
	@rm -rf a.out
	@rm -rf sut.c
	@vim sut.c
	@$(CC) sut.c -l pthread && ./a.out
