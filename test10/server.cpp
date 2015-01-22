#include "./CUDPSocket.h"
#include "./proto.h"
#include "./CMyUDPServer.h"

int main() {
	unsigned short port = SERVER_PORT;
	CUDPSocket<CMyUDPServer> myServer(port, "127.0.0.1");

	myServer.Run();

	return 0;

}
