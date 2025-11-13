#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include <string.h>

int main() {
    int fd;
    int choice;
    int timeout;
    int options;
    int temp;

    fd = open("/dev/watchdog", O_RDWR);
    if (fd < 0) {
        perror("Error opening /dev/watchdog");
        printf("Make sure a watchdog driver (e.g., softdog) is loaded.\n");
        return 1;
    }

    printf("\n===== WATCHDOG CONTROL MENU =====\n");
    printf("1. Start Watchdog\n");
    printf("2. Ping (Keep Alive)\n");
    printf("3. Get Timeout\n");
    printf("4. Set Timeout\n");
    printf("5. Get Status\n");
    printf("6. Stop Watchdog\n");
    printf("7. Exit\n");
    printf("=================================\n");

    while (1) {
        printf("\nEnter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Watchdog started.\n");
                break;

            case 2:
                if (ioctl(fd, WDIOC_KEEPALIVE, 0) < 0)
                    perror("Ping failed");
                else
                    printf("Watchdog pinged successfully.\n");
                break;

            case 3:
                if (ioctl(fd, WDIOC_GETTIMEOUT, &timeout) < 0)
                    perror("Get timeout failed");
                else
                    printf("Current watchdog timeout = %d seconds\n", timeout);
                break;

            case 4:
                printf("Enter new timeout (in seconds): ");
                scanf("%d", &timeout);
                if (ioctl(fd, WDIOC_SETTIMEOUT, &timeout) < 0)
                    perror("Set timeout failed");
                else
                    printf("Timeout successfully set to %d seconds\n", timeout);
                break;

            case 5:
                if (ioctl(fd, WDIOC_GETSTATUS, &options) < 0)
                    perror("Get status failed");
                else
                    printf("Watchdog status = 0x%X\n", options);
                break;

            case 6:
                printf("Stopping watchdog safely...\n");
                if (write(fd, "V", 1) < 0)
                    perror("Stop watchdog failed (may be nowayout mode)");
                else
                    printf("Watchdog stopped.\n");
                close(fd);
                return 0;

            case 7:
                printf("Exiting without disabling watchdog... system may reboot soon!\n");
                close(fd);
                return 0;

            default:
                printf("Invalid choice! Try again.\n");
                break;
        }
    }

    return 0;
}

