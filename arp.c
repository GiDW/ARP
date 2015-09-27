#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <net/ethernet.h>

// Socket descriptor
int sock;

// Stuct for ethernet ARP message
struct ether_arp {
  unsigned short arp_hrd;
  unsigned short arp_pro;
  unsigned char arp_hln;
  unsigned char arp_pln;
  unsigned short arp_op;
  unsigned char arp_sha[6];
  unsigned char arp_spa[4];
  unsigned char arp_tha[6];
  unsigned char arp_tpa[4];
};

void signal_handler(int signal) 
{
	// Close the socket file descriptor
	close(sock);

	printf("Closing the socket\n");

	exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
	// Buffer for the ethernet frame
	char buffer[1536];

	// Structure for the ethernet frame
	struct ether_arp *arp_frame;

	// Skipping the first 14 bytes of ethernet frame
	arp_frame = (struct ether_arp *) (buffer + 14);

	// Opening the socket
	sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));

	if (sock < 0)
	{
		perror("Socket creating failed");
		exit(EXIT_FAILURE);
	} 

	// Assigning a singal handler to close the socket when the program exits
	if (signal(SIGINT, signal_handler) == SIG_ERR) 
	{
        perror("Error occured when setting the signal handler");
        exit(EXIT_FAILURE);
    }

    // Keep reading untill there is an ARP packet received
    while (recv(sock, buffer, sizeof(buffer), 0))
    {
    	// Skip to the next frame if it isn't an ARP packet
    	if ((((buffer[12]) << 8) + buffer[13]) != ETH_P_ARP) 
    	{
    		printf("This was no ARP packet\n");
    		continue;
    	}

    	// Skip to next frame if itn't an ARP REPLY
    	if (ntohs(arp_frame->arp_op) != ARPOP_REPLY) 
    	{
    		printf("This was no ARP REPLY\n");
    		continue;
    	}

    	// This should be an ARP REPLY
    	printf("I got an arp reply from host with ip: %u.%u.%u.%u\n", 	arp_frame->arp_spa[0],
                                                                  		arp_frame->arp_spa[1],
                                                                  		arp_frame->arp_spa[2],
                                                                  		arp_frame->arp_spa[3]);
    }

	return EXIT_SUCCESS;
}