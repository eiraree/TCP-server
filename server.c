#include <stdio.h>

#ifdef DEBUG_MODE
	#include "functions.h"
#else 
	#include "libsc.h"
#endif

int main () {

	int sock = 0;
	int conn_fd = 0;
	char buff[BUFF_LEN] = {0};
	struct sockaddr_in socket_data;

	sock = create_connection_s (&socket_data); 
	if ((conn_fd = accept_conn (sock, conn_fd)) != -1) {
		recv_message_s (conn_fd, buff);
		recv_message_s (conn_fd, buff);
	} else {
		return 1;
	}

	close_socket (sock);
	close_socket (conn_fd);
	return 0;
}
