// client.c
// Usage: ./client <server_ip> <port> <client_id>

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SZ 256

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
    int r = recv_line(sock, buf, sizeof(buf));
    if (r <= 0) {
        printf("Server disconnected.\n");
        return 0;
    }

    printf("Received from server: %s\n", buf);
    send_line(sock, "ACK");

    // Main command loop
    while (1) {
        r = recv_line(sock, buf, sizeof(buf));
        if (r <= 0) break;

        if (strcmp(buf, "OK_TO_BLINK") == 0) {
            printf("Client %d: Blinking LED...\n", client_id);
            // Simulate blink (e.g., 3 blinks)
            for (int i = 0; i < 3; ++i) {
                printf("Client %d: Blink %d\n", client_id, i + 1);
                sleep(1);
            }
            send_line(sock, "DONE");
        } else if (strcmp(buf, "PLAY_AUDIO") == 0) {
            printf("Client %d: Playing audio beep.wav\n", client_id);
            system("paplay /home/mirafra/Desktop/Snigdha/Snigdha/Final_Project/beep.wav");
    printf("before open...");
	    int ret;
	    if (ret != 0) {
    			fprintf(stderr, "Client %d: Audio playback failed (aplay returned %d)\n", client_id, ret);
	    }
            send_line(sock, "DONE");
        } else {
            printf("Client %d: Unknown command '%s'\n", client_id, buf);
        }
    }

    close(sock);
    return 0;
}
