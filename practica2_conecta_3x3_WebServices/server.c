#include "soapH.h"
#include "game.h"
#include "conecta3x3.nsmap"
#include <pthread.h>

/** Maximum number of active games in the server */
#define MAX_GAMES 5

/** Flag to enable debugging */
#define DEBUG_SERVER 1

/** Type for game status */
typedef enum {gameEmpty, gameWaitingPlayer, gameReady} tGameState;

/**
 * Struct that contains a game for the players
 */
typedef struct game{
	tGameState state;
	char board[BOARD_WIDTH * BOARD_HEIGHT];

	char player1Name[STRING_LENGTH];
	char player2Name[STRING_LENGTH];
	char player3Name[STRING_LENGTH];
    
    int numPlayers;
	int activePlayers;
	conecta3x3ns__tPlayer currentPlayer;

    pthread_mutex_t mutex;
	pthread_cond_t condTurn;
    
}tGame;

/** Array with games */
tGame games[MAX_GAMES];



/**
 * Initializes server structures
 * @param soap Soap environment
 */
void initServerStructures (struct soap *soap);

/**
 * Calculates the next player
 *
 * @param currentPlayer Current player
 * @return Player that is currently awaiting.
 */
conecta3x3ns__tPlayer calculateNextPlayer (conecta3x3ns__tPlayer currentPlayer);

/**
 * Calculates a random player to start the game
 *
 * @return Player that starts the game.
 */
conecta3x3ns__tPlayer calculateRandomPlayer ();


/**
 * Search for a game.
 *
 * This function returns a number between [0-MAX_GAMES] if there
 * exists an empty game, or ERROR_SERVER_FULL in other case.
 */
int searchEmptyGame ();

/**
 * Thread function to process client requests
 */
void *processRequest(void*);





void initServerStructures (struct soap *soap){

	if (DEBUG_SERVER)
		printf ("Initializing...\n");

	// Init seed
	srand (time(NULL));

	// Init each game
	for (int i=0; i<MAX_GAMES; i++){
		games[i].state = gameEmpty;
        games[i].numPlayers = 0;
        games[i].activePlayers = 0;
        games[i].currentPlayer = player1;
        
        // Limpiar el tablero
        initBoard(games[i].board);
        
        // Inicializar los cerrojos y variables de condición
        pthread_mutex_init(&games[i].mutex, NULL);
        pthread_cond_init(&games[i].condTurn, NULL);

        // Limpiar cadenas de texto
        memset(games[i].player1Name, 0, STRING_LENGTH);
        memset(games[i].player2Name, 0, STRING_LENGTH);
        memset(games[i].player3Name, 0, STRING_LENGTH);
	
	}
}

conecta3x3ns__tPlayer calculateNextPlayer (conecta3x3ns__tPlayer currentPlayer){
	conecta3x3ns__tPlayer nextPlayer = player1;
	if (currentPlayer == player1)
        nextPlayer = player2;
    if (currentPlayer == player2)
        nextPlayer = player3;
	return nextPlayer;
}

conecta3x3ns__tPlayer calculateRandomPlayer (){
	conecta3x3ns__tPlayer player = player3;
	int r = rand() % 3;
    if (r == 0) player = player1;
    if (r == 1) player = player2;
    return player;
}

int searchEmptyGame (){
	for (int i = 0; i < MAX_GAMES; i++){
        if (games[i].state == gameEmpty || games[i].state == gameWaitingPlayer){
            return i;
        }
    }
    return ERROR_SERVER_FULL;
   
}

void copyGameStatusStructure (conecta3x3ns__tBlock* status, char* message, char* board, int newCode){

	// Copy the message
	strcpy ((status->msgStruct).msg, message);
	(status->msgStruct).__size = strlen ((status->msgStruct).msg);

	// Copy the board, only if it is not NULL
	if (board == NULL){
		status->board = NULL;
		status->__size = 0;
	}
	else{
		strncpy (status->board, board, BOARD_WIDTH*BOARD_HEIGHT);
		status->__size = BOARD_WIDTH*BOARD_HEIGHT;
	}

	// Set the new code
	status->code = newCode;
}

int conecta3x3ns__register (struct soap *soap, conecta3x3ns__tMessage playerName, int* code){

	
}



void *processRequest(void *soap){

	pthread_detach(pthread_self());

	if (DEBUG_SERVER)
		printf ("Processing a new request...");

	soap_serve((struct soap*)soap);
	soap_destroy((struct soap*)soap);
	soap_end((struct soap*)soap);
	soap_done((struct soap*)soap);
	free(soap);

	return NULL;
}

int main(int argc, char **argv){ 

	struct soap soap;
	struct soap *tsoap;
	pthread_t tid;
	int port;
	SOAP_SOCKET m, s;

		// Init soap environment
		soap_init(&soap);

		// Configure timeouts
		soap.send_timeout = 60; // 60 seconds
		soap.recv_timeout = 60; // 60 seconds
		soap.accept_timeout = 3600; // server stops after 1 hour of inactivity
		soap.max_keep_alive = 100; // max keep-alive sequence

		initServerStructures(&soap);

		// Get listening port
		port = atoi(argv[1]);

		// Bind
		m = soap_bind(&soap, NULL, port, 100);

		if (!soap_valid_socket(m)){
			exit(1);
		}

		printf("Server is ON!\n");

		while (TRUE){

			// Accept a new connection
			s = soap_accept(&soap);

			// Socket is not valid :(
			if (!soap_valid_socket(s)){

				if (soap.errnum){
					soap_print_fault(&soap, stderr);
					exit(1);
				}

				fprintf(stderr, "Time out!\n");
				break;
			}

			// Copy the SOAP environment
			tsoap = soap_copy(&soap);

			if (!tsoap){
				printf ("SOAP copy error!\n");
				break;
			}

			// Create a new thread to process the request
			pthread_create(&tid, NULL, (void*(*)(void*))processRequest, (void*)tsoap);
		}

	// Detach SOAP environment
	soap_done(&soap);
	return 0;
}
