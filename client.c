#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef DEBUG_MODE
	#include "functions.h"
#else 
	#include "libsc.h"
#endif

int main () {

	int sock = 0;
	int size_of_msg = 0;
	char file_info_msg [24];
	struct info client;

	if (read_info_c (&client) != NULL) {
		sock = create_connection_c (&client); 
		send_message (sock, &client);

		memset (file_info_msg, 0, 24);
		recv_message (sock, file_info_msg);

		char * ptr = strtok (file_info_msg, ";");
		if (ptr != NULL) {
			if ((size_of_msg = atoi (ptr)) != 0) {
				char * file_buff = (char *) malloc (sizeof (char) * size_of_msg);
				if (file_buff == NULL) {
					printf ("%s(), %d: called\n", __func__, __LINE__);
					printf ("ERROR: %d (%s)!\n", errno, strerror(errno));
					return -1;
				}
			}
		}

		ptr = strtok (NULL, ";");
		printf ("%s \n", ptr);

	} else {
		return 1;
	}

	close_socket (sock);

	return 0;
}
