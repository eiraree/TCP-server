#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

// include guard

#define BUFF_LEN 100
#define SERV_PORT 3838
#define MAX_CONNECT 5
#define MAX_SYM 80

#define REQ_INFO 0
#define RSP_INFO 1
#define REQ_DATA 2
#define RSP_DATA 3

#define ASSERT(x, code) if (!(x)) { \
			printf ("%s(), %d: assertion failed with errno = %d (%s) \n", __func__, __LINE__, errno, strerror(errno)); \
			return code; \
		}

struct msg {
	uint8_t type;
	uint8_t flags;
	uint32_t data_len;         
	char data[];
} __attribute__((packed));

struct info {
	unsigned int server_port;
	long server_ip; 
	char * message;
	unsigned int message_lenght;
};

int create_connection_c (struct info * client);
struct info * read_info_c (struct info * client);

int create_connection_s (struct sockaddr_in * socket_data);
int accept_conn (int sock, int conn_fd);
int listen_socket (int sock);
int recv_message_s (int socket, char * buff); 
int recv_message_c (int socket, char * buff); 
int proc_message_s (int conn_fd, int message_type);
int proc_message_c (int sock, struct msg * msg);

int send_message (int sock, struct msg * msg); 
int close_socket (int sock);
