# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <unistd.h>

# define CPUTIME 0
# define CPUUTIL 1
# define MAX_STR_LEN 1024

#ifndef __Stats_Function_header
#define __Stats_Function_header

typedef struct CPUStruct{
    double **cpu_usage;
}CPUStruct;

CPUStruct initCPUStruct (int samples);
void getCPUUsage(int iter, CPUStruct *cpu_usage);

/* Frees memory allocated for CPU info
*/
void deleteCPU(int samples, CPUStruct *cpu_usage);

#endif