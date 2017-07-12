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

	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1 ) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;
	}

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


int send_message (int sock, struct info * client) {

	int res = 0;

	if ( (res = send (sock, (void *) (client->message), strlen (client->message), 0)) != strlen(client->message)) {
#ifdef DEBUG_INFO
		printf ("-----> \n"); 
#endif
	} else if (res == -1) {
		printf ( "%s(), %d: called\n", __func__, __LINE__);
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


int accept_conn (int sock, int conn_fd) {
	

	if ((conn_fd = accept (sock, NULL, NULL)) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;	
	}
	return conn_fd;
}


int recv_message (int sock, char * buff) {

	if (recv (sock, buff, BUFF_LEN, 0) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;
	}

	printf ("%s \n", buff);
	
	return 0; 
}


int send_file_s (int conn_fd) {
	
	FILE * fd_to_send;
	long size_of_file = 0;
	char file_info_msg [24];
	
	if ((fd_to_send = fopen ("file.jpg", "r")) == NULL) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;
	}

	fseek (fd_to_send, 0, SEEK_END);
	size_of_file = ftell(fd_to_send);

	char * buff = (char *) malloc (sizeof(char) * size_of_file);

	if (buff == NULL) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return -1;
	}

	rewind (fd_to_send);
	fread (buff, 1, size_of_file, fd_to_send);

	memset (file_info_msg, 0, 24);
	sprintf (file_info_msg, "%ld;%s", size_of_file, "file.jpg");

	printf ("%s \n", file_info_msg);

	if (send (conn_fd, file_info_msg, strlen(file_info_msg), 0) == -1) {
		printf("%s(), %d: error!\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
	}

	fclose (fd_to_send);

	return 0;

}


int create_connection_c (struct info * client) {
	
	int sock = -1;

	if ((sock = create_socket ()) != -1)
		connect_socket(sock, client);
	
	return sock;
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


int send_message_c (int sock, struct info * client) {

	send_message (sock, client);
	close_socket(sock);

	return 0;
}


int recv_message_s (int sock, int conn_fd, char * buff) {

	if (buff == NULL) {
		return -1;
	} else {
		recv_message (conn_fd, buff);
	}
	close_socket(sock);
	close_socket(conn_fd);

	return 0;
}
