#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <time.h> 
#include <ctype.h>
#include <arpa/inet.h>

int main (int argc, char * argv[]) {
    int port;
    sscanf(argv[1], "%i", &port);
    int num_connections = 3;
    // handle if overloaded

    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(socket_id, (struct sockaddr *) &server_address, sizeof(server_address));
    listen(socket_id, num_connections);

    while(1) {
      struct sockaddr_in client_address;
      unsigned int return_len;
      char message[50] = {0};
      int connection_socket_id = accept( socket_id,(struct sockaddr *) &client_address,
                                        &return_len );

      int gamePlay = 0;
      char words[15][10];
      // Read words from file
      FILE * f;
      int i = 0;
      f = fopen("hangman_words.txt", "r");
      if (f) {
	while(fgets(words[i], 10, f)) {
	  words[i][strlen(words[i]) - 1] = '\0';
	  i++;
	}
	fclose(f);
      }

      // Select random index of word
      srand(time(0));
      int num = (rand() % (14 - 0 + 1)) + 0; 

      // Select random word
      char word[10];
      strncpy(word, words[num], 10);
      printf("%s\n", word);

      // Read client's choice to play or not
      int read_size;
      read_size = read(connection_socket_id, message, 20);
      printf("%s\n", message);

      // If client is not ready to play, terminate connection
      if(read_size == 0) {
        close(connection_socket_id);
        continue;
      }

      // Generate game packet
      int numIncorrect = 0;
      char packet[50] = {0};
      snprintf(packet, sizeof(packet), "0%li%d", strlen(word), numIncorrect);
      for(int i = 0; i < strlen(word); i++) {
	packet[strlen(packet)] = '_';
      }	
      printf("%s\n", packet);

      // If client is ready to play, send game packet
      if(strcmp(message, "0") == 0) {
        printf("Starting game.\n");
	gamePlay = 1;
	send(connection_socket_id, packet, sizeof(packet), 0);
      }
      
      long int numCorrect = 0;
      bool match = false;

      // While the game is running
      while(gamePlay) {
	read_size = read(connection_socket_id, message, 50); // Receive guess from client 
	char guesses[14];
	char c[2];
	char guess[2];
	guess[1] = '\0';
	c[1] = '\0';
        bool checkGuess = false; // Check for duplicate correct letter guess 
        for(int i = 0; i < strlen(guesses); i++) {
	  if(message[1] == guesses[i]) {
	    checkGuess = true;
	    match = true;
            break;
          }
        }
	if(!checkGuess) { // If not a duplicate  
	  for(int i = 0; i < strlen(word); i++) {
	    c[0] = word[i];
	    guess[0] = message[1];
	    if(strcmp(c, guess) == 0) { // If guess is in the word
	      numCorrect++;
	      packet[3+i] = word[i];
	      guesses[strlen(guesses)] = message[1];
	      match = true;
	    } else { // If guess is not in the word
	      c[0] = packet[3+i-1];
	      if(strcmp(c, "_")) {
		continue;
	      } else {
		packet[3+i-1] = '_';
	      }
	    }
	  }
	}

	// If letter not in word
	if(!match) {
	  numIncorrect++;
	  packet[strlen(packet)] = message[1];
	  guesses[strlen(guesses)] = message[1];
	  packet[2] = numIncorrect + '0';
	}

	// If client has 6 incorrect guesses, end game
	if(numIncorrect == 6) {
	  memset(packet, 0, sizeof(packet));
	  memset(guesses, 0, sizeof(guesses));
          snprintf(packet, sizeof(packet), "9You Lose.");
	  gamePlay = 0;
	}

	// If client correctly guesses word, end game
	if(numCorrect == strlen(word)) {
	  memset(packet, 0, sizeof(packet));
	  memset(guesses, 0, sizeof(guesses));
	  //snprintf(packet, sizeof(packet), "%ldThe word was %s\nYou Win!", 22+numCorrect, word );
	  snprintf(packet, sizeof(packet), "8You Win!");
	  gamePlay = 0;
	}

	// Send game packet to client
	send(connection_socket_id, packet, sizeof(packet), 0);

	// Reset check for match
	match = false;
      }

      // Terminate connection after game ends
      close(connection_socket_id);
    }
    
    close(socket_id);
}
