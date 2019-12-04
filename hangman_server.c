/* 
   > Skeleton code copied from Discussion Session Week 3 Slides by Michael Nekrasov
    - includes main function and general set up of server, connection, and socket
      - num_connections, bind(), listen(), accept(), close()
    - includes set up of structs and basic read/send commands
      - sockaddr_in (server/client) set-up, layout/format of read/send commands
   > Multiple Connections Socket Programming code retrieved from GeeksForGeeks
    https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
    - Article written by Akshat Sinha
    - Copied the provided code and replaced incoming connection section with what
      the hangman server needed to do to set up the game. Also replaced IO 
      operations on some other socket section with necessary code for each 
      iteration of a game.
   > Reading in File help found from w3resource.com
     https://www.w3resource.com/c-programming-exercises/file-handling/c-file-handling-exercise-4.php
     - Followed code as a template from lines 10-25
   > Generating a Random Number help from GeeksForGeeks
     https://www.geeksforgeeks.org/generating-random-number-range-c/
     - Article written by Upendra Bartwal
     - Used code as a template for pulling a random word
   > Building Packets/Strings from Variables help from StackOverflow
     https://stackoverflow.com/questions/4881937/building-strings-from-variables-in-c
     - Answer written by Ed S.
     - Used to construct packets in the appropriate form
*/

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
    int num_connections = 3, num_current_conn = 0, max_clients = 3;
    int connection_socket_id, sd, max_sd, read_size;
    char words[15][10]; // Words from txt file
    
    // Info to keep track of per client
    int client_sockets[3];
    int numIncorrect[3] = {0};
    long int numCorrect[3] = {0};
    char chosenWord[3][10] = {0}, packets[3][50] = {0}, guesses[3][14] = {0};

    fd_set readfds;

    // initialize clients to 0 so they are not checked
    for(int i = 0; i < max_clients; i++) {
      client_sockets[i] = 0;
    }

    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(socket_id, (struct sockaddr *) &server_address, sizeof(server_address));
    listen(socket_id, num_connections);

    while(1) { // awaiting new connections
      // clear socket set
      FD_ZERO(&readfds);

      // add main socket to set
      FD_SET(socket_id, &readfds);
      max_sd = socket_id;

      //add child sockets to set  
      for (int i = 0 ; i < max_clients ; i++) {
	//socket descriptor  
        sd = client_sockets[i];   
                 
        //if valid socket descriptor then add to read list  
        if(sd > 0)
	  FD_SET(sd , &readfds);   
                 
        //highest file descriptor number, need it for the select function  
        if(sd > max_sd)   
          max_sd = sd;
      }

      // check for activity on the sockets
      select( max_sd + 1 , &readfds , NULL , NULL , NULL);
      
      struct sockaddr_in client_address;
      unsigned int return_len;
      char message[50] = {0};

      // incoming connection
      if (FD_ISSET(socket_id, &readfds)) {
	if(num_current_conn == max_clients) { // if 3 clients already running game
	  connection_socket_id = accept( socket_id,(struct sockaddr *) &client_address,
					 &return_len );
	  char packet[20] = "Server overloaded.";
	  send(connection_socket_id, packet, sizeof(packet), 0);
	  close(connection_socket_id);
	} else { // if less than 3 clients running game
	  num_current_conn++;
	  connection_socket_id = accept( socket_id,(struct sockaddr *) &client_address,
                                        &return_len );

	  // add new connection to array of sockets
	  int conn;
	  for (int i = 0; i < max_clients; i++) {
	    //if position is empty  
            if( client_sockets[i] == 0 ) {
	      client_sockets[i] = connection_socket_id;
	      conn = i;
              break;   
            }   
          }   
	  
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
	  strncpy(chosenWord[conn], words[num], 10);

	  // Read client's choice to play or not
	  read_size = read(connection_socket_id, message, 20);

	  // If client is not ready to play, terminate connection
	  if(read_size == 0) {
	    close(connection_socket_id);
	    client_sockets[conn] = 0;
	    num_current_conn--;
	  }

	  // Generate game packet
	  snprintf(packets[conn], sizeof(packets[conn]), "0%li%d", strlen(chosenWord[conn]), numIncorrect[conn]);
	  for(int i = 0; i < strlen(chosenWord[conn]); i++) {
	    packets[conn][strlen(packets[conn])] = '_';
	  }	

	  // If client is ready to play, send game packet
	  if(strcmp(message, "0") == 0) {
	    send(connection_socket_id, packets[conn], sizeof(packets[conn]), 0);
	  }
	}
      }

      // While the game is running
      for (int i = 0; i < max_clients; i++) {
	sd = client_sockets[i];
        if (FD_ISSET( sd , &readfds)) {
	  bool match = false;
	
	  read_size = read(sd, message, 50); // Receive guess from client 
	  char c[2];
	  char guess[2];
	  guess[1] = '\0';
	  c[1] = '\0';
	  bool checkGuess = false; // Check for duplicate correct letter guess 
	  for(int j = 0; j < strlen(guesses[i]); j++) {
	    if(message[1] == guesses[i][j]) {
	      checkGuess = true;
	      match = true;
	      break;
	    }
	  }
	  if(!checkGuess) { // If not a duplicate  
	    for(int j = 0; j < strlen(chosenWord[i]); j++) {
	      c[0] = chosenWord[i][j];
	      guess[0] = message[1];
	      if(strcmp(c, guess) == 0) { // If guess is in the word
		numCorrect[i]++;
		packets[i][3+j] = chosenWord[i][j];
		guesses[i][strlen(guesses[i])] = message[1];
		match = true;
	      } else { // If guess is not in the word
		c[0] = packets[i][3+j-1];
		if(strcmp(c, "_")) {
		  continue;
		} else {
		  packets[i][3+j-1] = '_';
		}
	      }
	    }
	  }

	  // If letter not in word
	  if(!match) {
	    numIncorrect[i]++;
	    packets[i][strlen(packets[i])] = message[1];
	    guesses[i][strlen(guesses[i])] = message[1];
	    packets[i][2] = numIncorrect[i] + '0';
	  }

	  // If client has 6 incorrect guesses, end game
	  if(numIncorrect[i] == 6) {
	    memset(packets[i], 0, sizeof(packets[i]));
	    memset(guesses[i], 0, sizeof(guesses[i]));
	    char finalPacket[50] = {0};
	    snprintf(finalPacket, sizeof(finalPacket), "%ld%s", strlen(chosenWord[i]), chosenWord[i] );
	    snprintf(packets[i], sizeof(packets[i]), "9You Lose.");
	    send(sd, finalPacket, sizeof(finalPacket), 0); // Send word reveal
	    send(sd, packets[i], sizeof(packets[i]), 0); // Send loss message
	    close(sd); // Terminate connection after game ends
	    num_current_conn--;
	    
	    // Reset client info 
	    client_sockets[i] = 0;
	    numCorrect[i] = 0;
	    numIncorrect[i] = 0;
	    memset(packets[i], 0, sizeof(packets[i]));
	    memset(guesses[i], 0, sizeof(guesses[i]));
	    memset(chosenWord[i], 0, sizeof(chosenWord[i]));
	  }

	  // If client correctly guesses word, end game
	  if(numCorrect[i] == strlen(chosenWord[i])) {
	    memset(packets[i], 0, sizeof(packets[i]));
	    memset(guesses[i], 0, sizeof(guesses[i]));
	    char finalPacket[50] = {0};
	    snprintf(finalPacket, sizeof(finalPacket), "%ld%s", strlen(chosenWord[i]), chosenWord[i]);
	    snprintf(packets[i], sizeof(packets[i]), "8You Win!");
	    send(sd, finalPacket, sizeof(finalPacket), 0); // Send word reveal
	    send(sd, packets[i], sizeof(packets[i]), 0); // Send win message
	    close(sd); // Terminate connection after game ends
	    num_current_conn--;

	    // Reset client info
	    client_sockets[i] = 0;
	    numCorrect[i] = 0;
	    numIncorrect[i] = 0;
	    memset(packets[i], 0, sizeof(packets[i]));
	    memset(guesses[i], 0, sizeof(guesses[i]));
	    memset(chosenWord[i], 0, sizeof(chosenWord[i]));
	  }

	  // Send game packet to client
	  send(sd, packets[i], sizeof(packets[i]), 0);

	  // Reset check for match
	  match = false;
	}
      }
    }
    
    close(socket_id);
}
