// client.c
// Simple TCP client for the Dual-Client LED Blink Coordination System.
// Usage: ./client <server_ip> <server_port> <client_id>
//
// client_id is just for logging clarity. The protocol is identical for both.

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SZ 256

static int recv_line(int sock, char *buf, size_t buflen) {
    size_t idx = 0;
    while (idx < buflen - 1) {
        char c;
        int r = recv(sock, &c, 1, 0);
        if (r <= 0) return r; // error or closed
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
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <client_id>\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);
    int client_id = atoi(argv[3]);

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port   = htons(port);
    if (inet_pton(AF_INET, ip, &serv.sin_addr) <= 0) {
        perror("inet_pton");
        close(s);
        return 1;
    }

    if (connect(s, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("connect");
        close(s);
        return 1;
    }

    printf("Client %d connected to %s:%d\n", client_id, ip, port);

    char buf[BUF_SZ];

    // Wait for first message from server: "BLINK" or "BLINK.wav"
    int r = recv_line(s, buf, sizeof(buf));
    if (r <= 0) {
        printf("Server closed.\n");
        close(s);
        return 0;
    }
    printf("Client %d got: %s\n", client_id, buf);

    // Send ACK immediately
    if (send_line(s, "ACK") < 0) {
        perror("send ACK");
        close(s);
        return 1;
    }

    // Wait for OK_TO_BLINK
    r = recv_line(s, buf, sizeof(buf));
    if (r <= 0) {
        printf("Server closed.\n");
        close(s);
        return 0;
    }
    if (strcmp(buf, "OK_TO_BLINK") == 0) {
        printf("Client %d: blinking (simulated)\n", client_id);
        // Simulate blink time
        sleep(2);
    }
    else if(strcmp(buf,"BLINK.wav") == 0)
    {
	    printf("Client %d: Playing blink.wav\n",client_id);
	    system("aplay blink.wav");
    }
    else
	    printf("Client %d: unexpected msg: %s\n",client_id,buf);}       // Tell server we're done
        if (send_line(s, "DONE") < 0) {
            perror("send DONE");
        }


    close(s);
    return 0;
}
