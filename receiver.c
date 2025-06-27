#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_KEY 5678
#define MSG_SIZE 100

struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};

int main() {
    int msgid = msgget(MSG_KEY, 0666);
    if (msgid == -1) {
        perror("msgget failed");
        return 1;
    }

    struct msg_buffer message;

    // Receive message
    msgrcv(msgid, &message, sizeof(message.msg_text), 1, 0);
    printf("Receiver: Message received: %s\n", message.msg_text);

    // Remove the queue
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}

