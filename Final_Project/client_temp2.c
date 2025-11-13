#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> // Include for socket functions
#include <sys/types.h>  // Include for sys/types.h (often pulled in by other headers, but good to be explicit)


#define BUF_SZ 256
#define BLINK_TIMES 3
#define BLINK_DELAY_MS 500 // In milliseconds, for sleep

// Helper for millisecond sleep (from server code)
static void msleep(unsigned ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static int recv_line(int sock, char *buf, size_t buflen) {
    size_t idx = 0;
    while (idx < buflen - 1) {
        char c;
        int r = recv(sock, &c, 1, 0);
        if (r <= 0) return r;
        if (c == '\n') {
            buf[idx] = '\0';
            return (int)idx;
        }
        buf[idx++] = c;
    }
    buf[idx] = '\0';
    return (int)idx;
}

static int send_line(int sock, const char *s) {
    size_t len = strlen(s);
    if (send(sock, s, len, 0) != (ssize_t)len) return -1;
    if (send(sock, "\n", 1, 0) != 1) return -1;
    return 0;
}

// --- Client-side LED Control (Placeholder) ---
// You will need to replace this with actual code to control LEDs on your x86 clients.
// This might involve sysfs, a GPIO library, or a custom kernel module like your RPi server.
static void client_led_on() {
    printf("Client LED ON (Client Specific Action)\n");
    // TODO: Add actual code to turn on LED on this x86 client
}

static void client_led_off() {
    printf("Client LED OFF (Client Specific Action)\n");
    // TODO: Add actual code to turn off LED on this x86 client
}

static void client_led_blink(int times, int delay_ms) {
    for (int i = 0; i < times; ++i) {
        client_led_on();
        msleep(delay_ms);
        client_led_off();
        msleep(delay_ms);
    }
}
// --- End Client-side LED Control ---

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <client_id>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int client_id = atoi(argv[3]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Client %d connected to server %s:%d\n", client_id, server_ip, port);

    char buf[BUF_SZ];

    // First receive: BLINK_INIT from server, respond with ACK
    int r = recv_line(sock, buf, sizeof(buf));
    if (r <= 0) {
        printf("Server disconnected during initial handshake.\n");
        close(sock);
        return 0;
    }

    printf("Received from server: %s\n", buf);
    if (strcmp(buf, "BLINK_INIT") == 0) {
        if (send_line(sock, "ACK") < 0) {
            perror("send ACK failed");
            close(sock);
            return 1;
        }
        printf("Client %d: Sent ACK to server.\n", client_id);
    } else {
        printf("Client %d: Unexpected initial message '%s' from server.\n", client_id, buf);
        close(sock);
        return 1;
    }

    // Main command loop
    while (1) {
        r = recv_line(sock, buf, sizeof(buf));
        if (r <= 0) {
            printf("Server disconnected or error during command loop.\n");
            break;
        }

        printf("Received from server: %s\n", buf);

        if (strcmp(buf, "OK_TO_BLINK") == 0) {
            printf("Client %d: Received OK_TO_BLINK. Blinking LED...\n", client_id);
            client_led_blink(BLINK_TIMES, BLINK_DELAY_MS);
            printf("Client %d: LED blink complete. Sending DONE.\n", client_id);
            if (send_line(sock, "DONE") < 0) {
                perror("send DONE failed");
                break;
            }
        }
        // Removed the "PLAY_AUDIO" handler from client.
        // The server now handles this directly via system() or SSH.
        else {
            printf("Client %d: Unknown or unexpected command '%s' from server.\n", client_id, buf);
        }
    }

    close(sock);
    return 0;
}
