#include "defns.h"

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <iostream>
#include <fstream>
#include <vector>       //for remembering users, groups, and halted groups
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
    vector<user> users;              // current users
    vector<vector<user>> groups;     // current groups
    vector<string> halt;             // groups in msm
	struct command c1;               // message from user

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

//determin command-------------------------------------------------------------------------------------------------------
        if(c1.messagetype == 2)//register done
        {
            printf("register request from %s\n", inet_ntoa(echoClntAddr.sin_addr));
            // check if user already exist
            cout<<"check if user exists\n";
            for(int i=0;i<users.size();i++)
            {
                if(strcmp(users[i].ip,inet_ntoa(echoClntAddr.sin_addr))==0 && users[i].port == c1.port)
                {
                    c1.messagetype = 0;
                    i = users.size();
                }
                //user already using givin name
                if(strcmp(c1.name,users[i].name) == 0)//if user exists fail
                {
                    c1.messagetype = 1;
                    i = users.size();
                }
            }
            if(c1.messagetype == 2)
            {
                c1.messagetype = 0;
                //add user to users
                struct user newUser;
                strcpy(newUser.name,c1.name);
                strcpy(newUser.ip,inet_ntoa(echoClntAddr.sin_addr));
                newUser.port=c1.port;
                users.push_back(newUser);
                printf("SUCCESS: added user %s on %s from port %d\n", users[users.size()-1].name,users[users.size()-1].ip,users[users.size()-1].port);
                strcpy(c1.message,"user added\n");
            }
            else if(c1.messagetype == 1)
            {
                printf("FAILURE: user by the name %s already exists\n",c1.name);
                strcpy(c1.message,"user already exists\n");
            }
            else
            {
                c1.messagetype = 1;
                printf("FAILURE: user on that ip is already useing that port\n");
                strcpy(c1.message,"user on this ip is already using that port\n");
            }            
            c1.port = 0;
            
        }
        else if(c1.messagetype == 3)///create done
        {
            printf("create group %s\n", c1.group);
            // check if group already exists
            cout<<"check if group exists\n";
            for(int i=0;i<groups.size();i++)
            {
                if(strcmp(groups[i][0].name,c1.group)==0)// if group exists fail
                {
                    c1.messagetype = 1;
                }
            }
            if(c1.messagetype!=1)
            {
                c1.messagetype = 0;
                groups.push_back(vector<user>());
                //add user with name group to groups
                struct user newUser;
                strcpy(newUser.name,c1.group);
                groups[groups.size()-1].push_back(newUser);
                printf("SUCCESS: group \"%s\" added\n", groups[groups.size()-1][0].name);
                strcpy(c1.message,"group created\n");
            }
            else
            {
                printf("FAILURE: group %s already exists\n",c1.group);
                strcpy(c1.message,"group already exists\n");
            }
        }
        else if(c1.messagetype == 4)//join done
        {
            printf("adding %s to group \"%s\"\n", c1.name, c1.group);            
            bool inhalt = false;
            // find if group is in halt
            cout<<"check if group is in halt\n";
            for(int i = 0; i < halt.size();i++)
            {
                if(strcmp(halt[i].c_str(),c1.group)==0)
                {
                    inhalt = true;
                }
            }
            if(!inhalt)
            {
                int usr = -1;
                //find user
                cout<<"check if user exists\n";
                for(int i=0;i<users.size();i++)
                {
                    if(strcmp(users[i].name,c1.name)==0)
                    {
                        usr = i;
                    }
                }
                if(usr >= 0)//if user exists
                {
                    int grp = -1;
                    //find group
                    cout<<"check if group exists\n";
                    for(int i=0;i<groups.size();i++)
                    {
                        if(strcmp(groups[i][0].name,c1.group)==0)
                        {
                            grp = i;
                        }
                    }
                    if(grp >= 0)//if group exists
                    {
                        int loc = -1;
                        // find user in group
                        cout<<"check if user is in group\n";
                        for(int i=0;i<groups[grp].size();i++)
                        {
                            if(strcmp(groups[grp][i].name,c1.name)==0)
                            {
                                loc = i;
                            }
                        }
                        if(loc == -1)//if user not in group
                        {
                            c1.messagetype = 0;
                            struct user curentUser;
                            strcpy(curentUser.name,users[usr].name);
                            strcpy(curentUser.ip,users[usr].ip);  
                            curentUser.port = users[usr].port; 
                            groups[grp].push_back(curentUser); 

                            printf("SUCCESS: user %s added to group \"%s\"\n",groups[grp][groups[grp].size()-1].name,groups[grp][0].name);
                            strcpy(c1.message,"user added to group\n");
                        }
                        else
                        {
                            c1.messagetype = 1;
                            printf("FAILURE: user %s is already in group \"%s\"\n",c1.name,c1.group);
                            strcpy(c1.message,"user is alredy in group\n");
                        }
                    }
                    else
                    {
                        c1.messagetype = 1;
                        printf("FAILURE: group \"%s\" does not exists\n",c1.group);
                        strcpy(c1.message,"no such group\n");
                    }
                }
                else
                {
                    c1.messagetype = 1;
                    printf("FAILURE: user by the name %s does not exists\n",c1.name);
                    strcpy(c1.message,"no such user\n");
                } 
            }
            else
            {
                c1.messagetype = 1;
                printf("FAILURE: group \"%s\" is in msm\n",c1.group);
                strcpy(c1.message,"group is in msm\n");
            }       
        }
        else if(c1.messagetype == 5)//query-lists done
        {
            printf("geting list of groups\n");
            c1.messagetype = 0;
            c1.code = groups.size();
            c1.code2 = -1;
            for(int i=0;i<groups.size();i++)//get group names
            {
                strcpy(c1.groups[i], groups[i][0].name);
            }
            if(c1.code>0)//prevent index out of bounds
            {                
                printf("SUCCESS: there are %d groups: %s",c1.code,c1.groups[0]);
                for(int i=1;i<c1.code;i++)
                {
                    cout << ", " << c1.groups[i];
                }
                cout << "\n";
            }
            else
            {
                printf("SUCCESS: there are %d groups\n",c1.code);
            }
            
            strcpy(c1.message,"groups found");
            
        }
        else if(c1.messagetype == 6)//im-start done
        {
            printf("getting msm for %s: %s\n", c1.group, c1.name);
            int usr = -1;
            //find user
            cout<<"check if user exists\n";
            for(int i=0;i<users.size();i++)
            {
                if(strcmp(users[i].name,c1.name)==0)
                {
                    usr = i;
                }
            }
            if(usr >= 0)//if user exists
            {
                int grp = -1;
                //find group
                cout<<"check if group exists\n";
                for(int i=0;i<groups.size();i++)
                {
                    if(strcmp(groups[i][0].name,c1.group)==0)
                    {
                        grp = i;
                    }
                }
                if(grp >= 0)//if group exists
                {
                    int loc = -1;
                    // find user in group
                    cout<<"check if user is in group\n";
                    for(int i=0;i<groups[grp].size();i++)
                    {
                        if(strcmp(groups[grp][i].name,c1.name)==0)
                        {
                            loc = i;
                        }
                    }
                    if(loc != -1)//if user in group
                    {
                        //reorder group
                        cout<<"reorder names\n";
                        for(int i=1;i<groups[grp].size();i++)
                        {
                            if(i < loc)
                            {
                                c1.groupIM[i] = groups[grp][i];
                            }
                            else if(i > loc)
                            {
                                c1.groupIM[i-1] = groups[grp][i];
                            }
                            if(loc == i)
                            {
                                c1.groupIM[0]=groups[grp][i];
                            }
                            
                        }

                        c1.code = groups[grp].size()-1;
                        cout<<c1.code<<" members: " << c1.groupIM[0].name;   
                        for(int i=1;i<c1.code;i++)
                        {
                            cout << ", " << c1.groupIM[i].name;
                        }
                        cout << "\n";
                        //used to check if user is the start of the msm
                        c1.code2 = c1.code;
                        //push on halt to stop leave exit and join
                        halt.push_back(string(groups[grp][0].name));
                        printf("SUCCESS: user orderd for group \"%s\"\n",c1.group);
                        strcpy(c1.message,"user orderd for\n");
                    }
                    else
                    {
                        c1.messagetype = 1;
                        printf("FAILURE: user %s is not in group \"%s\"\n",c1.name,c1.group);
                        strcpy(c1.message,"user is not in group\n");
                    }
                }
                else
                {
                    c1.messagetype = 1;
                    printf("FAILURE: group \"%s\" does not exists\n",c1.group);
                    strcpy(c1.message,"no such group\n");
                }
            }
            else
            {
                c1.messagetype = 1;
                printf("FAILURE: user by the name %s does not exists\n",c1.name);
                strcpy(c1.message,"no such user\n");
            }
        }
        else if(c1.messagetype == 7)//im-complete done
        {
            printf("ending msm for %s: %s\n", c1.group, c1.name);
            int usr = -1;
            //find user
            cout<<"check if user exists\n";
            for(int i=0;i<users.size();i++)
            {
                if(strcmp(users[i].name,c1.name)==0)
                {
                    usr = i;
                }
            }
            if(usr >= 0)//if user exists
            {
                int grp = -1;
                //find group
                cout<<"check if group exists\n";
                for(int i=0;i<groups.size();i++)
                {
                    if(strcmp(groups[i][0].name,c1.group)==0)
                    {
                        grp = i;
                    }
                }
                if(grp >= 0)//if group exists
                {
                    int loc = -1;
                    cout<<"check if userr is in group\n";
                    for(int i=0;i<groups[grp].size();i++)
                    {
                        if(strcmp(groups[grp][i].name,c1.name)==0)
                        {
                            loc = i;
                        }
                    }
                    if(loc != -1)//if user in group
                    {

                        bool inhalt = false;
                        //check i group is in halt
                        cout<<"check if group is in halt\n";
                        for(int i=0;i<halt.size();i++)
                        {
                            if(strcmp(halt[i].c_str(),c1.group) == 0)
                            {                                
                                inhalt = true;
                                halt.erase(halt.begin()+i);
                                i = halt.size();
                            }
                        }
                        if(!inhalt)
                        {
                            c1.messagetype = 1;
                            printf("FAILURE: msm not in progress\n",c1.name,c1.group);
                            strcpy(c1.message,"msm not in progress\n");
                        }
                        printf("SUCCESS: group \"%s\" removed from halt\n",c1.group);
                        strcpy(c1.message,"group removed from halt\n");
                    }
                    else
                    {
                        c1.messagetype = 1;
                        printf("FAILURE: user %s is not in group \"%s\"\n",c1.name,c1.group);
                        strcpy(c1.message,"user is not in group\n");
                    }
                }
                else
                {
                    c1.messagetype = 1;
                    printf("FAILURE: group \"%s\" does not exists\n",c1.group);
                    strcpy(c1.message,"no such group\n");
                }
            }
            else
            {
                c1.messagetype = 1;
                printf("FAILURE: user by the name %s does not exists\n",c1.name);
                strcpy(c1.message,"no such user\n");
            }
        }
        else if(c1.messagetype == 8)//leave FS
        {
            printf("leaving %s: %s\n", c1.group, c1.name);
            int usr = -1;
            //find user
            cout<<"check if user exists\n";
            for(int i=0;i<users.size();i++)
            {
                if(strcmp(users[i].name,c1.name)==0)
                {
                    usr = i;
                }
            }
            if(usr >= 0)//if user exists
            {
                int grp = -1;
                //find group
                cout<<"check if group exists\n";
                for(int i=0;i<groups.size();i++)
                {
                    if(strcmp(groups[i][0].name,c1.group)==0)
                    {
                        grp = i;
                    }
                }
                if(grp >= 0)//if group exists
                {
                    int loc = -1;
                    cout<<"check if user is in group\n";
                    for(int i=0;i<groups[grp].size();i++)
                    {
                        if(strcmp(groups[grp][i].name,c1.name)==0)
                        {
                            loc = i;
                        }
                    }
                    if(loc != -1)//if user in group
                    {
                        c1.messagetype = 0;

                        bool inhalt = false;
                        //check if group is in halt
                        cout<<"check if group is in halt\n";
                        for(int i=0;i<halt.size();i++)
                        {
                            if(strcmp(halt[i].c_str(),c1.group) == 0)
                            {
                                inhalt = true;
                            }
                        }
                        if(inhalt)
                        {
                            c1.messagetype = 1;
                            printf("FAILURE: msm in progress\n",c1.name,c1.group);
                            strcpy(c1.message,"msm in progress\n");
                        }
                        else
                        {
                            cout<<"remove from group\n";
                            groups[grp].erase(groups[grp].begin()+loc);
                            printf("SUCCESS: user removed from group \"%s\"\n",c1.group);
                            strcpy(c1.message,"user removed from group\n");
                        }                        
                    }
                    else
                    {
                        c1.messagetype = 1;
                        printf("FAILURE: user %s is not in group \"%s\"\n",c1.name,c1.group);
                        strcpy(c1.message,"user is not in group\n");
                    }
                }
                else
                {
                    c1.messagetype = 1;
                    printf("FAILURE: group \"%s\" does not exists\n",c1.group);
                    strcpy(c1.message,"no such group\n");
                }
            }
            else
            {
                c1.messagetype = 1;
                printf("FAILURE: user by the name %s does not exists\n",c1.name);
                strcpy(c1.message,"no such user\n");
            }
        }
        else if(c1.messagetype == 9)//save done
        {
            printf("create file \"%s\" with groups and users\n", c1.name);
            //open file
            ofstream saveFile(c1.name);

            if(!saveFile)//fail if file did not open
            {
                c1.messagetype = 1;
                printf("FAILURE: file \"%s\" could not be created\n",c1.name);
                strcpy(c1.message,"file could not be created\n");
            }
            else
            {
                saveFile<<users.size()<<"\n";//users size
                for(int i=0;i<users.size();i++)//users
                {
                    saveFile<<users[i].name<<" "<<users[i].ip<<" "<<users[i].port<<"\n";
                }
                saveFile<<groups.size()<<"\n";//groups size 
                for(int i=0;i<groups.size();i++)
                {
                    saveFile<<groups[i][0].name<<" "<<groups[i].size()-1<<"\n";//group size
                    for(int j=1;j<groups[i].size();j++)//group users
                    {
                        saveFile<<groups[i][j].name<<" "<<groups[i][j].ip<<" "<<groups[i][j].port<<"\n";
                    }
                }
                c1.port = 0;

                //save file
                saveFile.close();
                printf("SUCCESS: file \"%s\" created\n", c1.name);
                strcpy(c1.message,"file created\n");
            }            
        }
        else if(c1.messagetype == 10)//exit todo/FS
        {
            printf("exit request from %s\n", c1.name);
            bool inhalt = false; 
            bool exists = false;
            //check if user exists 
            cout<<"check if user exists\n";          
            for(int i=0;i<users.size();i++)
            {
                if(strcmp(c1.name,users[i].name) == 0)//if user exist
                {
                    exists = true;
                    //check all groups
                    cout<<"check if user is in group that is in halt\n";
                    for(int j = 0; j < groups.size();j++)
                    {
                        //check if user is in group
                        for(int k = 1; k < groups[j].size();k++)
                        {
                            if(strcmp(groups[j][k].name,c1.name)==0)
                            {
                                //check if group is in halt
                                for(int l = 0; l<halt.size();l++)
                                {
                                    if(strcmp(halt[l].c_str(),groups[j][0].name)==0)
                                    {
                                        cout << "in\n";
                                        inhalt = true;
                                    }
                                }
                            }
                        }
                    }                    
                }
                
            }
            if(exists && !inhalt)
            {    
                cout<<"remove userr from groups\n";            
                for(int i=0;i<groups.size();i++)//search groups
                {
                    for(int j=1;j<groups[i].size();j++)//search group
                    {
                        if(strcmp(c1.name,groups[i][j].name) == 0)
                        {
                            groups[i].erase(groups[i].begin()+j);//remove if in group
                        }
                    }
                }
                cout<<"remove user from users\n";
                for(int i=0;i<users.size();i++)//search users
                {                    
                    if(strcmp(c1.name,users[i].name) == 0)
                    {
                        c1.port = 0; 
                        //echoClntAddr.sin_port = htons(users[i].port);
                        users.erase(users.begin()+i);//from users
                    }
                }
                printf("SUCCESS: user %s removed\n", c1.name);
                strcpy(c1.message,"user removed\n");
            }
            else if(!inhalt)
            {
                c1.messagetype = 1;
                printf("FAILURE: user %s does bot exist\n", c1.name);
                strcpy(c1.message,"user does not exist\n");
            }
            else
            {
                c1.messagetype = 1;
                printf("FAILURE: user %s is in a msm\n", c1.name);
                strcpy(c1.message,"user is in an msm\n");
            }
        }
//--------------------------------------------------------------------------------------------------------------------------------

        cout << endl;
        if (sendto(sock, &c1, sizeof(struct command), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(struct command))
            DieWithError("sendto() sent a different number of bytes than expected");
    }
    /* NOT REACHED */
}
