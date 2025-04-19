#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#define TIMEOUT 5
#define BUFLEN 512
#define PORT 8882

typedef struct {
    int sq_no;
} ACK_PKT;

typedef struct {
    int sq_no;
    char data[BUFLEN];
} DATA_PKT;

void die(char *s) {
    perror(s);
    exit(1);
}

void print_with_time(const char *format, ...) {
    va_list args;
    va_start(args, format);
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
    vprintf(format, args);
    va_end(args);
}

int drop(float p){
    float r = (float) rand() / RAND_MAX;
    return (r < p) ? 1 : 0;
}

int main(void) {
    srand(time(NULL));
    struct sockaddr_in si_me, si_other;
    int s, slen = sizeof(si_other), recv_len;

    DATA_PKT rcv_pkt;
    ACK_PKT ack_pkt;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }

    memset((char *)&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1) {
        die("bind");
    }

    int state = 0;
    while (1) {
        switch (state) {
            case 0: {
                if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1) {
                    die("recvfrom()");
                }
                if (drop(0.29f)) { 
                    print_with_time("Info data packet dropped! id: %d.\n", rcv_pkt.sq_no);
                    break;
                }
                if (rcv_pkt.sq_no == 0) {
                    print_with_time("Packet received with seq. no. %d and content = %s\n", rcv_pkt.sq_no, rcv_pkt.data);
                    ack_pkt.sq_no = 0;
                    if (sendto(s, &ack_pkt, recv_len, 0, (struct sockaddr *)&si_other, slen) == -1) {
                        die("sendto()");
                    }
                    state = 1;
                } else {
                    print_with_time("Packet received with seq. no. %d and content = %s\n", rcv_pkt.sq_no, rcv_pkt.data);
                    ack_pkt.sq_no = 1;
                    print_with_time("Resending ack 1\n");
                    if (sendto(s, &ack_pkt, recv_len, 0, (struct sockaddr *)&si_other, slen) == -1) {
                        die("sendto()");
                    }
                }
                break;
            }
            case 1: {
                if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1) {
                    die("recvfrom()");
                }
                if (drop(0.29f)) {
                    print_with_time("Info data packet dropped! id: %d.\n", rcv_pkt.sq_no);
                    break;
                }
                if (rcv_pkt.sq_no == 1) {
                    print_with_time("Packet received with seq. no. %d and content = %s\n", rcv_pkt.sq_no, rcv_pkt.data);
                    ack_pkt.sq_no = 1;
                    if (sendto(s, &ack_pkt, recv_len, 0, (struct sockaddr *)&si_other, slen) == -1) {
                        die("sendto()");
                    }
                    state = 0;
                } else {
                    print_with_time("Packet received with seq. no. %d and content = %s\n", rcv_pkt.sq_no, rcv_pkt.data);
                    print_with_time("Resending ack 0\n");
                    ack_pkt.sq_no = 0;
                    if (sendto(s, &ack_pkt, recv_len, 0, (struct sockaddr *)&si_other, slen) == -1) {
                        die("sendto()");
                    }
                }
                break;
            }
        }
    }
    close(s);
    return 0;
}
