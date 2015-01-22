#ifndef INITIATESOCKET_H
#define INITIATESOCKET_H

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>

int initiateTCPListener(int nPort, int nLengthOfQueueOfListen, char * strBoundIP);
int initiateUDPSocket(int nPort, char *strBoundIP);
int initiateTCPClient(sockaddr_in TCPServerAddress);

int initiateTCPClient(sockaddr_in TCPServerAddress) {
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket == -1)
		return -1;
	if(connect(clientSocket, (sockaddr *)&TCPServerAddress, sizeof(sockaddr_in)) == -1) {
		close(clientSocket);
		return -1;
	}
	return clientSocket;
}

int initiateTCPListener(int nPort, int nLengthOfQueueOfListen, char * strBoundIP) {
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

int initiateUDPSocket(int nPort, char *strBoundIP) {
	int sock;
	sock = ::socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
		return -1;
	
	sockaddr_in sockAddress;
	memset(&sockAddress, 0, sizeof(sockaddr_in));
	sockAddress.sin_family = AF_INET;
	if(strBoundIP == NULL)
		sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	else{
		if(::inet_pton(AF_INET, strBoundIP, &sockAddress.sin_addr) != 1)
			return -1;
	}
	sockAddress.sin_port = htons(nPort);

	if(::bind(sock, (sockaddr *)&sockAddress, sizeof(sockaddr_in)) == -1)
		return -1;
	return sock;
}
#endif
