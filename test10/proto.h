#ifndef PROTO_H
#define PROTO_H
#include "./CLogger.h"
#include <pthread.h>
#include <stdio.h>
#include <list>
#include <cdk.h>
#define SERVER_PORT 4430

#define USER_LISTEN_PORT 4460
#define LISTEN_QUEUE_LEN 40
#define MAX_NAME_LEN 63

using std::list;

enum MsgType_t  {

	//below are message type definition.
   	LOGIN = 0,
   	LOGOUT = 1,
   	GETALLUSER = 2,
	LOGFAIL = 3,
	LOGSUCCESS = 4,
	NUMOFUSER = 5,
	JUSTUSERINFO = 6,

	
	STARTTCPCHAT = 7,
	ENDTCPCHAT = 8,
	AGREETOCHAT = 9,
	DISAGREETOCHAT = 10,
	DELIMITER = 11
};


//The delimiter of every TCP chat message.
typedef struct {
	MsgType_t msgType;    //delimiter or ENDTCPCHAT
	unsigned int length;
}sChatControlMessage_t;


//user information
typedef struct {
	char userName[MAX_NAME_LEN + 1];
	unsigned int ip;
	unsigned short port;
} sUserInfo_t;


typedef struct {
	MsgType_t msgType;
	sUserInfo_t userInfo;
} sMessage_t;

typedef struct {
	CDKSCREEN *cdkscreen;
	CDKMENTRY *inputWidget;
	CDKSWINDOW *displayWidget;
	CDKSCROLL *onlineListWidget;
	CDKSCROLL *currentChatingListWidget;
	CDKSCROLL *requestListWidget;
}sMainPage_t;

typedef struct {
	char peerName[MAX_NAME_LEN + 1];
	int sock;
	FILE *sockInStream;
	CLogger *logger_ptr;
	pthread_mutex_t *logger_mutex;
} sChatNode_t;

typedef struct {
	char userName[MAX_NAME_LEN + 1]; //local name. //in
	char peerName[MAX_NAME_LEN + 1];
	sChatNode_t *activateChatNode_ptr;  //in and out
	CLogger *logger_ptr;
	list<sUserInfo_t> *onlineList_ptr; //in
	list<sChatNode_t> *chatingList_ptr;//in
	list<sChatNode_t> *requestList_ptr;//in
	sMainPage_t *main_page_ptr;//in
	int TCPClientSocket;   //in
	FILE *TCPClientStream;   //in
	int newLocalSocket;
	FILE *newLocalSocketStream;
	pthread_mutex_t *chatingListWidget_mutex; //in
	pthread_mutex_t *displayWidget_mutex; //in
	pthread_mutex_t *requestListWidget_mutex;//in
	pthread_mutex_t *logger_mutex;
	pthread_mutex_t *activateChatNode_mutex;	
} sChatResources_t;

typedef struct {
	int listener;  //listen socket
	CDKSCROLL *requestListWidget;
	list<sChatNode_t> *requestList_ptr;
	pthread_mutex_t *requestListWidget_mutex;
} sRequestParams_t;

typedef struct {
	sChatNode_t *activateChatNode_ptr;
	pthread_mutex_t *activateChatNode_mutex;
	CDKSCREEN *cdkscreen;
	CDKSWINDOW *displayWidget;
	pthread_mutex_t *displayWidget_mutex;
} sDisplayWidgetParams_t;

typedef struct {
	CDKSCROLL *onlineListWidget;
	list<sUserInfo_t> *onlineList_ptr;
	int TCPClientSocket;
	FILE *TCPClientStream;
} sRefreshParams_t;

typedef struct {
	char userName[MAX_NAME_LEN + 1];
	sMainPage_t *main_page_ptr;
	sChatNode_t *activateChatNode_ptr;
	pthread_mutex_t *activateChatNode_mutex;
	pthread_mutex_t *displayWidget_mutex;
} sInputWidgetParams_t; 
#endif
