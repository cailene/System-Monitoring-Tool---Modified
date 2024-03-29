
# include "main_program.h"
# include "stats_functions.h"

/*
to run: 
gcc -Wall -Werror stats_functions.c main_program.c -o stats
./stats with optional [num samples][tdelay]
*/
/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
RE  MEMERB TO USE ANSI CODESSSSS FJKDSL FJKLDS JFKD!!!!!!!

!!!!!!!!!!!!!!!!!!!!!!
FJLS JFSLKF
RN I'M USING CONSOLE COMMANDS TO CLEAR THE SCREEEN FJDSLKF JKLF JDKALF
 rEmEMRR TO FIX ITITITIIT JFIJITJI JFLD JFKLDSJFK!!!!!

PLSJFJKDSJF REMEMEBRE JKLJFKLD FIXX USEEE ANSI CODEESSSS
*/
void printCPUInfo(int iter, CPUStruct *cpu_usage);
void printMemUtil(int iter, int samples, MemStruct *mem_usage);

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


    for (int i = 0; i < samples; i++){
        system("clear");
        printf("Nbr samples: %d\n", samples);
        printf("every %d seconds\n", tdelay);
        printf("---------------------\n");
        getMemUsage(i, &mem_usage);
        printMemUtil(i, samples, &mem_usage);
        printf("---------------------\n");
        printf("iteration >> %d\n", i + 1);
        getCPUUsage(i, &cpu_usage);
        printCPUInfo(i, &cpu_usage);
        sleep(tdelay);
    }
}

// Calling it calculateCPUUtil + print would be more accurate
void printCPUInfo(int iter, CPUStruct *cpu_usage){
    // Ui = Ti - Ii where I is idle time... 
    // formula is (U2 - U1)/(T2 - T1) * 100 
    double usage_pre, usage_cur;
    double time_pre, time_cur;
    double cpu_util;

    usage_cur = cpu_usage->cpu_usage[iter][CPUUTIL];
    time_cur = cpu_usage->cpu_usage[iter][CPUTIME];

    if (iter == 0){
        cpu_util = (double)(usage_cur/time_cur)*100.0;
        printf(" total cpu use: %.2f%%\n", cpu_util);
        return;
    }

    usage_pre = cpu_usage->cpu_usage[iter - 1][CPUUTIL];
    time_pre = cpu_usage->cpu_usage[iter - 1][CPUTIME];
    cpu_util = (double) ((usage_cur - usage_pre)/(time_cur - time_pre)) * 100;
    
    printf(" total cpu use: %.2f%%\n", cpu_util);
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