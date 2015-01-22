#ifndef READLASTLINESFROMFILE_H
#define READLASTLINESFROMFILE_H

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

int readLastLinesFromFile(const char *fileName, char *message[], int *lines);


//when this function finish its call, remember to free the storage of the members of message.
//
//and the message parameter are both of input and output parameter.so when call this function
//you need guarantee that the message array can hold all these lines.
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


	//read at most 512 Bytes from file.
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

	int startOfLine;
	int endOfLine;


//split the content of buffer into line by line.
	startOfLine = endOfLine = 0;
	for(endOfLine = 0; endOfLine < readNums; endOfLine++) {
		if(buffer[endOfLine] == '\n') {
			if(notFromBegin && (startOfLine == 0)) {
				startOfLine = endOfLine + 1;
			}

			else{
				int length = endOfLine - startOfLine;
				char *temp = new char[length + 1];
				if(temp == NULL)
					return -1;
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

#endif

