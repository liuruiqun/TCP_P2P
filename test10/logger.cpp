#include "./proto.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#define MAX_BUFFER_SIZE 1024

class CLogger {
	public:
		CLogger(const char *fileName) {
			m_Fd = open(fileName, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
		}

		virtual ~CLogger() {
			if(m_Fd != -1)
				::close(m_Fd);
		}

		int writeLog(const char *ptrMsg) {
			if(m_Fd == -1)
				return -1;
			if(ptrMsg == NULL)
				return -1;

			char buffer[MAX_BUFFER_SIZE];
			snprintf(buffer, MAX_BUFFER_SIZE, "%s\n", ptrMsg);

			ssize_t r = write(m_Fd, buffer, strlen(buffer));
			if(r == -1) {
				::close(m_Fd);
				return -1;
			}

			return 0;
		}
	private:
		int m_Fd;
};

int main() {
	CLogger myLogger("./banchon.txt");
	int i;
	char msg[40];

	for(i = 0; i < 200; i++) {
		sprintf(msg, "%d: THIS IS TEST MESSAGE.", i);
		myLogger.writeLog(msg);
	}

	return 0;

}
