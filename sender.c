#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MSG_KEY 5678
#define MSG_SIZE 100

// Message structure
struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};

int main() {
    int msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget failed");
        return 1;
    }

    struct msg_buffer message;
    message.msg_type = 1;
    strcpy(message.msg_text, "Hello from sender!");

    // Send message
    msgsnd(msgid, &message, sizeof(message.msg_text), 0);
    printf("Sender: Message sent.\n");

    return 0;
}

