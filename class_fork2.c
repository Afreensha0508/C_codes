#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// Global variable
char my_var[] = "HelloShared";

// In exec'ed child, this will run if we pass special argv[1]
void handle_exec_child() {
    printf("👶 Exec-Child: my_var won't exist here anymore!\n");
    // my_var is gone after exec; not accessible unless passed explicitly
}

int main(int argc, char *argv[]) {
    // Case: child after exec
    if (argc == 2 && strcmp(argv[1], "exec_child") == 0) {
        handle_exec_child();
        return 0;
    }

    // Print variable info in parent before fork
    printf("🧓 Parent: my_var = \"%s\", address = %p, size = %lu\n",
           my_var, (void*)my_var, strlen(my_var));

    pid_t pid = fork();

    if (pid < 0) {
        perror("❌ Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process: my_var still exists (copy-on-write)
        printf("👶 Child (before exec): my_var = \"%s\", address = %p, size = %lu\n",
               my_var, (void*)my_var, strlen(my_var));

        // Now replace child process with exec (new image)
        char *args[] = {argv[0], "exec_child", NULL};
        execv(argv[0], args);

        // If exec fails
        perror("❌ Exec failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent waits
        wait(NULL);
        printf("🧓 Parent: Child completed.\n");
    }

    return 0;
}

