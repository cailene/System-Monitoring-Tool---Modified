
  

# CSCB09: A3 - System Monitoring Tool -- Concurrency & Signals

System Monitoring Tool is a C program that displays system information and reports metrics of user utilization and system utilization. The difference from A1, is that it now uses multiple processes to run concurrently and sigaction to handle signals from the user.

As usual, this program works in a Linux type OS.

  

## Table of Contents

- Project Name

- Table of Contents

- Problem Solving Approach

- Functions Overview

- Usage

  

## Problem Solving Approach
To make my program run in a concurrent fashion, I knew I needed to use processes with the addition of pipes. My idea was to launch **three child processes** from **one parent process**. The child process will handle computing a singular sample to which it will send over the results to the parent using **pipes**. The parent will then read the sample and store it in an array or print as required. I utilized `wait()` to ensure **no pathological states** and to maintain the correct order of output. 

## Functions Overview

## __Stats_Function_header

### SystemsStats Struct

  

The `SystemStats` struct stores the following reported system stats. It is used to help organize and manage information related to the system's performance.

  

**Members**

-  **header**: type char and stores a string of lines for design formatting

-  **uptime**: array of type int to store uptime information

-  **sys_info**: array of type char to store strings of system info

-  **cpu_cores**: a double to store the number of CPU cores

-  **self_mem_utl**: a long to store self memory utilization

  

**My Problem Solving Approach:**
- I just wanted something to easily store information while keeping it organized and easy to fetch. 

### CPUStruct
- **cpu_usage:** multidimensional array to store previously sampled cpu utilization and current sample. Useful for graphics and formatting when printing.

### MemStruct

- **mem_usage: **  multidimensional array to store previously sampled memory utilization and current sample. Similiarly to  CPUStruct, useful for graphics and formatting when printing.


## Init Functions

### initSystemStats();

Takes nothing as parameters. 

Runs `getUptime()`, `getSysInfo()`, `getCPUCores()`, `getSelfMemUtl()` to take and store a current sample of system information. 

Returns a `SystemStats` struct. 

### initMemStruct(int samples)

Takes `samples` integer in param and allocates space for all the memory utilization samples. Prints and exits if there's an error making space, otherwise returns a pointer to a `MemStruct` struct ready for populating. 

### initCPUStruct(int samples)
Takes `samples` integer in param and allocates space for all the cpu utilization samples. Prints and exits if there's an error making space, otherwise returns a pointer to a `CPUStruct` struct ready for populating. 

## Getter Functions

## getUsers(int fd)
Takes the file descriptor integer as parameter. Uses `utmp` struct and `setutent()` to read from file. It then makes a temporary buffer of a singluar user, calculates how much space to reallocate and add this user the final string containing all the current users. Returns `data`, a string containing all the current users.

*** Problem solving approach *** 
Because of the nature of the kernel, I was having issues writing and reading each current user through pipes. The kernel would switch processes and sleep processes including the process handling users in session. This affected the formatting when printing the system statistics. I tried adding a flag called "finished" where the parent would check for it, but I still ran into some issues. This will be a future project for me to solve. Instead, I'm sending over a singular string of all the users to which avoids this issue. 

## getMemUsage(double mem_usage[4])
Takes an array of 4 of type double as parameter and uses `sysinfo` struct to get memory utilization information. It stores the utilization in the `mem_usage` array for the child process to collect and write to the pipe where the parent will read the information and store it. 

It returns nothing. 

*** Problem solving approach *** 
I wanted the child to handling getting a sample and writing it through the pipe and needed a function to handle getting a singular memory utilization sample. 

## getCPUUsage(double cpu_usage[2])
Takes an array of 2 of type double as parameter and reads from `/proc/stat` to get cpu utilization information. It stores the utilization in the `cpu_usage` array for the child process to collect and write to the pipe where the parent will read the information and store it. 

It returns nothing. 

*** Problem solving approach *** 
I wanted the child to handling getting a sample and writing it through the pipe and needed a function to handle getting a singular cpu utilization sample. 

*** The follow getter functions are same as A1 ***
### getUptime Function

Takes as a param `stats`, a pointer to a `SystemStats` struct, where the uptime info will be stored. 

Reads from `/proc/uptime` file and displays and error if it goes wrong. Then converts the seconds into days, hours, minutes, seconds. (This is helpful for formatting to a readable time.) It is stored into `stats` into `uptime` array.

**My Problem Solving Approach:**
- I found uptime in one of the libraries, but I found reading from the file to be easier. 

### getSysInfo Function

Takes as a param `stats`, a pointer to a `SystemStats` struct, where the system info will be stored. 

From the `<sys/utsname.h>` library, uses the struct `utsname` to get the system info and stores this info into 'stats' into `sys_info` array. 

**My Problem Solving Approach:**
- This one is straight forward because the library is easy to understand.

### getCPUCores Function

Takes as a param `stats`, a pointer to a `SystemStats` struct, where cpu core number will be stored. 

Reads from `/proc/cpuinfo` file and displays and error is opening the file goes wrong. 

If getting the cores goes wrong, it displays an error, otherwise it stores the information into `stats` into `cpu_cores`.

**My Problem Solving Approach:**
- From reading the file, I saw that each processor has the same amount of cpu cores, so this function looks for the first finding of cpu cores, gets it's value then stores it into a variable. 

### getSelfMemUtl Function

Takes as a param `stats`, a pointer to a `SystemStats` struct, where memory self-utilization will be stored. 

Uses struct `rusage` to get the self-utilization and stores it into struct `stats` into `self_mem_utl`

## Setter Functions

## storeMemUsage(int iter, double new_mem[4], MemStruct *mem_usage)
Stores new memory utilization sample taken at `iter` iteration into the `mem_usage` of type `MemStruct`.

Return nothing. 

## storeCPUUsage(int iter, double new_cpu[2], CPUStruct *cpu_usage)
Stores new cpu utilization sample taken at `iter` iteration into the `cpu_usage` of type `CPUStruct`.

Return nothing. 

### Delete Functions 

Frees memory allocated for CPU info & Mem info: 
## void deleteCPU(int samples, CPUStruct *cpu_usage);
## void deleteMem(int samples, MemStruct *mem_usage);

### Helper Functions

## calculateCPUUtil(int iter, CPUStruct *cpu_usage)
Is called by `storeCPUUsage()` to calculate and store cpu utilization information in the `cpu_usage` at iteration `iter`. 
Follows the formula `(U2 - U1)/(T2 - T1) * 100` where `Ui = Ti - Ii` where I is idle time... 

Returns nothing. 