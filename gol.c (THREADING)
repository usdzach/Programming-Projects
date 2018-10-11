/*
 * File: gol.c  THIS IS DIFFERENT THAN PROJECT 6's gol.c in that this program implements THREADING
 *
 * This file reads in a starting file with a specified number of rows,
 * collumns, iterations, and starting pairs. Then creates a game board and
 * executes the game of life for the user-specified number of life cycles.
 * The total time for each game is also calculated and displayed at the end.
 *
 * The execution of each simulation is done using a certain number of threads,
 * either specified by the user or 4.
 *
 * Command line options include...
 * 		-v Verbose Mode: Prints the game board after each iteration of the
 * 				simulation
 * 		-c Allows the user to specify a configuration file to be used for the
 * 				simulation
 *		-l Lists the available configuration files contained in the
 *				comp280.sandiego.edu
 *		-n Works like the -c option, except that it will retrieve the
 *				configuration file from the comp280.sandiego.edu server.
 *		-p Prints thread partitioning information
 *		-t Allows the user to specify the number of threads
 *
 * comp280, Project 09 Threaded Game of Life
 *
 * Authors: Zach Fukuhara <zfukuhara@sandiego.edu> & Tyler Bullock <tbullock@sandiego.edu>
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
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <pthread.h>

//forward declarations
void readFile(FILE *file, int *num_rows, int *num_cols, int *iterations, int *live_pairs);
char *initializeBoard(int rows, int cols);
int translateId(int row, int col, int num_rows, int num_cols);
void setToLive(char *board, FILE *file, int num_rows, int num_cols);
void printBoard(char *board, int num_rows, int num_cols, int timestep);
void takeStep(char *board, int num_rows, int num_cols, int start_row, int thread_rows, pthread_barrier_t *barrier);
int getNeighbors(char *board, int r, int c,  int num_rows, int num_cols);
void timeval_subtract (struct timeval *result, struct timeval *end, struct timeval *start);
int open_clientfd(char *hostname, char *port);
void send_receive(const void *net_in, void *net_out);
void writeFile(char *net_out);
void createThreads(pthread_t *tid_arr, int num_threads, int num_rows, int num_cols, int iterations, char *board, int verbose, int print_partition);
void *threadFunc(void *thread_args);

void usage(char *executable_name) {
	printf("Usage: %s [-v] -c <filename>", executable_name);
}

struct ThreadArgs {
	unsigned int start_row;
	unsigned int rows;
	char *board;
	int board_rows;
	int board_cols;
	int iterations;
	pthread_barrier_t *barrier;
	int verbose;
	int *timestep;
	int tid;
	int start_index;
	int end_index;
	int print_p;
};


typedef struct ThreadArgs ThreadArgs;

int main(int argc, char *argv[]) {

	int verbose_mode = 0;
	opterr = 0;
	char *config_file = NULL;
	int c = -1;
	int num_rows, num_cols, iterations, live_pairs;
	char *net_in = malloc(50 * sizeof(char));
	char *net_out = malloc(1000 * sizeof(char));
	int case_l = 0, case_v = 0, case_c = 0, case_n = 0;
	int num_threads = 4;
	int print_partition = 0;

	while ((c = getopt(argc, argv, "vlpt:n:c:")) != -1) {
		switch(c) {
			case 'v':
				case_v = 1;
				verbose_mode = 1;
				break;
			case 'c':
				case_c = 1;
				config_file = optarg;
				break;
			case 'l':
				case_l = 1;
				net_in = "list";
				break;
			case 'n':
				case_n = 1;
				config_file = "netconfig.txt";
				snprintf(net_in, 32, "get %s", optarg);
				break;
			case 't':
				num_threads = strtol(optarg, NULL, 10);
				break;
			case 'p':
				print_partition = 1;
				break;
			default:
				usage(argv[0]);
				exit(1);
		}
	}
	if (case_n && case_c) {
		printf("ERROR: Cannot specify both the -n and -c options\n");
		exit(1);
	}
	if(case_l && (case_v || case_c || case_n)){
		printf("Error: Cannot specify -l with other options\n");
 		exit(1);
	}
	if(case_l){
		send_receive(net_in, net_out);
		printf("%s", net_out);
		exit(0);
	}
	if(case_n){
		send_receive(net_in, net_out);
		writeFile(net_out);
	}

	FILE *file = fopen(config_file, "r");
	if (file == NULL){
		printf("Cannot Open File %s\n", config_file);
		exit(1);
	}

	pthread_t *tid_arr = malloc(num_threads * sizeof(pthread_t));

	readFile(file, &num_rows, &num_cols, &iterations, &live_pairs);

	if (num_threads < 1 || num_threads > num_rows) {
		printf("ERROR: there must be more than 0 threads");
		exit(1);
	}


	char *board = initializeBoard(num_rows, num_cols);
	setToLive(board, file, num_rows, num_cols);
	if (board == NULL) {
		printf("Error, no array allocated");
		exit(1);
	}




	struct timeval start, end, result;
	gettimeofday(&start, NULL);

	createThreads(tid_arr, num_threads, num_rows, num_cols, iterations, board, verbose_mode, print_partition);

	gettimeofday(&end, NULL);
	timeval_subtract(&result, &end, &start);
	printf("Total time to run %d iterations of %dx%d world is %ld.%06ld seconds\n", iterations, num_rows, num_cols, result.tv_sec, result.tv_usec);
	fclose(file);
	free(tid_arr);
	free(board);
	free(net_in);
	free(net_out);
	return 0;
}

/**
 *	Reads the first four lines of the file and stores the values of each line
 *	in the variables declared in main by passing pointers to the original
 *	memory addresses.
 *
 *	@param *file the file pointer that was opened in main
 *	@param *num_rows pointer to the original num_rows variable
 *	@param *num_cols pointer to the original num_cols variable
 *	@param *iterations pointer to the original iterations variable
 *	@param *live_pairs pointer to the original live_pairs variable
 *
 */

void readFile(FILE *file, int *num_rows, int *num_cols, int *iterations, int *live_pairs){
	fscanf(file, "%d", num_rows);
	fscanf(file, "%d", num_cols);
	fscanf(file, "%d", iterations);
	fscanf(file, "%d", live_pairs);
}

/**
 * creates an array of characters that acts as a 2D game board. Allocates
 * spcace for each element in the array based on the number of rows and
 * collumns and initializes the board with all dead cells
 *
 * @param rows the number of rows on the game board
 * @param cols the number of collumns on the game board
 * @return an array of characters that makes the game board
 */

char *initializeBoard(int rows, int cols){
	char *board = malloc(rows * cols * sizeof(char));
	for (int i = 0; i < (rows * cols); i++){
		board[i] = '.';
	}
	return board;
}

/**
 * converts from 2D cooridinates to a single integer index in the 1D array and
 * accounts for the torus nature of the game of life.
 *
 *	@param row the row index
 *	@param col the collumn index
 *	@param num_rows the number of rows on the board
 *	@param num_cols the number of collumns on the board
 *	@return the single index in the 1D array
 */

int translateId(int row, int col, int num_rows, int num_cols){
	if (row < 0) {
		row += num_rows;
	}
	if (row >= num_rows) {
		row -= num_rows;
	}
	if (col < 0) {
		col += num_cols;
	}
	if (col >= num_cols) {
		col -= num_cols;
	}
	return (row * num_cols) + col;
}

/**
 * sets all of the initial live coordinates from live to dead in the game
 * board array
 *
 * @param *board the char array that makes up the game board
 * @param *file the command line file that was opened in main
 * @param num_rows the number of rows on the board
 * @param num_cols the number of collumns on the board
 */

void setToLive(char *board, FILE *file, int num_rows, int num_cols){
	int row, col;
	while(fscanf(file, "%d %d", &col, &row) == 2){
		board[translateId(row, col, num_rows, num_cols)] = '@';
	}
}

/**
 * clears the terminal display and then prints the time step and the board and
 * then sleeps in order to allow the user to view the board at each time step
 *
 * @param *board the char array that makes up the game board
 * @param num_rows the number of rows
 * @param num_cols the number of collumns
 * @param *timestep pointer to the original value of timestep in main
 */

void printBoard(char *board, int num_rows, int num_cols, int timestep){
	system("clear");
	printf("Time step: %d\n", timestep);
	for (int i = 0; i < num_rows; i++){
		for (int j = 0; j < num_cols; j++){
			if (j == (num_cols-1)){
				printf("%c\n", board[translateId(i, j, num_rows, num_cols)]);
			}
			else {
				printf("%c ", board[translateId(i, j, num_rows, num_cols)]);
			}
		}
	}

	usleep(100000);
}

/**
 * simulates one time step in the game of life by looping through each cell to
 * see how many neighbors each one has and then applying the three rules of
 * the game according to the number of neighbors and the current state of the
 * cell
 *
 * @param *board the char array that makes the board
 * @param num_rows same as all the other num_rows
 * @param num_cols ditto
 * @param start_row The row that each thread should start executing on
 * @param thread_rows The number of rows each thread needs to execute
 * @param *barrier Barrier used in the iteration
 */

void takeStep(char *board, int num_rows, int num_cols, int start_row, int thread_rows, pthread_barrier_t *barrier){
	int size = num_rows * num_cols;
	int offset = start_row;
	int neighbors[thread_rows * num_cols];
	char *board_copy = initializeBoard(num_rows, num_cols);
	pthread_barrier_wait(barrier);
	for (int i = 0; i < size; i++){
		board_copy[i] = board[i];
	}

	pthread_barrier_wait(barrier);

	for (int i = start_row; i < (start_row + thread_rows); i++){
		for (int j = 0; j < num_cols; j++){
			neighbors[translateId(i-offset, j, num_rows, num_cols)] = getNeighbors(board_copy, i, j, num_rows, num_cols);
		}
	}

	pthread_barrier_wait(barrier);

	for (int i = start_row; i < (start_row + thread_rows); i++){
		for (int j = 0; j < num_cols; j++){
			if (board_copy[translateId(i, j, num_rows, num_cols)] == '.'){ //applies the ressurrection rule
				if (neighbors[translateId(i-offset, j, num_rows, num_cols)] == 3) {
					board[translateId(i, j, num_rows, num_cols)] = '@';
				}
			}
			else if (neighbors[translateId(i-offset, j, num_rows, num_cols)] >= 4 || neighbors[translateId(i-offset, j, num_rows, num_cols)] <= 1){ //applies the lonliness and overpopulation rules
					board[translateId(i, j, num_rows, num_cols)] = '.';

			}
		}
	}
	pthread_barrier_wait(barrier);
	free(board_copy);
}

/**
 * takes in the coordinates of a single cell as parameters and determines how
 * many neighbors that cell has by looping through the eight surrounding
 * cells.
 *
 * @param *board the char array that makes the board
 * @param r the row index of the cell
 * @param c the collumn index of the cell
 * @param num_rows number of rows
 * @param num_cols number of collumns
 * @return neighbors the number of neighbors that the given cell has
 */

int getNeighbors(char *board, int r, int c, int num_rows, int num_cols) {
	int neighbors = 0;
	for (int i = r-1; i < r+2; i++) {
		for (int j = c-1; j < c+2; j++) {
			if (i == r && j == c) { //skips over the cell that is being checked
				continue;
			}
			else if (board[translateId(i, j, num_rows, num_cols)] == '@') {
				neighbors++;
			}
		}
	}
	return neighbors;
}

/**
 * Function given by Dr. Sat, it takes the start time and end time from
 * gettimeofday function and calculates the difference in order to return the
 * elapsed time
 *
 * @param *result struct that will hold the final elapsed time
 * @param *end the time returned at the second call of gettimeofday
 * @param *start the time returned at the first call of gettimeofday
 */
void timeval_subtract (struct timeval *result, struct timeval *end, struct timeval *start) {
	if (end->tv_usec < start->tv_usec) {
		int nsec = (start->tv_usec - end->tv_usec) / 1000000 + 1;
		start->tv_usec -= 1000000 * nsec;
		start->tv_sec -= nsec;
	}
	if (end->tv_usec - start->tv_usec > 1000000) {
		int nsec = (end->tv_usec - start->tv_usec) / 1000000;
		start->tv_usec += 1000000 *nsec;
		start->tv_sec -= nsec;
	}

	result->tv_sec = end->tv_sec - start->tv_sec;
	result->tv_usec = end->tv_usec - start->tv_usec;
}

/* Establishes a connection with a server running on host 'hostname'
 * and listening for connection requests on port number 'port'
 *
 * @param *hostname Name of the host
 * @param *port The port number
 * @return An open socket descriptor ready for input and output using
 * 		Unix I/O functions
 */
int open_clientfd(char *hostname, char *port) {
	int clientfd;
	struct addrinfo hints, *listp, *p;

	// Get list of potential server addresses
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM; // open a connection
	hints.ai_flags = AI_NUMERICSERV; // using numeric port arg
	hints.ai_flags |= AI_ADDRCONFIG; //recommended for connections
	getaddrinfo(hostname, port, &hints,&listp);

	// walk the list for one that we can successfully connect to
	for(p = listp; p; p = p->ai_next) {
		// create a socket descriptor
		if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			continue;

		// conext to the server
		if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
			break; // success
		close(clientfd); // connect failed, try annother
	}
	freeaddrinfo(listp);
		if(!p) // all connects failed
			return -1;
		else // the last connect succeeded
			return clientfd;
}

/* Connects to the comp280.sandiego.edu server and sends & receives a msg
 *
 * @param *net_in Pointer to the data to be sent
 * @param *net_out Buffer to read data into
 */
void send_receive(const void *net_in, void *net_out) {
	int clientfd = open_clientfd("comp280.sandiego.edu", "9181");
	if(clientfd == -1) {
		printf("Connection failed");
		exit(1);
	}
	int len = strlen(net_in);
	int bytes_sent = send(clientfd, net_in, len, 0);
	int bytes_received = recv(clientfd, net_out, 1000, 0);
	if(bytes_sent == -1 || bytes_received == -1) {
		printf("Error sending and receiving the message %s \n", strerror(errno));
		exit(1);
	}
}


/* Writes to the 'netconfig.txt' file
 *
 * @param *net_out Pointer to buffer to read data into
 */
void writeFile(char *net_out) {
	FILE *file = fopen("netconfig.txt", "r+");
	if(file == NULL){
		printf("error opening file - writeFile function \n");
		exit(1);
	}
	fprintf(file, "%s", net_out); // write to file
	fflush(file);
	fclose(file);
}

/* Creates threads that execute threadFunc to simulate Game of Life
 *
 * @param *tid_arr Array of thread id's
 * @param num_threads Number of threads to be created
 * @param num_rows Number of rows in game board
 * @param num_cols Number of columns in game board
 * @param iterations Number of iterations to execute
 * @param *board Copy of game board
 * @param verbose Verbose Mode enabled if 1, disabled if 0
 * @param print_partition Prints the partition information if 1, doesn't if 0
 */
void createThreads(pthread_t *tid_arr, int num_threads, int num_rows, int num_cols, int iterations, char *board, int verbose, int print_partition) {
	int tmp = 0;
	ThreadArgs *thread_args = calloc(num_threads, sizeof(ThreadArgs));
	pthread_barrier_t barrier;
	int timestep = 0;
	if (pthread_barrier_init(&barrier, NULL, num_threads) != 0) {
		perror("pthread_barrier_init");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < num_threads; i++) {
		thread_args[i].rows = num_rows/num_threads;
	}
	if (num_rows % num_threads > 0) {
		for (int i = 0; i < num_rows % num_threads; i++){
			thread_args[i].rows++;
		}
	}
	for (int i = 0; i < num_threads; i++) {
		thread_args[i].start_row = tmp;
		tmp = tmp + (thread_args[i].rows);
		thread_args[i].board_rows = num_rows;
		thread_args[i].board_cols = num_cols;
		thread_args[i].iterations = iterations;
		thread_args[i].board = board;
		thread_args[i].barrier = &barrier;
		thread_args[i].verbose = verbose;
		thread_args[i].timestep = &timestep;
		thread_args[i].tid = i;
		thread_args[i].start_index = translateId(thread_args[i].start_row, 0, num_rows, num_cols);
		thread_args[i].end_index = thread_args[i].start_index + (thread_args[i].rows * num_cols);
		thread_args[i].print_p = print_partition;
	}


	for (int i = 0; i < num_threads; i++) {
		pthread_create(&tid_arr[i], NULL, threadFunc, (void*)&thread_args[i]);
	}
	for (int i = 0; i < num_threads; i++) {
		pthread_join(tid_arr[i], NULL);
	}
	pthread_barrier_destroy(&barrier);
	free(thread_args);
}

/* Function that threads execute to simulate their respective portions of the
 * 		game board
 *
 * @param *thread_args Struct containing the variables each thread needs to
 * 		properly execute each iteration of the simulation
 */
void *threadFunc(void *thread_args) {
	ThreadArgs *args = (ThreadArgs*)thread_args;

	for (int i = 0; i < args->iterations; i++) {

		int r = pthread_barrier_wait(args->barrier);
		if (r == PTHREAD_BARRIER_SERIAL_THREAD && args->verbose == 1) {
			printBoard(args->board, args->board_rows, args->board_cols, i);
		}
		pthread_barrier_wait(args->barrier);
		takeStep(args->board, args->board_rows, args->board_cols, args->start_row, args->rows, args->barrier);
		pthread_barrier_wait(args->barrier);
	}
	pthread_barrier_wait(args->barrier);
	if (args->print_p == 1) {
		fprintf(stdout, "tid %d\t rows: %d:%d\t (%d)\n", args->tid, args->start_index, args->end_index, args->rows);
		fflush(stdout);
	}
	pthread_barrier_wait(args->barrier);
	return NULL;
}
