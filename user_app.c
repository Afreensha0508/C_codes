// user_app.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE "/dev/virtualtemp"

#define IOCTL_SET_THRESHOLD _IOW('t', 1, int)
#define IOCTL_GET_THRESHOLD _IOR('t', 2, int)
#define IOCTL_GET_ALERTS_COUNT    _IOR('t', 3, int)

int main() {
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    int choice, temp, threshold, alerts;

    while (1) {
        printf("\nVirtual Temperature Sensor Menu\n");
        printf("1. Read Current Temperature\n");
        printf("2. Set Temperature Threshold\n");
        printf("3. Get Temperature Threshold\n");
        printf("4. Get Alert Count\n");
        printf("5. Exit\n");
        printf("Choose option: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                if (read(fd, &temp, sizeof(temp)) < 0) {
                    perror("Read failed");
                } else {
                    printf("Current Temperature: %d\n", temp);
                }
                break;

            case 2:
                printf("Enter new threshold: ");
                scanf("%d", &threshold);
                if (ioctl(fd,IOCTL_SET_THRESHOLD, &threshold) < 0) {
                    perror("IOCTL SET_THRESHOLD failed");
                } else {
                    printf("Threshold set to %d\n", threshold);
                }
                break;

            case 3:
                if (ioctl(fd, IOCTL_GET_THRESHOLD, &threshold) < 0) {
                    perror("IOCTL GET_THRESHOLD failed");
                } else {
                    printf("Current Threshold: %d\n", threshold);
                }
                break;

            case 4:
                if (ioctl(fd, IOCTL_GET_ALERTS_COUNT, &alerts) < 0) {
                    perror("IOCTL GET_ALERTS failed");
                } else {
                    printf("Alert Count: %d\n", alerts);
                }
                break;

            case 5:
                close(fd);
                return 0;

            default:
                printf("Invalid choice\n");
        }
    }
}


