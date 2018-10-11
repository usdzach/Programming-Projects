/*
 * stats.c
 *
 * This file contains code that takes in a data file command line argument
 * from the user and computes the mean, median, and standard deviation.
 * The program then prints these statistics, along with the number of values
 * in the user input data and the unused capacity in the array that stores the values
 * ([] is called data).
 *
 * This file is part of COMP 280, Lab #3.
 *
 * Author:  Zach Fukuhara (zfukuhara@sandiego.edu)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "readfile.h"
#include <valgrind/memcheck.h>

// Forward Declaration
double *getValues(int *size, int *capacity, char *filename);
double getMean(int size, double *data);
double getMedian(int size, double *data);
double getStandardDev(int size, double *data, double mean);

// Begin main function
int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf ("usage: ./stats filename\n");
		exit(1);
	}
	// argv[1] contains filename string (pass argv[1] as the
	// third argument to getValues)
	int size = 0, capacity = 20;
	int *s = &size;
	int *c = &capacity;
	double *arr = getValues(s, c, argv[1]);
	double mean = getMean(size, arr);
	double std = getStandardDev(size, arr, mean);
	double median = getMedian(size, arr);
	int unused = capacity - size;
	printf("\nStatistics:\n--------\n");
	printf("num values:\t\t%d\n", size);
	printf("mean:\t\t\t%.3f\n", mean);
	printf("median:\t\t\t%.3f\n", median);
	printf("std dev:\t\t%.3f\n", std);
	printf("\nUnused array slots:\t%d\n\n", unused);
	free(arr);
	return 0;
}

/*
 * Dynamically allocates memory for array of data and initializes the array
 * 'data' with values from the parameter *filename data file.  Returns 'data'
 * array to caller.  Size and capacity get passed back to caller via pointer
 * parameters used for pass-by-reference values.
 *
 * @param size Stores current number of elements in data array
 * @param capacity Predefined maximum capacity of data array, subject to being
 * 		doubled if data array needs more space for more elements
 * @param filename Name of the file containing values to initialize data array
 *
 * @return Array data which contains all values from file filename
 */
double *getValues(int *size, int *capacity, char *filename) {
	// Allocate memory for array of doubles
	double *data = malloc(*capacity * sizeof(double));
	int i = 0;
	double ret = openFile(filename);
	// Check if file is empty
	if(ret == -1){
		printf("The File is empty");
	    exit(1);
	}
	// Read in values to data array from filename file
	while(ret != -1){
	    ret = readDouble(&data[i]);
	    i++;
	    if(ret) { break; }
		// If array runs out of space, double capacity
	    if(i > *capacity - 1){
			*capacity *= 2;
			// Create new array and copy over values
	    	double *more_data = malloc(*capacity * sizeof(double));
			for(int j = 0; j < i; j++){
				more_data[j] = data[j];
			}
			free(data); // Free space allocated for data
			data = more_data;
		}
	}
	*size = i - 1;
	closeFile();
	return data;
}

/*
 * Computes and returns the mean of the data values stored in array 'data'
 *
 * @param size Number of elements in array
 * @param data Array of data values
 *
 * @return computed mean of the data array
 */
double getMean(int size, double *data){
	double result = 0;
	for(int i = 0; i < size; i++){
		result += data[i];
	}
	result = result / size;
	return result;
}

/*
 * Sorts the elements stored in array 'data' using Bubble Sort.  Then computes
 * median of data values in array 'data'.  If the number of elements in 'data'
 * is even, return the mean of the two middle-most values.  If the number of
 * elements is odd, return the middle most value.
 *
 * @param size Number of elements in array
 * @param data Array of data values
 *
 * @return computed median of the data array
 */
double getMedian(int size, double *data){
    int length = size;
	// Sort array
    for(int i = 0; i < length - 1; i++){
        for(int j = 0; j < length - i - 1; j++){
            if(data[j] > data[j+1]){
                double temp = data[j];
				data[j] = data[j+1];
				data[j+1] = temp;
			}
		}
	}
	// If even number of elements, return avg of two middle values
	if(length % 2 == 0){
		int middle_left = (length + 1) / 2 - 1;
		int middle_right = middle_left + 1;
		return (data[middle_right] + data[middle_left]) / 2;
	}
	// If odd number of elements, return middle value
	if(length % 2 == 1){
		int middle = (length + 1) / 2 - 1;
	    return data[middle];
	}
	return 0;
}

/*
 * Computes standard deviation of values within array using formula from
 * project instructions.  Returns result to caller.
 *
 * @param size Number of elements in array
 * @param data Array of data values
 *
 * @return computed standard deviation of data array
 */
double getStandardDev(int size, double *data, double mean){
	double stdv = 0;
	for(int i = 0; i < size; i++){
	    stdv += pow((data[i] - mean), 2);
    }
    double result = stdv / (size - 1);
    return sqrt(result);
}
