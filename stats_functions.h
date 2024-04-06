# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <unistd.h>
# include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
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

typedef struct SystemStats{
    char* header;
    int uptime[4];
    char sys_info[5][256];
    int cpu_cores;
    long self_mem_utl;
}SystemStats;

/*Initializing SystemStats and populating with system information */
SystemStats initSystemStats();
/*Initializing MemStruct for usage in getting memory utilization samples */
MemStruct initMemStruct(int samples);
/*Initializing CPUStruct for usage in getting memory utilization samples */
CPUStruct initCPUStruct (int samples);

/* reads from /proc/uptime file to get system uptime */
void getUptime(SystemStats *stats);
/* uses rusage struct to get self memory utilization */
void getSelfMemUtl(SystemStats *stats);
/* uses utsname struct to get system information*/
void getSysInfo(SystemStats *stats);
// Function to get the number of CPU cores
void getCPUCores(SystemStats *stats);
/* Uses utmp struct to collect all current users in a singular string and returns it*/
char *getUsers(int fd);
/* uses sysinfo struct to take a singular sample of memory utilization 
and stores it in array of 4 of type double*/
void getMemUsage(double mem_usage[4]);
/* reads from /proc/stat file to take a singular sample of cpu utilization 
and stores it in an array of 2 of type double*/
void getCPUUsage(double cpu_usage[2]);
/* calculates cpu utilization of all previously taken samples and the current 
iteration/sample taken. The formula used is (U2 - U1)/(T2 - T1) * 100 
where Ui = Ti - Ii where I is idle time... */
void calculateCPUUtil(int iter, CPUStruct *cpu_usage);

/* stores new memory utilization sample into the mem_usage of type MemStruct*/
void storeMemUsage(int iter, double new_mem[4], MemStruct *mem_usage);
/* stores new cpu utilization sample into the cpu_usage of type CPUStruct*/
void storeCPUUsage(int iter, double new_cpu[2], CPUStruct *cpu_usage);

/* Frees memory allocated for CPU info & Mem info
*/
void deleteCPU(int samples, CPUStruct *cpu_usage);
void deleteMem(int samples, MemStruct *mem_usage);

#endif
