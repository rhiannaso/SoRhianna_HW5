# Copied from template makefile provided to us for HW #2 by Professor Belding. 
# Basic set-up/skeleton from the template                                       
# Replaced the sample values/names with the values/names I used for this progra\
m (i.e. hangman_client, hangman_server))                                                

CC = gcc
ARGS = -Wall


# Compiling all the dependencies                                                
all: hangman_client hangman_server

hangman_client: hangman_client.c
	$(CC) $(ARGS) -o hangman_client hangman_client.c

hangman_server: hangman_server.c
	$(CC) $(ARGS) -o hangman_server hangman_server.c

clean:
	rm -f *.o hangman_client *~
	rm -f *.o hangman_server *~
