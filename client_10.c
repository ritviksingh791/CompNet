#include <stdio.h>  //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#define BUFLEN 1024 // Max length of buffer
#define PORT 8888  // The port on which to send data
#define IP "127.0.0.1"
#define drop_prob 0.1

void error(char *s)
{
    perror(s);
    exit(1);
}

typedef struct packet
{
    int payload_size;
    int seqnum;
    int ack;
    int last_packet;
    char data[BUFLEN];
} packet;

struct sockaddr_in make_address(int port, char* address)
{
    struct sockaddr_in si_other;
    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(port);
    si_other.sin_addr.s_addr = inet_addr(address);
}

int drop(int prob)
{
    srand(time(NULL));
    float p = (float) rand()/RAND_MAX;
    return (p<prob)? 1 : 0;
}

packet * make_packet(FILE* fp, int seq)
{
    if(feof(fp))
    {
        return NULL;
    }
    packet * pkt = (packet*) malloc(sizeof(pkt));
    int size = 0;
    fgets(pkt->data, BUFLEN, fp);
    pkt->payload_size = sizeof(packet) - BUFLEN + strlen(pkt->data);
    pkt->seqnum = seq++;
    pkt->ack = 0;
    if(feof(fp))
    pkt->last_packet = 1;
    else
    pkt->last_packet = 0;
    return pkt;
}

int main()
{
    struct sockaddr_in si_other = make_address(PORT, IP);
    int s, i, slen = sizeof(si_other);
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    FILE* fp = fopen("input.txt", "r");
    int seq = 0;
    // packet* pkt = make_packet(fp,seq);
    // while (pkt!=NULL)
    // {
    //     if (sendto(s, pkt, sizeof(packet), 0, (struct sockaddr *)&si_other,slen) == -1)
    //     {
    //         die("sendto()");
    //     }
        
    // }
        int state = 0;
    packet* pkt = make_packet(fp, seq);
    seq++;
    while (!feof(fp))
    {
        switch (state)
        {
        case 0:
            printf("Enter message %d: \n", seq); // wait for sending packet with
            // seq. no. 0
            // fgets(send_pkt.data, sizeof(send_pkt), stdin);
            // send_pkt.sq_no = 0;
            // packet* pkt = make_packet(fp, seq);
            // seq++;
            if (sendto(s, pkt, pkt->payload_size, 0, (struct sockaddr *)&si_other, slen) == -1)
            {
                die("sendto()");
            }
            state = 1;
            break;
        case 1: // waiting for ACK 0
            packet* ack = (packet* ) malloc(sizeof(packet));
            if (recvfrom(s, ack, sizeof(packet), 0, (struct sockaddr *)&si_other, &slen) == -1)
            {
                die("recvfrom()");
            }
            if (ack->seqnum%2 == 0)
            {
                printf("Received ack seq. no. %d\n", ack->seqnum);
                state = 2;
                free(pkt);
                pkt = make_packet(fp, seq);
                seq++;
                break;
            }
        case 2:
            // printf("Enter message 1: ");
            printf("Enter message %d: \n", seq);
            // wait for sending packet with seq. no. 1
            // fgets(send_pkt.data, sizeof(send_pkt), stdin);
            // send_pkt.sq_no = 1;
            // if (sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, slen) == -1)
            // {
            //     die("sendto()");
            // }
            // state = 3;
            // break;
            packet* pkt = make_packet(fp, seq);
            seq++;
            if (sendto(s, pkt, pkt->payload_size, 0, (struct sockaddr *)&si_other, slen) == -1)
            {
                die("sendto()");
            }
            state = 3;
            break;
        case 3: // waiting for ACK 1
        //     if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, &slen) == -1)
        // {
        //         die("recvfrom()");
        // }
        //     if (rcv_ack.sq_no == 1)
        //     {
        //         printf("Received ack seq. no. %d\n", rcv_ack.sq_no);
        //         state = 0;
        //         break;
        //     }
            packet* ack = (packet* ) malloc(sizeof(packet));
            if (recvfrom(s, ack, sizeof(packet), 0, (struct sockaddr *)&si_other, &slen) == -1)
            {
                die("recvfrom()");
            }
            if (ack->seqnum%2 == 0)
            {
                printf("Received ack seq. no. %d\n", ack->seqnum);
                state = 0;
                free(pkt);
                pkt = make_packet(fp, seq);
                break;
            }
        }
    }
    close(s);
    return 0;
}