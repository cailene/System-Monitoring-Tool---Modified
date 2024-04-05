#include <sys/types.h>
#include <sys/wait.h>
#include <utmp.h>
#include <signal.h> 


# include "main_program.h"
# include "stats_functions.h"

/*
to run: 
gcc -Wall -Werror stats_functions.c main_program.c -o stats
./stats with optional [num samples][tdelay]

*   Compile:
 *   gcc -Wall -Werror graph.c minheap.c graph_algos.c graph_tester.c -o tester
 *
 *   Run:
 *   ./tester sample_input.txt
 *
 *   SEE FILE expected_output.txt FOR EXPECTED OUTPUT
 *
 *   Don't forget:
 *   valgrind --show-leak-kinds=all --leak-check=full ./tester sample_input.txt
 *   clang-format -style=Google --dry-run myfile.c
 *   clang-tidy --config-file=if-you-want-custom.txt myfile.c
*/

void ignore(int signum);
void leave(int signum);

void printCPUInfo(int iter, int samples, CPUStruct *cpu_usage, SystemStats *stats);
void printMemUtil(int iter, int samples, MemStruct *mem_usage);
void printCPUInfoGraphics(int iter, int samples, CPUStruct *cpu_usage);
void printMemUtilGraphics(int iter, int samples, MemStruct *mem_usage);
void printSysInfo(SystemStats *stats);
void Message();

/*helper*/
int getdifference(double cur_mem, double pre_mem, int *difference);

int main(int argc, char ** argv){
    int option;
    int system_flag = 0, cpu_flag = 0, user_flag = 0, 
        sequential_flag = 0, graphics_flag = 0, samples = 10, tdelay = 1;
    int memFD[2], userFD[2], cpuFD[2];
    SystemStats myStats;

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
        return 1;
    }
    MemStruct mem_usage = initMemStruct(samples);
    if (mem_usage.mem_usage == NULL){
        return 1;
    }

    if (!sequential_flag){
        printf(CLEAR_SCREEN);
    }
    for (int i = 0; i < samples; i++){
        if (!sequential_flag){
            printf(CLEAR_TO_HOME);
            printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
            printf("Memory Self-Utilization: %ld KB\n", myStats.self_mem_utl);
            printf("---------------------------------------\n");
        }else{
            printf(">>> iteration %d\n", i+1);
            printf("Memory Self-Utilization: %ld KB\n", myStats.self_mem_utl);
            printf("---------------------------------------\n");
        }
        myStats = initSystemStats();

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

                double new_mem_usage[4];

                getMemUsage(new_mem_usage);

                write(memFD[1], new_mem_usage, sizeof(double) * 4);
                close(memFD[1]);
                exit(EXIT_SUCCESS);
            }else{
                close(memFD[1]);

                double new_mem_usage[4];
                if ((read(memFD[0], new_mem_usage, sizeof(double)* 4)) == -1){
                    perror("Error reading from mem pipe");
                    exit(EXIT_FAILURE);
                }
                
                close(memFD[0]);
                wait(NULL);
                
                storeMemUsage(i, new_mem_usage, &mem_usage);
                if (graphics_flag){
                    printMemUtilGraphics(i, samples, &mem_usage);
                }else{
                    printMemUtil(i, samples, &mem_usage);
                }
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

                char *data = getUsers(userFD[1]);

                // char *fin = "finished\n";
                // write(userFD[1], fin, strlen(fin));
                write(userFD[1], data, strlen(data));

                free(data);

                close(userFD[1]);
                exit(EXIT_SUCCESS);
            } else {
                close(userFD[1]);
                char buffer[MAX_STR_LEN];
                ssize_t bytesRead;
                
                printf("### Sessions/users ### \n");
                if ((bytesRead = read(userFD[0], buffer, MAX_STR_LEN)) > 0){
                    printf("%.*s", (int)bytesRead, buffer);
                }
                printf("---------------------------------------\n");

                if (bytesRead <= 0) {
                    fprintf(stderr, "Error reading from pipe\n");
                }

                close(userFD[0]);
                wait(NULL);
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

                double new_cpu[2];

                getCPUUsage(new_cpu);

                write(cpuFD[1], new_cpu, sizeof(double) * 2);
                close(cpuFD[1]);
                exit(EXIT_SUCCESS);
            }else{
                close(cpuFD[1]);

                double new_cpu[2];
                if ((read(cpuFD[0], new_cpu, sizeof(double)* 2)) == -1){
                    perror("Error reading from cpu pipe");
                    exit(EXIT_FAILURE);
                }
                close(cpuFD[0]);
                wait(NULL);

                storeCPUUsage(i, new_cpu, &cpu_usage);
                
                printCPUInfo(i, samples, &cpu_usage, &myStats);
                if (graphics_flag){
                    printCPUInfoGraphics(i, samples, &cpu_usage);
                }
                printf("---------------------------------------\n");
            }
        }
        
        if (!sequential_flag){
            printSysInfo(&myStats);
        }
        sleep(tdelay);
    }
    if (sequential_flag){
        printSysInfo(&myStats);
    }
    
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

    // printf("Number of CPU cores: %d\n", stats->cpu_cores);
    // printf("total cpu use: %.2f\n", cpu_usage->cpu_usage[iter][CPUUTIL]);

    // printCPUInfo(iter, samples, cpu_usage, stats);
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
    // printf("---------------------------------------\n");
}

void printMemUtilGraphics(int iter, int samples, MemStruct *mem_usage){
    int util_g = 0;
    double cur_mem, pre_mem;
    int difference; 

    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
    for (int i = 0; i < iter + 1; i++){
        printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB    %s", 
                mem_usage->mem_usage[i][MEMUSED], 
                mem_usage->mem_usage[i][MEMTOT], 
                mem_usage->mem_usage[i][MEMUSEDVIRT], 
                mem_usage->mem_usage[i][MEMTOTVIRT],
                PERCPOS);
        if (i == 0){
            // here it's printing the used virtual memory
            printf("%s %.2f (%.2f)\n", NEUT, (double)ZERO, mem_usage->mem_usage[i][MEMUSEDVIRT]);
        }else{
            // getting the difference between the current sample and previous by using virt used memory
            cur_mem = mem_usage->mem_usage[i][MEMUSEDVIRT];
            pre_mem = mem_usage->mem_usage[i - 1][MEMUSEDVIRT];
            util_g = getdifference(cur_mem, pre_mem, &difference);
            // printf("difference is: %d = %f ", util_g, cur_mem - pre_mem);

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
            printf(" %.2f (%.2f)\n", (double) (util_g / 100), mem_usage->mem_usage[i][MEMUSEDVIRT]);
        }
    }
    for (int i = iter + 1; i < samples; i++){
        printf("\n");
    }
    printf("---------------------------------------\n");
}

void printCPUInfo(int iter, int samples, CPUStruct *cpu_usage, SystemStats *stats){
    
    // for(int i = 0; i < iter + 1; i++){
    //     printf(" total cpu use: %.2f%%\n", cpu_usage->cpu_usage[i][CPUUTIL]);
    // }
    // for (int i = iter + 1; i < samples; i++){
    //     printf("\n");
    // }
    printf("Number of CPU cores: %d\n", stats->cpu_cores);
    printf(" total cpu use: %.2f%%\n", cpu_usage->cpu_usage[iter][CPUUTIL]);
}

void printMemUtil(int iter, int samples, MemStruct *mem_usage){
    // want to print from 0 to iter
    // then want to print blanks from iter to samples
    // so suppose we have samples = 5, iter = 2
    // print 0 1 2
    // print blanks 3 4
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
    for (int i = 0; i < iter + 1; i++){
        printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n", 
                mem_usage->mem_usage[i][MEMUSED], 
                mem_usage->mem_usage[i][MEMTOT], 
                mem_usage->mem_usage[i][MEMUSEDVIRT], 
                mem_usage->mem_usage[i][MEMTOTVIRT]);
    }
    for (int i = iter + 1; i < samples; i++){
        printf("\n");
    }
    printf("---------------------------------------\n");
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
