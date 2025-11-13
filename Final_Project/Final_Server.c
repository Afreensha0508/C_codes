//Server code which is run on raspberrypi board and controls both x86 machines(clients)


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
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <sys/wait.h>
#include "oled_display.h"
#define I2C_DEVICE "/dev/i2c-1"
#define SSD1306_ADDR 0x3C

#define PORT            5555
#define BACKLOG         2
#define BUF_SZ          256
#define BLINK_TIMES     3
#define BLINK_DELAY_MS  500
#define AUDIO_FILE_PATH_ON_X86 "/home/mirafra/Desktop/Snigdha/Snigdha/Final_Project/beep.wav"
#define SSH_USER "mirafra"
extern int i2c_fd;


// Function to measure time diffrence between two timespec structures
static long diff_ms(struct timespec a, struct timespec b) 
{
    long sec  = b.tv_sec  - a.tv_sec;
    long nsec = b.tv_nsec - a.tv_nsec;
    return sec * 1000 + nsec / 1000000;
}


// Function to provide a milli- second delay 
static void msleep(unsigned ms) 
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

// Function to interact with custom kernel module to turn LED ON 
static void led_on() 
{
    int fd = open("/dev/led_gpio", O_RDWR);
    if (fd < 0) 
    {
        perror("led_on: open");
        return;
    }
    write(fd, "1", 1);
    close(fd);
}


// Function to interact with custom kernel module to turn LED OFF
static void led_off() 
{
    int fd = open("/dev/led_gpio", O_RDWR);
    if (fd < 0) 
    {
        perror("led_off: open");
        return;
    }
    write(fd, "0", 1);
    close(fd);
}


// Function which calls led_on() and led_off functions to blink LED
static void led_blink(int times, int delay_ms) 
{
    for (int i = 0; i < times; ++i) 
    {
        led_on();
        msleep(delay_ms);
        led_off();
        msleep(delay_ms);
    }
}


// Function to create a new socket for IPV4 TCP communication
static int create_server_socket(uint16_t port) 
{
    int s = socket(AF_INET, SOCK_STREAM, 0);  // Connection-based,reliable,two-way byte streams
    if (s < 0) 
    {
        perror("socket");
        return -1;
    }
    int opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) //Allows socket to be bound to recently used address specified as SO_REUSEADDR
    {
        perror("setsockopt");
        close(s);
        return -1;
    }
    // Sets up server address structure which tells OS how and where to bind 
    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) // Assigns socket to specific IP address addr and port
    {
        perror("bind");
        close(s);
        return -1;
    }

    if (listen(s, BACKLOG) < 0) //Puts the socket into listening state
    {
        perror("listen");
        close(s);
        return -1;
    }
    return s;
}


// Function to receive data from socket char by char until '\n' is encoutered
static int recv_line(int sock, char *buf, size_t buflen) 
{
    size_t idx = 0;
    while (idx < buflen - 1) 
    {
        char c;
        int r = recv(sock, &c, 1, 0);
        if (r <= 0) return r;
        if (c == '\n') 
	{
            buf[idx] = '\0';
            return (int)idx;
        }
        buf[idx++] = c;
    }
    buf[idx] = '\0';
    return (int)idx;
}


// Function to send null terminated string 
static int send_line(int sock, const char *s) 
{
    size_t len = strlen(s);
    if (send(sock, s, len, 0) != (ssize_t)len) return -1;
    if (send(sock, "\n", 1, 0) != 1) return -1;
    return 0;
}



// Function to play audio locally on the server's host x86
static void play_audio_on_server_host_x86() 
{
    char command[512];
    snprintf(command, sizeof(command), "aplay %s", AUDIO_FILE_PATH_ON_X86);
    printf("Executing local audio command on server's host x86: %s\n", command);
    int status = system(command);
    if (status == -1) 
    {
        perror("Error executing aplay locally");
    } 
    else if (status != 0) 
    { // status is the return value of system()
        // Use WIFEXITED and WEXITSTATUS for system() return value
        if (WIFEXITED(status)) 
	{
            fprintf(stderr, "aplay exited with error status %d\n", WEXITSTATUS(status));
        } 
	else if (WIFSIGNALED(status)) 
	{
            fprintf(stderr, "aplay terminated by signal %d\n", WTERMSIG(status));
        } 
	else 
	{
            fprintf(stderr, "aplay exited with unknown status %d\n", status);
        }
    } 
    else 
    {
        printf("Audio played successfully locally on server's host x86.\n");
    }
}

// Function to play audio on a remote x86 via SSH
static void play_audio_on_remote_x86(const char *remote_ip) 
{
    char command[1024]; // Increased size for SSH command
    snprintf(command, sizeof(command), "ssh -o BatchMode=yes %s@%s aplay %s",SSH_USER, remote_ip, AUDIO_FILE_PATH_ON_X86);

    printf("Executing remote audio command via SSH on %s: %s\n", remote_ip, command);
    int status = system(command); 

    if (status == -1) 
    {
        perror("Error executing SSH command");
    } 
    else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) 
    {
        fprintf(stderr, "SSH command exited with error status %d on %s\n", WEXITSTATUS(status), remote_ip);
    } 
    else if (WIFSIGNALED(status)) 
    {
        fprintf(stderr, "SSH command terminated by signal %d on %s\n", WTERMSIG(status), remote_ip);
    } 
    else 
    {
        printf("Audio command sent successfully to %s via SSH.\n", remote_ip);
    }
}


// Function to execute a shell command and print its output
void execute_and_print(const char *command) 
{
    printf("Executing command: %s\n", command);
    int ret = system(command);
    if (ret == -1) 
    {
        perror("system command failed");
    } 
    else 
    {
        printf("Command exited with status: %d\n", WEXITSTATUS(ret));
    }
}




int main(void) 
{
    printf("Dual-Client LED Blink Coordination System (Server using Kernel Module)\n");
    i2c_fd = open(I2C_DEVICE, O_RDWR);
    if (i2c_fd < 0) 
    {
        perror("Failed to open I2C device");
        exit(1);
    }

    if (ioctl(i2c_fd, I2C_SLAVE, SSD1306_ADDR) < 0) // Configuration of OLED address with I2C address for file operations on I2C driver
    {
        perror("Failed to set I2C address");
        close(i2c_fd);
        exit(1);
    }
    oled_init();
    oled_print("System Ready");


    printf("--- Network Simulation Demonstration ---\n");

    execute_and_print("sudo insmod network_driver.ko");
    sleep(1); 

    //execute_and_print("sudo ip link set dummy0 up");
    //execute_and_print("sudo ip addr add 127.0.0.1/8 dev dummy0"); 


    execute_and_print("ping 192.168.0.139 -c 3");

    printf("\n--- Kernel Log for dummy_net ---\n");
    execute_and_print("dmesg | grep dummy_net");
    printf("--- End Kernel Log ---\n");

    execute_and_print("sudo ip link set dummy0 down");
    execute_and_print("sudo rmmod network_driver");

    printf("--- Network Simulation Complete ---\n\n");

    int srv = create_server_socket(PORT);   // Creation of socket
    if (srv < 0) 
	    exit(EXIT_FAILURE);
    printf("Server listening on port %d...\n", PORT);
    oled_print("Waiting");

    int cfd[2] = {-1, -1};   // Store clients file descriptors
    char client_ips[2][INET_ADDRSTRLEN]; // Store IPs of connected clients
    struct sockaddr_in caddr;       // caddr contains information like ip and port 
    socklen_t clen = sizeof(caddr);
    int clients_connected = 0;
    time_t start_time = time(NULL);
    
    // Loop which waits for 10 sec to accept connections from clients
    while (time(NULL) - start_time < 10 && clients_connected < 2) 
    {
        fd_set readfds;       // fd_set is a data type used with select() to manage sets of file descriptors
        FD_ZERO(&readfds);
        FD_SET(srv, &readfds);   // Monitors server's listening socket srv for new incoming connections 
        struct timeval timeout = {5, 0};  // Setting timeout for select()

        int ret = select(srv + 1, &readfds, NULL, NULL, &timeout); // Allows the program to monitor multiple file descriptors until one or more fds are ready
        if (ret < 0) 
	{
            perror("select");
            exit(EXIT_FAILURE);
        } 
	else if (ret == 0) 
	{
            continue;
        }

        int fd = accept(srv, (struct sockaddr *)&caddr, &clen); // Extracts first connection request from queue and creates a new socket with new fd
        if (fd < 0) 
	{
            perror("accept");
            continue;
        }

        cfd[clients_connected] = fd;
        inet_ntop(AF_INET, &caddr.sin_addr, client_ips[clients_connected], sizeof(client_ips[clients_connected])); // Converts IPV4 address from binary to human readable string and stores in destination
        printf("Client %d connected from %s:%d\n", clients_connected + 1, client_ips[clients_connected], ntohs(caddr.sin_port));
        char msg[64];
        snprintf(msg, sizeof(msg), "Client %d connected", clients_connected + 1);
        oled_print(msg);
        clients_connected++;
    }
    

    // If no clients are connected in specified timeout, exit....
    if (clients_connected == 0) 
    {
        printf("No clients connected in 20 seconds. Exiting.\n");
        oled_print("No Clients. Exit.");
        close(srv);
        return 0;
    }
    


    //Keyboard Interrupt by pressing enter 
    printf("Press ENTER to start coordination...\n");
    getchar();

    const char *msg_blink_init = "BLINK_INIT"; // General message for clients to prepare for blink
    

    struct timespec send_ts[2], ack_ts[2];  // Arrays to store message sent timestamps and acknowledgement timestamps 
    long rtt_ms[2] = {0};
    int got_ack[2] = {0};
    
    // Loop to send the BLINK_INIT message to both clients 
    for (int i = 0; i < clients_connected; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &send_ts[i]);  // Gets the current time irrespective of changed clocks 
        if (send_line(cfd[i], msg_blink_init) < 0) perror("send BLINK_INIT"); // Send to both clients to get RTT
    }

    char buf[BUF_SZ];
    int total_acks = 0;

    // Loop to check all clients which have data but haven't sent acknowledgement
    while (total_acks < clients_connected) 
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        int maxfd = -1;
        for (int i = 0; i < clients_connected; ++i) 
	{
            if (!got_ack[i]) 
	    {
                FD_SET(cfd[i], &rfds);
                if (cfd[i] > maxfd) maxfd = cfd[i];
            }
        }
        struct timeval timeout_ack = {5, 0}; // Add a timeout for ACK reception
        int ret = select(maxfd + 1, &rfds, NULL, NULL, &timeout_ack);
        if (ret < 0) 
	{
            perror("select for ACK");
            exit(EXIT_FAILURE);
        } 
	else if (ret == 0) 
	{
            printf("Timeout waiting for client ACKs. Exiting.\n");
            exit(EXIT_FAILURE);
        }
        

	// Loop to confirm whether all clients have sent acknowledgement and have no data
        for (int i = 0; i < clients_connected; ++i) 
	{
            if (!got_ack[i] && FD_ISSET(cfd[i], &rfds)) 
	    {
                int r = recv_line(cfd[i], buf, sizeof(buf));
                if (r <= 0) 
		{
                    printf("Client %d disconnected during ACK wait.\n", i + 1);
                    exit(EXIT_FAILURE);
                }

		// Compares the data in buffer with ACK 
                if (strcmp(buf, "ACK") == 0) 
		{
                    clock_gettime(CLOCK_MONOTONIC, &ack_ts[i]);
                    rtt_ms[i] = diff_ms(send_ts[i], ack_ts[i]);  // Calculate RTT based on message sent time and acknowledgement received time 
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
    // Only one client is connected during timeout of 20 sec
    if (clients_connected == 1) 
    {
        oled_print("Only one client");
        if (send_line(cfd[0], "OK_TO_BLINK") < 0) // Sends OK_TO_BLINK to client 
		perror("send OK");
        oled_print("Blinking (Only)");
        led_blink(BLINK_TIMES, BLINK_DELAY_MS);   // Blink LED for 3 times 
        // Waits for ACK from client 
        while (1) 
	{
            int r = recv_line(cfd[0], buf, sizeof(buf));
            if (r <= 0) break;
            if (strcmp(buf, "DONE") == 0) 
	    {
                printf("Client 1 DONE\n");
                oled_print("DONE");
                break;
            }
        }
    } 
    else 
    { // clients_connected == 2
        int first_client_idx = (rtt_ms[0] <= rtt_ms[1]) ? 0 : 1; // If two clients are connected, check the RTT time
        int second_client_idx = 1 - first_client_idx;

        char line[64];
        snprintf(line, sizeof(line), "First: C%d", first_client_idx + 1);
        oled_print(line);

        // Tell the first client to blink
        if (send_line(cfd[first_client_idx], "OK_TO_BLINK") < 0) perror("send OK first");
        oled_print("Blinking (First)");
        led_blink(BLINK_TIMES, BLINK_DELAY_MS); // RPi blinks its own LED (connected via kernel module)

        // Wait for the first client to acknowledge it's done blinking
        while (1) 
	{
            int r = recv_line(cfd[first_client_idx], buf, sizeof(buf));
            if (r <= 0) break;
            if (strcmp(buf, "DONE") == 0) 
	    {
                printf("Client %d (first) DONE with blinking.\n", first_client_idx + 1);
                oled_print("First DONE");
                break;
            }
        }
        
        // --- AUDIO PLAYBACK LOGIC ---
        printf("Now handling audio for the second client (Client %d).\n", second_client_idx + 1);
        oled_print("Play Audio");
        printf("Client %d (%s) is a remote x86. Playing audio via SSH.\n",second_client_idx + 1, client_ips[second_client_idx]);
        play_audio_on_remote_x86(client_ips[second_client_idx]);

        msleep(3000); // Give some time for audio to play before triggering second blink

        // Tell the second client to blink (after audio)
        if (send_line(cfd[second_client_idx], "OK_TO_BLINK") < 0)
            perror("send OK second");
        oled_print("Blinking (Second)");
        led_blink(BLINK_TIMES, BLINK_DELAY_MS); // RPi blinks its own LED again for the second client's turn

        // Wait for the second client to acknowledge it's done blinking
        while (1) 
	{
            int r = recv_line(cfd[second_client_idx], buf, sizeof(buf));
            if (r <= 0) break;
            if (strcmp(buf, "DONE") == 0) 
	    {
                printf("Client %d (second) DONE with blinking.\n", second_client_idx + 1);
                oled_print("Second DONE");
                break;
            }
        }
    }

    oled_print("Sequence Complete");
    printf("All done. Closing.\n");

    // Loop through all clients connected and close their respective socket files 
    for (int i = 0; i < clients_connected; ++i) close(cfd[i]);  
    close(srv);  // Closes the server's listening socket 
    oled_close();   
    return 0;
}

