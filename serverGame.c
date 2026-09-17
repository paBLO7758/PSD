#include "game.h"
#include <pthread.h>

/**
 * Sends a message to the player. This includes the length of the message and the message itself
 * @param socketClient Socket descriptor
 * @param message Message to be sent
 */
void sendMessageToPlayer (int socketClient, char* message){

	
}

/**
 * Receives a message from the player. This includes the length of the message and the message itself
 * @param socketClient Socket descriptor
 * @param message Message to be received
 */
void receiveMessageFromPlayer (int socketClient, char* message){


}

/**
 * Sends a code to the player.
 * @param socketClient Socket descriptor
 * @param code Code to be send
 */
void sendCodeToClient (int socketClient, unsigned int code){

	
}

/**
 * Sends a board to the player.
 * @param socketClient Socket descriptor
 * @param board Board of the game
 */
void sendBoardToClient (int socketClient, tBoard board){

	
}

/**
 * Receives a move from the player.
 * @param socketClient Socket descriptor
 * @return Move performed by the player
 */
unsigned int receiveMoveFromPlayer (int socketClient){

	
}

/**
 * Gets the socket of the current player
 *
 * @param player Current player
 * @param player1socket Socket that connects with player 1
 * @param player2socket Socket that connects with player 2
 * @param player3socket Socket that connects with player 3
 * @return Associated socket to player
 */
int getSocketPlayer (tPlayer player, int player1socket, int player2socket, int player3socket){

	
}

/**
 * Gets the next player to move.
 *
 * @param currentPlayer Current player
 * @return Next player
 */
tPlayer getNextPlayer (tPlayer currentPlayer){

	
}

void *threadProcessing(void *threadArgs){

	
}

int main(int argc, char *argv[]){

	int socketfd;						/** Socket descriptor */
	struct sockaddr_in serverAddress;	/** Server address structure */
	unsigned int port;					/** Listening port */
	struct sockaddr_in player1Address;	/** Client address structure for player 1 */
	struct sockaddr_in player2Address;	/** Client address structure for player 2 */
	struct sockaddr_in player3Address;	/** Client address structure for player 3 */
	int socketPlayer1;					/** Socket descriptor for player 1 */
	int socketPlayer2;					/** Socket descriptor for player 2 */
	int socketPlayer3;					/** Socket descriptor for player 3 */
	unsigned int clientLength;			/** Length of client structure */
	tThreadArgs *threadArgs; 			/** Thread parameters */
	pthread_t threadID;					/** Thread ID */


	// Check arguments
	if (argc != 2) {
		fprintf(stderr,"ERROR wrong number of arguments\n");
		fprintf(stderr,"Usage:\n$>%s port\n", argv[0]);
		exit(1);
	}
    
	// Get the port
	port = atoi(argv[1]);

	socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	socketfd < 0 ? fprintf(stderr,"ERROR opening socket\n") : fprintf(stdout,"Socket created successfully\n");
    
	// Fill server address structure
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);

	// Bind
	if(bind(socketfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
		fprintf(stderr,"ERROR binding socket\n");
		exit(1);
	}

	// Listen
	listen(socketfd, 10);

	// Get length of client structure
	clientLength = sizeof(player1Address);

	// Accept connections from players
	socketPlayer1 = accept(socketfd, (struct sockaddr *) &player1Address, &clientLength);
	socketPlayer1 < 0 ? fprintf(stderr,"ERROR opening player 1 socket\n") : fprintf(stdout,"Player 1 socket created successfully\n");

	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	recv(socketPlayer1, buffer, sizeof(buffer) - 1, 0);
	printf("Mensaje recibido del cliente: %s\n", buffer);


	/*
	socketPlayer2 = accept(socketfd, (struct sockaddr *) &player2Address, &clientLength);
	socketPlayer2 < 0 ? fprintf(stderr,"ERROR opening player 2 socket\n") : fprintf(stdout,"Player 2 socket created successfully\n");

	socketPlayer3 = accept(socketfd, (struct sockaddr *) &player3Address, &clientLength);
	socketPlayer3 < 0 ? fprintf(stderr,"ERROR opening player 3 socket\n") : fprintf(stdout,"Player 3 socket created successfully\n");
	*/
}
