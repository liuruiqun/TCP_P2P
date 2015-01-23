#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <string.h>
#include <pthread.h>
#include <list>
#include "./proto.h"
#include "./initiateSocket.h"
#include "./tools.h"

using std::list;

typedef struct {
	list<sUserInfo_t> *onlineList_ptr;
	pthread_mutex_t *mutex_ptr;
	int sock;
} sHandleParams_t;



int loginHandle(int socket, sMessage_t *rMsg_ptr, sMessage_t *sMsg_ptr, pthread_mutex_t *mutex, list<sUserInfo_t> *list_ptr, char *clientName) {
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

	if(send(socket, sMsg_ptr, sizeof(sMessage_t), 0) != sizeof(sMessage_t)) {
		close(socket);
		return -1;
	}
	if(sMsg_ptr->msgType == LOGSUCCESS) {
		printf("%s login.\n", (rMsg_ptr->userInfo).userName);
		strcpy(clientName, (rMsg_ptr->userInfo).userName);
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
	if(it != list_ptr->end()) {
		list_ptr->erase(it);
		printf("%s logout.\n", (rMsg_ptr->userInfo).userName);
	}
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

	if(send(socket, sMsg_ptr, sizeof(sMessage_t), 0) != sizeof(sMessage_t)) {
		close(socket);
		return -1;
	}
	
	list<sUserInfo_t>::iterator it;
	for(it = userList_copy.begin(); it != userList_copy.end(); it++) {
		sMsg_ptr->msgType = JUSTUSERINFO;
		(sMsg_ptr->userInfo) = (*it);
		if(send(socket, sMsg_ptr, sizeof(sMessage_t), 0) != sizeof(sMessage_t)) {
			close(socket);
			return -1;
		}

	}

	return 0;
}


void * handleConnectedSocket(void *params) {
	sHandleParams_t *ptr = (sHandleParams_t *)params;
	//在线程退出之前一定要free掉ptr指向的heap上的内存空间，不然会造成内存泄露
	if(ptr == NULL)
		return (void*)-1;

	if(ptr->sock == -1) {
		delete ptr;
		return (void*)-1;
	}

	if(ptr->mutex_ptr == NULL || ptr->onlineList_ptr == NULL) {
		close(ptr->sock);
		delete ptr;
		return (void*)-1;
	}

	FILE* inStream = ::fdopen(ptr->sock, "r");
	if(inStream == NULL) {
		close(ptr->sock);
		delete ptr;
		return (void*)-1;
	}
	
	pthread_detach(pthread_self());

	char clientName[MAX_NAME_LEN + 1] = "";


	while(true) {
		sMessage_t sMsg;
		sMessage_t rMsg;

		memset(&sMsg, 0, sizeof(sMessage_t));
		memset(&rMsg, 0, sizeof(sMessage_t));

		size_t readNum = ::fread(&rMsg, sizeof(sMessage_t), 1, inStream);
		//NOTE: when the peer socket close ，furthermore there is nothing in RecvQue, the recv routine returns 0;
		
		if(readNum == 1) {
			switch(rMsg.msgType) {
				case LOGIN :
					if(loginHandle(ptr->sock, &rMsg, &sMsg, ptr->mutex_ptr, ptr->onlineList_ptr, clientName) == -1) {
						delete ptr;
						fclose(inStream);
						return (void*)-1;
					}
					break;
				case LOGOUT :
					if(logoutHandle(&rMsg, ptr->mutex_ptr, ptr->onlineList_ptr) == -1) {
						close(ptr->sock);
						delete ptr;
						fclose(inStream);
						return (void*)-1;
					}
					break;
				case GETALLUSER :
					if(getAllUserHandle(ptr->sock, &sMsg, ptr->mutex_ptr, ptr->onlineList_ptr) == -1) {
						delete ptr;
						fclose(inStream);
						return (void*)-1;
					}
					break;
				default :
					break;
			}
		}
		else
		{	
			if(strlen(clientName) > 0) {
				//当用户异常退出时的处理
				strcpy(rMsg.userInfo.userName, clientName);
				logoutHandle(&rMsg, ptr->mutex_ptr, ptr->onlineList_ptr); 
			}
			close(ptr->sock);
			delete ptr;
			fclose(inStream);
			return (void*)-1;
		}

	}

	close(ptr->sock);
	delete ptr;
	fclose(inStream);
	return NULL;
}


int main() {

	list<sUserInfo_t> onlineList;
	
	int listener = initiateTCPListener(SERVER_PORT, 50, "127.0.0.1");

	if(listener == -1)
		return -1;

	printf("SERVER SRARTED\n");

	pthread_mutex_t mutex;   //互斥量用于保护对在线列表的访问
	pthread_mutex_init(&mutex, 0);

	sockaddr_in clientAddr;
	socklen_t clientAddrLen;


	while(true) {

		memset(&clientAddr, 0, sizeof(sockaddr_in));
		clientAddrLen = sizeof(sockaddr_in);
		//NOTE: initiate the clientAddrLen before call the accept routine.
		int newConnectedSocket = accept(listener, (sockaddr *)&clientAddr, &clientAddrLen);
		if(newConnectedSocket == -1) {
			printf("ACCEPT ERROR\n");
			close(listener);
			return -1;
		}

		
		printf("ESTABLISHED A NEW CONNECT\n");

		sHandleParams_t * param_ptr = new sHandleParams_t;
		if(param_ptr == NULL) {
			close(newConnectedSocket);
			close(listener);
			printf("NEW EEROR\n");
			return -1;
		}
		
		param_ptr->mutex_ptr = &mutex;
		param_ptr->onlineList_ptr = &onlineList;
		param_ptr->sock = newConnectedSocket;
		//创建新线程处理新建立的连接
		pthread_t tid;
		if(pthread_create(&tid, NULL, handleConnectedSocket, param_ptr) != 0){
			delete param_ptr;
			close(newConnectedSocket);
			close(listener);
			printf("CREATE THREAD ERROR");
			return -1;
		}
	}

	close(listener);
	printf("SAFE EXIST");
	return 0;
}
