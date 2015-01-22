#include "./CUDPSocket.h"
#include "./proto.h"
#include "./CMyUDPServer.h"

int main() {
	unsigned short port = SERVER_PORT;
	CUDPSocket<CMyUDPServer> myServer(port);

	myServer.Run();

	return 0;

}
