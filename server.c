#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERV_PORT 3838
#define MAX_CONNECT 5

int create_socket (int sock) {

	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1 ) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return 1;
	}

	printf ("Socket is created.. \n");
	return sock;
}

void * bind_socket (int sock, struct sockaddr_in * sll) {

	struct sockaddr_in * socket_data = sll;

	memset (socket_data, 0, sizeof(*socket_data));

	socket_data->sin_family = AF_INET;
	socket_data->sin_port = htons(SERV_PORT);
	socket_data->sin_addr.s_addr = htonl(INADDR_ANY); 

	if (bind (sock, (struct sockaddr *) socket_data, sizeof(*socket_data)) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		exit (EXIT_FAILURE);
	}

	printf ("Binded successfully.. \n");
	return (void *) socket_data;	
}


int listen_socket (int sock) {
	
	if (listen (sock, MAX_CONNECT) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return 1;	
	}

	printf ("Listening.. \n");
	return 0;
}

int accept_conn (int sock) {
	
	if (accept (sock, NULL, NULL) == -1) {
		printf ("%s(), %d: called\n", __func__, __LINE__);
		printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
		return 1;	
	}

	return 0;
}


int main () {

	int sock = 0;
	struct sockaddr_in socket_data;

	sock = create_socket (sock);
	bind_socket (sock, &socket_data);
	listen_socket (sock);
	accept_conn (sock);

	return 0;
}
