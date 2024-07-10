#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int global = 1;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Nieprawidlowy format uruchomienia programu. Prawidlowy format:\n");
        printf("'`./parent_child <sciezka_katalogu>`.\n");
        return 1;
    }

    printf("Nazwa programu: %s\n", argv[0]);

    int local = 1;

    pid_t child_pid = fork();

    if (child_pid == 0) {
        printf("child process\n");

        global++;
        local++;

        printf("child pid = %d, parent pid = %d\n", (int)getpid(), (int)getppid());
        printf("child's local = %d, child's global = %d\n", local, global);

        return execl("/bin/ls", "ls", argv[1], NULL);
    } else {
        int child_return;
        wait(&child_return);

        printf("\nparent process\n");
        printf("parent pid = %d, child pid = %d\n", (int)getpid(), (int)child_pid);

        printf("Child exit code: %d\n", child_return);
        printf("Parent's local = %d, parent's global = %d\n\n", local, global);

        return child_return;
    }

    return 0;
}
