#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

struct Test1 
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

struct __attribute__((packed)) Test2 
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

struct Test3 
{
	unsigned char c;
};

struct Test4 
{
	unsigned char c1;
	unsigned char c2;
};

struct __attribute__((packed)) Test5 
{
	unsigned char c1;
	unsigned char c2;
};

struct Test6 
{
	unsigned char c1;
	unsigned int i2;
};

struct __attribute__((packed)) Test7 
{
	unsigned char c1;
	unsigned int i2;
};

struct Test8 
{
	unsigned int i1;
	unsigned char c2;
};

void signal_handler(int signal) 
{
	switch(signal) 
	{
		case SIGABRT:
			printf("SIGABRT %d\n", signal);
			break;
		case SIGFPE:
			printf("SIGFPE %d\n", signal);
			break;
		case SIGILL:
			printf("SIGILL %d\n", signal);
			break;
		case SIGINT:
			printf("SIGINT %d\n", signal);
			break;
		case SIGSEGV:
			printf("SIGSEGV %d\n", signal);
			break;
		case SIGTERM:
			printf("SIGTERM %d\n", signal);
			break;
		default:
			printf("Signal not standard %d\n", signal);

	}

	exit(EXIT_SUCCESS);

}

int main() 
{
	if (signal(SIGINT, signal_handler) == SIG_ERR) 
	{
        fputs("An error occurred while setting a signal handler.\n", stderr);
        return -1;
    }

	struct Test1 test1;
	struct Test2 test2;
	struct Test3 test3;
	struct Test4 test4;
	struct Test5 test5;
	struct Test6 test6;
	struct Test7 test7;
	struct Test8 test8;

	printf("Size of struct 1: %li\n", sizeof(test1));
	printf("Size of struct 2: %li\n", sizeof(test2));
	printf("Size of struct 3: %li\n", sizeof(test3));
	printf("Size of struct 4: %li\n", sizeof(test4));
	printf("Size of struct 5: %li\n", sizeof(test5));
	printf("Size of struct 6: %li\n", sizeof(test6));
	printf("Size of struct 7: %li\n", sizeof(test7));
	printf("Size of struct 8: %li\n", sizeof(test8));

	printf("Size of an integer: %li\n", sizeof(int));
	printf("Size of a long int: %li\n", sizeof(long int));
	printf("Size of a long: %li\n", sizeof(long));
	printf("Size of a long long: %li\n", sizeof(long long));

	printf("EXIT_SUCCESS = %i\n", EXIT_SUCCESS);
	printf("EXIT_FAILURE = %i\n", EXIT_FAILURE);l,

	char* s = NULL;

	scanf("%s", s);

	return EXIT_SUCCESS;
}