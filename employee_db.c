/* employee_db.c 
 *
 * This file contains code to implement an employee database program in C.
 * The program stores employee info in an array of employee structs, sorted by
 * employee ID value.  Employee info will be read in from an input file when
 * the program first starts.  Users have the option to (1) Print the Database,
 * (2) Lookup by ID, (3) Lookup by Last Name, (4) Add an Employee, (5) Quit.
 *
 *
 * This file is part of COMP 280, Lab #1
 *
 * Author: Zach Fukuhara
 */

#include <stdio.h>      // the C standard I/O library
#include <stdlib.h>     // the C standard library
#include <string.h>     // the C string library
#include "readfile.h"   // my file reading routines
#include <strings.h>	// for strcasecmp
 
// Constants 
#define MAXFILENAME  128
#define MAXNAME       64
#define MAXEMPLOYEE 1024

// Employee struct
struct Employee {
	char first[MAXNAME];
	char last[MAXNAME];
	int id;
	int salary;	
};

// The following line allows us to use "Employee" rather than 
// "struct Employee" throughout this code.
typedef struct Employee Employee;

// Allows for use of boolean expressions
typedef int bool;
enum {false, true};

// Forward Declaration of functions
void getFilenameFromCommandLine(char filename[], int argc, char *argv[]);
int readFile(char *filename, Employee *db);
void printArray(Employee *db, int numEmp);
void sortById(Employee *db, int numEmp);
void sortByLast(Employee *db, int numEmp);
void swap(Employee *emp1, Employee *emp2);
void lookupById(Employee *db, int id, int numEmp);
void lookupByLast(Employee *db, char last[], int numEmp);
void addEmployee(Employee *db, int numEmp, int id, char first[], char last[], int salary);
void updateEmp(Employee *emp, int id, char first[], char last[], int salary);


//----------------------------------------Begin Main Method-------------------------------------------

/* Main method of program.  Used to call methods to create & manipulate
 * employee database.
 */
int main (int argc, char *argv[]) {
	
	// Allocate memory on heap for employee database
	char filename[MAXFILENAME];
	Employee *db = calloc(MAXEMPLOYEE, sizeof(Employee));

	// This initializes the filename string from the command line arguments
	getFilenameFromCommandLine(filename, argc, argv);
	
	// Create database of employees
	int numEmp = readFile(filename, db);

	// Sort database by ID values
	sortById(db, numEmp);

	// Prompt user & execute desired task
	while (true) {
		printf("\n\nEmployee DB Menu:\n\
_____________________________\n\
(1) Print the Database.\n\
(2) Lookup by ID.\n\
(3) Lookup by Last Name.\n\
(4) Add an Employee.\n\
(5) Quit.\n\
(6) Update an employee's information.\n\
(7) Sort employees by Last Name.\n\
(8) Sort employees by ID.\n\
_____________________________\n\
Enter your choice: ");
		
		int inpt[1];
		scanf("%d", inpt);

		if (*inpt == 1) { // Print the database
			printf("\n\n			Employee Database:\n_____________________________________________________________________\n");
			printArray(db, numEmp);
		}
		else if (*inpt == 2) { // Lookup by ID
			printf("Enter Employee ID: ");
			int id[1];
			scanf("%d", id);
			lookupById(db, *id, numEmp);
		}
		else if (*inpt == 3) { // Lookup by Last Name
			printf("Enter Employee's Last Name: ");
			char last[MAXNAME];
			scanf("%s", last);
			lookupByLast(db, last, numEmp);
		}
		else if (*inpt == 4) { // Add an Employee
			int id[1], salary[1], error, yesOrNo[1];
			char first[MAXNAME], last[MAXNAME];	
			while (true) {
				error = 0;
				// Gather employee info
				printf("Enter the employee's ID: ");
				scanf("%d", id);
				printf("Enter the employee's first and last name: ");
				scanf("%s %s", first, last);	
				printf("Enter the employee's salary: ");
				scanf("%d", salary);

				if (*id < 100000 || *id > 999999) {
					printf("\n\nInvalid ID.  Enter a six digit number.\n\n");
					error = 1;
				}
				if (*salary > 150000 || *salary < 30000) {
					printf("\n\nInvalid salary.  Enter a value between 30,000 and 150,000\n\n");
					error = 1;
				}
				if (first[0] == '\0' || last[0] == '\0') {
					printf("\n\nPlease enter a valid first and last name\n\n");
					error = 1;
				}
				if (error == 0) {
					printf("\n\nConfirm Employee Info\n\nID: %d \nName: %s %s \nSalary: $%d \n\nEnter '0' to confirm or '1' to cancel the add: ", *id, first, last, *salary);
					scanf("%d", yesOrNo);
					if (*yesOrNo == 0) {
						addEmployee(db, numEmp, *id, first, last, *salary);
						numEmp++;
						break;
					}
					else
						break;
				}
			}	
		}
		else if (*inpt == 5) { // Quit
			break;
		}
		else if (*inpt == 6) { // Update Employee Info
			printf("Enter the employee's current ID: ");
			int currId[1];
			scanf("%d", currId);
			int newId[1], newSalary[1];
			char firstName[MAXNAME], lastName[MAXNAME];
			printf("Enter the employee's new ID: ");
			scanf("%d", newId);
			printf("Enter the employee's new first and last name: ");
			scanf("%s %s", firstName, lastName);
			printf("Enter the employee's new salary: ");
			scanf("%d", newSalary);
			int i;
			for (i = 0; i < numEmp; i++) {
				if (db[i].id == *currId) // Locate Employee to be updated
					updateEmp(&db[i], *newId, firstName, lastName, *newSalary);
			}
			sortById(db, numEmp); // Sort database by ID once again
		}
		else if (*inpt == 7) { // Sort Employees by Last Name
			sortByLast(db, numEmp);
		}
		else if (*inpt == 8) { // Sort Employees by ID
			sortById(db, numEmp);
		}
		else { // If user enters number other than 1-8
			printf("Invalid Entry.  Enter a value from 1 to 8.");
			continue;
		}
	}
	printf("\nGoodbye.\n");
	return 0;
}
//-------------------------------------End of Main Method-------------------------------------------

/* Creates Employee database, initializing each Employee w/ user input.
 * 
 * @param filename Name of the file containing the Employees used to
 * 		initialize database
 * @param db The database
 * @return total number of employees contained in the database
 */
int readFile(char *filename, Employee *db) {
	int ret = openFile(filename);
	if (ret == -1) {
		printf("Error: cannot open %s\n", filename);
		exit(1);
	}

	Employee emp;
	int id, salary;
	int i = 0;
	char first[MAXNAME], last[MAXNAME];

	while (ret != -1) {
		ret = readInt(&id);
		if (ret) { break; }
		ret = readString(first);
		if (ret) { break; }
		ret = readString(last);
		if (ret) { break; }
		ret = readInt(&salary);
		if (ret == 0) { // stuff was read in okay
			emp.id = id;
			emp.salary = salary;
			strcpy(emp.first, first);
			strcpy(emp.last, last);
			db[i] = emp;
			i++;	
		}
	}	
	closeFile();
	return i; // Return # of employees in database
}

/* Prints out the Employee database in table format along w/ number of
 * 		employees currently in the database
 * @param db The Employee database
 * @param numEmp The number of employees in the database
 */ 
void printArray(Employee *db, int numEmp) {
	int i;
	for (i = 0; i < numEmp; i++) {
		printf("%d %20s %20s %20d\n", db[i].id, db[i].first, 
			db[i].last, db[i].salary);
	}
	printf("\nTotal Employees: %d", numEmp);
}

/* Sorts the database by ID number
 *
 * @param db The Employee database
 * @param numEmp The number of employees in the database
 */ 
void sortById(Employee *db, int numEmp) {
	int i, j;
	for (i = 0; i < numEmp - 1; i++) {
		for (j = 0; j < numEmp - i - 1; j++) {
			if (db[j].id > db[j + 1].id) {
				swap(&db[j], &db[j + 1]);
			}
		}	
	} 
}
/* Sorts the database by Employees last name
 *
 * @param db The Employee database
 * @param numEmp The number of employees in the database
 */
void sortByLast(Employee *db, int numEmp) {
	int i, j;
	for (i = 0; i < numEmp - 1; i++) {
		for (j = 0; j < numEmp - i - 1; j++) {
			int ret = strcasecmp(db[j].last, db[j + 1].last);
			if (ret > 0) // Swap last names if necessary
				swap(&db[j], &db[j + 1]);
			else if (ret == 0) { // If same last name, check first
				int ret2 = strcasecmp(db[j].first, db[j + 1].first);
				if (ret2 > 0) 
					swap(&db[j], &db[j + 1]);	
			}
		}
	}
}

/* Helper method for sortById and sortByLast, swaps employee locations in
 * 		memory
 *
 * @param emp1 First Employee to be swapped
 * @param emp2 Second Employee to be swapped
 */ 
void swap(Employee *emp1, Employee *emp2) {
	Employee temp = *emp1;
	*emp1 = *emp2;
	*emp2 = temp;
}

/* Searches for employee in database using Binary Search based on ID.  
 * Prints employee info if found, otherwise prints error message.
 *
 * @param db The Employee database
 * @param id The Employee's ID number
 * @param numEmp The number of employees in the database
 */ 
void lookupById(Employee *db, int id, int numEmp) {
	int first = 0, last = numEmp - 1, middle = (first + last) / 2;
	while (first <= last) {
		if (db[middle].id < id)
			first = middle + 1;
		else if (db[middle].id == id) {
			printf("\n\nEmployee found:\nID: %d\nName: %s %s\nSalary: $%d\n\n", db[middle].id,\
			db[middle].first, db[middle].last, db[middle].salary);
			break;
		}
		else
			last = middle - 1;

		middle = (first + last) / 2;
	}
	if (first > last)
		printf("\n\nEmployee not found.  Try again\n\n");
}

/* Searches database for employee based on last name.  Prints employee info if
 * found, otherwise prints error message.
 *
 * @param db The employee database
 * @param last Employee's last name
 * @param numEmp The number of employees in the database
 */ 
void lookupByLast(Employee *db, char last[], int numEmp) {
	int i;
	for (i = 0; i < numEmp; i++) {
		int ret = strcasecmp(db[i].last, last);
		if (ret == 0) {
			printf("\n\nEmployee found:\nID: %d\nName: %s %s\nSalary: $%d\n\n", db[i].id,\
			db[i].first, db[i].last, db[i].salary);
			break;
		}
	}
	if (i == numEmp)
		printf("\n\nEmployee not found.\n\n");
}

/* Adds an employee to the database.
 *
 * @param db The Employee database
 * @param numEmp The number of employees in the database
 * @param id The Employee's ID number
 * @param first The Employee's first name
 * @param last The Employee's last name
 * @param salary The Employee's salary
 */ 
void addEmployee(Employee *db, int numEmp, int id, char first[], char last[], int salary) {
	Employee emp;
	emp.id = id;
	strcpy(emp.first, first);
	strcpy(emp.last, last);
	emp.salary = salary;
	db[numEmp++] = emp;
	sortById(db, numEmp);
	printf("\n\nEmployee added to database.\n\n");
}

/* Updates an employee's information w/i the database.  Prints success message
 * 		when complete.
 * 
 * @param emp The Employee to be updated
 * @param id The Employee's new ID
 * @param first The Employee's new First Name
 * @param last The Employee's new Last Name
 * @param salary The Employee's new Salary
 */ 
void updateEmp(Employee *emp, int id, char first[], char last[], int salary) {
	(*emp).id = id;
	strcpy((*emp).first, first);
	strcpy((*emp).last, last);
	(*emp).salary = salary;
	printf("\n\nEmployee information updated.\n\n");
}

/*  DO NOT MODIFY THIS FUNCTION. It works "as is".
 *
 *  This function gets the filename passed in as a command line option
 *  and copies it into the filename parameter. It exits with an error 
 *  message if the command line is badly formed.
 *  @param filename the string to fill with the passed filename
 *  @param argc, argv the command line parameters from main 
 *               (number and strings array)
 */
void getFilenameFromCommandLine(char filename[], int argc, char *argv[]) {

	if (argc != 2) {
		printf("Usage: %s database_file\n", argv[0]);
		// exit function: quits the program immediately...some errors are not 
		// recoverable by the program, so exiting with an error message is 
		// reasonable error handling option in this case
		exit(1);   
	}
	if (strlen(argv[1]) >= MAXFILENAME) { 
		printf("Filename, %s, is too long, cp to shorter name and try again\n",
				filename);
		exit(1);
	}
	strcpy(filename, argv[1]);
}
