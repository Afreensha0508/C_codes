#include <stdio.h>
#include <stdlib.h>
#include<string.h>


int main() {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/meminfo");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "MemTotal") || strstr(line, "MemFree") ||
            strstr(line, "Buffers") || strstr(line, "Cached")) {
            printf("%s", line);
        }
    }

    fclose(fp);
    return 0;
}

