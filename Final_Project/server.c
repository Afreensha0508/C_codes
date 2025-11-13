// server.c
// Raspberry Pi TCP Server coordinating two clients with RTT-based priority.
// Triggers on ENTER key (keyboard "interrupt").
// Blinks LED via /sys/class/gpio (no external GPIO library).
// OLED functions are stubbed. Replace oled_print() with your SSD1306 code if needed.

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
#define LED_PIN         17      // BCM numbering
#define BLINK_TIMES     3
#define BLINK_DELAY_MS  500

// --------- Utility ---------
static long diff_ms(struct timespec a, struct timespec b) {
    // returns (b - a) in milliseconds
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

// --------- GPIO via sysfs (no external libs needed) ---------
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
    // Try exporting (ignore error if already exported)
    gpio_export(pin);
    // Give udev a moment to create the node (optional small delay)
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

// --------- OLED (stub) ---------
static void oled_print(const char *line) {
    // Replace with your SSD1306 / I2C implementation.
    // For now, we just log to console.
    printf("[OLED] %s\n", line);
}

// --------- Networking helpers ---------
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
    // simplistic line receiver (ends at '\n' or buffer full); blocking
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

int main(void) {
    printf("Dual-Client LED Blink Coordination System (Server, C)\n");

    // Init LED
    if (led_init(LED_PIN) < 0) {
        fprintf(stderr, "LED init failed. Are you running as root?\n");
    }
    gpio_write(LED_PIN, 0);

    // OLED init (stub)
    oled_print("System Ready");

    // Create server
    int srv = create_server_socket(PORT);
    if (srv < 0) exit(EXIT_FAILURE);
    printf("Server listening on port %d. Waiting for 2 clients...\n", PORT);
    oled_print("Waiting for clients...");

    // Accept two clients
    int cfd[2];
    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);

    for (int i = 0; i < 2; ++i) {
        cfd[i] = accept(srv, (struct sockaddr *)&caddr, &clen);
        if (cfd[i] < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));
        printf("Client %d connected from %s:%d\n", i + 1, ip, ntohs(caddr.sin_port));
        char msg[64];
        snprintf(msg, sizeof(msg), "Client %d connected", i + 1);
        oled_print(msg);
    }

    printf("Press ENTER to start the blink coordination...\n");
    getchar(); // keyboard "interrupt"

    // Send "BLINK" to both clients and measure RTT
    const char *msg_blink = "BLINK";
    const char *msg_blinkwav = "BLINK.wav";

    struct timespec send_ts[2], ack_ts[2];
    long rtt_ms[2] = {0};

    // Send to both
    clock_gettime(CLOCK_MONOTONIC, &send_ts[0]);
    if (send_line(cfd[0], msg_blink) < 0) perror("send C1");
    clock_gettime(CLOCK_MONOTONIC, &send_ts[1]);
    if (send_line(cfd[1], msg_blinkwav) < 0) perror("send C2");

    // Wait for ACKs
    int got_ack[2] = {0, 0};
    char buf[BUF_SZ];
    while (!(got_ack[0] && got_ack[1])) {
        fd_set rfds;
        FD_ZERO(&rfds);
        int maxfd = -1;
        for (int i = 0; i < 2; ++i) {
            if (!got_ack[i]) {
                FD_SET(cfd[i], &rfds);
                if (cfd[i] > maxfd) maxfd = cfd[i];
            }
        }
        if (maxfd < 0) break; // shouldn't happen
        int ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < 2; ++i) {
            if (!got_ack[i] && FD_ISSET(cfd[i], &rfds)) {
                int r = recv_line(cfd[i], buf, sizeof(buf));
                if (r <= 0) {
                    printf("Client %d disconnected while waiting for ACK.\n", i + 1);
                    exit(EXIT_FAILURE);
                }
                if (strcmp(buf, "ACK") == 0) {
                    clock_gettime(CLOCK_MONOTONIC, &ack_ts[i]);
                    rtt_ms[i] = diff_ms(send_ts[i], ack_ts[i]);
                    got_ack[i] = 1;
                    printf("Client %d ACK, RTT = %ld ms\n", i + 1, rtt_ms[i]);

                    char line[64];
                    snprintf(line, sizeof(line), "C%d RTT: %ld ms", i + 1, rtt_ms[i]);
                    oled_print(line);
                } else {
                    printf("Client %d unexpected: %s\n", i + 1, buf);
                }
            }
        }
    }

    // Priority selection
    int first = (rtt_ms[0] <= rtt_ms[1]) ? 0 : 1;
    int second = (first == 0) ? 1 : 0;

    printf("Priority: Client %d first, Client %d second\n", first + 1, second + 1);
    {
        char line[64];
        snprintf(line, sizeof(line), "First: C%d", first + 1);
        oled_print(line);
    }

    // Send OK_TO_BLINK to first
    if (send_line(cfd[first], "OK_TO_BLINK") < 0) perror("send OK first");
    oled_print("Blinking (First)");
    led_blink(LED_PIN, BLINK_TIMES, BLINK_DELAY_MS);

    // Wait for DONE from first
    {
        int done = 0;
        while (!done) {
            int r = recv_line(cfd[first], buf, sizeof(buf));
            if (r <= 0) {
                printf("Client %d disconnected unexpectedly.\n", first + 1);
                exit(EXIT_FAILURE);
            }
            if (strcmp(buf, "DONE") == 0) {
                printf("Client %d DONE\n", first + 1);
                oled_print("First DONE");
                done = 1;
            }
        }
    }

    // 2-second delay before second
    msleep(2000);

    // Send OK_TO_BLINK to second
    if (send_line(cfd[second], "OK_TO_BLINK") < 0) perror("send OK second");
    oled_print("Blinking (Second)");
    led_blink(LED_PIN, BLINK_TIMES, BLINK_DELAY_MS);

    // Wait for DONE from second
    {
        int done = 0;
        while (!done) {
            int r = recv_line(cfd[second], buf, sizeof(buf));
            if (r <= 0) {
                printf("Client %d disconnected unexpectedly.\n", second + 1);
                exit(EXIT_FAILURE);
            }
            if (strcmp(buf, "DONE") == 0) {
                printf("Client %d DONE\n", second + 1);
                oled_print("Second DONE");
                done = 1;
            }
        }
    }

    oled_print("Sequence Complete");
    printf("All done. Closing.\n");

    // Cleanup
    for (int i = 0; i < 2; ++i) close(cfd[i]);
    close(srv);
    gpio_unexport(LED_PIN);
    return 0;
}
