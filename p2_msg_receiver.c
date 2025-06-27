#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_KEY 1234

// Message structure
struct message {
    long msg_type;
    int a;
    int b;
};

int main() {
    struct message msg;
    int msgid;

    // Get existing message queue
    msgid = msgget(MSG_KEY, 0666);
    if (msgid < 0) {
        perror("msgget");
        exit(1);
    }

    // Receive message
    if (msgrcv(msgid, &msg, sizeof(struct message) - sizeof(long), 1, 0) < 0) {
        perror("msgrcv");
        exit(1);
    }

    int sum = msg.a + msg.b;
    printf("Received a = %d, b = %d\n", msg.a, msg.b);
    printf("Sum = %d\n", sum);

    // Optionally remove message queue
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}

