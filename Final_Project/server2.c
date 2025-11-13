#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PORT            5555
#define BACKLOG         2
#define BUF_SZ          256
#define LED_PIN         17
#define BLINK_TIMES     3
#define BLINK_DELAY_MS  500

static long diff_ms(struct timespec a, struct timespec b) {
    long sec  = b.tv_sec  - a.tv_sec;
    long nsec = b.tv_nsec - a.tv_nsec;
    return sec * 1000 + nsec / 1000000;
}

static void msleep(unsigned ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static int write_file(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    if (write(fd, val, strlen(val)) < 0) {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static int gpio_export(int pin) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", pin);
    return write_file("/sys/class/gpio/export", buf);
}

static int gpio_unexport(int pin) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", pin);
    return write_file("/sys/class/gpio/unexport", buf);
}

static int gpio_set_dir(int pin, int is_out) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    return write_file(path, is_out ? "out" : "in");
}

static int gpio_write(int pin, int val) {
    char path[64];
    char v[2];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    snprintf(v, sizeof(v), "%d", val);
    return write_file(path, v);
}

static int led_init(int pin) {
    gpio_export(pin);
    msleep(50);
    if (gpio_set_dir(pin, 1) < 0) {
        perror("gpio_set_dir");
        return -1;
    }
    return 0;
}

static void led_blink(int pin, int times, int delay_ms) {
    for (int i = 0; i < times; ++i) {
        gpio_write(pin, 1);
        msleep(delay_ms);
        gpio_write(pin, 0);
        msleep(delay_ms);
    }
}

static void oled_print(const char *line) {
    printf("[OLED] %s\n", line);
}

static int create_server_socket(uint16_t port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return -1;
    }
    int opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(s);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s);
        return -1;
    }

    if (listen(s, BACKLOG) < 0) {
        perror("listen");
        close(s);
        return -1;
    }
    return s;
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

int main(void) {
    printf("Dual-Client LED Blink Coordination System (Server)\n");

    if (led_init(LED_PIN) < 0) {
        fprintf(stderr, "LED init failed. Are you root?\n");
    }
    gpio_write(LED_PIN, 0);

    oled_print("System Ready");

    int srv = create_server_socket(PORT);
    if (srv < 0) exit(EXIT_FAILURE);
    printf("Server listening on port %d...\n", PORT);
    oled_print("Waiting for clients...");

    int cfd[2] = {-1, -1};
    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);
    int clients_connected = 0;

    time_t start_time = time(NULL);

    while (time(NULL) - start_time < 10 && clients_connected < 2) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(srv, &readfds);
        struct timeval timeout = {1, 0};

        int ret = select(srv + 1, &readfds, NULL, NULL, &timeout);
        if (ret < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        } else if (ret == 0) {
            continue;
        }

        int fd = accept(srv, (struct sockaddr *)&caddr, &clen);
        if (fd < 0) {
            perror("accept");
            continue;
        }

        cfd[clients_connected] = fd;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));
        printf("Client %d connected from %s:%d\n", clients_connected + 1, ip, ntohs(caddr.sin_port));
        char msg[64];
        snprintf(msg, sizeof(msg), "Client %d connected", clients_connected + 1);
        oled_print(msg);
        clients_connected++;
    }

    if (clients_connected == 0) {
        printf("No clients connected in 10 seconds. Exiting.\n");
        oled_print("No Clients. Exit.");
        close(srv);
        gpio_unexport(LED_PIN);
        return 0;
    }

    printf("Press ENTER to start coordination...\n");
    getchar();

    const char *msg_blink[2] = {"BLINK", "BLINK.wav"};
    struct timespec send_ts[2], ack_ts[2];
    long rtt_ms[2] = {0};
    int got_ack[2] = {0};

    for (int i = 0; i < clients_connected; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &send_ts[i]);
        if (send_line(cfd[i], msg_blink[i]) < 0) perror("send");
    }

    char buf[BUF_SZ];
    int total_acks = 0;
    while (total_acks < clients_connected) {
        fd_set rfds;
        FD_ZERO(&rfds);
        int maxfd = -1;
        for (int i = 0; i < clients_connected; ++i) {
            if (!got_ack[i]) {
                FD_SET(cfd[i], &rfds);
                if (cfd[i] > maxfd) maxfd = cfd[i];
            }
        }
        int ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < clients_connected; ++i) {
            if (!got_ack[i] && FD_ISSET(cfd[i], &rfds)) {
                int r = recv_line(cfd[i], buf, sizeof(buf));
                if (r <= 0) {
                    printf("Client %d disconnected.\n", i + 1);
                    exit(EXIT_FAILURE);
                }
                if (strcmp(buf, "ACK") == 0) {
                    clock_gettime(CLOCK_MONOTONIC, &ack_ts[i]);
                    rtt_ms[i] = diff_ms(send_ts[i], ack_ts[i]);
                    got_ack[i] = 1;
                    total_acks++;
                    printf("Client %d ACK, RTT = %ld ms\n", i + 1, rtt_ms[i]);
                    char line[64];
                    snprintf(line, sizeof(line), "C%d RTT: %ld ms", i + 1, rtt_ms[i]);
                    oled_print(line);
                }
            }
        }
    }

    if (clients_connected == 1) {
        oled_print("Only one client");
        if (send_line(cfd[0], "OK_TO_BLINK") < 0) perror("send OK");
        oled_print("Blinking (Only)");
        led_blink(LED_PIN, BLINK_TIMES, BLINK_DELAY_MS);

        while (1) {
            int r = recv_line(cfd[0], buf, sizeof(buf));
            if (r <= 0) break;
            if (strcmp(buf, "DONE") == 0) {
                printf("Client 1 DONE\n");
                oled_print("DONE");
                break;
            }
        }
    } else {
        int first = (rtt_ms[0] <= rtt_ms[1]) ? 0 : 1;
        int second = 1 - first;

        char line[64];
        snprintf(line, sizeof(line), "First: C%d", first + 1);
        oled_print(line);

        if (send_line(cfd[first], "OK_TO_BLINK") < 0) perror("send OK first");
        oled_print("Blinking (First)");
        led_blink(LED_PIN, BLINK_TIMES, BLINK_DELAY_MS);

        while (1) {
            int r = recv_line(cfd[first], buf, sizeof(buf));
            if (r <= 0) break;
            if (strcmp(buf, "DONE") == 0) {
                printf("Client %d DONE\n", first + 1);
                oled_print("First DONE");
                break;
            }
        }

        msleep(2000);

        if (send_line(cfd[second], "OK_TO_BLINK") < 0) perror("send OK second");
        oled_print("Blinking (Second)");
        led_blink(LED_PIN, BLINK_TIMES, BLINK_DELAY_MS);

        while (1) {
            int r = recv_line(cfd[second], buf, sizeof(buf));
            if (r <= 0) break;
            if (strcmp(buf, "DONE") == 0) {
                printf("Client %d DONE\n", second + 1);
                oled_print("Second DONE");
                break;
            }
        }
    }

    oled_print("Sequence Complete");
    printf("All done. Closing.\n");

    for (int i = 0; i < clients_connected; ++i) close(cfd[i]);
    close(srv);
    gpio_unexport(LED_PIN);
    return 0;
}

