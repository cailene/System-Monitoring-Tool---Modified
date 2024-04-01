# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <unistd.h>
# include <sys/sysinfo.h>

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

/*Initializing MemStruct for usage in getting memory utilization samples */
MemStruct initMemStruct(int samples);
CPUStruct initCPUStruct (int samples);

void getMemUsage(int iter, MemStruct *mem_usage);
void getCPUUsage(int iter, CPUStruct *cpu_usage);
void calculateCPUUtil(int iter, CPUStruct *cpu_usage);

/* Frees memory allocated for CPU info
*/
void deleteCPU(int samples, CPUStruct *cpu_usage);
void deleteMem(int samples, MemStruct *mem_usage);

#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <utmp.h>

# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <unistd.h>
# include <sys/sysinfo.h>

# define MEMTOT       0
# define MEMUSED      1
# define MEMTOTVIRT   2
# define MEMUSEDVIRT  3

# define MAX_STR_LEN 1024
# define LINE_LEN 256

typedef struct userStruct{
    char user[LINE_LEN];
    char line[LINE_LEN];
    char host[LINE_LEN];
    struct userStruct *next;
}userNode;

// userNode *newUser(char user[LINE_LEN], char line[LINE_LEN], char host[LINE_LEN]){
//     userNode *new_node = (userNode *) malloc (sizeof(userNode));

//     strcpy(new_node->user, user);
//     strcpy(new_node->line, line);
//     strcpy(new_node->host, host);
//     new_node->next = NULL;

//     return new_node;
// }
userNode *newUser(){
    userNode *new_user = (userNode *) malloc (sizeof(userNode));

    strcpy(new_user->user, "");
    strcpy(new_user->line, "");
    strcpy(new_user->host, "");
    new_user->next = NULL;

    return new_user;
}

userNode *insert_at_head(userNode *user, userNode *head){
    user->next = head;
    return user;
}

int main(int argc, char **argv){
    // each call to this is a single sample
    // but we're launching 1-3 processes depending on the user 

    int mem_utilsF = 1, user_F = 1;
    int f0[2];
    int f1[2];
    int f2[2];

    if (user_F){
        if (pipe(f2) == -1){
            perror("PIPE USER");
            exit(EXIT_FAILURE);
        }

        int pid = fork();
        if (pid < 0){
            perror("FORK USER");
            exit(EXIT_FAILURE);
        }else if(pid == 0){
            close(f2[0]);

            struct utmp *ut;
            userNode *head = NULL;

            setutent();
            while ((ut = getutent()) != NULL) {
                if (ut->ut_type == USER_PROCESS) {
                    // userNode *new_user = newUser(ut->ut_user, ut->ut_line, ut->ut_host);
                    userNode *new_user  = newUser();
                    // strcpy(new_user->user, ut->ut_user);
                    // strcpy(new_user->line, ut->ut_line);
                    // strcpy(new_user->host, ut->ut_host);
                    // head = insert_at_head(new_user, head);
                    strncpy(new_user->user, ut->ut_user, LINE_LEN - 1);
                    new_user->user[LINE_LEN - 1] = '\0'; // Ensure null-termination
                    strncpy(new_user->line, ut->ut_line, LINE_LEN - 1);
                    new_user->line[LINE_LEN - 1] = '\0'; // Ensure null-termination
                    strncpy(new_user->host, ut->ut_host, LINE_LEN - 1);
                    new_user->host[LINE_LEN - 1] = '\0'; // Ensure null-termination
                }
            }
            endutent();

            userNode *tr = head;
            while(tr != NULL){
                write(f2[1], tr->user, sizeof(tr->user));
                write(f2[1], tr->line, sizeof(tr->line));
                write(f2[1], tr->host, sizeof(tr->host));
                // i might need to free stuff here
                tr = tr->next;
            }
            close(f2[1]);
            exit(EXIT_SUCCESS);
        }else{
            close(f2[1]);
            userNode *head = NULL;
            userNode *tail = NULL;

            char user[LINE_LEN];
            char line[LINE_LEN];
            char host[LINE_LEN];
            while((read(f2[0], user, LINE_LEN)) > 0 && (read(f2[0], line, LINE_LEN)) > 0 && (read(f2[0], host, LINE_LEN)) > 0){
                userNode *new_user = newUser();
                strcpy(new_user->user, user);
                strcpy(new_user->line, line);
                strcpy(new_user->host, host);

                if (head == NULL) {
                    head = new_user;
                    tail = new_user;
                } else {
                    tail->next = new_user;
                    tail = new_user;
                }
            }
            close(f2[0]);
            wait(NULL); 

            userNode *tr = head;
            while (tr != NULL){
                printf("%-12s %-16s (%s)\n", tr->user, tr->line, tr->host);
            }
        }
    }

    if (mem_utilsF){
        if (pipe(f0) == -1){
            perror("PIPE MEM");
            exit(EXIT_FAILURE);
        }

        int pid = fork();
        if (pid < 0){
            perror("FORK MEM");
            exit(EXIT_FAILURE);
        }else if (pid == 0){
            // child only writes to the pipe, so close reading end
            close(f0[0]);

            double mem_usage[4];

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

            write(f0[1], mem_usage, sizeof(double) * 4);
            printf("send mem util info to parent\n");
            close(f0[1]); // close writing end b/c finished writing in child
            exit(EXIT_SUCCESS);
        }else{
            // we're in parent reading
            // so close the writing end of the pipe
            close(f0[1]);

            // we're reading from the pipe and storing the 
            // information into mem_util array
            double mem_usage[4];
            if ((read(f0[0], mem_usage, sizeof(double)* 4)) == -1){
                perror("Error reading from mem pipe");
                exit(EXIT_FAILURE);
            }
            close(f0[0]);
            wait(NULL);

            // do we need to check if we got everything correctly? 
            // should probably do that
            // how to ?
            printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n", 
                mem_usage[MEMUSED], 
                mem_usage[MEMTOT], 
                mem_usage[MEMUSEDVIRT], 
                mem_usage[MEMTOTVIRT]);
        }
    }

    return 0;
}