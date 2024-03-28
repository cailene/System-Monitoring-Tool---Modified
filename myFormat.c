# include <string.h>
# include <stdlib.h>
# include <stdio.h>
#include<unistd.h>

#define ANSI_CLEAR_SCREEN "\033[2J"

void printMemUtil(int samples, char *mem_util[], int iter){
    for (int i = 0; i < samples - iter + 1; i++){
        printf("%s\n", mem_util[i]);
    }
    for (int i = samples - iter + 1; i < samples; i++){
        printf("\n");
    }
}
void printCPUInfo(int samples, int cpu[], int iter){
    printf("%d\n", cpu[iter - 1]);
}

int main(int argc, char **argv){
    int samples = 5, tdelay = 1;
    char * mem_util[5] = {
        "first sample", 
        "second sample", 
        "third sample", 
        "fourth sample", 
        "fifth sample"
    };

    int cpu[5] = {1, 2, 3, 4, 5};

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

    if (samples > 5){
        printf("pls give sample <= 5\n");
        return 1;
    }

    for (int i = samples; i > 0; i--){
        system("clear");
        printf("Nbr samples: %d\n", samples);
        printf("every %d seconds\n", tdelay);
        printMemUtil(samples, mem_util, i);
        printf("---------------------\n");
        printCPUInfo(samples, cpu, i);
        sleep(tdelay);
    }

    return 0;
}