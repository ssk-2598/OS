/*
 * CODE TRACE - Linux Process & System Call Explorer
 * Native C version for Ubuntu/Linux.
 *
 * Build: gcc -Wall -Wextra -std=c11 -o code_trace code_trace.c
 * Run:   ./code_trace
 *
 * Educational note: Process creation, signals, file system calls and /proc
 * inspection are performed by this native program. GDB and strace modules
 * display representative commands/output; run the tools separately to
 * inspect this program at the OS level.
 */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t signal_received = 0;

static void signal_handler(int signo) {
    signal_received = signo;
}

static void pause_line(void) {
    printf("\nPress Enter to return to the main menu...");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
}

static void header(const char *title) {
    printf("\n============================================================\n");
    printf("  CODE TRACE | %s\n", title);
    printf("============================================================\n");
}

static void dashboard(void) {
    header("LINUX PROCESS & SYSTEM CALL EXPLORER");
    printf("Explore how Linux processes and system calls work.\n\n");
    printf("  1. Process Creation     fork -> child -> exec -> wait\n");
    printf("  2. Signals              SIGUSR1 handler demonstration\n");
    printf("  3. System Calls         open -> write -> read -> close\n");
    printf("  4. /proc Process Monitor  inspect this process\n");
    printf("  5. GDB Debugging        debugging commands and example\n");
    printf("  6. Strace Explorer      representative syscall trace\n");
    printf("  7. Project Information\n");
}

static void process_demo(void) {
    header("PROCESS CREATION: fork -> exec -> wait");
    printf("Parent started. PID=%ld\n", (long)getpid());
    printf("Calling fork()...\n");
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        printf("Child process created. PID=%ld, PPID=%ld\n",
               (long)getpid(), (long)getppid());
        printf("Child calls exec() to run /bin/echo...\n");
        execl("/bin/echo", "echo", "Child executed /bin/echo successfully.", (char *)NULL);
        perror("execl");
        _exit(127);
    }
    int status = 0;
    printf("Parent: child PID=%ld. Calling waitpid()...\n", (long)pid);
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return;
    }
    if (WIFEXITED(status))
        printf("Child completed with exit status %d.\n", WEXITSTATUS(status));
    else
        printf("Child ended without a normal exit.\n");
    printf("Parent continues. Process lifecycle completed.\n");
}

static void signals_demo(void) {
    header("SIGNALS: SIGUSR1");
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    signal_received = 0;

    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction");
        return;
    }
    printf("Registered signal_handler for SIGUSR1 (%d).\n", SIGUSR1);
    printf("Sending SIGUSR1 to this process (PID=%ld)...\n", (long)getpid());
    if (kill(getpid(), SIGUSR1) < 0) {
        perror("kill");
        return;
    }
    if (signal_received == SIGUSR1)
        printf("Signal received: %d\nHandler ran successfully.\n", (int)signal_received);
    else
        printf("Signal was not recorded.\n");
    printf("Signal demonstration completed.\n");
}

static void syscalls_demo(void) {
    header("SYSTEM CALL LAB: open / write / read / close");
    const char *filename = "trace_output.txt";
    const char *message = "Code Trace - Linux System Call Demonstration";
    char buffer[128] = {0};

    int fd = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }
    printf("open(\"%s\", O_CREAT|O_TRUNC|O_RDWR) = %d\n", filename, fd);
    ssize_t written = write(fd, message, strlen(message));
    if (written < 0) {
        perror("write");
        close(fd);
        return;
    }
    printf("write(%d, message, %zu) = %ld\n", fd, strlen(message), (long)written);
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        close(fd);
        return;
    }
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("read");
        close(fd);
        return;
    }
    buffer[n] = '\0';
    printf("read(%d, buffer, ...) = %ld\n", fd, (long)n);
    printf("File content: %s\n", buffer);
    if (close(fd) < 0) perror("close");
    else printf("close(%d) = 0\n", fd);
    printf("Saved demonstration file: %s\n", filename);
    printf("System call demonstration completed.\n");
}

static void proc_demo(void) {
    header("/proc PROCESS MONITOR");
    char path[128], line[256];
    snprintf(path, sizeof(path), "/proc/%ld/status", (long)getpid());
    printf("$ cat %s\n", path);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen /proc status");
        return;
    }
    const char *wanted[] = {"Name:", "State:", "Pid:", "PPid:", "Threads:", "Uid:", "VmRSS:"};
    while (fgets(line, sizeof(line), fp)) {
        for (size_t i = 0; i < sizeof(wanted) / sizeof(wanted[0]); ++i) {
            if (strncmp(line, wanted[i], strlen(wanted[i])) == 0) {
                fputs(line, stdout);
                break;
            }
        }
    }
    fclose(fp);
    printf("Platform: Linux (/proc filesystem)\n");
}

static void gdb_demo(void) {
    header("GDB DEBUGGING LAB");
    printf("Example source:\n\n");
    printf("    int a = 20;\n    int b = 5;\n    int result = a / b;\n");
    printf("    printf(\"Result = %%d\\\\n\", result);\n\n");
    printf("Expected result: 4\n\n");
    printf("To debug this program with GDB:\n");
    printf("  gcc -g -O0 -Wall -Wextra -o code_trace code_trace.c\n");
    printf("  gdb ./code_trace\n");
    printf("  (gdb) break process_demo\n");
    printf("  (gdb) run\n  (gdb) next\n  (gdb) print getpid()\n");
    printf("  (gdb) continue\n  (gdb) quit\n");
    printf("\nThis module explains the workflow; launch GDB using the commands above.\n");
}

static void strace_demo(void) {
    header("STRACE EXPLORER");
    printf("Representative system calls used by this program:\n");
    printf("  execve(\"./code_trace\", ...)\n");
    printf("  openat(..., \"/proc/<pid>/status\", O_RDONLY, ...)\n");
    printf("  read(...)\n  write(...)\n  close(...)\n");
    printf("  clone(...) / wait4(...)   [process creation]\n");
    printf("\nRun a real trace from the terminal:\n");
    printf("  strace -f -e trace=process,file,read,write ./code_trace\n");
    printf("\nThe lines above are illustrative; the command shows actual calls on your system.\n");
}

static void project_info(void) {
    header("PROJECT INFORMATION");
    printf("Objective:\n");
    printf("  Demonstrate Linux process management and system calls in C.\n\n");
    printf("Modules:\n");
    printf("  process_demo()  - fork(), exec(), waitpid()\n");
    printf("  signals_demo()  - sigaction(), kill(), SIGUSR1\n");
    printf("  syscalls_demo() - open(), write(), read(), close()\n");
    printf("  proc_demo()     - reads /proc/<pid>/status\n");
    printf("  gdb_demo()      - debugging workflow\n");
    printf("  strace_demo()   - syscall tracing workflow\n\n");
    printf("Requirements: Ubuntu/Linux, GCC. GDB and strace are optional.\n");
}

int main(void) {
    char input[32];
    for (;;) {
        dashboard();
        printf("\nSelect a module (1-7, 0 to exit): ");
        if (!fgets(input, sizeof(input), stdin)) break;
        char *end = NULL;
        long choice = strtol(input, &end, 10);
        if (end == input) {
            printf("Please enter a number from 0 to 7.\n");
            continue;
        }
        switch (choice) {
            case 1: process_demo(); pause_line(); break;
            case 2: signals_demo(); pause_line(); break;
            case 3: syscalls_demo(); pause_line(); break;
            case 4: proc_demo(); pause_line(); break;
            case 5: gdb_demo(); pause_line(); break;
            case 6: strace_demo(); pause_line(); break;
            case 7: project_info(); pause_line(); break;
            case 0: printf("Exiting Code Trace. Goodbye!\n"); return 0;
            default: printf("Invalid option. Choose 0-7.\n"); break;
        }
    }
    return 0;
}
