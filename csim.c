/*
 * csim.c
 *
 * Authors: Tyler Bullock <tbullock@sandiego.edu>
 * 		      Zach Fukuhara <zfukuhara@sandiego.edu>
 *
 * Description: This program simulates a computer's cache
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cachelab.h"
#include <string.h>
#include <math.h>

typedef unsigned long int mem_addr;
typedef struct Block Block;
typedef struct Cache Cache;
typedef struct Set Set;

struct Block {
	unsigned int valid;
	mem_addr tag;
	unsigned int set_index;
	unsigned int lru;
};
struct Set {
	Block *block;
};
struct Cache {
	unsigned int big_s;
	unsigned int s;
	unsigned int big_b;
	unsigned int b;
	unsigned int num_lines;
	unsigned int E;
	Set *set;
};

// forward declaration
void simulateCache(char *trace_file, int num_sets, int block_size, int lines_per_set, int verbose);
void readFile(char *tracefile);
void createCache(int s, int b, int E, Cache *cache);
void checkCache(int *hit_count, int *miss_count, int *eviction_count, Cache *cache, mem_addr address, int s, int b, int verbose);
void updateLRU(Cache *cache, int set_index, int block_index);

/**
 * Prints out a reminder of how to run the program.
 *
 * @param executable_name String containing the name of the executable.
 */
void usage(char *executable_name) {
	printf("Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>", executable_name);
}

int main(int argc, char *argv[]) {

	int verbose_mode = 0;
	int num_sets = -1, block_size = -1, lines_per_set = -1;
	char *trace_filename = NULL;

	opterr = 0;

	int c = -1;

	// Note: adding a colon after the letter states that this option should be
	// followed by an additional value (e.g. "-s 1")
	while ((c = getopt(argc, argv, "vs:E:b:t:")) != -1) {
		switch (c) {
			case 'v':
				// enable verbose mode
				verbose_mode = 1;
				break;
			case 's':
				// specify the number of sets
				// Note: optarg is set by getopt to the string that follows
				// this option (e.g. "-s 2" would assign optarg to the string "2")
				num_sets = 1 << strtol(optarg, NULL, 10);
				break;
			case 'E':
				lines_per_set = strtol(optarg, NULL, 10);
				break;
			case 'b':
				block_size = 1 << strtol(optarg, NULL, 10);
				break;
			case 't':
				// specify the trace filename
				trace_filename = optarg;
				break;
			case '?':
			default:
				usage(argv[0]);
				exit(1);
		}
	}

	if(num_sets == -1 || block_size == -1 || lines_per_set == -1 || trace_filename == NULL){
		printf("You must specify all options(-s -E -b -t).\n");
		exit(1);
	}
	if (verbose_mode) {
		printf("Verbose mode enabled.\n");
		printf("Trace filename: %s\n", trace_filename);
		printf("Number of sets: %d\n", num_sets);
	}

	simulateCache(trace_filename, num_sets, block_size, lines_per_set, verbose_mode);
	return 0;
}

/**
 * Simulates cache with the specified organization (S, E, B) on the given
 * trace file.
 *
 * @param trace_file Name of the file with the memory addresses.
 * @param num_sets Number of sets in the simulator.
 * @param block_size Number of bytes in each cache block.
 * @param lines_per_set Number of lines in each cache set.
 * @param verbose Whether to print out extra information about what the
 *   simulator is doing (1 = yes, 0 = no).
 */
void simulateCache(char *trace_file, int num_sets, int block_size,
						int lines_per_set, int verbose) {
	// Variables to track how many hits, misses, and evictions we've had so
	// far during simulation.
	int hit_count = 0;
	int miss_count = 0;
	int eviction_count = 0;
	Cache *cache = malloc(sizeof(Cache));
	int set_bits = log(num_sets) / log(2);
	int block_bits = log(block_size) / log(2);
	printf("%d\n", set_bits);

	createCache(set_bits, block_bits, lines_per_set, cache);

	FILE *trace = fopen(trace_file, "r");
	if (trace == NULL){
		printf("Cannot Open File %s\n", trace_file);
		exit(1);
	}
	char operation[2] = "";
	mem_addr address;
	int size;
	while (fscanf(trace, "%s %lx,%d", operation, &address, &size) == 3){

		if(!strcmp(operation, "I")){
			continue;
		}
		else if(!strcmp(operation, "M")){
			checkCache(&hit_count, &miss_count, &eviction_count, cache, address, set_bits, block_bits, verbose);
			checkCache(&hit_count, &miss_count, &eviction_count, cache, address, set_bits, block_bits, verbose);
		}
		else {
			checkCache(&hit_count, &miss_count, &eviction_count, cache, address, set_bits, block_bits, verbose);
		}

	}

	for (int i = 0; i < num_sets; i++){
		free(cache->set[i].block);
	}
	free(cache->set);
	free(cache);
	fclose(trace);
    printSummary(hit_count, miss_count, eviction_count);
}

/*
 * Takes in a cache struct and initializes it to an empty cache with user
 * defined number of sets and blocks
 *
 * @param s Number of set bits
 * @param b Block offset bits
 * @param E Number of lines in set
 * @param *cache Pointer to the Cache struct to be initialized
 */
void createCache(int s, int b, int E, Cache *cache){
	cache->s = s;
	cache->b = b;
	cache->E = E;

	cache->big_s = pow(2, s);
	cache->num_lines = cache->big_s * cache->E;
    cache->set = (Set*) malloc( sizeof(Set) * cache->num_lines );

	for (int i = 0; i < cache->big_s; i++){
		cache->set[i].block = (Block*) malloc(sizeof(Block) * cache->E);
		for (int j = 0; j < E; j++){
			cache->set[i].block[j].valid = 0;
			cache->set[i].block[j].tag = 0;
			cache->set[i].block[j].lru = j;
		}
	}
}

/*
 * Simulates hits, misses, and miss evictions in Cache
 *
 * @param hit_count initial number of hits before function call
 * @param miss_count initial number of misses before function call
 * @param eviction_count initial number of evictions before function call
 * @param cache Cache struct to be simulated
 * @param address Memory address containing tag, set, and block offset bits
 * @param s Number of set bits
 * @param b Number of block offset bits
 * @param verbose Verbose Mode Enabled if 1, Disabled if not
 */
void checkCache(int *hit_count, int *miss_count, int *eviction_count, Cache *cache, mem_addr address, int s, int b, int verbose) {


	mem_addr tag = address >> (s + b);
	mem_addr tmp = address >> b;
	mem_addr mask = 1 << s;
	mem_addr mask_full = mask - 1;
	mem_addr set_index = tmp & mask_full;



	int hit = 0;
	for (int i = 0; i < cache->E; i++){
		if (cache->set[set_index].block[i].valid == 0){
			continue;
		}
		if (cache->set[set_index].block[i].valid == 1){
			if (cache->set[set_index].block[i].tag == tag){
				(*hit_count)++;
				hit = 1;
				updateLRU(cache, set_index, i);
				if (verbose == 1) {
					printf("%lx %s\n", address, "hit");
				}
				break;
			}
		}
	}
	if (hit == 0){
		int block_index = 0;
		for (int i = 1; i < cache->E; i++){
			if	(cache->set[set_index].block[block_index].lru > cache->set[set_index].block[i].lru){
				block_index = i;
			}
		}
		if (cache->set[set_index].block[block_index].valid == 0){
			(*miss_count)++;
			cache->set[set_index].block[block_index].valid = 1;
			cache->set[set_index].block[block_index].tag = tag;
			updateLRU(cache, set_index, block_index);
			if (verbose == 1){
				 printf("%lx %s\n", address, "miss");
			}
		}
		else {
			(*miss_count)++;
			(*eviction_count)++;
			cache->set[set_index].block[block_index].tag = tag;
			updateLRU(cache, set_index, block_index);
			if (verbose == 1){
				printf("%lx %s %s\n", address, "miss", "eviction");
			}
		}
	}

}

/*
 * Updates the Least Recently Used bits.  Stores the LRU of the block being accessed in base_lru.
 * Updates the LRU bit of the block being accessed to the number of lines per set.
 * Then loops through the LRU's of the blocks in the set: if the LRU of
 * another block is greater than the base_lru, the LRU of the other block will
 * decrement.
 *
 * Basically, if the most recently used block is the one being accessed, nothing is changed.
 * If not, it will update the LRU's of all of the blocks in the set that were used
 * more recently than that.
 *
 * @param cache Cache struct to be manipulated
 * @set_index The index of the set containing the block that is being accessed
 * @block_index The index of the block being accessed
 */
void updateLRU(Cache *cache, int set_index, int block_index){
	int base_lru = cache->set[set_index].block[block_index].lru;
	cache->set[set_index].block[block_index].lru = cache->E;
	for (int i = 0; i < cache->E; i++){
		if (base_lru < cache->set[set_index].block[i].lru){
			cache->set[set_index].block[i].lru = cache->set[set_index].block[i].lru - 1;
		}
	}
}
