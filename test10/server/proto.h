#ifndef PROTO_H
#define PROTO_H

#define SERVER_PORT 4430
#define MAX_NAME_LEN 63

enum MsgType_t  {
   	LOGIN = 0,
   	LOGOUT = 1,
   	GETALLUSER = 2,
	LOGFAIL = 3,
	LOGSUCCESS = 4,
	JUSTUSERINFO = 5
};

typedef struct {
	char userName[MAX_NAME_LEN + 1];
	unsigned int ip;
	unsigned short port;
} sUserInfo_t;

typedef struct {
	MsgType_t msgType;
	sUserInfo_t userInfo;
} sMessage_t;


#endif
