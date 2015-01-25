#ifndef HANDLEACCEPT_H
#define HANDLEACCEPT_H

#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <list>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./proto.h"
#include "./tools.h"

using std::list;

void *handleAccept(void *params) {
	sRequestParams_t *ptr = (sRequestParams_t *)params;

	if(ptr == NULL)
		return (void *)-1;

	if(ptr->listener == -1)
		return (void *)-1;
	if(ptr->requestListWidget == NULL || ptr->requestList_ptr == NULL || 
			ptr->requestListWidget_mutex == NULL) {
		close(ptr->listener);
		return (void *)-1;
	}

	sockaddr_in peerAddr;
	socklen_t peerAddrLen;
	while(true) {

		peerAddrLen = sizeof(sockaddr_in);
		int newLocalSocket = accept(ptr->listener, (sockaddr *)&peerAddr, &peerAddrLen);
		if(newLocalSocket == -1) {
			close(ptr->listener);
			return (void *)-1;
		}

		FILE *newLocalSocketStream = ::fdopen(newLocalSocket, "r");
		if(newLocalSocketStream == NULL) {
			close(newLocalSocket);
			continue;
		}

		sMessage_t rMsg;
		if(::fread(&rMsg, sizeof(sMessage_t), 1, newLocalSocketStream) != 1) {
			fclose(newLocalSocketStream);
			close(newLocalSocket);
			continue;
		}

		if(rMsg.msgType != STARTTCPCHAT) {
			fclose(newLocalSocketStream);
			close(newLocalSocket);
			continue;
		}

		sChatNode_t requestNode;
		requestNode.sock = newLocalSocket;
		requestNode.sockInStream = newLocalSocketStream;
		strcpy(requestNode.peerName, rMsg.userInfo.userName);

		pthread_mutex_lock(ptr->requestListWidget_mutex);
		(ptr->requestList_ptr)->push_back(requestNode);
		flushChatingListWidget(ptr->requestListWidget, ptr->requestList_ptr);
		pthread_mutex_unlock(ptr->requestListWidget_mutex);

	}

	close(ptr->listener);
	return NULL;
}

#endif
