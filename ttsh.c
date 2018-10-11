/*
 * File: ttsh.c
 *
 * The Tiny Torero Shell (TTSH)
 *
 * Names: Tyler Bullock & Zach Fukuhara
 *
 * Description:  A Linux shell program named TTSH that allows the user to:
 * 		- Execute simple commands run in the foreground
 * 		- Execute simple commands run in the background
 * 		- Exit the shell with the 'exit' command
 * 		- Display a list of N most recently executed commands using 'history' command
 * 		- Execute a command from history using the '!num' command, where num is the ID
 * 			corresponding to a command from the history log (ex. !4)
 * 		-Change the current working directory with the 'cd' command
 */

// NOTE: all new includes should go after the following #define
#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "parse_args.h"
#include "history_queue.h"

void foreground_handler(char *argv[]);
void background_handler(char *argv[]);
void child_handler(__attribute__ ((unused)) int sig);
void add_entry(HistoryEntry val);
void print_history();
char *run_history(char *old_cmd);
char *find_cmd(unsigned int hist_id);
void execute_command(char *argv[], int process_state);
void add_history(unsigned int *history_id, char *cmdline);

int main() {
	// Call to sigaction to register your SIGCHLD signal handler
	struct sigaction sa;
	sa.sa_handler = child_handler;
	sa.sa_flags = SA_NOCLDSTOP;     // don't call handler on child pause/resume
	sigaction(SIGCHLD, &sa, NULL);
	unsigned int history_id = 1;

	while(1) {
		// Print the shell prompt
		fprintf(stdout, "ttsh> ");
		fflush(stdout);

		// (2) Read in the next command entered by the user
		char cmdline[MAXLINE];
		if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)) {
			// fgets could be interrupted by a signal.
			// This checks to see if that happened, in which case we simply
			// clear out the error and restart our loop so it re-prompts the
			// user for a command.
			clearerr(stdin);
			continue;
		}

		if (feof(stdin)) {
			fflush(stdout);
			exit(0);
		}

		// TODO: remove this line after you are done testing/debugging.
		fprintf(stdout, "DEBUG: %s\n", cmdline);

		// Call to parseArguments function to parse it into its argv format
		char *argv[MAXARGS];
		//checks for the !num command
		if ('!' == cmdline[0]) {
			if (run_history(cmdline) == NULL) {
				continue;
			}
			else
				strcpy(cmdline, run_history(cmdline));

				 //restarts the loop if the history_id does not exist
		}
		add_history(&history_id, cmdline);
		// 0 means foreground, non-zero means background
		int process_state = parseArguments(cmdline, argv);
		execute_command(argv, process_state);
	}
	return 0;
}

void add_history(unsigned int *history_id, char *cmdline) {
	HistoryEntry entry;
	entry.cmd_num = (*history_id)++;
	strcpy(entry.cmdline, cmdline);
	add_entry(entry);
}

/* Function that executes the user specified command in either the foreground
 * or background
 *
 * @param argv user's command line
 * @param process_state executes in foreground if 0, background if 1
 */
void execute_command(char *argv[], int process_state){
	if(strcmp(argv[0], "exit") == 0)
		exit(0);
	else if (strcmp(argv[0], "cd") == 0) {
		if (argv[1] == NULL)
			chdir(getenv("HOME"));
		else if (chdir(argv[1]) == -1)
			printf("Error: Directory %s does not exist.\n", argv[1]);
	}
	else if (strcmp(argv[0], "history") == 0)
		print_history();
	else if (process_state == 0)
		foreground_handler(argv);
	else
		background_handler(argv);
}

/* User specified command is to be executed in foreground: command is executed by child
 * process while the parent process waits for it to complete.  The parent then
 * completes.  During this time, the shell cannot execute other commands.
 *
 * @param argv user's command line
 */
void foreground_handler(char *argv[]) {
	pid_t pid;
	int status;

	pid = fork();
	if(pid == 0) {
		if(execvp(argv[0], argv) == -1) {
			printf("Error: Invalid command: %s\n", argv[0]);
			exit(1);
		}
		exit(0);
	}
	else if(pid < 0) {
		printf("Error: Forking error.\n");
		exit(1);
	}
	waitpid(pid, &status, WUNTRACED);
}

/* User specified command is to be executed in the background: command is
 * executed by child process, but the parent process does not wait for the
 * child to exit.  Instead, the parent process is free to take in new
 * commands.
 *
 * @param argv user's command line
 */
void background_handler(char *argv[]) {
	pid_t pid;

	pid = fork();
	if(pid == 0) {
		if (execvp(argv[0], argv) == -1) {
			printf("Error: Invalid command: %s\n", argv[0]);
			exit(1);
		}
	}
	else if (pid < 0) {
		printf("Error: Forking error.\n");
		exit(1);
	}
}

/* Reaps child processes from commands run in the background
 *
 * @param sig UNUSED PARAMETER
 */
void child_handler(__attribute__ ((unused)) int sig){
	pid_t pid;
	int status;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		printf("reaped child %d\n", pid);
	}
}

/* Runs a specified command from the history with the corresponding ID that is
 * contained in old_cmd
 *
 * @param old_cmd user's '!num' command
 * @return returns the string command corresponding to the specified ID, NULL if command cannot
 * 		be found int the history
 */
char *run_history(char *old_cmd) {
	old_cmd++;

	int hist_id = strtol(old_cmd, NULL, 10);
	char *new_cmd = find_cmd(hist_id);
	if (new_cmd == NULL) {
		printf("Error: command does not exist in history\n");
		return NULL;
	}
	else {
		return new_cmd;
	}
}
