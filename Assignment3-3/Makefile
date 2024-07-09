CC = gcc
all : cli srv
EXEC1 = cli
EXEC2 = srv

cli: cli.c
	$(CC) -o $(EXEC1) $^

srv: srv.c
	$(CC) -o $(EXEC2) $^

clean: 
	rm -rf $(EXEC1)
	rm -rf $(EXEC2)

	
	