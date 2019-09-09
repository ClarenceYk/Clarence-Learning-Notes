#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

void onNewValue(int value);
int readGPIO(int fd, void (*callback)(int));

int main(int arc, char **argv)
{
    int res = 0;
    int fd;
    if (arc != 2) {
        printf("Usage:\r\n  %s <path to gpio value file>\n", argv[0]);
        res = -1;
        goto DONE;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        printf("Open file [%s] error\n", argv[1]);
        res = -1;
        goto DONE;
    }

    res = readGPIO(fd, onNewValue);

DONE:
    return res;
}

int readGPIO(int fd, void (*callback)(int))
{
    int res = 0;
    off_t offset = 0;
    size_t num = 0;
    char buf[11];
    struct pollfd gpio_poll_fd = {
        .fd = fd,
        .events = POLLPRI,
        .revents = 0
    };

    for (;;) {
        res = poll(&gpio_poll_fd, 1, -1);
        if (res == -1) {
            perror("error polling");
            break;
        }

        if ((gpio_poll_fd.revents & POLLPRI) == POLLPRI) {
            printf("POLLPRI\n");
            offset = lseek(fd, 0, SEEK_SET);
            if (offset == -1) {
                break;
            }
            memset(&buf[0], 0, sizeof(buf)*sizeof(char));
            num = read(fd, &buf[0], (sizeof(buf)-1)*sizeof(char));
            callback(atoi(buf));
        }
    }
}

void onNewValue(int value)
{
    printf("On New Value: %d\n", value);
}