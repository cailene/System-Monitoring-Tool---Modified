# include <string.h>
# include <stdlib.h>
# include <stdio.h>

# define CPUGRAPHICS "|"

# include "stats_functions.h"

#ifndef __Main_Program_header
#define __Main_Program_header

int main(int argc, char ** argv);
void printMemUtil(int iter, int samples, MemStruct *mem_usage);
void printMemUtil(int iter, int samples, MemStruct *mem_usage);
void printCPUInfoGraphics(int iter, int samples, CPUStruct *cpu_usage);
#endif