#include <netinet/in.h>

// include guard

#define BUFF_LEN 25
#define SERV_PORT 3838
#define MAX_CONNECT 5

/*
REQ_INFO = 0
RSP_INFO = 1
REQ_DATA = 2
RSP_DATA = 3
 */

struct msg {
	uint8_t type;
	uint32_t flags;
	uint32_t data_len;         
	char data[];
} __attribute__((packed));

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

int send_file_s (int conn_fd);
int create_connection_c (struct info * client);
int create_connection_s (struct sockaddr_in * socket_data);
int send_message (int sock, struct info * client);
int recv_message_s (int sock, int conn_fd, char * buff); 


struct info * read_info_c (struct info * client);
int accept_conn (int sock, int conn_fd);
int recv_message (int sock, char * buff);
int listen_socket (int sock);
int close_socket (int sock);
