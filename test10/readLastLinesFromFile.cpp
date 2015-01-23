#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>


//when this function finish its call, remember to free the storage of the members of message.
//
//
int readLastLinesFromFile(const char *fileName, char *message[], int *lines) {
	
	(*lines) = 0;
	off_t fileLen;
	char buffer[520];
	bool notFromBegin = false;
	int fd = open(fileName, O_RDONLY, 0);

	if(fd == -1)
		return -1;
	fileLen = lseek(fd, 0, SEEK_END);

	if(fileLen < 512)
		lseek(fd, 0, SEEK_SET);
	else {
		lseek(fd, -512, SEEK_END);
		notFromBegin = true;
	}

	ssize_t readNums = read(fd, buffer, sizeof(buffer));
	if(readNums == -1)
		return -1;

	close(fd);
	buffer[readNums] = '\0';
//	printf("%s\n\n\n", buffer);

	int startOfLine;
	int endOfLine;



	startOfLine = endOfLine = 0;
	for(endOfLine = 0; endOfLine < readNums; endOfLine++) {
		if(buffer[endOfLine] == '\n') {
			if(notFromBegin && (startOfLine == 0)) {
				startOfLine = endOfLine + 1;
			}

			else{
				int length = endOfLine - startOfLine;
				char *temp = new char[length + 1];
				if(temp == NULL) {
					for(int i = 0; i < (*lines); i++)
						delete message[i];
					return -1;
				}
				memcpy(temp, buffer + startOfLine, length);
				*(temp + length) = '\0';
				message[(*lines)] = temp;
				(*lines)++;
				startOfLine = endOfLine + 1;
			}
		}
	}
	return 0;
}

int main()
{
	int lines;
	char *message[100];
	

	int tag = readLastLinesFromFile("./mentry_ex.c", message, &lines);

	if( tag != -1) {
		int i = 0;
			for(; i < lines; i++) {
				printf("%s\n", message[i]);
				delete [] message[i];
	}
	return 0;

	}
}
