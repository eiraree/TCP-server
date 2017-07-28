#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef DEBUG_MODE
	#include "functions.h"
#else 
	#include "libsc.h"
#endif

int main () {

	int sock = 0;
	char file_info_msg [BUFF_LEN] = {0};
	int counter = 0;
	struct info client;

	if (read_info_c (&client) != NULL) {
		sock = create_connection_c (&client);

		const char * file_name = "file.txt";
		int file_name_len = strlen(file_name);

		struct msg * file_req = malloc(sizeof(struct msg) + file_name_len);

		file_req->type = REQ_INFO;
		file_req->flags = 0;
		file_req->data_len = file_name_len;
		memcpy(file_req->data, file_name, file_name_len);

		send_message (sock, file_req);
		free(file_req);

		recv_message_c (sock, file_info_msg);
		struct msg * rsp = (struct msg * ) file_info_msg;
		counter = rsp->data_len;

		while (counter > 0) {
			recv_message_c (sock, file_info_msg);
			rsp = (struct msg * ) file_info_msg;
			
			struct msg * request = malloc(sizeof(struct msg));
			request->type = 4;
			request->flags = 0;
			request->data_len = rsp->data_len;
			send_message (sock, request);
			free (request);
			counter -= (rsp->data_len);
			printf ("COUNTER: %d \n", counter);
			memset (file_info_msg, 0, BUFF_LEN);

		}

	} else {
		return 1;
	}

	close_socket (sock);

	return 0;
}
