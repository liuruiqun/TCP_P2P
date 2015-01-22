#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <string.h>
#include <pthread.h>
#include <list>
#include "./proto.h"
#include "./initiateSocket.h"

using std::list;

typedef struct {
	list<sUserInfo_t> *onlineList_ptr;
	pthread_mutex_t *mutex_ptr;
	int sock;
} handleParams;

bool isUserNameExisted(list<sUserInfo_t> *list_ptr, char * userName) {
	list<sUserInfo_t>::iterator it;
	for(it = list_ptr->begin(); it != list_ptr->end(); it++) {
		if(strcmp(it->userName, userName) == 0)
			return true;
	}

	return false;
}

list<sUserInfo_t>::iterator getIterator(list<sUserInfo_t> *list_ptr, char * userName) {
	list<sUserInfo_t>::iterator it;
	for(it = list_ptr->begin(); it != list_ptr->end(); it++) {
		if(strcmp(it->userName, userName) == 0)
			break;
	}
	return it;
}



int loginHandle(int socket, sMessage_t *rMsg_ptr, sMessage_t *sMsg_ptr, pthread_mutex_t *mutex, list<sUserInfo_t> *list_ptr) {
	if(socket == -1)
		return -1;
	if(rMsg_ptr == NULL || sMsg_ptr == NULL ||
			mutex == NULL || list_ptr == NULL){
		close(socket);
		return -1;
	}

	pthread_mutex_lock(mutex);
	if(isUserNameExisted(list_ptr, (rMsg_ptr->userInfo).userName))
		sMsg_ptr->msgType = LOGFAIL;
	else {
		sMsg_ptr->msgType = LOGSUCCESS;
		list_ptr->push_back(rMsg_ptr->userInfo);
	}
	pthread_mutex_unlock(mutex);

	if(send(socket, sMsg_ptr, sizeof(sMessage_t), 0) == -1) {
		close(socket);
		return -1;
	}

	return 0;
	
}

int logoutHandle(sMessage_t *rMsg_ptr, pthread_mutex_t *mutex, list<sUserInfo_t> *list_ptr) {
	if(rMsg_ptr == NULL || mutex == NULL || list_ptr == NULL) {
		return -1;
	}
	list<sUserInfo_t>::iterator it;
	pthread_mutex_lock(mutex);
	it = getIterator(list_ptr, (rMsg_ptr->userInfo).userName);
	if(it != list_ptr->end())
		list_ptr->erase(it);
	pthread_mutex_unlock(mutex);

	return 0;

}


int getAllUserHandle(int socket, sMessage_t *sMsg_ptr, pthread_mutex_t *mutex, list<sUserInfo_t> *list_ptr) {
	if(socket == -1)
		return -1;
	if(sMsg_ptr == NULL || mutex == NULL || list_ptr == NULL) {
		close(socket);
		return -1;
	}
	int userNum;
	pthread_mutex_lock(mutex);
	list<sUserInfo_t> userList_copy(*list_ptr);
	pthread_mutex_unlock(mutex);

	userNum = userList_copy.size();
	sMsg_ptr->msgType = NUMOFUSER;
	(sMsg_ptr->userInfo).ip = userNum;

	if(send(socket, sMsg_ptr, sizeof(sMessage_t), 0) == -1) {
		close(socket);
		return -1;
	}
	
	list<sUserInfo_t>::iterator it;
	for(it = userList_copy.begin(); it != userList_copy.end(); it++) {
		sMsg_ptr->msgType = JUSTUSERINFO;
		(sMsg_ptr->userInfo) = (*it);
		if(send(socket, sMsg_ptr, sizeof(sMessage_t), 0) == -1) {
			close(socket);
			return -1;
		}

	}

	return 0;
}


void * handleConnectedSocket(void *params) {
	handleParams *ptr = (handleParams *)params;
	if(ptr == NULL)
		return (void*)-1;
	if(ptr->sock == -1)
		return (void*)-1;
	if(ptr->mutex_ptr == NULL || ptr->onlineList_ptr == NULL) {
		close(ptr->sock);
		return (void*)-1;
	}
	
	pthread_detach(pthread_self());

	while(true) {
		sMessage_t sMsg;
		sMessage_t rMsg;

		memset(&sMsg, 0, sizeof(sMessage_t));

		if(::recv(ptr->sock, &rMsg, sizeof(sMessage_t), 0) == -1)
		{
			close(ptr->sock);
			return (void*)-1;
		}

		switch(rMsg.msgType) {
			case LOGIN :
				if(loginHandle(ptr->sock, &rMsg, &sMsg, ptr->mutex_ptr, ptr->onlineList_ptr) == -1)
					return (void*)-1;
				break;
			case LOGOUT :
				if(logoutHandle(&rMsg, ptr->mutex_ptr, ptr->onlineList_ptr) == -1)
					return (void*)-1;
				break;
			case GETALLUSER :
				if(getAllUserHandle(ptr->sock, &sMsg, ptr->mutex_ptr, ptr->onlineList_ptr) == -1)
					return (void*)-1;
				break;
			default :
				break;
		}
	}

	close(ptr->sock);
	return NULL;
}


int main() {

	list<sUserInfo_t> onlineList;
	
	int listener = initiateTCPListener(SERVER_PORT, 50, NULL);

	if(listener == -1)
		return -1;

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, 0);

	while(true) {
		sockaddr_in clientAddr;
		socklen_t clientAddrLen;
		int newConnectedSocket = accept(listener, (sockaddr *)&clientAddr, &clientAddrLen);
		if(newConnectedSocket == -1) {
			close(listener);
			return -1;
		}

		handleParams param;
		param.mutex_ptr = &mutex;
		param.onlineList_ptr = &onlineList;
		param.sock = newConnectedSocket;

		pthread_t tid;
		if(pthread_create(&tid, NULL, handleConnectedSocket, &param) != 0){
			close(newConnectedSocket);
			close(listener);
			return -1;
		}
	}

	close(listener);
	return 0;
}




