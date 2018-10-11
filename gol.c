/**
 * File: gol.c
 *
 * Starter code for COMP280 Project 6 ("Game of Life")
 *
 * Name: Zach Fukuhara
 * Description: A program the simulates the Game of Life.
 * 	@ symbols are live characters
 * 	- symbols are dead characters
 *
 * 	A live cell with zero or one live neighbors dies from loneliness
 * 	A live cell with four or more live neighbors dies due to overpopulation
 * 	A dead cell with exactly three live neighbors becomes alive
 *
 * 	Command Options:
 * 		-v Enables verbose mode, printing the iterations of the game to the
 * 			console
 * 		-c Allows the user to enter in a configuration file that will specify
 * 			how to initialize the game playing variables
 * 		-l Allows the simulator to connect to comp280.sandiego.edu and print its
 * 			list of available configuration files
 * 		-n Works like the -c option, except that it will execute a simulation
 * 			using the user-specified configuration file from comp280.sandiego.edu
 *
 * 		NOTE:
 * 			-c and -n cannot be run together
 * 			-l cannot be run with any other options
 */
	
#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <getopt.h>
#include <errno.h>

// Forward Declaration
int *init_board(FILE *cfg_file, int num_rows, int num_cols, int num_living);
int translate_to_1D(int row, int col, int num_rows, int num_cols);
int check_neighbors(int *game_board, int row, int col, int num_rows, int num_cols);
void iterate(int *game_board, int num_rows, int num_cols);
void play(int *game_board, int num_iters, int num_rows, int num_cols, int verbose_mode);
void print_board(int *game_board, int num_rows, int num_cols, int curr_steps);
int open_clientfd(char *hostname, char *port);
void send_recv(const void *msg, void *buf);
void writef(char *buf);

void usage(char *executable_name) {
	printf("Usage: %s [-v] -c [-l] [-n] <textfile>", executable_name);
}

static void timeval_subtract (struct timeval *result, struct timeval *end, 
		struct timeval *start){
	// Perform the carry for the later subtraction by updating start.
	if (end->tv_usec < start->tv_usec) {
		int nsec = (start->tv_usec - end->tv_usec) / 1000000 + 1;
		start->tv_usec -= 1000000 * nsec;
		start->tv_sec += nsec;
	}
	if (end->tv_usec - start->tv_usec > 1000000) {
		int nsec = (end->tv_usec - start->tv_usec) / 1000000;
		start->tv_usec += 1000000 * nsec;
		start->tv_sec -= nsec;
	}
	// Compute the time remaining to wait.tv_usec is certainly positive.
	result->tv_sec = end->tv_sec - start->tv_sec;
	result->tv_usec = end->tv_usec - start->tv_usec;
}

//_______________________________________MAIN METHOD__________________________________________
int main(int argc, char *argv[]) {
	int verbose_mode = 0, c = -1;
	int num_rows, num_cols, num_iters, num_living;
	int case_v = 0, case_c = 0, case_l = 0, case_n = 0;
	struct timeval start, end, result;
	char *config_file = NULL;
	int *game_board = NULL;
	char msg[100];
	char *buf = calloc(3000, sizeof(char));
	while ((c = getopt(argc, argv, "vc:ln:")) != -1) {
		switch(c) {
			case 'v':
				// Enable verbose mode
				verbose_mode = 1;
				case_v = 1;
				break;
			case 'c':
				case_c = 1;
				// Store name of configuration file
				config_file = optarg;
				break;
			case 'l':
				case_l = 1;
				snprintf(msg, 100, "list");
				break;
			case 'n':
				case_n = 1;
				config_file = "config.txt";
				snprintf(msg, 100, "get %s", optarg);
				break;
			default:
				usage(argv[0]);
				exit(1);
		}
	}
	// Make sure command is valid
	if(case_c && case_n) {
		printf("Error: Unable to execute commands with both -c and -n.\n");
		exit(1);
	}
	else if(case_l && (case_n || case_c || case_v)) {
		printf("Error: Unable to execute command with -l and -n/-c/-v.\n");
		exit(1);
	}
	
	// Execute -l and -n commands
	if(case_l) {
		send_recv(msg, buf);
		printf("%s\n", buf);
		free(buf); //free buf for -l command
		exit(0);
	}
	if(case_n) {
		send_recv(msg, buf);
		writef(buf);
	}
	free(buf); //free buf for non -l commands

	if(config_file == NULL) {
		printf("You must specify the configuration file (-c <textfile>)\n");
		exit(1);
	}
	// Open the configuration file
	FILE *cfg_file = fopen(config_file, "r");
	if(cfg_file == NULL) {
		printf("Cannot open %s file\n", config_file);
		exit(1);
	}
	// Store # of rows, # of columns, # of iterations, and # of living
	fscanf(cfg_file, "%d", &num_rows);
	fscanf(cfg_file, "%d", &num_cols);
	fscanf(cfg_file, "%d", &num_iters);
	fscanf(cfg_file, "%d", &num_living);

	// Initialize game board
	game_board = init_board(cfg_file, num_rows, num_cols, num_living);
	
	// Close configuration file
	fclose(cfg_file);

	// Start timer
	gettimeofday(&start, NULL);

	// Play game
	play(game_board, num_iters, num_rows, num_cols, verbose_mode);
	
	// Stop timer and print result
	gettimeofday(&end, NULL);
	timeval_subtract(&result, &end, &start);
	printf("Total time for %d iterations of %dx%d world is %ld.%06ld secs\n", num_iters, 
			num_rows, num_cols, result.tv_sec, result.tv_usec);
	
	// Free memory
	free(game_board);
	return 0;
}
//_________________________________________________________________________________________

/* Initializes the game board with alive (1's) and dead (0's) using input from
 * the configuration file.  Checks to make sure the intended number of living
 * are in fact initialized properly, exits if not.
 *
 * @param cfg_file Configuration file
 * @param num_rows Number of rows in board
 * @param num_cols Number of columns in board
 * @param num_living Number of living spaces to be initialized
 * @return Initialized board
 */
int *init_board(FILE *cfg_file, int num_rows, int num_cols, int num_living) {
	int *board = calloc(num_rows * num_cols, sizeof(int));
	int c, r, num_alive = 0;
	while(fscanf(cfg_file, "%d %d", &c, &r) == 2) {
		board[translate_to_1D(r, c, num_rows, num_cols)] = 1; // Set alive spots
		num_alive++;
	}
	if(num_living != num_alive) {
		printf("Incorrect number of Living Spots read in\nExpected: %d\nCurrent: %d\n",
				num_living, num_alive);
		exit(1);
	}
	return board;
}

/* Translates 2D coordinates to a 1D format that can be used as an index for
 * the 1D game board array
 *
 * @param row Row index
 * @param col Column index
 * @param num_rows Number of total rows on the board
 * @param num_cols Number of total columns on the board
 * @return 1D index representation of 2D coordinate
 */
int translate_to_1D(int row, int col, int num_rows, int num_cols) {
	if(row < 0)
        row += num_rows;
    if(row >= num_rows)
        row -= num_rows;
    if(col < 0)
        col += num_cols;
    if(col >= num_cols)
        col -= num_cols;
    return num_cols * row + col;
}

/* Checks the eight immediate neighbors of the current spot on the board.
 * Counts up the number of neighbors that are alive and returns the sum.
 *
 * @param game_board The initialized game board
 * @param row Row index of current spot
 * @param col Column index of current spot
 * @param num_rows Number of total rows on the board
 * @param num_cols Number of total columns on the board
 * @return sum of alive neighbors
 */
int check_neighbors(int *game_board, int row, int col, int num_rows, int num_cols) {
    int alive_neighbors = 0;
    int top_left = translate_to_1D(row - 1, col - 1, num_rows, num_cols);
    int top = translate_to_1D(row - 1, col, num_rows, num_cols);
	int top_right = translate_to_1D(row - 1, col + 1, num_rows, num_cols);
    int left = translate_to_1D(row, col - 1, num_rows, num_cols);
    int right = translate_to_1D(row, col + 1, num_rows, num_cols);
    int bot_left = translate_to_1D(row + 1, col - 1, num_rows, num_cols);
    int bot = translate_to_1D(row + 1, col, num_rows, num_cols);
    int bot_right = translate_to_1D(row + 1, col + 1, num_rows, num_cols);
									    
    alive_neighbors += game_board[top_left] + game_board[top] + game_board[top_right]
	        + game_board[left] + game_board[right] + game_board[bot_left] 
			+ game_board[bot] + game_board[bot_right];
							    
    return alive_neighbors;
}

/* Performs one iteration of the gameplay
 *
 * @param game_board Initialized game board
 * @param num_rows Number of total rows on the board
 * @param num_cols Number of total columns on the board
 */
void iterate(int *game_board, int num_rows, int num_cols) {
    int *board_copy = calloc(num_rows * num_cols, sizeof(int));
    for(int i = 0; i < num_rows * num_cols; i++) {
        board_copy[i] = game_board[i];
    }   
    int alive_neighbors, index;
    for(int r = 0; r < num_rows; r++) {
        for(int c = 0; c < num_cols; c++) {
	        alive_neighbors = check_neighbors(board_copy, r, c, num_rows, num_cols);
            index = translate_to_1D(r, c, num_rows, num_cols);
            if(game_board[index] == 0) { // if dead
                if (alive_neighbors == 3)
                    game_board[index] = 1;
			}   
			else { // if living
                if (alive_neighbors <= 1 || alive_neighbors >= 4)
                    game_board[index] = 0;
            }   
        }   
    }   
    free(board_copy);
}

/* Plays the Game of Life, executing the specified number of iterations.
 * 
 * @param game_board Initialized game board
 * @param num_iters Number of iterations to be executed
 * @param num_rows Number of total rows on the board
 * @param num_cols Number of total columns on the board
 * @param verbose_mode Verbose Mode is on if 1, off if not.  Prints each
 * 		iteration of gameplay if on, does not print if off.
 */
void play(int *game_board, int num_iters, int num_rows, int num_cols, int verbose_mode) {
	for(int i = 0; i < num_iters; i++) {
		if(verbose_mode) {
			print_board(game_board, num_rows, num_cols, i);
		}
		iterate(game_board, num_rows, num_cols);
	}
	// Get last print
	if(verbose_mode)
		print_board(game_board, num_rows, num_cols, num_iters);
}

/* Prints the current iteration of the game
 *
 * @param game_board Initialized game board
 * @param num_rows Number of total rows on board
 * @param num_cols Number of total columns on board
 * @param curr_step Current iteration of the game
 */
void print_board(int *game_board, int num_rows, int num_cols, int curr_step) {
	system("clear");
	printf("Time step: %d\n", curr_step);
	int i = 0;
	for(int r = 0; r < num_rows; r++) {
		for(int c = 0; c < num_cols; c++) {
			i = translate_to_1D(r, c, num_rows, num_cols);
			if(game_board[i])
				printf("@");
			else
				printf("-");
		}
		printf("\n");
	}
	usleep(200000);
}
//__________________________________Advanced Component Start____________________________________

/* Establishes a connection with a server running on host 'hostname'
 * and listening for connection requests on port number 'port'
 *
 * @param *hostname Name of the host 
 * @param *port The port number
 * @return An open socket descriptor ready for input and output using Unix I/O
 * 		functions
 */
int open_clientfd(char *hostname, char *port) {
	int clientfd;
	struct addrinfo hints, *listp, *p;

	// Get list of potential server addresses
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM; // Open a connection
	hints.ai_flags = AI_NUMERICSERV; // Using numeric port arg
	hints.ai_flags |= AI_ADDRCONFIG; // Recommended for connections
	getaddrinfo(hostname, port, &hints, &listp);

	// Walk the list for one that we can successfully connect to
	for(p = listp; p; p = p->ai_next) {
		// Create a socket descriptor
		if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			continue; // Socket failed, try the next

		// Connect to the server
		if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
			break;   // Success
		close(clientfd); // Connect failed, try another
	}
							
	// Clean up
	freeaddrinfo(listp);
	if(!p) // All connects failed
		return -1;
	else   // The last connect succeeded
		return clientfd;
}

/* Connects to the comp280.sandiego.edu server and sends & receives a msg
 *
 * @param *msg Pointer to the data to be sent
 * @param *buf Buffer to read data into
 */
void send_recv(const void *msg, void *buf) {
	int clientfd = open_clientfd("comp280.sandiego.edu", "9181");
	if(clientfd == -1) { // Check if connection to client was successful
		printf("Error:  Failed to connect to client");
		exit(1);
	}

	int len = strlen(msg); // Store length of data pointed to by msg in bytes
	int bytes_sent = send(clientfd, msg, len, 0); // Number of bytes actually sent out
	int bytes_recv = recv(clientfd, buf, 3000, 0); // Max buffer length arbitrarily set to 3000 bytes
	if(bytes_sent == -1 || bytes_recv == -1) { // Check that the msg was sent and received properly
		printf("Error: Message not sent/received properly. %s\n", strerror(errno));
		exit(1);
	}
}

/* Writes to the 'config.txt' file
 *
 * @param *buf Pointer to buffer to read data into
 */
void writef(char *buf) {
	FILE *output_file = fopen("config.txt", "w+");
	if(output_file == NULL) {
		printf("Error:  Unable to open file 'config.txt'\n");
		exit(1);
	}
	fprintf(output_file, "%s", buf);
	fflush(output_file);
	fclose(output_file);
}
