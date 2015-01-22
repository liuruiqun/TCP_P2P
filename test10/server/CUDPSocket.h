#ifndef CUDPSOCKET_H
#define CUDPSOCKET_H

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "./proto.h"
#include <memory.h>


template<typename ConnectionProcessor>
class CUDPSocket : public ConnectionProcessor {
	public:
		CUDPSocket(int nPort, const char* strBoundIP = NULL) {
			m_nPort = nPort;

			if(strBoundIP == NULL)
				m_strBoundIP = NULL;
			else {
				int length = strlen(strBoundIP);
				m_strBoundIP = new char[length + 1];
				strcpy(m_strBoundIP, strBoundIP);
			}
		}

		virtual ~CUDPSocket() {
			if(m_strBoundIP != NULL)
				delete [] m_strBoundIP;
		}

	public:
		int Run() {
			int UDPSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
			if(UDPSocket == -1)
				return -1;
			sockaddr_in SockAddress;
			memset(&SockAddress, 0, sizeof(sockaddr_in));
			SockAddress.sin_family = AF_INET;

			if(m_strBoundIP == NULL)
				SockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
			else {
				if(::inet_pton(AF_INET, m_strBoundIP, &SockAddress.sin_addr) != 1)
					return -1;
			}

			SockAddress.sin_port = htons(m_nPort);

			if(::bind(UDPSocket, (sockaddr *)&SockAddress, sizeof(sockaddr_in)) == -1)
				return -1;

			ConnectionProcessor *pProcessor = static_cast<ConnectionProcessor *>(this);

			pProcessor->ServerFunction(UDPSocket);

			::close(UDPSocket);	
			return 0;
		}
	private:
		int m_nPort;
		char* m_strBoundIP;
};
#endif
