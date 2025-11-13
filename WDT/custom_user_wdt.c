#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>

#define WDT_MAGIC 'W'
#define WDT_START _IO(WDT_MAGIC, 0)
#define WDT_STOP  _IO(WDT_MAGIC, 1)
#define WDT_PING  _IO(WDT_MAGIC, 2)

int fd;

void handle_signal(int sig)
{
    printf("\nStopping watchdog and exiting...\n");
    ioctl(fd, WDT_STOP, 0);
    close(fd);
    exit(0);
}

int main()
{
    fd = open("/dev/custom_wdt", O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/custom_wdt");
        return 1;
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    int choice;
    printf("Custom Watchdog Control:\n");
    printf("1. Start Watchdog\n");
    printf("2. Stop Watchdog\n");
    printf("3. Ping continuously until Ctrl+C\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    switch (choice) {
    case 1:
        ioctl(fd, WDT_START, 0);
        break;
    case 2:
        ioctl(fd, WDT_STOP, 0);
        break;
    case 3:
        ioctl(fd, WDT_START, 0);
        printf("Pinging watchdog every 5 seconds... Press Ctrl+C to stop.\n");
        while (1) {
            ioctl(fd, WDT_PING, 0);
            sleep(5);
        }
        break;
    default:
        printf("Invalid choice.\n");
        break;
    }

    close(fd);
    return 0;
}

