#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "functions.h"

static int create_socket () {

	int sock = -1;

	ASSERT((sock = socket (AF_INET, SOCK_STREAM, 0)), -1);

#ifdef DEBUG_INFO
	printf ("Socket is created.. \n");
#endif
	return sock;
}


int close_socket (int sock) {

	if (close(sock) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;
	}
	return 0; 
}

							/* CLIENT FUNCTIONS */


struct info * read_info_c (struct info * client) {
	
	FILE * config_fd;
	char buff_str [100] = {0};
	char parameter [BUFF_LEN] = {0};
	char value [BUFF_LEN] = {0};

	struct info * client_info = client;

	if ((config_fd = fopen ("param.txt", "r") ) == NULL ) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return NULL;
	}

	while (fgets(buff_str, sizeof(buff_str), config_fd) != NULL)  { 
		
		sscanf (buff_str, "%s %s", parameter, value);
		
		if (strcmp (parameter, "SERVER_PORT") == 0) {
			client_info->server_port = strtol(value, NULL, 10);
		}

		if (strcmp (parameter, "SERVER_IP") == 0) {
			struct in_addr inet_address;
			inet_aton(value, &inet_address);
			client_info->server_ip = inet_address.s_addr;
		}

		if (strcmp (parameter, "MESSAGE") == 0) {
			char * msg = malloc(strlen(value) + 1);
			strcpy(msg, value);
			client_info->message = msg;
		}
	}

	fclose(config_fd);

	return client_info; 
}


static int connect_socket (int sock, struct info * client) {

	struct sockaddr_in serv_info;  

	serv_info.sin_family = AF_INET;
	serv_info.sin_port = htons(client->server_port);
	serv_info.sin_addr.s_addr = client->server_ip; 	
	
	if (connect (sock, (struct sockaddr *) &serv_info, sizeof(serv_info)) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;
	}

#ifdef DEBUG_INFO
	printf ("Connected to server.. \n");
#endif
	return 0; 
}


int create_connection_c (struct info * client) {
	
	int sock = -1;

	if ((sock = create_socket ()) != -1)
		connect_socket(sock, client);
	
	return sock;
}


int send_message (int sock, struct msg * msg) {

	int res = 0;
#if defined SERVER_MODE
	char * prefix = "[Server]";
#elif defined CLIENT_MODE
	char * prefix = "[Client]";
#else
#error "Please specify mode (SERVER_MODE/CLIENT_MODE)"
#endif

	if (!msg)
		return -1;

	printf("%s: Send started\n", prefix);
	printf("%s: Type: %hhd\n", prefix, msg->type); 
	printf("%s: Flags: %hhd\n", prefix, msg->flags); 
	printf("%s: Data length: %d\n", prefix, msg->data_len); 
	printf("%s: Send complete.\n\n", prefix);

	if ( send (sock, (void *) msg, sizeof(struct msg) + strlen(msg->data), 0) != -1) {
	#ifdef DEBUG_INFO
		printf ("-----> \n"); 
#endif
	} else if (res == -1) {
		printf ( "%s(), %d: error\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;
	}
	return 0; 
}


			/* SERVER FUNCTIONS */


static void * bind_socket (int sock, struct sockaddr_in * sll) {

	struct sockaddr_in * socket_data = sll;

	memset (socket_data, 0, sizeof(*socket_data));

	socket_data->sin_family = AF_INET;
	socket_data->sin_port = htons(SERV_PORT);
	socket_data->sin_addr.s_addr = htonl(INADDR_ANY); 

	if (bind (sock, (struct sockaddr *) socket_data, sizeof(*socket_data)) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return NULL;
	}

#ifdef DEBUG_INFO
	printf ("Binded successfully.. \n");
#endif
	return (void *) socket_data;	
}


int listen_socket (int sock) {
	
	if (listen (sock, MAX_CONNECT) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;	
	}

#ifdef DEBUG_INFO
	printf ("Listening.. \n");
#endif
	return 0;
}


int create_connection_s (struct sockaddr_in * socket_data) {

	int sock = -1; 

	if ((sock = create_socket ()) != -1) {
		if (bind_socket (sock, socket_data) != NULL) {
			listen_socket (sock);
		}
	}
	return sock;
}


int accept_conn (int sock, int conn_fd) {
	

	if ((conn_fd = accept (sock, NULL, NULL)) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;	
	}
	return conn_fd;
}


int recv_message_s (int socket, char * buff) {

	if (recv (socket, buff, BUFF_LEN, 0) == -1) {
		printf ("%s(), %d: error\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;
	}
	struct msg * req = malloc(sizeof(struct msg) + MAX_SYM);
	req = (struct msg * ) buff;

	printf("[Server]: Received message\n"); 
	printf("[Server]: Type = %hhd\n", req->type); 
	printf("[Server]: Flags: %hhd\n", req->flags); 
	printf("[Server]: Data length: %d\n", req->data_len); 
	if (req->data_len)
		printf ("[Server]: Data: %s \n", req->data); 
	printf ("[Server]: Receive complete\n\n");

	switch (req->type) {
		case REQ_INFO: 
			proc_message_s (socket, req->type);
			break;
		case REQ_DATA:
			proc_message_s (socket, req->type);
			break;
		default: 
			return 1;
	}

	return 0;
}


int recv_message_c (int socket, char * buff) {

	int res = 0;
	if ((res = recv (socket, buff, BUFF_LEN, 0)) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;
	}

	printf("debug: received %d\n", res);

	struct msg * req = (struct msg * ) buff;

	printf("[Client]: Received message\n"); 
	printf("[Client]: Type = %hhd\n", req->type); 
	printf("[Client]: Flags: %hhd\n", req->flags); 
	printf("[Client]: Data length: %d\n", req->data_len); 
	if (req->data_len)
		printf ("[Client]: Data: %s \n", req->data); 
	printf ("[Client]: Receive complete\n\n");

/*     printf ("RECV_MESSAGE \n");  */
/*     printf("type: %hhd\n", req->type);  */
/*     printf("flags: %hhd\n", req->flags);  */
/*     printf("data length: %d\n", req->data_len);  */
/* //	if (req->data_len) */
		/* printf ("data: %s \n \n", req->data);  */

	switch (req->type) {
		case RSP_INFO:
			proc_message_c (socket, req);
			break; 
		case RSP_DATA:
			proc_message_c (socket, req);
			break;
	//	default:
	//		return 1;
	}

	return 0;
}


int proc_message_s (int conn_fd, int message_type) {

	FILE * fd_to_send;
	long size_of_file = 0;

	if ((fd_to_send = fopen ("file.txt", "r")) == NULL) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;
	}

	fseek (fd_to_send, 0, SEEK_END);
	size_of_file = ftell(fd_to_send);
	rewind (fd_to_send);
	
	if (message_type == REQ_INFO) {

		struct msg * file_info = malloc(sizeof(struct msg));

		file_info->type = RSP_INFO;
		file_info->flags = 0;
		file_info->data_len = size_of_file;


		printf ("SERVER, RESP_INFO MESSAGE \n"); 
		printf("type: %hhd\n", file_info->type); 
		printf("flags: %hhd\n", file_info->flags); 
		printf("data length: %d\n", file_info->data_len); 
		printf ("data: %s \n \n", file_info->data);


		send_message (conn_fd, file_info);
		free (file_info);
	}

	if (message_type == REQ_DATA) {
	
		struct msg * file = malloc(sizeof(struct msg) + MAX_SYM);
		char buff [MAX_SYM] = {0};
		char req_buff [10] = {0};

		file->type = RSP_DATA;
		file->flags = 0;
		//file->data_len = MAX_SYM;

		while (!feof(fd_to_send)) {
			file->data_len = fread (buff, 1, MAX_SYM, fd_to_send);
			memcpy (file->data, buff, MAX_SYM);

			printf ("SERVER, RESP_DATA MESSAGE \n"); 
			printf("type: %hhd\n", file->type); 
			printf("flags: %hhd\n", file->flags); 
			printf("data length: %d\n", file->data_len); 
			printf ("data: %s \n \n", file->data);
			send_message (conn_fd, file);
			recv_message_s (conn_fd, req_buff);

			struct msg * request = malloc(sizeof(struct msg));
			request = (struct msg *) req_buff;

			if ((request->type != 5) && (request->data_len != file->data_len))
				printf ("MEOW! \n");
			memset (file->data, 0, MAX_SYM);
			memset (buff, 0, MAX_SYM);
			
		}

		free (file);
	}

	fclose (fd_to_send);

	return 0;
}

int proc_message_c (int sock, struct msg * msg) {

	printf("%s(), line %d\n", __func__, __LINE__);

	if (msg->type == RSP_INFO) {

		struct msg * file_req = malloc (sizeof(struct msg));
		
		file_req->type = REQ_DATA;
		file_req->flags = 0;
		file_req->data_len = 0;

		printf ("CLIENT, REQ_DATA MESSAGE \n"); 
		printf("type: %hhd\n", file_req->type); 
		printf("flags: %hhd\n", file_req->flags); 
		printf("data length: %d\n", file_req->data_len); 
		printf ("data: %s \n \n", file_req->data);

	//	memcpy(file_req->data, msg->data, msg->data_len);
	//	printf("REQ_DATA: data_len = %d\n", msg->data_len);
		char * msg_buff = (char *) malloc (sizeof(char) * (msg->data_len));
		if (msg_buff == NULL) {
			printf ("ERROR: Memory allocation failed! \n");
			return 1;
		} 

		send_message (sock, file_req);
	} 

	if (msg->type == RSP_DATA) {
		FILE * file_fd;
		file_fd = fopen ("myfile.txt", "a+");
		if (file_fd != NULL) {
			fputs (msg->data, file_fd);
			fclose (file_fd);
		} 
	}
	return 0;
}
