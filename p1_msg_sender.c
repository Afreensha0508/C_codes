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

    // Create message queue
    msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        perror("msgget");
        exit(1);
    }

    // Take input
    printf("Enter value of a: ");
    scanf("%d", &msg.a);
    printf("Enter value of b: ");
    scanf("%d", &msg.b);

    msg.msg_type = 1; // Custom type

    // Send message
    if (msgsnd(msgid, &msg, sizeof(struct message) - sizeof(long), 0) < 0) {
        perror("msgsnd");
        exit(1);
    }

    printf("Values sent to P2 successfully.\n");
    return 0;
}

