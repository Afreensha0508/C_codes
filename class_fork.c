#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Function to be called via exec
void run_other_function() {
    printf("✅ Inside 'run_other_function' called via exec!\n");
    // You can place any logic here
}

int main(int argc, char *argv[]) {
    // If argument passed is "run_function", call that function
    if (argc == 2 && strcmp(argv[1], "run_function") == 0) {
        run_other_function();
        return 0; // Exit after function completes
    }

    // Parent process logic starts here
    printf("🚀 Parent: Starting...\n");

    pid_t pid = fork(); // Create a child process

    if (pid < 0) {
        // Error occurred
        perror("❌ Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        printf("👶 Child: Created, now calling exec...\n");

        // Prepare arguments to call the same binary with "run_function"
        char *args[] = {argv[0], "run_function", NULL};

        // Replace child process with new instance running the same program
        if (execv(argv[0], args) == -1) {
            perror("❌ Exec failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        printf("🧓 Parent: Waiting for child to complete...\n");
        wait(NULL); // Wait for child to finish
        printf("🧓 Parent: Child has finished execution.\n");
    }

    return 0;
}

