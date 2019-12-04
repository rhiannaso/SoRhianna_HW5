/* 
    Skeleton code copied from Discussion Session Week 3 Slides by Michael Nekrasov
    - includes main function and general set up of server, connection, and socket
      - num_connections, bind(), listen(), accept(), close()
    - includes set up of structs and basic read/send commands
      - sockaddr_in (server/client) set-up, layout/format of read/send commands
*/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

int main(int argc, char * argv[]) {
    char server_ip[strlen(argv[1])+1];
    strcpy(server_ip, argv[1]);
    int port;
    sscanf(argv[2], "%i", &port);
    char message[50] = {0};

    int socket_id = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr( server_ip );
    server_address.sin_port = htons(port);

    int connection_socket_id = connect(socket_id,(struct sockaddr *) &server_address,
                                    sizeof(server_address) );

    int gamePlay = 0;

    // Check if client is ready to start the game or not
    while(strcmp(message, "y") != 0 && strcmp(message, "n") != 0) {
      printf("Ready to start game? (y/n): ");
      fgets(message, 50, stdin);

      // Add delimiter for string
      if((message[strlen(message)-1] == '\n') && strlen(message) > 0) {
        message[strlen(message)-1] = '\0';
      }

      if(strcmp(message, "y") == 0) { // If client is ready to play 
	gamePlay = 1;
	send(socket_id, "0", strlen(message), 0);
      } else if (strcmp(message, "n") == 0) {  // If client is not ready to play, terminate connection 
	close(connection_socket_id);
	return 0;
      } else { // If client enters something other than y or n.
	printf("Invalid input. Please enter y or n.\n");
      }
    }

    // When the game is being played
    while(gamePlay) {
      int check;
      check = read(socket_id, message, 50); // Read game packet from server

      message[check] = '\0';

      // If too many connections from server, close connection
      if(strcmp(message,"Server overloaded.") == 0) {
	printf("%s\n", message);
	break;
      }

      char c[2];
      c[0] = message[0];
      c[1] = '\0';
      char finalMsg[50] = {0};
      
      if(strcmp(c, "0")) { // If the server is sending a message 
	int length = atoi(c);
	printf("The word was ");
	for(int i = 0; i < length; i++) {
	  if(i != length-1) {
	    printf("%c", message[1+i]);
	  } else {
	    printf("%c\n", message[1+i]);
	  }
	}
	read(socket_id, finalMsg, 50);
	c[0] = finalMsg[0];
	length = atoi(c);
	for(int i = 0; i < length; i++) {
	  if(i != length-1) {
	    printf("%c", finalMsg[1+i]);
	  } else {
	    printf("%c\n", finalMsg[1+i]);
	  }
	}
	printf("Game Over!\n");
	gamePlay = 0;
      } else { // If the server is sending a game packet
	c[0] = message[1];
	int length = atoi(c);
	// Print the slots for the letters
	for (int i = 0; i < length; i++) {
	  if(i != length-1) {
	    printf("%c ", message[3+i]);
	  } else {
	    printf("%c\n", message[3+i]);
	  }
	}
	c[0] = message[2];
	if(strcmp(c,"0")) { // If there are incorrect guesses
	  printf("Incorrect Guesses: ");
	} else { // If there are no incorrect guesses
	  printf("Incorrect Guesses: \n\n");
	}
	// Print the incorrect guesses
	int guesses = atoi(c);
	for(int i = 0; i < guesses; i++) {
	  if(i != guesses-1) {
            printf("%c ", message[3+length+i]);
          } else {
            printf("%c\n\n", message[3+length+i]);
          }
        }

	char temp[10];
	do { // Ask user for a valid guess
	  printf("Letter to Guess: ");
	  fgets(temp, 10, stdin);
	  temp[strlen(temp)-1] = '\0';
	  if(strlen(temp) != 1 || !isalpha(temp[0]))
	    printf("Error! Please guess one letter.\n");
	} while(strlen(temp) != 1 || !isalpha(temp[0]));

	temp[0] = tolower(temp[0]);
	snprintf(message, sizeof(message), "1%s", temp);
	send(socket_id, message, strlen(message), 0); // Send response packet
	memset(temp, 0, sizeof(temp));
      }
    }
    // Close connection when game is over
    close(connection_socket_id);
    close(socket_id);
}

