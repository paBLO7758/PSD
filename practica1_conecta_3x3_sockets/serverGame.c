#include "game.h"
#include <pthread.h>

/**
 * Sends a message to the player. This includes the length of the message and the message itself
 * @param socketClient Socket descriptor
 * @param message Message to be sent
 */
void sendMessageToPlayer (int socketClient, char* message){
	// Get message length
	unsigned int messageLength= strlen(message);

	// Send message length
	send(socketClient, &messageLength, sizeof(unsigned int), 0);

	// Send message
	send(socketClient, message, messageLength, 0);
	
}

/**
 * Receives a message from the player. This includes the length of the message and the message itself
 * @param socketClient Socket descriptor
 * @param message Message to be received
 */
void receiveMessageFromPlayer (int socketClient, char* message){
	unsigned int messageLength;

	// Receive message length
	recv(socketClient, &messageLength, sizeof(unsigned int), 0);

	// Receive message
	recv(socketClient, message, messageLength, 0);

	/*
	unsigned int totalTextReceived = 0;
    while (totalTextReceived < messageLength) {
        int bytes = recv(socketClient, message + totalTextReceived, 
                         messageLength - totalTextReceived, 0);
        if (bytes <= 0) {
            // Manejar error o desconexión
            break;
        }
        totalTextReceived += bytes;
    }
	*/

	message[messageLength] = '\0';

}

/**
 * Sends a code to the player.
 * @param socketClient Socket descriptor
 * @param code Code to be send
 */
void sendCodeToClient (int socketClient, unsigned int code){
	// Send code
	send(socketClient, &code, sizeof(unsigned int), 0);
	
}

/**
 * Sends a board to the player.
 * @param socketClient Socket descriptor
 * @param board Board of the game
 */
void sendBoardToClient (int socketClient, tBoard board){
	// Get message length
	unsigned int messageLength= sizeof(tBoard);

	// Send message length
	send(socketClient, &messageLength, sizeof(unsigned int), 0);

	// Send message
	send(socketClient, board, messageLength, 0);
	
}

/**
 * Receives a move from the player.
 * @param socketClient Socket descriptor
 * @return Move performed by the player
 */
unsigned int receiveMoveFromPlayer (int socketClient){
	unsigned int move;

	// Receive move
	recv(socketClient, &move, sizeof(unsigned int), 0);

	return move;

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
	switch (player){
		case player1:
			return player1socket;
		case player2:
			return player2socket;
		case player3:
			return player3socket;
		default:
			return -1;
	}
}

/**
 * Gets the next player to move.
 *
 * @param currentPlayer Current player
 * @return Next player
 */
tPlayer getNextPlayer (tPlayer currentPlayer){
	switch (currentPlayer){
		case player1:
			return player2;
		case player2:
			return player3;
		case player3:
			return player1;
		default:
			return -1;
	}
}

char getChipForPlayer (tPlayer player){
	switch (player){
		case player1:
			return PLAYER_1_CHIP;
		case player2:
			return PLAYER_2_CHIP;
		case player3:
			return PLAYER_3_CHIP;
		default:
			return -1;
	}
}

void *threadProcessing(void *threadArgs){
	int socketPlayer1 = ((tThreadArgs *)threadArgs)->socketPlayer1;
	int socketPlayer2 = ((tThreadArgs *)threadArgs)->socketPlayer2;
	int socketPlayer3 = ((tThreadArgs *)threadArgs)->socketPlayer3;

	free(threadArgs);

	//Receive names from players
	char player1Name[STRING_LENGTH];
	receiveMessageFromPlayer(socketPlayer1, player1Name);
	char player2Name[STRING_LENGTH];
	receiveMessageFromPlayer(socketPlayer2, player2Name);
	char player3Name[STRING_LENGTH];
	receiveMessageFromPlayer(socketPlayer3, player3Name);
	
	// Print received names
	printf("Name of player 1 received: %s\n", player1Name);
	printf("Name of player 2 received: %s\n", player2Name);
	printf("Name of player 3 received: %s\n", player3Name);

	// Send names to players
	char message1[512];
	char message2[512];
	char message3[512];

	snprintf(message1, sizeof(message1), "You are playing against %s and %s", player2Name, player3Name);	
	snprintf(message2, sizeof(message2), "You are playing against %s and %s", player1Name, player3Name);
	snprintf(message3, sizeof(message3), "You are playing against %s and %s", player1Name, player2Name);

	sendMessageToPlayer(socketPlayer1, message1);
	sendMessageToPlayer(socketPlayer2, message2);
	sendMessageToPlayer(socketPlayer3, message3);

	// Initialize the board
	tBoard board;
	initBoard(board);

	tPlayer currentPlayer = player1;
	unsigned int finished = FALSE;

	while(!finished){
		// Send turn to players
		for(tPlayer player = player1; player <= player3; player++){
			if(player == currentPlayer){
				sendCodeToClient(getSocketPlayer(player, socketPlayer1, socketPlayer2, socketPlayer3), TURN_MOVE);
				char message[STRING_LENGTH];
				snprintf(message, STRING_LENGTH, "\n\nIt's your turn. You play with %c\n", getChipForPlayer(player));
				sendMessageToPlayer(getSocketPlayer(player, socketPlayer1, socketPlayer2, socketPlayer3), message);
				sendBoardToClient(getSocketPlayer(player, socketPlayer1, socketPlayer2, socketPlayer3), board);
			}
			else {
				sendCodeToClient(getSocketPlayer(player, socketPlayer1, socketPlayer2, socketPlayer3), TURN_WAIT);
				char message[STRING_LENGTH];
				snprintf(message, STRING_LENGTH, "\n\nYour rival is thinking... please, wait! You play with %c\n", getChipForPlayer(player));
				sendMessageToPlayer(getSocketPlayer(player, socketPlayer1, socketPlayer2, socketPlayer3), message);
				sendBoardToClient(getSocketPlayer(player, socketPlayer1, socketPlayer2, socketPlayer3), board);
			}
		}

		unsigned int moveIsValid = FALSE;
		while(!moveIsValid){
			// Receive move from current player
			unsigned int move =receiveMoveFromPlayer(getSocketPlayer(currentPlayer, socketPlayer1, socketPlayer2, socketPlayer3));

			unsigned int moveResult = insertChip(board, currentPlayer, move);
			if(moveResult == OK_move){
				moveIsValid = TRUE;
			}

			sendCodeToClient(getSocketPlayer(currentPlayer, socketPlayer1, socketPlayer2, socketPlayer3), moveResult);
		}

		// Check if the current player has won
		if(checkWinner(board, currentPlayer)){
			sendCodeToClient(getSocketPlayer(currentPlayer, socketPlayer1, socketPlayer2, socketPlayer3), GAMEOVER_WIN);
			for(tPlayer player = player1; player <= player3; player++){
				if(player != currentPlayer) sendCodeToClient(getSocketPlayer(player, socketPlayer1, socketPlayer2, socketPlayer3), GAMEOVER_LOSE);
			}
			finished = TRUE;
		}
		// Check if the board is full
		else if(isBoardFull(board)){
			for(tPlayer player = player1; player <= player3; player++){
				sendCodeToClient(getSocketPlayer(player, socketPlayer1, socketPlayer2, socketPlayer3), GAMEOVER_DRAW);
			}
			finished = TRUE;
		}
		// If the game is not finished, move to the next player
		else{
			currentPlayer = getNextPlayer(currentPlayer);
		}
	}

	for(tPlayer player = player1; player <= player3; player++){
		sendBoardToClient(getSocketPlayer(player, socketPlayer1, socketPlayer2, socketPlayer3), board);
	}


	// close sockets and exit thread
	close(socketPlayer1);
	close(socketPlayer2);
	close(socketPlayer3);
	return NULL;
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

	while(1){
		// Get length of client structure
		clientLength = sizeof(player1Address);

		// Accept connections from players
		socketPlayer1 = accept(socketfd, (struct sockaddr *) &player1Address, &clientLength);
		socketPlayer1 < 0 ? fprintf(stderr,"ERROR opening player 1 socket\n") : fprintf(stdout,"Player 1 is conected!\n");

		socketPlayer2 = accept(socketfd, (struct sockaddr *) &player2Address, &clientLength);
		socketPlayer2 < 0 ? fprintf(stderr,"ERROR opening player 2 socket\n") : fprintf(stdout,"Player 2 is conected!\n");

		socketPlayer3 = accept(socketfd, (struct sockaddr *) &player3Address, &clientLength);
		socketPlayer3 < 0 ? fprintf(stderr,"ERROR opening player 3 socket\n") : fprintf(stdout,"Player 3 is conected!\n");
		
		// Create thread to manage the game
		threadArgs = malloc(sizeof(tThreadArgs));
		threadArgs->socketPlayer1 = socketPlayer1;
		threadArgs->socketPlayer2 = socketPlayer2;
		threadArgs->socketPlayer3 = socketPlayer3;

		pthread_create(&threadID, NULL, threadProcessing, (void *)threadArgs);
		pthread_detach(threadID);
	}
	close(socketfd);

}
