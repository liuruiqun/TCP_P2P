#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
	int i;

	::inet_pton(AF_INET, "127.0.0.1", &i);

	printf("%d.\n", ntohl(i));

	return 0;
}
