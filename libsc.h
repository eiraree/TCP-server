#include <netinet/in.h>

// include guard

#define BUFF_LEN 25
#define SERV_PORT 3838
#define MAX_CONNECT 5

struct info {
	unsigned int server_port;
	long server_ip; 
	char * message;
	unsigned int message_lenght;
};

// #if defined SERVER
// #warning "Compile as server"
// #elif defined CLIENT
// #warning "Compile as client"
// #else
// #error "Specify how do you want to compile this program"
// #endif

int create_connection_c (struct info * client);
int create_connection_s (struct sockaddr_in * socket_data);
int send_message_c (int sock, struct info * client);
int recv_message_s (int sock, int conn_fd, char * buff); 


struct info * read_info_c (struct info * client);
int accept_conn (int sock, int conn_fd); 
