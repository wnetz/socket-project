#include "defns.h"
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <iostream>
#include <vector>
#include <string>

#define ITERATIONS	5
using namespace std;
void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
	int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    struct sockaddr_in fromAddr;     /* Source address of echo */
    unsigned short echoServPort;     /* Echo server port */
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    char *servIP;                    /* IP address of server */
    int nBytes;              		 /* Length of received response */
    vector<user> sendingList;
    struct command c1;

    if (argc < 3)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP address> <Echo Port>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];           /* First arg: server IP address (dotted quad) */
    echoServPort = atoi(argv[2]);  /* Second arg: Use given port, if any */

	printf( "Arguments passed: server IP %s, port %d\n", servIP, echoServPort );

    /* Create a datagram/UDP socket */

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */

    memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    echoServAddr.sin_port   = htons(echoServPort);     /* Server port */

	
    
    //initialize with server-----------------------------------------------------------------------------
    c1.messagetype = 2;
    strcpy(c1.name,"ben");
    strcpy(c1.ip,"255.255.255.255");
    c1.port = 40001;	

    /* Send the struct to the server */
    if (sendto(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct command))
        DieWithError("sendto() sent a different number of bytes than expected");

    /* Receive a response */
    fromSize = sizeof(fromAddr);

    if ((nBytes = recvfrom(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct command) )
        DieWithError("recvfrom() failed");

    if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
            fprintf(stderr,"Error: received a packet from unknown source.\n");
            exit(1);
    }
    //----------------------------------------------------------------------------------------------
    if(c1.messagetype == 0)//if user added 
    {
        printf("SUCCESS: user added\n");   
        /* Pass string back and forth between server 5 times */
        printf( "Echoing %d iterations.\n", ITERATIONS );

        for( int i = 0; i < ITERATIONS; i++ )
        {
            char echoBuffer[100];
            printf( "\nEnter command: \n" );
            scanf( "%s", echoBuffer );
            char messagetype[20];
            strcpy(messagetype,strtok(echoBuffer, " "));

            if(strcmp(messagetype,"create") == 0)
            {
                c1.messagetype = 3;
                char group[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.group,strtok(echoBuffer, " "));
            }            
            else if(strcmp(messagetype,"join") == 0)
            {
                c1.messagetype = 4;
                char group[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.group,strtok(echoBuffer, " "));
                char name[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.name,strtok(echoBuffer, " "));
            }
            //done-----------------^
            else if(strcmp(messagetype,"query-lists") == 0)
            {
                c1.messagetype = 5;
                char group[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.group,strtok(echoBuffer, " "));
                char name[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.name,strtok(echoBuffer, " "));
            }
            else if(strcmp(messagetype,"im-start") == 0)
            {
                c1.messagetype = 6;
                char group[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.group,strtok(echoBuffer, " "));
                char name[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.name,strtok(echoBuffer, " "));
            }
            else if(strcmp(messagetype,"im-complete") == 0)
            {
                c1.messagetype = 7;
                char group[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.group,strtok(echoBuffer, " "));
                char name[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.name,strtok(echoBuffer, " "));
            }
            else if(strcmp(messagetype,"leave") == 0)
            {
                c1.messagetype = 8;
                char group[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.group,strtok(echoBuffer, " "));
                char name[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.name,strtok(echoBuffer, " "));
            }            
            else if(strcmp(messagetype,"save") == 0)
            {
                c1.messagetype = 9;
                char name[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.name,strtok(echoBuffer, " "));
            }
            else if(strcmp(messagetype,"exit") == 0)
            {
                c1.messagetype = 10;
                char name[50];
                scanf( "%s", echoBuffer );
                strcpy(c1.name,strtok(echoBuffer, " "));
            }
            else
            {
                printf("not a command");
            }

            /* Send the struct to the server */
            if (sendto(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct command))
                DieWithError("sendto() sent a different number of bytes than expected");

            /* Receive a response */

            fromSize = sizeof(fromAddr);

            if ((nBytes = recvfrom(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct command) )
                DieWithError("recvfrom() failed");

            if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
            {
                    fprintf(stderr,"Error: received a packet from unknown source.\n");
                    exit(1);
            }
            if(c1.messagetype == 0)
            {
                if(c1.sendingList.size()>0)
                {
                    for(int i=0;i<c1.sendingList.size();i++)
                    {
                        sendingList.push_back(c1.sendingList[i]);
                    }
                }
                printf("SUCCESS: %s",c1.message);
                c1.messagetype = -1;
                strcpy(c1.name,"");
                strcpy(c1.group,"");
                strcpy(c1.ip,"");
                int port = 0;
                int code = 0;
                vector<user> sendingList = vector<user>();
                strcpy(c1.message,"");
            }
            else
            {
                printf("FAILURE: %s",c1.message);
            }
            
    
        }
    }
    else//initialize failed
    {
        printf("FAILURE: %s",c1.message);
    }
    
    close(sock);
    exit(0);
}