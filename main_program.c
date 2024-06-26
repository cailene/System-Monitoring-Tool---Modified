# include "main_program.h"

void ignore(int signum);
void leave(int signum);

void printCPUInfo(int iter, int samples, CPUStruct *cpu_usage, SystemStats *stats);
void printMemUtil(double mem_usage[4]);
void printCPUInfoGraphics(int iter, int samples, CPUStruct *cpu_usage);
void printMemUtilGraphics(int iter, int samples, MemStruct *mem_usage);
void printSysInfo(SystemStats *stats);

/* print functions for sequential printing*/
void printCPUInfoGraphicsSEQ(int iter, int samples, CPUStruct *cpu_usage);
void printCPUInfoSEQ(int iter, int samples, CPUStruct *cpu_usage, SystemStats *stats);

void Message();

/*helper*/
int getdifference(double cur_mem, double pre_mem, int *difference);

int main(int argc, char ** argv){
    int option;
    int system_flag = 0, cpu_flag = 0, user_flag = 0, 
        sequential_flag = 0, graphics_flag = 0, samples = 10, tdelay = 1;
    int memFD[2], userFD[2], cpuFD[2];
    SystemStats myStats;

    // signal handlers
    struct sigaction act;
	act.sa_handler = ignore;
	act.sa_flags =0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTSTP, &act, NULL);

	struct sigaction act2;
	act2.sa_handler = leave;
	act2.sa_flags =0;
	sigemptyset(&act2.sa_mask);
	sigaction(SIGINT, &act2, NULL);
	// while (1);

    // Define long options for command-line arguments
    static struct option long_options[] = {
        {"system", no_argument, 0, 's'},
        {"user", no_argument, 0, 'u'},
        {"sequential", no_argument, 0, 'q'},
        {"graphics", no_argument, 0, 'g'},
        {"samples", required_argument, NULL, 'n'},
        {"tdelay", required_argument, NULL, 't'},
        {">", required_argument, NULL, '>'},
        {NULL, 0, NULL, 0}
    };

    // Parse command-line arguments using getopt_long
    while ((option = getopt_long(argc, argv, "suqgn:t:>:", long_options, NULL)) != -1) {
        switch (option) {
            case 's':
                system_flag = 1;
                cpu_flag = 1;
                break;
            case 'u':
                user_flag = 1;
                break;
            case 'q':
                sequential_flag = 1;
                break;
            case 'g':
                graphics_flag = 1;
                break;
            case 'n':
                // num_samples = atoi(optarg);
                if (sscanf (optarg, "%i", &samples) != 1){
                    fprintf(stderr, "error - input not an integer");
                    return 1;
                }
                break;
            case 't':
                // tdelay = atoi(optarg);
                if (sscanf (optarg, "%i", &tdelay) != 1){
                    fprintf(stderr, "error - input not an integer");
                    return 1;
                }
                break;
            case '?':
            default:
                Message();
                exit(EXIT_FAILURE);
        }
    }

    // Check for additional arguments for number of samples and tdelay
    if (optind < argc) {
        // samples = atoi(argv[optind]);
        if (sscanf (argv[optind], "%i", &samples) != 1){
            fprintf(stderr, "error - input not an integer");
            return 1;
        }
        optind++;
    }

    if (optind < argc) {
        // tdelay = atoi(argv[optind]);
        if (sscanf (argv[optind], "%i", &tdelay) != 1){
            fprintf(stderr, "error - input not an integer");
            return 1;
        }
    }

    if (!user_flag && ! system_flag){
        user_flag = 1;
        system_flag = 1;
        cpu_flag = 1;
    }

    CPUStruct cpu_usage = initCPUStruct(samples);
    if (cpu_usage.cpu_usage == NULL){
        deleteCPU(samples, &cpu_usage);
        return 1;
    }
    MemStruct mem_usage = initMemStruct(samples);
    if (mem_usage.mem_usage == NULL){
        deleteMem(samples, &mem_usage);
        return 1;
    }
    if (system_flag){
        if (pipe(memFD) == -1){
            perror("PIPE MEM");
            exit(EXIT_FAILURE);
        }

        int pid = fork();
        if (pid < 0){
            perror("FORK MEM");
            exit(EXIT_FAILURE);
        }else if (pid == 0){
            close(memFD[0]);

            for (int i = 0; i < samples; i++){
                double new_mem_usage[4];

                getMemUsage(new_mem_usage);

                write(memFD[1], new_mem_usage, sizeof(double) * 4);
                sleep(tdelay);
            }
            close(memFD[1]);
            exit(EXIT_SUCCESS);
        }
    }
    if (user_flag) {
        if (pipe(userFD) == -1) {
            perror("PIPE USER");
            exit(EXIT_FAILURE);
        }

        int pid = fork();
        if (pid < 0) {
            perror("FORK USER");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(userFD[0]);

            for (int i = 0; i < samples; i++){
                char *data = getUsers(userFD[1]);

                write(userFD[1], data, strlen(data));

                free(data);
                sleep(tdelay);
            }

            close(userFD[1]);
            exit(EXIT_SUCCESS);
        }
    }
    if (cpu_flag){
        if (pipe(cpuFD) == -1){
            perror("PIPE CPU");
            exit(EXIT_FAILURE);
        }
        int pid = fork();
        if (pid < 0){
            perror("FORK MEM");
            exit(EXIT_FAILURE);
        }else if (pid == 0){
            close(cpuFD[0]);
            for (int i = 0; i < samples; i++){
                double new_cpu[2];

                getCPUUsage(new_cpu);

                write(cpuFD[1], new_cpu, sizeof(double) * 2);
                sleep(tdelay);
            }
            close(cpuFD[1]);
            exit(EXIT_SUCCESS);
        }
    }

    // close writing end of pipes in parent
    if (cpu_flag) close(cpuFD[1]);
    if (system_flag) close(memFD[1]);
    if (user_flag) close(userFD[1]);

    // preparing terminal for printing results
    printf(CLEAR_SCREEN);
    printf(CLEAR_TO_HOME);
    if (!sequential_flag){
        // not sequential, iterate samples times and set cursor to home for printing
        for (int i = 0; i < samples; i++){
            myStats = initSystemStats();
            // print running param
            printf(CLEAR_TO_HOME);
            printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
            printf("Memory Self-Utilization: %ld KB\n", myStats.self_mem_utl);

            // if system_flag, child process was launched
            // get info from pipe and print based on flag
            if (system_flag){
                double new_mem_usage[4];
                // check if everything was read from pipe ok
                // if not send an error to kernel using perror
                if ((read(memFD[0], new_mem_usage, sizeof(double)* 4)) == -1){
                    perror("Error reading from mem pipe");
                    exit(EXIT_FAILURE);
                }
                printf("---------------------------------------\n");
                printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
                printf(CLEAR_TO_HOME);
                printf("\33[%dB", i+4);
                storeMemUsage(i, new_mem_usage, &mem_usage);
                if (graphics_flag){
                    printMemUtilGraphics(i, samples, &mem_usage);
                }else{
                    printMemUtil(new_mem_usage);
                }
            }
            // if user_flag, child process was launched
            // get info from pipe and print based on flag
            int users = 0; // for print formatting
            if (user_flag){
                printf(CLEAR_TO_HOME);
                if (!system_flag){
                    printf("\33[%dB", 2);
                }else{
                    printf("\33[%dB", samples + 4);
                }
                printf("---------------------------------------\n");
                printf("### Sessions/users ###\n");
                char buffer[MAX_STR_LEN];
                ssize_t bytesRead;
                // read from pipe and makes sure the string length is properly read
                if ((bytesRead = read(userFD[0], buffer, MAX_STR_LEN)) >= 0){
                    // // parse for only the user string part with strtok
                    // char *token;
                    // const char delimiter = '#';
                    // token = strtok(buffer, &delimiter);
                    // 
                    // printf("%s", token);
                    printf("%.*s", (int)bytesRead, buffer);
                    printf("---------------------------------------\n");
                }else{
                    perror("Error reading from user pipe");
                    exit(EXIT_FAILURE);
                }
                // added a count of users in string
                // parse and get from string for formatting
                char *temp = strstr(buffer, "#");
                temp++;
                users = atoi(temp);
            }
            
            if (cpu_flag){
                double new_cpu[2];
                if ((read(cpuFD[0], new_cpu, sizeof(double)* 2)) == -1){
                    perror("Error reading from cpu pipe");
                    exit(EXIT_FAILURE);
                }
                storeCPUUsage(i, new_cpu, &cpu_usage);
                printf(CLEAR_TO_HOME);
                printf("\33[%dB", samples+users+7);
                printCPUInfo(i, samples, &cpu_usage, &myStats);
                if (graphics_flag){
                    printCPUInfoGraphics(i, samples, &cpu_usage);
                }
                printf("---------------------------------------\n");
            }
        }
        printSysInfo(&myStats);
    }else{
        for (int i = 0; i < samples; i++){
            // initiate and populate myStats
            myStats = initSystemStats();
            printf(">>> iteration %d\n", i+1);
            printf("Memory Self-Utilization: %ld KB\n", myStats.self_mem_utl);

            // handle and print mem util if requested
            if (system_flag){
                double new_mem_usage[4];
                // send error if read wrong
                if ((read(memFD[0], new_mem_usage, sizeof(double)* 4)) == -1){
                    perror("Error reading from mem pipe");
                    exit(EXIT_FAILURE);
                }
                printf("---------------------------------------\n");
                printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
                // store the information for graphic printing
                storeMemUsage(i, new_mem_usage, &mem_usage);
                if (graphics_flag){
                    // print spacing for sequential printing
                    for (int j = 0; j < i; j++){
                        printf("\n");
                    }
                    printMemUtilGraphics(i, samples, &mem_usage);
                    for (int j = i + 1; j < samples; j++){
                        printf("\n");
                    }
                }else{
                     // print spacing for sequential printing
                    for (int j = 0; j < i; j++){
                        printf("\n");
                    }
                    printMemUtil(new_mem_usage);
                    for (int j = i + 1; j < samples; j++){
                        printf("\n");
                    }
                }
            }
            // handle and print users in session if requested
            if (user_flag){
                printf("---------------------------------------\n");
                printf("### Sessions/users ###\n");
                // check bytes read to ensure properly read from the pipe
                char buffer[MAX_STR_LEN];
                ssize_t bytesRead;
                if ((bytesRead = read(userFD[0], buffer, MAX_STR_LEN)) >= 0){
                    printf("%.*s", (int)bytesRead, buffer);
                    printf("---------------------------------------\n");
                }else{
                    // send error otherwise
                    perror("Error reading from user pipe");
                    exit(EXIT_FAILURE);
                }
            }
            // handle and pring cpu util if requested
            if (cpu_flag){
                double new_cpu[2];
                // send error if read wrong
                if ((read(cpuFD[0], new_cpu, sizeof(double)* 2)) == -1){
                    perror("Error reading from cpu pipe");
                    exit(EXIT_FAILURE);
                }

                // store cpu info for graphics
                storeCPUUsage(i, new_cpu, &cpu_usage);
                printCPUInfoSEQ(i, samples, &cpu_usage, &myStats);
                if (graphics_flag){
                    printCPUInfoGraphicsSEQ(i, samples, &cpu_usage);
                }
                printf("---------------------------------------\n");
            }
        }
        // print system information last
        printSysInfo(&myStats);
    }


    // handle closing the reading of the pipes from parent
    if (cpu_flag) close(cpuFD[0]);
    if (system_flag) close(memFD[0]);
    if (user_flag) close(userFD[0]);

    // collect all the children to prevent pathological states
    if (cpu_flag) wait(NULL);
    if (system_flag) wait(NULL);
    if (user_flag) wait(NULL);

    deleteCPU(samples, &cpu_usage);
    deleteMem(samples, &mem_usage);

    return 0;
}

// Function to display a message for invalid commands
void Message() {
    printf("Valid commands: --system, --user, --graphics, --sequential, --samples N, --tdelay\n");
}

void ignore(int signum){
	printf("Ignored stop signal\n");
	return;
}

void leave(int signum){
    char input[2];
	printf("Want to quit program? (y/n) ");
    scanf("%s", input);
    if (strcmp("y", input) == 0 || strcmp("yes", input) == 0){
        exit(0);
    }else{
        return;
    }
}

// Function to print system information
void printSysInfo(SystemStats *stats){
    printf("### System Information ###  \n");
    printf("System Name: %s\n", stats->sys_info[0]);
    printf("Machine Name: %s\n", stats->sys_info[1]);
    printf("Version: %s\n", stats->sys_info[2]);
    printf("Release: %s\n", stats->sys_info[3]);
    printf("Architecture: %s\n", stats->sys_info[4]);
    printf("System running since last reboot: ");

    //print uptime
    printf("%d days, %02d:%02d:%02d (%02d:%02d:%02d)\n",
                                stats->uptime[3], stats->uptime[2], 
                                stats->uptime[1], stats->uptime[0], 
                                (stats->uptime[3])*24 + stats->uptime[2], 
                                stats->uptime[1], stats->uptime[0]);
    printf("%s", stats->header);
}

void printCPUInfoGraphics(int iter, int samples, CPUStruct *cpu_usage){
    double util;
    int util_g;

    for(int i = 0; i < iter + 1; i++){
        util = cpu_usage->cpu_usage[i][CPUUTIL];
        util_g = (int) util;
        if (util == 0){
            util_g += 2;
        }else{
            util_g += 4;
        }
        printf("    ");
        for (int i = 0; i < util_g; i++){
            printf("%s", PERCPOS);
        }
        printf(" %.2f%%\n", util);
    }
    for (int i = iter + 1; i < samples; i++){
        printf("\n");
    }
}

void printMemUtilGraphics(int iter, int samples, MemStruct *mem_usage){
    int util_g = 0;
    double cur_mem, pre_mem;
    int difference; 

    printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB    %s", 
            mem_usage->mem_usage[iter][MEMUSED], 
            mem_usage->mem_usage[iter][MEMTOT], 
            mem_usage->mem_usage[iter][MEMUSEDVIRT], 
            mem_usage->mem_usage[iter][MEMTOTVIRT],
            PERCPOS);
    if (iter == 0){
        // here it's printing the used virtual memory
        printf("%s %.2f (%.2f)\n", NEUT, (double)ZERO, mem_usage->mem_usage[iter][MEMUSEDVIRT]);
    }else{
        // getting the difference between the current sample and previous by using virt used memory
        cur_mem = mem_usage->mem_usage[iter][MEMUSEDVIRT];
        pre_mem = mem_usage->mem_usage[iter - 1][MEMUSEDVIRT];
        util_g = getdifference(cur_mem, pre_mem, &difference);

        if (difference > 0){
            for (int j = 0; j < util_g; j++){
                printf("#");
            }
            printf("*");
        }else{
            // difference = -1
            for (int j = 0; j < util_g; j++){
                printf(":");
            }
            printf("@");
        }
        printf(" %.2f (%.2f)\n", (double) (util_g / 100), mem_usage->mem_usage[iter][MEMUSEDVIRT]);
    }
}

void printCPUInfo(int iter, int samples, CPUStruct *cpu_usage, SystemStats *stats){
    printf("Number of CPU cores: %d\n", stats->cpu_cores);
    printf(" total cpu use: %.2f%%\n", cpu_usage->cpu_usage[iter][CPUUTIL]);
}

void printMemUtil(double mem_usage[4]){
    printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n", 
                mem_usage[MEMUSED], 
                mem_usage[MEMTOT], 
                mem_usage[MEMUSEDVIRT], 
                mem_usage[MEMTOTVIRT]);
}

int getdifference(double cur_mem, double pre_mem, int *difference){
    double diff = cur_mem - pre_mem;

    if (diff >= 0){
        // positive difference 
        *difference = 1;
    }else{
        // negative difference
        *difference = -1;
    }
    return (int) (diff * 100); 
}


/* for sequential printing */
void printCPUInfoGraphicsSEQ(int iter, int samples, CPUStruct *cpu_usage){
    double util;
    int util_g;

    for(int i = 0; i < iter + 1; i++){
        util = cpu_usage->cpu_usage[i][CPUUTIL];
        util_g = (int) util;
        if (util == 0){
            util_g += 2;
        }else{
            util_g += 4;
        }
        printf("    ");
        for (int i = 0; i < util_g; i++){
            printf("%s", PERCPOS);
        }
        printf(" %.2f%%\n", util);
    }
    for (int i = iter + 1; i < samples; i++){
        printf("\n");
    }
}

void printCPUInfoSEQ(int iter, int samples, CPUStruct *cpu_usage, SystemStats *stats){
    
    printf("Number of CPU cores: %d\n", stats->cpu_cores);
    printf(" total cpu use: %.2f%%\n", cpu_usage->cpu_usage[iter][CPUUTIL]);
}
