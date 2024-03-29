# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>
# include<unistd.h>

# define CPUTIME 0
# define CPUUTIL 1

typedef struct CPUStruct{
    double **cpu_usage;
}CPUStruct;

void getCPUUsage(int iter, CPUStruct *cpu_usage){
    char line[256];
    unsigned long user, nice, system, idle, iowait, irq, softirq;
    double current_cpu_time, current_cpu_usage;

    //printf("calling cpu w i: %d\n", iter);

    FILE *statfile = NULL;

    statfile = fopen("/proc/stat", "r");
    if (statfile == NULL){
        fprintf(stderr, "Error opening /proc/stat\n");
        exit(EXIT_FAILURE);
    }

    if (fgets(line, sizeof(line), statfile) == NULL){
        fprintf(stderr, "Error reading /proc/stat\n");
        exit(EXIT_FAILURE);
    }

    sscanf(line, "cpu %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq);

    fclose(statfile);

    current_cpu_time = user + nice + system + idle + iowait + irq + softirq;
    current_cpu_usage = current_cpu_time - idle;

    cpu_usage->cpu_usage[iter][CPUTIME] = current_cpu_time;
    cpu_usage->cpu_usage[iter][CPUUTIL] = current_cpu_usage;

    //printf("%.2f %.2f\n", current_cpu_time, current_cpu_usage);
}

// Calling it calculateCPUUtil + print would be more suitable
void printCPUInfo(int iter, CPUStruct *cpu_usage){
    // Ui = Ti - Ii where I is idle time... 
    // formula is (U2 - U1)/(T2 - T1) * 100 
    double usage_pre, usage_cur;
    double time_pre, time_cur;
    double cpu_util;

    usage_pre = cpu_usage->cpu_usage[iter - 1][CPUUTIL];
    usage_cur = cpu_usage->cpu_usage[iter][CPUUTIL];

    time_pre = cpu_usage->cpu_usage[iter - 1][CPUTIME];
    time_cur = cpu_usage->cpu_usage[iter][CPUTIME];

    cpu_util = (double) (usage_cur - usage_pre)/(time_cur - time_pre) * 100;
    
    printf(" total cpu use: %.2f%%\n", cpu_util);
}

CPUStruct initCPUStruct (int samples){
    // allocating memory to use CPUStruct
    // initializing CPUStruct
    CPUStruct myStruct;
    myStruct.cpu_usage = (double **) malloc (sizeof(double) * samples);
    if (myStruct.cpu_usage == NULL){
        fprintf(stderr, "Error allocating memory for cpu usage\n");
        exit(EXIT_FAILURE);
    }
    // printf("allocating space for %d entries\n", samples);
    for(int i = 0; i < samples; i++){
        myStruct.cpu_usage[i] = (double *) malloc (sizeof(double) * 2);
        if (myStruct.cpu_usage[i] == NULL) {
            fprintf(stderr, "Error allocating memory for cpu usage\n");
            exit(EXIT_FAILURE);
        }
        myStruct.cpu_usage[i][CPUTIME] = 0.0;
        myStruct.cpu_usage[i][CPUUTIL] = 0.0;
    }

    return myStruct;
}

int main(int argc, char ** argv){
    int samples = 5, tdelay = 1;
    CPUStruct cpu_usage = initCPUStruct(samples + 1);

    getCPUUsage(0, &cpu_usage);
    sleep(tdelay);
    // for (int i = 0; i < samples; i++){
    //     // printf("iteration >> %d\n", i + 1);
    //     getCPUUsage(i + 1, &cpu_usage);
    //     printCPUInfo(i + 1, &cpu_usage);
    //     sleep(tdelay);
    //     printf("\n");
    // }
    // for(int i = 0; i < samples + 1; i++){
    //     printf("%.2f %.2f\n", cpu_usage.cpu_usage[i][CPUTIME], cpu_usage.cpu_usage[i][CPUUTIL]);
    //     //printCPUInfo(i, &cpu_usage);
    // }
    for (int i = 0; i < samples; i++){
        system("clear");
        printf("Nbr samples: %d\n", samples);
        printf("every %d seconds\n", tdelay);
        printf("---------------------\n");
        printf("iteration >> %d\n", i + 1);
        getCPUUsage(i + 1, &cpu_usage);
        printCPUInfo(i + 1, &cpu_usage);
        sleep(tdelay);
    }
    // we can use another function to handle getting the previous cpu and printing it.. 
	// printf("%.2f %.2f\n", cpu_usage.cpu_usage[0][CPUTIME], cpu_usage.cpu_usage[0][CPUUTIL]);
}