# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <utmp.h>
#include <signal.h> 

// CPU Graphics
# define PERCPOS     "|"

// Mem Graphics 
# define NEG         "@"
# define POS         "*"
# define NEUT        "o"
# define NEGPOINT    ":"
# define POSPOINT    "#"

# define ZERO 0

# define CLEAR_TO_HOME "\033[H"
# define CLEAR_SCREEN  "\033[2J"

# include "stats_functions.h"

#ifndef __Main_Program_header
#define __Main_Program_header

/* Handles reading command line arguments and information to print. 
Implements pipes and forking to handle concurrent running of memory
utilization, cpu utilization and getting user sessions.
Also implements signals, specifically CTRL-Z and CTRL-C */
int main(int argc, char ** argv);

/* Signal functions*/
/* If CTRL-Z is signalled, the program ignores this signal*/
void ignore(int signum);
/* If CTRL-C is signalled, the program asks the user if they want
to terminate the program and handles approriately*/
void leave(int signum);

/*Print functions*/
/*Prints System information including:
    System Name, 
    Machine Name, 
    Version, 
    Release, 
    Architecture, 
    and Uptime */
void printSysInfo(SystemStats *stats);
/*Prints CPU utilization and the number of cores of your system */
void printCPUInfo(int iter, int samples, CPUStruct *cpu_usage, SystemStats *stats);
/*Prints memory utilization including physical and virtual memory */
void printMemUtil(int iter, int samples, MemStruct *mem_usage);
/*Prints the CPU utilization with graphics with PERCPOS sign*/
void printCPUInfoGraphics(int iter, int samples, CPUStruct *cpu_usage);
/*Prints the memory utilization with graphics with defined memory graphics*/
void printMemUtilGraphics(int iter, int samples, MemStruct *mem_usage);

/*Prints a message on how to use the program*/
void Message();

/*helper for printing graphics of cpu utilization*/
int getdifference(double cur_mem, double pre_mem, int *difference);
#endif