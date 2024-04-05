#include <sys/types.h>
#include <sys/wait.h>
#include <utmp.h>


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
/*
    ___    _   _______ ____   ______          __     ____                                                            
   /   |  / | / / ___//  _/  / ____/___  ____/ /__  / / /                                                            
  / /| | /  |/ /\__ \ / /   / /   / __ \/ __  / _ \/ / /                                                             
 / ___ |/ /|  /___/ // /   / /___/ /_/ / /_/ /  __/_/_/                                                              
/_/ _|_/_/_|_//____/___/   \____/\____/\__,_/\___(_|_)   _   _______ ____  __    ______                      ________
   / __ \/ ____/___ ___  ____ _   _____     / ____/___  / | / / ___// __ \/ /   / ____/  _________ ___  ____/ / / / /
  / /_/ / __/ / __ `__ \/ __ \ | / / _ \   / /   / __ \/  |/ /\__ \/ / / / /   / __/    / ___/ __ `__ \/ __  / / / / 
 / _, _/ /___/ / / / / / /_/ / |/ /  __/  / /___/ /_/ / /|  /___/ / /_/ / /___/ /___   / /__/ / / / / / /_/ /_/_/_/  
/_/ |_/_____/_/ /_/ /_/\____/|___/\___/   \____/\____/_/ |_//____/\____/_____/_____/   \___/_/ /_/ /_/\__,_(_|_|_)   
                                                                                                                     
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
RE  MEMERB TO USE ANSI CODESSSSS FJKDSL FJKLDS JFKD!!!!!!!
AAAAANNNNSSSSSIIIII CODESSSSS
AAANNNSSSIII CODESSS
!!!!!!!!!!!!!!!!!!!!!!
FJLS JFSLKF
RN I'M USING CONSOLE COMMANDS TO CLEAR THE SCREEEN FJDSLKF JKLF JDKALF
 rEmEMRR TO FIX ITITITIIT JFIJITJI JFLD JFKLDSJFK!!!!!

PLSJFJKDSJF REMEMEBRE JKLJFKLD FIXX USEEE ANSI CODEESSSS
*/
void printCPUInfo(int iter, int samples, CPUStruct *cpu_usage);
void printMemUtil(int iter, int samples, MemStruct *mem_usage);
void printCPUInfoGraphics(int iter, int samples, CPUStruct *cpu_usage);
void printMemUtilGraphics(int iter, int samples, MemStruct *mem_usage);
void myPipes(int iter, int samples, int mem_utilsF, int user_F, int cpu_utilsF, MemStruct *mem_usage, CPUStruct *cpu_usage);

/*helper*/
int getdifference(double cur_mem, double pre_mem, int *difference);

int main(int argc, char ** argv){
    int samples = 5, tdelay = 1;

    if (argc > 3){
        printf("usage: ./prog [num samples] [time delay]\n");
        return 1;
    }
    if (argc > 2){
        if (sscanf (argv[2], "%i", &tdelay) != 1){
            fprintf(stderr, "error - input not an integer");
            return 1;
        }
    }
    if (argc > 1){
        if (sscanf (argv[1], "%i", &samples) != 1){
            fprintf(stderr, "error - input not an integer");
            return 1;
        }
    }

    CPUStruct cpu_usage = initCPUStruct(samples);
    MemStruct mem_usage = initMemStruct(samples);

    // printf(CLEAR_SCREEN);
    // for (int i = 0; i < samples; i++){
    //     printf(CLEAR_TO_HOME);
    //     printf("Nbr samples: %d\n", samples);
    //     printf("every %d seconds\n", tdelay);
    //     printf("---------------------\n");
    //     myPipes(i, samples, 0, 1, 0, &mem_usage, &cpu_usage);
    //     sleep(tdelay);
    // }
    printf(CLEAR_SCREEN);
    for (int i = 0; i < samples; i++){
        printf(CLEAR_TO_HOME);
        printf("---------------------\n");
        myPipes(i, samples, 1, 1, 1, &mem_usage, &cpu_usage);
        sleep(tdelay);
    }
}

void printCPUInfoGraphics(int iter, int samples, CPUStruct *cpu_usage){
    double util;
    int util_g;

    printf("total cpu use: %.2f\n", cpu_usage->cpu_usage[iter][CPUUTIL]);
    for(int i = 0; i < iter + 1; i++){
        util = cpu_usage->cpu_usage[i][CPUUTIL];
        util_g = (int) util;
        if (util == 0){
            util_g += 2;
        }else{
            util_g += 4;
        }
        printf(" ");
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
            printf("difference is: %d = %f ", util_g, cur_mem - pre_mem);

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
}

void printCPUInfo(int iter, int samples, CPUStruct *cpu_usage){
    
    // for(int i = 0; i < iter + 1; i++){
    //     printf(" total cpu use: %.2f%%\n", cpu_usage->cpu_usage[i][CPUUTIL]);
    // }
    // for (int i = iter + 1; i < samples; i++){
    //     printf("\n");
    // }
    printf(" total cpu use: %.2f%%\n", cpu_usage->cpu_usage[iter][CPUUTIL]);
}

void printMemUtil(int iter, int samples, MemStruct *mem_usage){
    // want to print from 0 to iter
    // then want to print blanks from iter to samples
    // so suppose we have samples = 5, iter = 2
    // print 0 1 2
    // print blanks 3 4

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

void myPipes(int iter, int samples, int mem_utilsF, int user_F, int cpu_utilsF, MemStruct *mem_usage, CPUStruct *cpu_usage){
    int memFD[2];
    int userFD[2];
    int cpuFD[2];

    if (mem_utilsF){
        if (pipe(memFD) == -1){
            perror("PIPE MEM");
            exit(EXIT_FAILURE);
        }

        int pid = fork();
        if (pid < 0){
            perror("FORK MEM");
            exit(EXIT_FAILURE);
        }else if (pid == 0){
            // child only writes to the pipe, so close reading end
            close(memFD[0]);

            double new_mem_usage[4];

            getMemUsage(new_mem_usage);

            write(memFD[1], new_mem_usage, sizeof(double) * 4);
            close(memFD[1]); // close writing end b/c finished writing in child
            exit(EXIT_SUCCESS);
        }else{
            // we're in parent reading
            // so close the writing end of the pipe
            close(memFD[1]);

            // we're reading from the pipe and storing the 
            // information into mem_util array
            double new_mem_usage[4];
            if ((read(memFD[0], new_mem_usage, sizeof(double)* 4)) == -1){
                perror("Error reading from mem pipe");
                exit(EXIT_FAILURE);
            }
            // r u supposed to get all the data in the child, then print everything in the parent?????
            // or u just get one sample in the chlid, then send it over to the parent to handle printing??? 
            close(memFD[0]);
            wait(NULL);
            
            storeMemUsage(iter, new_mem_usage, mem_usage);
            printMemUtil(iter, samples, mem_usage);
        }
    }

    if (user_F) {
        if (pipe(userFD) == -1) {
            perror("PIPE USER");
            exit(EXIT_FAILURE);
        }

        int pid = fork();
        if (pid < 0) {
            perror("FORK USER");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(userFD[0]);  // Close reading end 

            char *data = getUsers(userFD[1]);

            // char *fin = "finished\n";
            // write(userFD[1], fin, strlen(fin));
            write(userFD[1], data, strlen(data));

            free(data);

            close(userFD[1]);  // Close writing end 
            exit(EXIT_SUCCESS);
        } else {
            close(userFD[1]);  // Close writing end
            char buffer[MAX_STR_LEN];
            ssize_t bytesRead;
            
            if ((bytesRead = read(userFD[0], buffer, MAX_STR_LEN)) > 0){
                printf("%.*s", (int)bytesRead, buffer);
            }

            if (bytesRead <= 0) {
                fprintf(stderr, "Error reading from pipe\n");
            }

            close(userFD[0]);  // Close reading end
            wait(NULL);
        }
    }
    
    if (cpu_utilsF){
        if (pipe(cpuFD) == -1){
            perror("PIPE CPU");
            exit(EXIT_FAILURE);
        }
        int pid = fork();
        if (pid < 0){
            perror("FORK MEM");
            exit(EXIT_FAILURE);
        }else if (pid == 0){
            // child only writes to the pipe, so close reading end
            close(cpuFD[0]);

            double new_cpu[2];

            getCPUUsage(new_cpu);

            write(cpuFD[1], new_cpu, sizeof(double) * 2);
            close(cpuFD[1]); // close writing end b/c finished writing in child
            exit(EXIT_SUCCESS);
        }else{
            // in parent close the writing end
            close(cpuFD[1]);

            double new_cpu[2];
            if ((read(cpuFD[0], new_cpu, sizeof(double)* 2)) == -1){
                perror("Error reading from cpu pipe");
                exit(EXIT_FAILURE);
            }
            close(cpuFD[0]);
            wait(NULL);

            storeCPUUsage(iter, new_cpu, cpu_usage);
            printCPUInfo(iter, samples, cpu_usage);
        }
    }
}

