#ifndef PROTO_H
#define PROTO_H

#define SERVER_PORT 4430

#define USER_LISTEN_PORT 4460
#define LISTEN_QUEUE_LEN 40
#define MAX_NAME_LEN 63

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
	DISAGREETOCHAT = 10
};


//The delimiter of every TCP chat message.
typedef struct {
	unsigned int length;
}sDelimit_t;


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


#endif
