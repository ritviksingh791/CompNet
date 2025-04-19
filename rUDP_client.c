#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

#define TIMEOUT 3
#define BUFLEN 512
#define PORT 8882

typedef struct {
    int sq_no;
} ACK_PKT;

typedef struct {
    int sq_no;
    char data[BUFLEN];
} DATA_PKT;

void print_with_time(const char *format, ...) {
    va_list args;
    va_start(args, format);
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
    vprintf(format, args);
    va_end(args);
}

struct sockaddr_in makeAddress(int port, const char *address) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address);
    return addr;
}

int drop(float p) {
    srand(time(NULL));
    float r = (float)rand() / RAND_MAX;
    return (r < p) ? 1 : 0;
}

int main(void) {
    struct sockaddr_in si_other = makeAddress(PORT, "127.0.0.1");
    int slen = sizeof(si_other);
    DATA_PKT send_pkt, rcv_ack;
    int retval;
    fd_set rfds;
    struct timeval tv;

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    int state = 0;
    while (1) {
        switch (state) {
            case 0:
                send_pkt.sq_no = 0;
                print_with_time("Enter message 0: ");
                scanf(" %[^\n]", send_pkt.data);
                sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, sizeof(si_other));
                state = 1;
                break;

            case 1:
                FD_ZERO(&rfds);
                FD_SET(s, &rfds);
                tv.tv_sec = TIMEOUT;
                tv.tv_usec = 0;
                retval = select(s + 1, &rfds, NULL, NULL, &tv);

                if (retval == -1)
                    perror("select()");
                else if (retval) {
                    recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, &slen);
                    if (drop(0.29)) {
                        print_with_time("Info ack dropped! id: %d.\n", rcv_ack.sq_no);
                        break;
                    }
                    if (rcv_ack.sq_no == 0) {
                        print_with_time("Received ack seq. no. %d\n", rcv_ack.sq_no);
                        state = 2;
                        break;
                    } else {
                        break;
                    }
                } else {
                    print_with_time("TIMEOUT: RESENDING MESSAGE 0\n");
                    sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, sizeof(si_other));
                    break;
                }

            case 2:
                send_pkt.sq_no = 1;
                print_with_time("Enter message 1: ");
                scanf(" %[^\n]", send_pkt.data);
                sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, sizeof(si_other));
                state = 3;
                break;

            case 3:
                FD_ZERO(&rfds);
                FD_SET(s, &rfds);
                tv.tv_sec = TIMEOUT;
                tv.tv_usec = 0;
                retval = select(s + 1, &rfds, NULL, NULL, &tv);

                if (retval == -1)
                    perror("select()");
                else if (retval) {
                    recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, &slen);
                    if (drop(0.29)) {
                        print_with_time("Info ack dropped! id: %d.\n", rcv_ack.sq_no);
                        break;
                    }
                    if (rcv_ack.sq_no == 1) {
                        print_with_time("Received ack seq. no. %d\n", rcv_ack.sq_no);
                        state = 0;
                        break;
                    } else {
                        break;
                    }
                } else {
                    print_with_time("TIMEOUT: RESENDING MESSAGE 1\n");
                    sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, sizeof(si_other));
                    break;
                }
        }
    }
    close(s);
    return 0;
}
