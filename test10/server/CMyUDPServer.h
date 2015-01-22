#ifndef CMYUDPSERVER_H
#define CMYUDPSERVER_H


#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "./proto.h"
#include "./CUDPSocket.h"
#include <memory.h>
#include <list>

using std::list;

class CMyUDPServer {
	public:
		CMyUDPServer() { }
		virtual ~CMyUDPServer() { }

		int ServerFunction(int nUDPServerSocket) {
			sMessage_t rMsg;
			sMessage_t sMsg;
			sockaddr_in clientAddress;
			socklen_t clientAddressLen;

			for(; ;) {

				memset(&clientAddress, 0 , sizeof(sockaddr_in));

				if(::recvfrom(nUDPServerSocket,
							   	&rMsg, 
								sizeof(sMessage_t),
								0,
							(sockaddr*)&clientAddress,
								&clientAddressLen)	== -1)
						return -1;
				
				memset(&sMsg, 0, sizeof(sMessage_t));
				
				switch(rMsg.msgType) {
					case LOGIN:
						loginHandle(nUDPServerSocket, &rMsg, &sMsg, &clientAddress);
						break;
					case LOGOUT:
						logoutHandle(&rMsg);
						break;
					case GETALLUSER:
						getAllUserHandle(nUDPServerSocket, &sMsg, &clientAddress);
						break;
					case LOGFAIL:
					case LOGSUCCESS:
					case JUSTUSERINFO:
						break;
					default:
						break;
				}

			}
		}

		bool isUserNameExisted(const char *name) {
			list<sUserInfo_t>::iterator it;
			for(it = m_userList.begin(); it != m_userList.end(); it++) {
				if(strcmp(it->userName, name) == 0)
					return true;
			}

			return false;
		}
		
		list<sUserInfo_t>::iterator getIterator(const char *name) {
			list<sUserInfo_t>::iterator it;
			for(it = m_userList.begin(); it != m_userList.end(); it++) {
				if(strcmp(it->userName, name) == 0)
					break;
			}
			return it;
		}
		
		int loginHandle(int sock, const sMessage_t *rMsg_ptr, sMessage_t *sMsg_ptr, const sockaddr_in *peerAddress) {

			if(sock == -1)
				return -1;

			if(isUserNameExisted(rMsg_ptr->userInfo.userName))
				sMsg_ptr->msgType = LOGFAIL;
			else {
				sMsg_ptr->msgType = LOGSUCCESS;
				m_userList.push_back(rMsg_ptr->userInfo);
			}

			if(::sendto(sock, sMsg_ptr, sizeof(sMessage_t), 0, (sockaddr *)peerAddress, sizeof(sockaddr_in)) == -1)
				return -1;

			return 0;
		}

		int logoutHandle(const sMessage_t *rMsg_ptr) {
			
			list<sUserInfo_t>::iterator it;
			it = getIterator((rMsg_ptr->userInfo).userName);

			if(it != m_userList.end())
				m_userList.erase(it);

			return 0;
		}

		int getAllUserHandle(int sock, sMessage_t *sMsg_ptr, const sockaddr_in *peerAddress) {
			if(sock == -1)
				return -1;
			sMsg_ptr->msgType = JUSTUSERINFO;
			list<sUserInfo_t>::iterator it;
			for(it = m_userList.begin(); it != m_userList.end(); it++) {
				sMsg_ptr->userInfo = *it;
				::sendto(sock, sMsg_ptr, sizeof(sMessage_t), 0, (sockaddr *)peerAddress, sizeof(sockaddr_in));
			}

			return 0;
		}

	private:
		 list<sUserInfo_t> m_userList;
};

#endif
