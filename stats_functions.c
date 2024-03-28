# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>

# define CPUTIME 0
# define CPUUTIL 1

typedef struct CPUStruct{
    double **cpu_usage;
}CPUStruct;

void getCPUUsage(int iter, CPUStruct *cpu_usage){
    char line[256];
    unsigned long user, nice, system, idle, iowait, irq, softirq, current_cpu_time, current_cpu_usage;

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
    printf("%ld %ld\n", current_cpu_time, current_cpu_usage);
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
    CPUStruct cpu_usage = initCPUStruct(samples);

    getCPUUsage(0, &cpu_usage);
    for (int i = samples; i > 0; i--){
        system("clear");
        printf("Nbr samples: %d\n", samples);
        printf("every %d seconds\n", tdelay);
        printf("---------------------\n");
        getCPUUsage(i, &cpu_usage);
        // printCPUInfo(samples, cpu, i);
        sleep(tdelay);
    }
	printf("%.2f %.2f\n", cpu_usage.cpu_usage[0][CPUTIME], cpu_usage.cpu_usage[0][CPUUTIL]);
}