# include "stats_functions.h"


MemStruct initMemStruct(int samples){
    MemStruct myStruct;
    myStruct.mem_usage = (double **) malloc (sizeof(double) * samples);
    if (myStruct.mem_usage == NULL){
        fprintf(stderr, "Error allocating memory for cpu usage\n");
        free(myStruct.mem_usage);
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < samples; i++){
        myStruct.mem_usage[i] = (double *) malloc (sizeof(double) * 4);
        if (myStruct.mem_usage[i] == NULL) {
            fprintf(stderr, "Error allocating memory for cpu usage\n");
            deleteMem(samples, &myStruct);
            exit(EXIT_FAILURE);
        }
        myStruct.mem_usage[i][MEMTOT] = 0.0;
        myStruct.mem_usage[i][MEMUSED] = 0.0;
        myStruct.mem_usage[i][MEMTOTVIRT] = 0.0;
        myStruct.mem_usage[i][MEMUSEDVIRT] = 0.0;
    }

    return myStruct;
}

void storeMemUsage(int iter, double new_mem[4], MemStruct *mem_usage){
    mem_usage->mem_usage[iter][MEMTOT] = new_mem[MEMTOT];
    mem_usage->mem_usage[iter][MEMUSED] = new_mem[MEMUSED];

    mem_usage->mem_usage[iter][MEMTOTVIRT] = new_mem[MEMTOTVIRT];
    mem_usage->mem_usage[iter][MEMUSEDVIRT] = new_mem[MEMUSEDVIRT];
}

void getMemUsage(double mem_usage[4]){
    double total_mem, used_mem, total_virt_mem, used_virt_mem;
    struct sysinfo info;

    if (sysinfo(&info) != 0) {
        perror("Error getting system information");
        exit(EXIT_FAILURE);
    }

    total_mem = (double)info.totalram * info.mem_unit / (1024.0 * 1024 * 1024);
    used_mem = (double)(info.totalram - info.freeram) * info.mem_unit / (1024.0 * 1024 * 1024);

    total_virt_mem = (double)(info.totalram + info.totalswap) * info.mem_unit / (1024 * 1024 * 1024);
    used_virt_mem = (double)((info.totalram + info.totalswap) - (info.freeram - info.freeswap)) * info.mem_unit / (1024 * 1024 * 1024);
    
    mem_usage[MEMTOT] = total_mem;
    mem_usage[MEMUSED] = used_mem;

    mem_usage[MEMTOTVIRT] = total_virt_mem;
    mem_usage[MEMUSEDVIRT] = used_virt_mem;
}

void deleteMem(int samples, MemStruct *mem_usage){
    for (int i = 0; i < samples; i++) {
        free(mem_usage->mem_usage[i]);
    }
    free(mem_usage->mem_usage);
}

CPUStruct initCPUStruct (int samples){
    // allocating memory to use CPUStruct
    // initializing CPUStruct
    CPUStruct myStruct;
    myStruct.cpu_usage = (double **) malloc (sizeof(double) * samples);
    if (myStruct.cpu_usage == NULL){
        fprintf(stderr, "Error allocating memory for cpu usage\n");
        free(myStruct.cpu_usage);
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < samples; i++){
        myStruct.cpu_usage[i] = (double *) malloc (sizeof(double) * 3);
        if (myStruct.cpu_usage[i] == NULL) {
            fprintf(stderr, "Error allocating memory for cpu usage\n");
            deleteCPU(samples, &myStruct);
            exit(EXIT_FAILURE);
        }
        myStruct.cpu_usage[i][CPUTIME] = 0.0;
        myStruct.cpu_usage[i][CPUUSE] = 0.0;
        myStruct.cpu_usage[i][CPUUTIL] = 0.0;
    }

    return myStruct;
}

void getCPUUsage(double cpu_usage[2]){
    char line[MAX_STR_LEN];
    unsigned long user, nice, system, idle, iowait, irq, softirq;
    double current_cpu_time, current_cpu_usage;

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

    cpu_usage[CPUTIME] = current_cpu_time;
    cpu_usage[CPUUSE] = current_cpu_usage;
}

void storeCPUUsage(int iter, double new_cpu[2], CPUStruct *cpu_usage){
    cpu_usage->cpu_usage[iter][CPUTIME] = new_cpu[CPUTIME];
    cpu_usage->cpu_usage[iter][CPUUSE] = new_cpu[CPUUSE];
    
    calculateCPUUtil(iter, cpu_usage);
}

/* helper */
void calculateCPUUtil(int iter, CPUStruct *cpu_usage){
    // Ui = Ti - Ii where I is idle time... 
    // formula is (U2 - U1)/(T2 - T1) * 100 
    double usage_pre, usage_cur;
    double time_pre, time_cur;
    double cpu_util;

    usage_cur = cpu_usage->cpu_usage[iter][CPUUSE];
    time_cur = cpu_usage->cpu_usage[iter][CPUTIME];

    if (iter == 0){
        cpu_util = (double)(usage_cur/time_cur)*100.0;
        cpu_usage->cpu_usage[iter][CPUUTIL] = cpu_util;
        return;
    }

    usage_pre = cpu_usage->cpu_usage[iter - 1][CPUUSE];
    time_pre = cpu_usage->cpu_usage[iter - 1][CPUTIME];
    cpu_util = (double) ((usage_cur - usage_pre)/(time_cur - time_pre)) * 100;
    cpu_usage->cpu_usage[iter][CPUUTIL] = cpu_util;
}

void deleteCPU(int samples, CPUStruct *cpu_usage){
    for (int i = 0; i < samples; i++) {
        free(cpu_usage->cpu_usage[i]);
    }
    free(cpu_usage->cpu_usage);
}

char *getUsers(int fd){
    struct utmp *ut;
    char *data = NULL;  // Initialize data pointer
    size_t data_len = 0;  // Initialize data length

    setutent();
    while ((ut = getutent()) != NULL) {
        if (ut->ut_type == USER_PROCESS) {
            char temp[MAX_STR_LEN];  // Temporary buffer for sprintf
            sprintf(temp, "%-12s %-16s (%s)\n", ut->ut_user, ut->ut_line, ut->ut_host);

            // Reallocate memory for data and concatenate the strings
            size_t temp_len = strlen(temp);
            data = realloc(data, data_len + temp_len + 1);  // +1 for null terminator
            if (data == NULL) {
                // Handle memory allocation failure
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            strcpy(data + data_len, temp);
            data_len += temp_len;
        }
    }
    endutent();

    return data;
}