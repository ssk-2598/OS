```c
/*
============================================================
        CODE TRACE
        Linux Process & System Call Explorer
============================================================

Project Description:
This project demonstrates important Linux Operating System
concepts using C system calls and process management.

Topics Demonstrated:
1. Process Creation using fork() and exec()
2. Inter-Process Signals
3. Linux File System Calls
4. Process Information using /proc
5. GDB Debugging
6. System Call Tracing using strace

Compile:
    gcc -Wall -Wextra -g code_trace.c -o code_trace

Run:
    ./code_trace
============================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/* =========================================================
   UTILITY FUNCTIONS
   ========================================================= */

void clear_input_buffer()
{
    int c;

    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void pause_screen()
{
    printf("\n------------------------------------------------------------\n");
    printf("Press ENTER to return to the main menu...");
    getchar();
}

void print_header(const char *title)
{
    printf("\n\n");
    printf("============================================================\n");
    printf("  %s\n", title);
    printf("============================================================\n");
}

void print_status(const char *message)
{
    printf("[+] %s\n", message);
}

void print_info(const char *label, int value)
{
    printf("    %-25s : %d\n", label, value);
}

void show_time()
{
    time_t current_time;
    struct tm *time_info;
    char buffer[80];

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(buffer, sizeof(buffer),
             "%Y-%m-%d %H:%M:%S",
             time_info);

    printf("    Timestamp                 : %s\n", buffer);
}

/* =========================================================
   PROJECT INTRODUCTION
   ========================================================= */

void project_about()
{
    print_header("ABOUT CODE TRACE");

    printf("\n");
    printf(" Code Trace is an interactive Linux Operating System\n");
    printf(" demonstration project written in C.\n\n");

    printf(" The project provides a practical view of what happens\n");
    printf(" inside a Linux process when common system operations\n");
    printf(" are performed.\n\n");

    printf(" ----------------------------------------------------------\n");
    printf(" Demonstrations\n");
    printf(" ----------------------------------------------------------\n");

    printf("  [1] Process Creation       fork() + exec() + wait()\n");
    printf("  [2] Signal Handling        SIGUSR1 + signal()\n");
    printf("  [3] File System Calls      open() + read() + write()\n");
    printf("  [4] Process Information    Linux /proc filesystem\n");
    printf("  [5] GDB Debugging          Debugging workflow\n");
    printf("  [6] Strace                 System call tracing\n");

    printf("\n");
    printf(" The objective is to connect C programming with Linux\n");
    printf(" Operating System concepts.\n");
}

/* =========================================================
   1. PROCESS CREATION
   ========================================================= */

void process_demo()
{
    pid_t pid;
    int status;

    print_header("PROCESS CREATION & EXECUTION");

    printf("\n");
    printf("This demonstration shows:\n");
    printf("  fork()  -> creates a new process\n");
    printf("  exec()  -> replaces the child program\n");
    printf("  wait()  -> parent waits for child\n\n");

    printf("BEFORE fork()\n");
    printf("------------------------------------------------------------\n");

    print_info("Parent PID", getpid());
    print_info("Parent PPID", getppid());

    printf("\n[*] Calling fork()...\n");

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return;
    }

    /* CHILD PROCESS */
    if (pid == 0)
    {
        printf("\n");
        printf("**************** CHILD PROCESS ****************\n");

        print_status("Child process created successfully.");

        print_info("Child PID", getpid());
        print_info("Child PPID", getppid());

        printf("\nProcess Relationship:\n");
        printf("    Parent PID (%d)\n", getppid());
        printf("          |\n");
        printf("          +---- Child PID (%d)\n", getpid());

        printf("\n[*] Child is now executing another program...\n");

        sleep(1);

        execl("/bin/echo",
              "echo",
              ">>> exec() successfully replaced the child program <<<",
              NULL);

        perror("execl");

        exit(EXIT_FAILURE);
    }

    /* PARENT PROCESS */
    else
    {
        printf("\n");
        printf("**************** PARENT PROCESS ****************\n");

        print_status("fork() returned successfully.");

        print_info("Parent PID", getpid());
        print_info("Child PID", pid);

        printf("\n[*] Parent is waiting for child using waitpid()...\n");

        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return;
        }

        printf("\n");

        if (WIFEXITED(status))
        {
            printf("[+] Child exited normally.\n");
            printf("    Exit status : %d\n",
                   WEXITSTATUS(status));
        }

        printf("\n");
        print_status("Parent continues execution.");
    }

    pause_screen();
}

/* =========================================================
   2. SIGNAL DEMONSTRATION
   ========================================================= */

volatile sig_atomic_t signal_received = 0;

void signal_handler(int sig)
{
    signal_received = sig;
}

void signal_demo()
{
    print_header("SIGNAL HANDLING");

    printf("\n");
    printf("Signals are software interrupts used by Linux to\n");
    printf("notify a process about an event.\n\n");

    printf("Signal used: SIGUSR1\n");
    printf("Signal number: %d\n", SIGUSR1);
    printf("Current process PID: %d\n\n", getpid());

    printf("[1] Registering signal handler...\n");

    if (signal(SIGUSR1, signal_handler) == SIG_ERR)
    {
        perror("signal");
        return;
    }

    print_status("Signal handler registered.");

    printf("\n[2] Sending SIGUSR1 to current process...\n");

    if (kill(getpid(), SIGUSR1) == -1)
    {
        perror("kill");
        return;
    }

    sleep(1);

    printf("\n[3] Checking signal status...\n");

    if (signal_received == SIGUSR1)
    {
        print_status("SIGUSR1 received successfully!");

        printf("\nSignal Flow:\n");
        printf("    Process\n");
        printf("       |\n");
        printf("       | kill(SIGUSR1)\n");
        printf("       v\n");
        printf("    Kernel\n");
        printf("       |\n");
        printf("       | SIGUSR1\n");
        printf("       v\n");
        printf("    Signal Handler\n");
    }

    printf("\n");
    print_status("Signal demonstration completed.");

    pause_screen();
}

/* =========================================================
   3. FILE SYSTEM CALLS
   ========================================================= */

void file_system_calls()
{
    int fd;
    int bytes_written;
    int bytes_read;

    char buffer[256];

    const char *filename = "trace_output.txt";

    const char *message =
        "CODE TRACE\n"
        "Linux System Call Demonstration\n"
        "This file was created using the open() system call.\n";

    print_header("LINUX FILE SYSTEM CALLS");

    printf("\n");
    printf("This demonstration performs the following operations:\n\n");

    printf("    open()\n");
    printf("      |\n");
    printf("    write()\n");
    printf("      |\n");
    printf("    close()\n");
    printf("      |\n");
    printf("    open()\n");
    printf("      |\n");
    printf("    read()\n");
    printf("      |\n");
    printf("    close()\n");

    printf("\nTarget file: %s\n", filename);

    /* OPEN FOR WRITING */
    printf("\n[1] open() - Creating file...\n");

    fd = open(filename,
              O_CREAT | O_WRONLY | O_TRUNC,
              0644);

    if (fd < 0)
    {
        perror("open");
        return;
    }

    printf("    File descriptor : %d\n", fd);

    /* WRITE */
    printf("\n[2] write() - Writing data...\n");

    bytes_written =
        write(fd, message, strlen(message));

    if (bytes_written < 0)
    {
        perror("write");
        close(fd);
        return;
    }

    printf("    Bytes written   : %d\n", bytes_written);

    /* CLOSE */
    printf("\n[3] close() - Closing file...\n");

    close(fd);

    print_status("File closed successfully.");

    /* OPEN FOR READING */
    printf("\n[4] open() - Opening file for reading...\n");

    fd = open(filename, O_RDONLY);

    if (fd < 0)
    {
        perror("open");
        return;
    }

    printf("    File descriptor : %d\n", fd);

    /* READ */
    printf("\n[5] read() - Reading file contents...\n");

    bytes_read =
        read(fd, buffer, sizeof(buffer) - 1);

    if (bytes_read < 0)
    {
        perror("read");
        close(fd);
        return;
    }

    buffer[bytes_read] = '\0';

    printf("\n    -------- FILE CONTENT --------\n");
    printf("%s", buffer);
    printf("    -------------------------------\n");

    printf("\n    Bytes read      : %d\n", bytes_read);

    /* CLOSE */
    printf("\n[6] close() - Closing file...\n");

    close(fd);

    print_status("File system call demonstration completed.");

    pause_screen();
}

/* =========================================================
   4. /PROC PROCESS INFORMATION
   ========================================================= */

void show_process_info()
{
    char path[128];
    char line[256];

    FILE *file;

    print_header("LINUX /proc PROCESS INFORMATION");

    printf("\n");
    printf("/proc is a virtual filesystem provided by Linux.\n");
    printf("It exposes information about running processes and\n");
    printf("the Linux kernel.\n\n");

    print_info("Current PID", getpid());
    print_info("Parent PID", getppid());

    snprintf(path,
             sizeof(path),
             "/proc/%d/status",
             getpid());

    printf("\nReading:\n");
    printf("    %s\n", path);

    printf("\nSelected process information:\n");
    printf("------------------------------------------------------------\n");

    file = fopen(path, "r");

    if (file == NULL)
    {
        perror("fopen");
        return;
    }

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "Name:", 5) == 0 ||
            strncmp(line, "State:", 6) == 0 ||
            strncmp(line, "Pid:", 4) == 0 ||
            strncmp(line, "PPid:", 5) == 0 ||
            strncmp(line, "Uid:", 4) == 0 ||
            strncmp(line, "Gid:", 4) == 0 ||
            strncmp(line, "VmSize:", 7) == 0 ||
            strncmp(line, "VmRSS:", 6) == 0 ||
            strncmp(line, "Threads:", 8) == 0)
        {
            printf("%s", line);
        }
    }

    fclose(file);

    printf("------------------------------------------------------------\n");

    printf("\nUseful /proc locations:\n\n");

    printf("    /proc/[PID]/status\n");
    printf("    /proc/[PID]/cmdline\n");
    printf("    /proc/[PID]/fd/\n");
    printf("    /proc/[PID]/maps\n");
    printf("    /proc/cpuinfo\n");
    printf("    /proc/meminfo\n");

    printf("\n");
    print_status("Process information retrieved from /proc.");

    pause_screen();
}

/* =========================================================
   5. GDB DEBUGGING
   ========================================================= */

void debugging_demo()
{
    int a = 20;
    int b = 5;
    int result;

    print_header("GDB DEBUGGING DEMONSTRATION");

    printf("\n");
    printf("GDB (GNU Debugger) allows programmers to inspect\n");
    printf("program execution and variables.\n\n");

    printf("Current program variables:\n");
    printf("------------------------------------------------------------\n");

    printf("    a      = %d\n", a);
    printf("    b      = %d\n", b);

    result = a / b;

    printf("    result = %d\n", result);

    printf("\nGDB Workflow\n");
    printf("------------------------------------------------------------\n");

    printf("1. Compile with debugging information:\n");
    printf("   gcc -g -Wall -Wextra code_trace.c -o code_trace\n\n");

    printf("2. Start GDB:\n");
    printf("   gdb ./code_trace\n\n");

    printf("3. Set breakpoint:\n");
    printf("   break debugging_demo\n\n");

    printf("4. Start program:\n");
    printf("   run\n\n");

    printf("5. Execute next line:\n");
    printf("   next\n\n");

    printf("6. Display variables:\n");
    printf("   print a\n");
    printf("   print b\n");
    printf("   print result\n\n");

    printf("7. Display call stack:\n");
    printf("   backtrace\n");

    printf("\n");
    print_status("GDB demonstration information displayed.");

    pause_screen();
}

/* =========================================================
   6. STRACE
   ========================================================= */

void strace_instructions()
{
    print_header("STRACE SYSTEM CALL TRACING");

    printf("\n");
    printf("strace is a Linux diagnostic tool that displays\n");
    printf("system calls made by a program.\n");

    printf("\nBasic workflow:\n");
    printf("------------------------------------------------------------\n");

    printf("\n[1] Compile:\n");
    printf("    gcc -g -Wall -Wextra code_trace.c -o code_trace\n");

    printf("\n[2] Run normally:\n");
    printf("    ./code_trace\n");

    printf("\n[3] Trace all system calls:\n");
    printf("    strace ./code_trace\n");

    printf("\n[4] Trace file operations:\n");
    printf("    strace -e trace=file ./code_trace\n");

    printf("\n[5] Trace process operations:\n");
    printf("    strace -e trace=process ./code_trace\n");

    printf("\n[6] Trace signals:\n");
    printf("    strace -e trace=signal ./code_trace\n");

    printf("\n[7] Save trace to a file:\n");
    printf("    strace -o trace.txt ./code_trace\n");

    printf("\nWhat you may see in strace:\n");
    printf("------------------------------------------------------------\n");

    printf("    execve()\n");
    printf("    openat()\n");
    printf("    read()\n");
    printf("    write()\n");
    printf("    close()\n");
    printf("    fork()/clone()\n");
    printf("    wait4()\n");
    printf("    kill()\n");

    printf("\n");
    print_status("Strace instructions displayed.");

    pause_screen();
}

/* =========================================================
   SYSTEM STATUS
   ========================================================= */

void system_status()
{
    print_header("SYSTEM STATUS");

    printf("\n");
    printf("Current Process Information\n");
    printf("------------------------------------------------------------\n");

    printf("Process Name       : Code Trace\n");
    printf("Current PID        : %d\n", getpid());
    printf("Parent PID         : %d\n", getppid());
    printf("User ID            : %d\n", getuid());
    printf("Group ID           : %d\n", getgid());

    show_time();

    printf("\n");
    printf("Platform:\n");
    printf("------------------------------------------------------------\n");

    printf("Operating System   : Linux\n");
    printf("Architecture       : %ld-bit process environment\n",
           sizeof(void *) * 8);

    printf("\n");
    print_status("System status displayed.");

    pause_screen();
}

/* =========================================================
   MAIN MENU
   ========================================================= */

void menu()
{
    printf("\n\n");

    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║                    CODE TRACE                            ║\n");
    printf("║       Linux Process & System Call Explorer              ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║                                                          ║\n");
    printf("║   1. Process Creation       fork + exec + wait          ║\n");
    printf("║   2. Signal Demonstration   SIGUSR1                     ║\n");
    printf("║   3. System Calls           File Operations             ║\n");
    printf("║   4. Process Information    /proc                       ║\n");
    printf("║   5. GDB Debugging          Debug Workflow              ║\n");
    printf("║   6. Strace                 System Call Tracing         ║\n");
    printf("║   7. System Status          Process Details             ║\n");
    printf("║   8. About Project          Project Information         ║\n");
    printf("║   9. Exit                   Close Program               ║\n");
    printf("║                                                          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    printf("\nEnter your choice: ");
}

/* =========================================================
   MAIN
   ========================================================= */

int main()
{
    int choice;

    printf("\n");
    printf("============================================================\n");
    printf("                 CODE TRACE v1.0\n");
    printf("        Linux Process & System Call Explorer\n");
    printf("============================================================\n");

    printf("\nInitializing project...\n");

    sleep(1);

    print_status("Linux environment detected.");
    print_status("Process initialized.");
    print_status("Code Trace is ready.");

    while (1)
    {
        menu();

        if (scanf("%d", &choice) != 1)
        {
            printf("\n[!] Invalid input. Please enter a number.\n");

            clear_input_buffer();

            continue;
        }

        clear_input_buffer();

        switch (choice)
        {
            case 1:
                process_demo();
                break;

            case 2:
                signal_demo();
                break;

            case 3:
                file_system_calls();
                break;

            case 4:
                show_process_info();
                break;

            case 5:
                debugging_demo();
                break;

            case 6:
                strace_instructions();
                break;

            case 7:
                system_status();
                break;

            case 8:
                project_about();
                pause_screen();
                break;

            case 9:
                printf("\n");
                printf("============================================================\n");
                printf("        Thank you for using CODE TRACE!\n");
                printf("        Linux Process & System Call Explorer\n");
                printf("============================================================\n");
                printf("\nExiting program...\n\n");

                return 0;

            default:
                printf("\n[!] Invalid choice. Please select 1-9.\n");
        }
    }

    return 0;
}
```