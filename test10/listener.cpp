#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <string.h>
#include "./proto.h"


int createTCPListener(int nPort, int nLengthOfQueueOfListen = 50, char * strBoundIP) {
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener == -1)
		return -1;

	sockaddr_in localAddress;
	memset(&localAddress, 0, sizeof(sockaddr_in));

	localAddress.sin_family = AF_INET;
	if(strBoundIP == NULL)
		localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	else {
		if(::inet_pton(AF_INET, strBoundIP, &localAddress.sin_addr) != 1) {
			::close(listener);
			return -1;
		}
	}

	localAddress.sin_port = htons(nPort);

	if(::bind(listener, (sockaddr *)&localAddress, sizeof(sockaddr_in)) == -1) {
		::close(listener);
		return -1;
	}

	if(::listen(listener, nLengthOfQueueOfListen) == -1) {
		::close(listener);
		return -1;
	}

	return listener;

}


int handleListen(int listener) {
	if(listener == -1)
		return -1;

	sockaddr_in remoteAddress;
	socklen_t remoteLen;

	int newConnectedSocket = ::accept(listener, (sockaddr *)&remoteAddress, &remoteLen);

	if(newConnectedSocket == -1) {
		::close()
	}
}
