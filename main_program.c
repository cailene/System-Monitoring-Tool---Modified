
# include "main_program.h"
# include "stats_functions.h"

/*
to run: 
gcc -Wall -Werror stats_functions.c main_program.c -o stats
./stats with optional [num samples][tdelay]
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

    printf(CLEAR_SCREEN);
    for (int i = 0; i < samples; i++){
        /* OVBER HERREJEKFLJAKLJFKLDJKHERReEEEREEERere */
        // system("clear");
        printf(CLEAR_TO_HOME);
        /* FIZXX TJISSSSSSSS*/
        printf("Nbr samples: %d\n", samples);
        printf("every %d seconds\n", tdelay);
        printf("---------------------\n");
        getMemUsage(i, &mem_usage);
        printMemUtilGraphics(i, samples, &mem_usage);
        printf("---------------------\n");
        printf("iteration >> %d\n", i + 1);
        getCPUUsage(i, &cpu_usage);
        printCPUInfoGraphics(i, samples, &cpu_usage);
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
    
    for(int i = 0; i < iter + 1; i++){
        printf(" total cpu use: %.2f%%\n", cpu_usage->cpu_usage[i][CPUUTIL]);
    }
    for (int i = iter + 1; i < samples; i++){
        printf("\n");
    }
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