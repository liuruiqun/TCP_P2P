#ifndef INITIATESOCKET_H
#define INITIATESOCKET_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include "./proto.h"

int initiateTCPListener(int nLengthOfQueueOfListen, sUserInfo_t *userInfo_ptr);
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

int initiateTCPListener(int nLengthOfQueueOfListen, sUserInfo_t *userInfo_ptr) {
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener == -1)
		return -1;

	sockaddr_in listenerAddr;
	memset(&listenerAddr, 0, sizeof(sockaddr_in));

	listenerAddr.sin_family = AF_INET;
	if(::inet_pton(AF_INET, "127.0.0.1", &listenerAddr.sin_addr.s_addr) != 1) {
		close(listener);
		return -1;
	}
	listenerAddr.sin_port = htons(0);
	if(::bind(listener, (sockaddr *)&listenerAddr, sizeof(sockaddr_in)) == -1) {
		close(listener);
		return -1;
	}

	if(::listen(listener, nLengthOfQueueOfListen) == -1) {
		::close(listener);
		return -1;
	}

	sockaddr_in localListenSocketAddr;
	socklen_t localListenSocketAddrLen = sizeof(sockaddr_in);

	if(::getsockname(listener, (sockaddr *)&localListenSocketAddr, &localListenSocketAddrLen) == -1) {
		close(listener);
		return -1;
	}

	userInfo_ptr->ip = ntohl(localListenSocketAddr.sin_addr.s_addr);
	userInfo_ptr->port = ntohs(localListenSocketAddr.sin_port);
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
