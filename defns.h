#include <vector>
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
	char message[200];
};
