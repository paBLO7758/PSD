#include "game.h"

/**
 * Sends a message to the server. This includes the length of the message and the message itself
 * @param socketServer Socket descriptor
 * @param message Message to be sent
 */
void sendMessageToServer (int socketServer, char* message){

	
}

/**
 * Receives a message from the server. This includes the length of the message and the message itself
 * @param socketServer Socket descriptor
 * @param message Message to be received
 */
void receiveMessageFromServer (int socketServer, char* message){

	
}


/**
 * Receives a board from the server.
 * @param socketServer Socket descriptor
 * @param board Board of the game
 */
void receiveBoard (int socketServer, tBoard board){

	
}


/**
 * Receives a code from the server.
 * @param socketServer Socket descriptor
 * @return Code
 */
unsigned int receiveCode (int socketServer){

	
}

/**
 * Reads a move entered by the player
 * @return A number between [0-BOARD_WIDTH] that represents the column where the chip is going to be inserted
 */
unsigned int readMove (){

	int i, isValid, column;
	tString enteredMove;

		// Init...
		column = 0;

		// While player does not enter a correct move...
		do{

			// Init...
			bzero (enteredMove, STRING_LENGTH);
			isValid = TRUE;

			// Show input message
			printf ("Enter a move [0-%d]:", BOARD_WIDTH-1);

			// Read move
			fgets(enteredMove, STRING_LENGTH-1, stdin);

			// Remove new-line char
			enteredMove[strlen(enteredMove)-1] = 0;

			// Check if each character is a digit
			for (i=0; i<strlen(enteredMove) && isValid; i++){

				if (!isdigit(enteredMove[i]))
					isValid = FALSE;
			}

			// Entered move is not a number
			if (!isValid){
				printf ("Entered move is not correct. It must be a number between [0-%d]\n", BOARD_WIDTH-1);
			}

			// Entered move is a number...
			else{

				// Conver entered text to an int
				column = atoi (enteredMove);
			}

		}while (!isValid);

	return ((unsigned int) column);
}

/**
 * Sends a move to the server.
 * @param socketServer Socket descriptor
 * @param move A number between [0-6] that represents the column where the chip is going to be inserted
 */
void sendMoveToServer (int socketServer, unsigned int move){

	
}


int main(int argc, char *argv[]){

	int socketfd;						/** Socket descriptor */
	unsigned int port;					/** Port number (server) */
	struct sockaddr_in server_address;	/** Server address structure */
	char* serverIP;						/** Server IP */
    tString playerName;                    /** Name of the player */

	



		// Check arguments!
		if (argc != 3){
			fprintf(stderr,"ERROR wrong number of arguments\n");
			fprintf(stderr,"Usage:\n$>%s serverIP port\n", argv[0]);
			exit(0);
		}

		// Get the server address
		serverIP = argv[1];

		// Get the port
		port = atoi(argv[2]);

		// Create socket
		socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		socketfd < 0 ? fprintf(stderr,"ERROR opening  socket\n") : fprintf(stdout,"Socket created successfully\n");

		// Fill server address structure
		memset(&server_address, 0, sizeof(server_address));
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = inet_addr(serverIP);
		server_address.sin_port = htons(port);

if (connect(socketfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    showError("ERROR connecting");

		// Connect with server
		char *msgPrueba = "¡Hola servidor, funcioan!";
		send(socketfd, msgPrueba, strlen(msgPrueba), 0);
		

		// Init player's name
		do{
			memset(playerName, 0, STRING_LENGTH);
			printf ("Enter player name:");
			fgets(playerName, STRING_LENGTH-1, stdin);

			// Remove '\n'
			playerName[strlen(playerName)-1] = 0;

		}while (strlen(playerName) <= 2);


		
    
    
    
    
    
    
    
    
    
    
    

	// Close socket
	close (socketfd);
}
