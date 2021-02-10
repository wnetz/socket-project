#include "defns.h"

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <iostream>
#include <vector>
#include <string>
using namespace std;
void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    unsigned short echoServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
    vector<user> users;
    vector<vector<user>> groups;
	struct command c1;

    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
  
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");
        //determin command
        if(c1.messagetype == 2)
        {
            printf("register request from %s\n", inet_ntoa(echoClntAddr.sin_addr));
            for(int i=0;i<users.size();i++)
            {
                if(strcmp(c1.name,users[i].name) == 0)
                {
                    c1.messagetype = 1;
                }
            }
            if(c1.messagetype != 1)
            {
                c1.messagetype = 0;
                struct user newUser;
                strcpy(newUser.name,c1.name);
                strcpy(newUser.ip,c1.ip);
                newUser.port=c1.port;
                users.push_back(newUser);
                printf("SUCCESS: added user %s on %s from port %d\n", users[users.size()-1].name,users[users.size()-1].ip,users[users.size()-1].port);
                strcpy(c1.message,"user added");
            }
            else
            {
                printf("FAILURE: user by the name %s already exists\n",c1.name);
                strcpy(c1.message,"user already exists");
            }
            
        }
        else if(c1.messagetype == 3)
        {
            printf("create group %s\n", c1.group);
            for(int i=0;i<groups.size();i++)
            {
                if(strcmp(groups[i][0].name,c1.group)==0)
                {
                    c1.messagetype = 1;
                }
            }
            if(c1.messagetype!=1)
            {
                c1.messagetype = 0;
                groups.push_back(vector<user>());
                struct user newUser;
                strcpy(newUser.name,c1.group);
                groups[groups.size()-1].push_back(newUser);
                printf("SUCCESS: group \"%s\" added\n", groups[groups.size()-1][0].name);
                strcpy(c1.message,"group created");
            }
            else
            {
                printf("FAILURE: group %s already exists\n",c1.name);
                strcpy(c1.message,"group already exists");
            }
        }
        else if(c1.messagetype == 4)
        {
            printf("adding %s to group \"%s\"\n", c1.name, c1.group);
            int usr = -1;
            //find user
            for(int i=0;i<users.size();i++)
            {
                if(strcmp(users[i].name,c1.name)==0)
                {
                    usr = i;
                }
            }
            if(usr >= 0)
            {
                int grp = -1;
                //find group
                for(int i=0;i<groups.size();i++)
                {
                    if(strcmp(groups[i][0].name,c1.group)==0)
                    {
                        grp = i;
                    }
                }
                if(grp >= 0)
                {
                    int loc = -1;
                    for(int i=0;i<groups[grp].size();i++)
                    {
                        if(strcmp(groups[grp][i].name,c1.name)==0)
                        {
                            loc = i;
                        }
                    }
                    if(loc == -1)
                    {
                        c1.messagetype = 0;
                        struct user curentUser;
                        strcpy(curentUser.name,users[usr].name);
                        strcpy(curentUser.ip,users[usr].ip);  
                        curentUser.port = users[usr].port; 
                        groups[grp].push_back(curentUser); 

                        printf("SUCCESS: user %s added to group \"%s\"\n",groups[grp][groups[grp].size()-1].name,groups[grp][0].name);
                        strcpy(c1.message,"user added to group");
                    }
                    else
                    {
                        c1.messagetype = 1;
                        printf("FAILURE: user %s is already in group \"%s\"\n",c1.name,c1.group);
                        strcpy(c1.message,"user is alredy in group");
                    }
                }
                else
                {
                    c1.messagetype = 1;
                    printf("FAILURE: group \"%s\" does not exists\n",c1.group);
                    strcpy(c1.message,"no such group");
                }
            }
            else
            {
                c1.messagetype = 1;
                printf("FAILURE: user by the name %s does not exists\n",c1.name);
                strcpy(c1.message,"no such user");
            }        
        }
        else if(c1.messagetype == 5)
        {
            printf("geting list for %s\n", c1.group);
            int usr = -1;
            //find user
            for(int i=0;i<users.size();i++)
            {
                if(strcmp(users[i].name,c1.name)==0)
                {
                    usr = i;
                }
            }
            if(usr >= 0)
            {
                int grp = -1;
                //find group
                for(int i=0;i<groups.size();i++)
                {
                    if(strcmp(groups[i][0].name,c1.group)==0)
                    {
                        grp = i;
                    }
                }
                if(grp >= 0)
                {
                    int loc = -1;
                    for(int i=0;i<groups[grp].size();i++)
                    {
                        if(strcmp(groups[grp][i].name,c1.name)==0)
                        {
                            loc = i;
                        }
                    }
                    if(loc >= 0)
                    {
                        printf("%d\n", loc);
                        c1.messagetype = 0;
                        //deep copy
                        struct user curentUser;

                        strcpy(curentUser.name,groups[grp][loc].name);
                        strcpy(curentUser.ip,groups[grp][loc].ip);  
                        curentUser.port = groups[grp][loc].port; 
                        c1.sendingList.push_back(curentUser);
                        for(int i=1;i<groups[grp].size();i++) 
                        {
                            if(i!=loc)
                            {
                                c1.sendingList.push_back(groups[grp][i]);
                            }

                        }
                        printf("SUCCESS: sending list: %s",c1.sendingList[0].name);
                        for(int i=1;i<c1.sendingList.size();i++)
                        {
                            cout << ", " << c1.sendingList[i].name;
                        }
                        cout << "\n";
                        strcpy(c1.message,"group created");
                    }
                    else
                    {
                        c1.messagetype = 1;
                        printf("FAILURE: user %s is not in group \"%s\"\n",c1.name,c1.group);
                        strcpy(c1.message,"no such group");
                    }
                }
                else
                {
                    c1.messagetype = 1;
                    printf("FAILURE: group \"%s\" does not exists\n",c1.group);
                    strcpy(c1.message,"no such group");
                }
            }
            else
            {
                c1.messagetype = 1;
                printf("FAILURE: user by the name %s does not exists\n",c1.name);
                strcpy(c1.message,"no such user");
            }
        }
        else if(c1.messagetype == 6)
        {

        }
        else if(c1.messagetype == 7)
        {

        }
        else if(c1.messagetype == 8)
        {

        }
        else if(c1.messagetype == 9)
        {

        }
        else if(c1.messagetype == 10)
        {

        }
        


        /* Send received datagram back to the client */
        if (sendto(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(struct command))
            DieWithError("sendto() sent a different number of bytes than expected");
    }
    /* NOT REACHED */
}
