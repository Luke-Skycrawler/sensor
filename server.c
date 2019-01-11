#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "acceleration.h"

void die(char *err){
	printf("%s\n", err);
	exit(1);
}


int main(int argv, char *argc[] ){
	int sock = socket(AF_INET, SOCK_STREAM, 0 );
	struct accelerator_data acc_data;
	struct sockaddr_in addr;
	struct sockaddr_in phone;

	if ( sock < 0 )
		die("socket");

	memset( &addr, 0, sizeof(addr) );
	memset( &phone, 0, sizeof(phone) );
	memset( &acc_data, 0, sizeof(acc_data) );

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons( 9000 );

	int reuse = 1;
	setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	if( bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0 )
		die("bind");

	if( listen(sock, 1 ) < 0)
		die("listen");

	unsigned int len = sizeof(phone);
	int client = accept( sock, (struct sockaddr *)&phone, &len);
	int msg = 0;

	printf("[ok] connected to :%s\n", inet_ntoa(phone.sin_addr));
	float proximity = 0;

	for(;;){
		msg = recv( client, &acc_data, sizeof(acc_data), 0 );
		if( msg < sizeof(acc_data) || (msg == -1))
			break;
		printf("acceleration : X=%f Y=%f Z=%f\n", acc_data.x, acc_data.y, acc_data.z );
//		printf("proximity : %f\n", proximity );
	}

	close(client);
	close(sock);
	return 0;
}







