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
#include <thread>
#include <future>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#define ITERATIONS	1
#define MAXBYTES 200
using namespace std;
int port;
bool done = false;
bool got = false;
bool got2 = false;
bool rc = false;
struct sockaddr_in echoServAddr; /* Echo server address */

void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

void messagethread()
{
    cout << "in cmd" << got << " " << got2 << endl;
    struct sockaddr_in fromAddr;
    struct sockaddr_in msmAddr;
    unsigned short msmPort;
    unsigned int fromSize;
    char *msmIP; 
    int nBytes; 
    int sock;
    struct command c1;

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (struct sockaddr *) &msmAddr, sizeof(msmAddr)) < 0)
        DieWithError("setsockopt(SO_REUSEADDR) failed");

    memset(&msmAddr, 0, sizeof(msmAddr));   /* Zero out structure */
    msmAddr.sin_family = AF_INET;                /* Internet address family */
    msmAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    msmAddr.sin_port = htons(port);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &msmAddr, sizeof(msmAddr)) < 0)
        DieWithError("bind() failed");
    fromSize = sizeof(fromAddr);

    if ((nBytes = recvfrom(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct command) )
        DieWithError("recvfrom() failed");
    rc = true;

    if(c1.messagetype == 0)
    {
        cout<<c1.code<<" "<<c1.code2<<endl;
        printf("SUCCESS: %s\n",c1.message);
        if(strcmp(c1.message,"user removed\n") == 0)
        {
            done = true;
        }
        if(c1.code == 0)
        {
            c1.messagetype = 7;
            /* Send the struct to the server */
            if (sendto(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct command))
                DieWithError("sendto() sent a different number of bytes than expected");
        }
        else if(c1.code > -1)
        {
            if(c1.code2 == 5)
            {
                cout<<"groups: " << c1.groups[0];
                for(int i=1;i<c1.code;i++)
                {
                    cout << ", " << c1.groups[i];
                }
                cout << "\n";
            }
            else if(c1.code2 == 6)
            {
                bool start = false;
                cout << c1.groupIM[0].name <<" "<<c1.name<< " "<< strcmp(c1.groupIM[0].name,c1.name)<<endl;
                if(strcmp(c1.groupIM[0].name,c1.name) == 0)
                {
                    string input;
                    cout << "enter message: ";
                    getline(cin, input);                    
                    strcpy(c1.message, input.c_str());
                    cout<<endl;
                    start = true;
                }
                else
                {
                    cout << c1.message << endl;
                }
                
                cout<<"members: " << c1.groupIM[0].name;
                for(int i=1;i<c1.code;i++)
                {
                    cout << ", " << c1.groupIM[i].name;
                }
                cout << "\n";

                c1.code = c1.code - 1;

                msmIP = c1.groupIM[c1.code].ip;
                msmPort = c1.groupIM[c1.code].port;

                printf( "sending msg to %s on port %d\n", msmIP, msmPort );

                memset(&msmAddr, 0, sizeof(msmAddr));    /* Zero out structure */
                msmAddr.sin_family = AF_INET;                 /* Internet addr family */
                msmAddr.sin_addr.s_addr = inet_addr(msmIP);  /* Server IP address */
                msmAddr.sin_port   = htons(msmPort);     /* Server port */
                
                if (sendto(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &msmAddr, sizeof(msmAddr)) != sizeof(struct command))
                    DieWithError("sendto() sent a different number of bytes than expected");
            }
        }
        
        c1.code = -1;
    }
    else
    {
        printf("FAILURE: %s",c1.message);
    }    
    got2 = true;
    cout << "done cmd" << got << " " << got2 << endl;
    close(sock);
    return;
}

string getString()
{
    cout << "in string\n";
    cout << "\nEnter command-----: ";
    string input;
    getline(cin, input);
    cout << "\n";
    got = true;
    cout << "done string" << got << " " << got2 << endl;
    return input;
}

int main(int argc, char *argv[])
{
	int sock;                        /* Socket descriptor */
    
    struct sockaddr_in msmAddr;
    struct sockaddr_in fromAddr;     /* Source address of echo */
    unsigned short echoServPort;     /* Echo server port */
    unsigned short msmPort;
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    char *servIP;                    /* IP address of server */
    char *msmIP;
    int nBytes;              		 /* Length of received response */
    struct command c1;              //message that is sent
    fd_set readfds;
    int num_readable;
    struct timeval tv;
    int num_bytes;
    char buf[MAXBYTES];
    int fd_stdin;
    bool incmd = false;
    bool instring = false;

    fd_stdin = fileno(stdin);

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
    echoServAddr.sin_port   = htons(40000);     /* Server port */

	
    
    //initialize with server-----------------------------------------------------------------------------
    do
    {   
        string in;
        c1.messagetype = 2;
        cout << "name: ";
        scanf("%s",&c1.name);
        cout << "ip: ";
        scanf("%s",&c1.ip);
        strcpy(c1.ip,"255.255.255.255");
        do
        {
            cout << "port(40001-40499): ";
            scanf("%d",&c1.port);
            if(c1.port<40001 || c1.port>40499)
            {
                cout<<c1.port<<" is not in range\n";
            }
        }while(c1.port<40001 || c1.port>40499);
        port = c1.port;
        	

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
        if(c1.messagetype==0)
        {
            printf("SUCCESS: user added\n");            
        }
        else//initialize failed
        {
            printf("FAILURE: %s",c1.message);
        }
    } 
    while (c1.messagetype!=0);
    c1.port = 1;
    //----------------------------------------------------------------------------------------------

    
    cin.ignore(numeric_limits<streamsize>::max(),'\n'); 
    std::chrono::milliseconds span (100);
    future<void> messagethrd;
    std::future<string> futureObj;
 

    //loop while conected
    while(!done)
    {
        //std::promise<string> promiseObj;
        if(!incmd)
        {
            incmd = true;
            messagethrd = async(launch::async, messagethread); 
        }
        if(!instring)
        {
            instring = true;
            futureObj = async(getString);
        }
        //std::thread th(getString, &promiseObj);
        if(futureObj.valid() && futureObj.wait_for(span) == std::future_status::ready) 
        { 
            cout << "string" << got << " " << got2 << endl;
            got = false;
            string in;
            in = futureObj.get();
            //th.join();
            char echoBuffer[100];
            int loc = in.find(" ");
            char messagetype[20];
            strcpy(messagetype, in.substr(0,loc).c_str());
            
            //logic for server commands---------------------------------------------------------------------
            //done
            if(strcmp(messagetype,"create") == 0)
            {
                int loc2 = in.find(" ",loc+1);
                c1.messagetype = 3;
                strcpy(c1.group, in.substr(loc+1).c_str());
            }            
            //done
            else if(strcmp(messagetype,"join") == 0)
            {
                c1.messagetype = 4;
                int loc2 = in.find(" ",loc+1);
                strcpy(c1.group, in.substr(loc+1,loc2-loc-1).c_str());
                loc = loc2;
                loc2 = in.find(" ",loc+1);
                strcpy(c1.name, in.substr(loc+1).c_str());
            }            
            //done
            else if(strcmp(messagetype,"query-lists") == 0)
            {
                c1.messagetype = 5;
            }
            else if(strcmp(messagetype,"im-start") == 0)
            {
                c1.messagetype = 6;
                int loc2 = in.find(" ",loc+1);
                strcpy(c1.group, in.substr(loc+1,loc2-loc-1).c_str());
                loc = loc2;
                loc2 = in.find(" ",loc+1);
                strcpy(c1.name, in.substr(loc+1).c_str());
            }
            else if(strcmp(messagetype,"im-complete") == 0)
            {
                c1.messagetype = 7;
                int loc2 = in.find(" ",loc+1);
                strcpy(c1.group, in.substr(loc+1,loc2-loc-1).c_str());
                loc = loc2;
                loc2 = in.find(" ",loc+1);
                strcpy(c1.name, in.substr(loc+1).c_str());
            }
            else if(strcmp(messagetype,"leave") == 0)
            {
                c1.messagetype = 8;
                int loc2 = in.find(" ",loc+1);
                strcpy(c1.group, in.substr(loc+1,loc2-loc-1).c_str());
                loc = loc2;
                loc2 = in.find(" ",loc+1);
                strcpy(c1.name, in.substr(loc+1).c_str());
            }            
            //done
            else if(strcmp(messagetype,"save") == 0)
            {
                c1.messagetype = 9;
                int loc2 = in.find(" ",loc+1);
                strcpy(c1.name, in.substr(loc+1).c_str());
            }
            //done
            else if(strcmp(messagetype,"exit") == 0)
            {
                c1.messagetype = 10;
                int loc2 = in.find(" ",loc+1);
                strcpy(c1.name, in.substr(loc+1).c_str());
            }
            else
            {
                printf("%s not a command\n",messagetype);
                c1.messagetype = 1;
            }
            //----------------------------------------------------------------------------------------------

            if(c1.messagetype!=1)
            {
                /* Send the struct to the server */
                if (sendto(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct command))
                    DieWithError("sendto() sent a different number of bytes than expected");

                /* Receive a response */
            }  
            instring = false;
            cout << "-string" << got << " " << got2 << endl;          
        }

        if(messagethrd.valid() && messagethrd.wait_for(span) == std::future_status::ready)
        {
            cout << "cmd" << got << " " << got2 << endl;
            got2 = false;
            messagethrd.get();
            incmd = false;
            cout << "-cmd" << got << " " << got2 << endl;
        }
    }
    
    
    close(sock);
    exit(0);
}
