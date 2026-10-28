#include "soapH.h"
#include "conecta3x3.nsmap"
#include "game.h"

#define DEBUG_CLIENT 1

/**
 * Reads a player move
 * @return A number between [0-6]
 */
unsigned int readMove (){

	xsd__string enteredMove;
	unsigned int move;
	unsigned int isRightMove;

		// Init...
		enteredMove = (xsd__string) malloc (STRING_LENGTH);
		memset (enteredMove, 0, STRING_LENGTH);
		isRightMove = FALSE;
		move = STRING_LENGTH;

		while (!isRightMove){

			printf ("Enter a move [0-6]:");

			// Read move
			fgets (enteredMove, STRING_LENGTH-1, stdin);

			// Remove new-line char
			enteredMove[strlen(enteredMove)-1] = 0;

			// Length of entered move is not correct
			if (strlen(enteredMove) != 1){
				printf ("Entered move is not correct. It must be a number between [0-6]\n");
			}

			// Check if entered move is a number
			else if (isdigit(enteredMove[0])){

				// Convert move to an int
				move =  enteredMove[0] - '0';

				if (move > 6)
					printf ("Entered move is not correct. It must be a number between [0-6]\n");
				else
					isRightMove = TRUE;
			}

			// Entered move is not a number
			else
				printf ("Entered move is not correct. It must be a number between [0-6]\n");
		}

	return move;
}


int main(int argc, char **argv){

	struct soap soap;					/** Soap struct */
	char *serverURL;					/** Server URL */
	unsigned int endOfGame;				/** Flag to control the end of the game */
	conecta3x3ns__tMessage playerName;	/** Player name */
	conecta3x3ns__tBlock gameStatus;	/** Game status */
	unsigned int playerMove;			/** Player move */
	int gameId;						/** Return code from server */
	char nameBuffer[STRING_LENGTH];


	// Init gSOAP environment
	soap_init(&soap);

	// Obtain server address
	serverURL = argv[1];

	
	// Check arguments
	if (argc !=2) {
		printf("Usage: %s http://server:port\n",argv[0]);
		exit(0);
	}


    // Pedir nombre del jugador
    do {
        memset(nameBuffer, 0, STRING_LENGTH);
        printf("Enter player name: ");
        fgets(nameBuffer, STRING_LENGTH - 1, stdin);
        nameBuffer[strlen(nameBuffer) - 1] = '\0';
    } while (strlen(nameBuffer) <= 2);

    playerName.msg = nameBuffer;
    playerName.__size = strlen(nameBuffer);

    // Registrar al jugador en el servidor
    if (soap_call_conecta3x3ns__register(&soap, serverURL, "", playerName, &gameId) != SOAP_OK) {
        soap_print_fault(&soap, stderr);
        soap_done(&soap);
        exit(1);
    }

    if (gameId == ERROR_SERVER_FULL) {
        printf("Error: Server is full! No more games allowed.\n");
        soap_done(&soap);
        exit(1);
    } else if (gameId == ERROR_NAME_REPEATED) {
        printf("Error: A player with the same name is already registered in this game!\n");
        soap_done(&soap);
        exit(1);
    }

    printf("Successfully registered! Game ID: %d\nWaiting for game to start...\n", gameId);

    // Bucle principal del juego
    while (!endOfGame) {

        // Solicitar estado de la partida (bloqueará en el servidor si no es nuestro turno)
        if (soap_call_conecta3x3ns__getStatus(&soap, serverURL, "", playerName, gameId, &gameStatus) != SOAP_OK) {
            soap_print_fault(&soap, stderr);
            break;
        }

        // Mostrar tablero si está disponible
        if (gameStatus.board != NULL && strlen(gameStatus.board) > 0) {
            printBoard(gameStatus.board, gameStatus.msgStruct.msg);
        } else if (gameStatus.msgStruct.msg != NULL) {
            printf("%s\n", gameStatus.msgStruct.msg);
        }

        // Evaluar código de la partida
        switch (gameStatus.code) {
            case TURN_MOVE:
                // Es nuestro turno para insertar ficha
                playerMove = readMove();
                if (soap_call_conecta3x3ns__insertChip(&soap, serverURL, "", playerName, gameId, playerMove, &gameStatus) != SOAP_OK) {
                    soap_print_fault(&soap, stderr);
                    endOfGame = TRUE;
                }
                break;

            case GAMEOVER_WIN:
                printf("\n=============================\n  *** YOU WIN THE GAME! ***\n=============================\n");
                endOfGame = TRUE;
                break;

            case GAMEOVER_LOSE:
                printf("\n=============================\n  *** YOU LOSE THE GAME! ***\n=============================\n");
                endOfGame = TRUE;
                break;

            case GAMEOVER_DRAW:
                printf("\n=============================\n  *** DRAW GAME! ***\n=============================\n");
                endOfGame = TRUE;
                break;

            case ERROR_PLAYER_NOT_FOUND:
                printf("Error: Player not found in this game!\n");
                endOfGame = TRUE;
                break;

            default:
                break;
        }
    }

		// Clean the environment
		soap_destroy(&soap);
		soap_end(&soap);
		soap_done(&soap);

  return 0;
}
