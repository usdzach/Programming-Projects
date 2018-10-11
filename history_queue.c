/*
 * File: history_queue.c
 *
 * The Tiny Torero Shell (TTSH)
 *
 * Names: Tyler Bullock & Zach Fukuhara
 *
 * Description: Program that implements a circular queue of history items
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "history_queue.h"

static HistoryEntry history[MAXHIST];
static int start = 0;
static int next = 0;
static int size = 0;

/* Adds a HistoryEntry struct to the history queue
 *
 * @param val HistoryEntry struct to be added to the queue
 */
void add_entry(HistoryEntry val){
	if (next < MAXHIST) {
		history[next] = val;
		next++;
		size++;
	}
	else {
		history[start] = val;
		if (start < MAXHIST - 1) {
			start++;
		}
		else {
			start = 0;
		}
	}
}

/* Prints the history log
 */
void print_history() {
	int index = start;
	for (int i = 0; i < size; i++) {
		if (index >= size) {
			index = 0;
		}
		printf("%d\t%s", history[index].cmd_num , history[index].cmdline);
		index++;
	}
}

/* Finds the command that corresponds to hist_id
 *
 * @param hist_id ID corresponding a command in the history queue
 * @return returns the command from the history or NULL if the command cannot
 * 		be found in the history
 */
char *find_cmd(unsigned int hist_id){
	for (int i = 0; i < size; i++) {
		if (history[i].cmd_num == hist_id) {
			return history[i].cmdline;
		}
	}
	return NULL;
}
