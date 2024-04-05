# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <unistd.h>
# include <sys/sysinfo.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <utmp.h>

# define CPUTIME 0
# define CPUUSE  1
# define CPUUTIL 2

# define MEMTOT       0
# define MEMUSED      1
# define MEMTOTVIRT   2
# define MEMUSEDVIRT  3

# define MAX_STR_LEN 1024

#ifndef __Stats_Function_header
#define __Stats_Function_header

typedef struct CPUStruct{
    double **cpu_usage;
}CPUStruct;

typedef struct MemStruct{
    double **mem_usage;
}MemStruct;

/*Initializing MemStruct for usage in getting memory utilization samples */
MemStruct initMemStruct(int samples);
CPUStruct initCPUStruct (int samples);

void getMemUsage(double mem_usage[4]);
void getCPUUsage(double cpu_usage[2]);
void calculateCPUUtil(int iter, CPUStruct *cpu_usage);

void storeMemUsage(int iter, double new_mem[4], MemStruct *mem_usage);
void storeCPUUsage(int iter, double new_cpu[2], CPUStruct *cpu_usage);

char *getUsers(int fd);

/* Frees memory allocated for CPU info
*/
void deleteCPU(int samples, CPUStruct *cpu_usage);
void deleteMem(int samples, MemStruct *mem_usage);

#endif
