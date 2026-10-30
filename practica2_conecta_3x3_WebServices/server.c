#include "soapH.h"
#include "game.h"
#include "utils.h"
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
    
    int isFinished; /**< Indica si la partida ha finalizado */
    int winner;     /**< Almacena el jugador ganador o -1 en caso de empate */
} tGame;

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
        games[i].isFinished = FALSE;
        games[i].winner = -1;
        
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
    if (message != NULL && status->msgStruct.msg != NULL) {
        strncpy ((status->msgStruct).msg, message, STRING_LENGTH - 1);
        (status->msgStruct).msg[STRING_LENGTH - 1] = '\0';
        (status->msgStruct).__size = strlen ((status->msgStruct).msg) + 1; // Incluir '\0'
    }

    // Copy the board, only if it is not NULL
    if (board == NULL || status->board == NULL){
        status->board = NULL;
        status->__size = 0;
    }
    else{
        strncpy (status->board, board, BOARD_WIDTH * BOARD_HEIGHT);
        status->board[BOARD_WIDTH * BOARD_HEIGHT] = '\0';
        status->__size = strlen(status->board) + 1; // Incluir '\0'
    }

    // Set the new code
    status->code = newCode;
}

int conecta3x3ns__register (struct soap *soap, conecta3x3ns__tMessage playerName, int* code){
    
    int gameId = searchEmptyGame();

    if (gameId == ERROR_SERVER_FULL){
        *code = ERROR_SERVER_FULL;
        return SOAP_OK;
    }

    char cleanName[STRING_LENGTH];
    memset(cleanName, 0, STRING_LENGTH);
    if (playerName.msg != NULL) {
        strncpy(cleanName, playerName.msg, STRING_LENGTH - 1);
    }

    // Entramos en la sección crítica para modificar el estado del juego
    pthread_mutex_lock(&games[gameId].mutex);

    // Comprobamos si ya existe un jugador con el mismo nombre en el juego
    if ((games[gameId].numPlayers >= 1 && strcmp(games[gameId].player1Name, cleanName) == 0) ||
        (games[gameId].numPlayers >= 2 && strcmp(games[gameId].player2Name, cleanName) == 0) ||
        (games[gameId].numPlayers >= 3 && strcmp(games[gameId].player3Name, cleanName) == 0)) {
        
        pthread_mutex_unlock(&games[gameId].mutex);
        *code = ERROR_NAME_REPEATED;
        return SOAP_OK;
    }

    // Asignamos nombre al jugador correspondiente y actualizamos el número de jugadores
    if (games[gameId].numPlayers == 0) {
        strncpy(games[gameId].player1Name, cleanName, STRING_LENGTH - 1);
        games[gameId].player1Name[STRING_LENGTH - 1] = '\0';
        games[gameId].state = gameWaitingPlayer;
    } else if (games[gameId].numPlayers == 1) {
        strncpy(games[gameId].player2Name, cleanName, STRING_LENGTH - 1);
        games[gameId].player2Name[STRING_LENGTH - 1] = '\0';
    } else if (games[gameId].numPlayers == 2) {
        strncpy(games[gameId].player3Name, cleanName, STRING_LENGTH - 1);
        games[gameId].player3Name[STRING_LENGTH - 1] = '\0';
    }

    // Incrementar contadores de jugadores
    games[gameId].numPlayers++;
    games[gameId].activePlayers++;

    // Si ya hay 3 jugadores, cambiamos el estado del juego a "listo" y seleccionamos un jugador aleatorio para comenzar
    if (games[gameId].numPlayers == 3) {
        games[gameId].state = gameReady;
        games[gameId].currentPlayer = calculateRandomPlayer();
        pthread_cond_broadcast(&games[gameId].condTurn);
    }

    // Devolver el ID de la partida asignada y liberar la sección crítica
    *code = gameId;
    pthread_mutex_unlock(&games[gameId].mutex);

    return SOAP_OK;
}

int conecta3x3ns__getStatus(struct soap *soap, conecta3x3ns__tMessage playerName, int gameId, conecta3x3ns__tBlock *status) {
    
    // Reservar memoria gestionada por gSOAP para el bloque de respuesta
    allocClearBlock(soap, status);

    // Validar identificador de partida
    if (gameId < 0 || gameId >= MAX_GAMES) {
        copyGameStatusStructure(status, "Invalid game ID", NULL, ERROR_PLAYER_NOT_FOUND);
        return SOAP_OK;
    }

    char cleanName[STRING_LENGTH];
    memset(cleanName, 0, STRING_LENGTH);
    if (playerName.msg != NULL) {
        strncpy(cleanName, playerName.msg, STRING_LENGTH - 1);
    }

    // Entrar en la sección crítica de la partida
    pthread_mutex_lock(&games[gameId].mutex);

    // Identificar al jugador que invoca el servicio
    conecta3x3ns__tPlayer playerEnum;
    if (strcmp(games[gameId].player1Name, cleanName) == 0) {
        playerEnum = player1;
    } else if (strcmp(games[gameId].player2Name, cleanName) == 0) {
        playerEnum = player2;
    } else if (strcmp(games[gameId].player3Name, cleanName) == 0) {
        playerEnum = player3;
    } else {
        // El jugador no pertenece a esta partida
        copyGameStatusStructure(status, "Player not registered in this game", NULL, ERROR_PLAYER_NOT_FOUND);
        pthread_mutex_unlock(&games[gameId].mutex);
        return SOAP_OK;
    }

    // Esperar mientras la partida no esté lista o no sea el turno de este jugador (y la partida no haya acabado)
    while ((games[gameId].state == gameWaitingPlayer || games[gameId].currentPlayer != playerEnum) 
           && !games[gameId].isFinished) {
        pthread_cond_wait(&games[gameId].condTurn, &games[gameId].mutex);
    }

    // Si la partida ha terminado mientras esperaba
    if (games[gameId].isFinished) {
        if (games[gameId].winner == playerEnum) {
            copyGameStatusStructure(status, "You win!", games[gameId].board, GAMEOVER_WIN);
        } else if (games[gameId].winner == -1) {
            copyGameStatusStructure(status, "Draw game!", games[gameId].board, GAMEOVER_DRAW);
        } else {
            copyGameStatusStructure(status, "You lose!", games[gameId].board, GAMEOVER_LOSE);
        }

        // Gestión del fin de partida por el último jugador activo
        games[gameId].activePlayers--;
        if (games[gameId].activePlayers == 0) {
            games[gameId].state = gameEmpty;
            games[gameId].isFinished = FALSE;
            games[gameId].numPlayers = 0;
            initBoard(games[gameId].board);
        }

        pthread_mutex_unlock(&games[gameId].mutex);
        return SOAP_OK;
    }

    // Es su turno para mover
    copyGameStatusStructure(status, "It's your turn!", games[gameId].board, TURN_MOVE);

    pthread_mutex_unlock(&games[gameId].mutex);
    return SOAP_OK;
}

int conecta3x3ns__insertChip(struct soap *soap, conecta3x3ns__tMessage playerName, int gameId, unsigned int column, conecta3x3ns__tBlock *gameStatus) {

    // Reservar memoria gestionada por gSOAP para la respuesta
    allocClearBlock(soap, gameStatus);

    // Validar identificador de la partida
    if (gameId < 0 || gameId >= MAX_GAMES) {
        copyGameStatusStructure(gameStatus, "Invalid game ID", NULL, ERROR_PLAYER_NOT_FOUND);
        return SOAP_OK;
    }

    char cleanName[STRING_LENGTH];
    memset(cleanName, 0, STRING_LENGTH);
    if (playerName.msg != NULL) {
        strncpy(cleanName, playerName.msg, STRING_LENGTH - 1);
    }

    // Entrar en la sección crítica de la partida
    pthread_mutex_lock(&games[gameId].mutex);

    // Identificar al jugador que invoca el servicio
    conecta3x3ns__tPlayer playerEnum;
    if (strcmp(games[gameId].player1Name, cleanName) == 0) {
        playerEnum = player1;
    } else if (strcmp(games[gameId].player2Name, cleanName) == 0) {
        playerEnum = player2;
    } else if (strcmp(games[gameId].player3Name, cleanName) == 0) {
        playerEnum = player3;
    } else {
        copyGameStatusStructure(gameStatus, "Player not registered in this game", NULL, ERROR_PLAYER_NOT_FOUND);
        pthread_mutex_unlock(&games[gameId].mutex);
        return SOAP_OK;
    }

    // Verificar que sea el turno del jugador y la partida esté activa
    if (games[gameId].state != gameReady || games[gameId].currentPlayer != playerEnum || games[gameId].isFinished) {
        copyGameStatusStructure(gameStatus, "It is not your turn or the game is not ready!", games[gameId].board, TURN_WAIT);
        pthread_mutex_unlock(&games[gameId].mutex);
        return SOAP_OK;
    }

    // Validar si la jugada es válida (columna dentro de rango y no llena)
    conecta3x3ns__tMove moveResult = checkMove(games[gameId].board, column);
    if (moveResult != OK_move) {
        // La columna está llena o fuera de rango
        copyGameStatusStructure(gameStatus, "Invalid move! Column full or out of bounds.", games[gameId].board, TURN_MOVE);
        pthread_mutex_unlock(&games[gameId].mutex);
        return SOAP_OK;
    }

    // Insertar la ficha en el tablero
    insertChip(games[gameId].board, playerEnum, column);

    // Comprobar condiciones de fin de juego o cambio de turno
    if (checkWinner(games[gameId].board, playerEnum)) {
        games[gameId].isFinished = TRUE;
        games[gameId].winner = playerEnum;

        copyGameStatusStructure(gameStatus, "You win!", games[gameId].board, GAMEOVER_WIN);

        pthread_cond_broadcast(&games[gameId].condTurn);
    }
    else if (isBoardFull(games[gameId].board)) {
        games[gameId].isFinished = TRUE;
        games[gameId].winner = -1; // -1 indica empate

        copyGameStatusStructure(gameStatus, "Draw game!", games[gameId].board, GAMEOVER_DRAW);

        // Notificar el fin de juego a todos
        pthread_cond_broadcast(&games[gameId].condTurn);
    }
    else {
        games[gameId].currentPlayer = calculateNextPlayer(games[gameId].currentPlayer);

        copyGameStatusStructure(gameStatus, "Move played successfully.", games[gameId].board, TURN_WAIT);

        // Despertar a los hilos esperando en getStatus() para que continúe el jugador del siguiente turno
        pthread_cond_broadcast(&games[gameId].condTurn);
    }

    // Liberar la sección crítica y retornar
    pthread_mutex_unlock(&games[gameId].mutex);
    return SOAP_OK;
}

void *processRequest(void *soap){

    pthread_detach(pthread_self());

    if (DEBUG_SERVER)
        printf ("Processing a new request...\n");

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

    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Init soap environment
    soap_init(&soap);

    // Configure timeouts
    soap.send_timeout = 60;     // 60 seconds
    soap.recv_timeout = 60;     // 60 seconds
    soap.accept_timeout = 3600; // server stops after 1 hour of inactivity
    soap.max_keep_alive = 100;  // max keep-alive sequence

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

        // Socket is not valid
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