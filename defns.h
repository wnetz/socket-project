#include <vector>
#include <string>
struct user
{
	char name[50];
	char ip[16];
	int port;
};

struct command
{
	int messagetype;
	char name[50];
	char group[50];
    char ip[16];
    int port;
    int code;
	std::vector<user> sendingList;
	char groups[10][50];
	char message[200];
};
