/*
TITLE: Simple Chat Server		(chatserve.c)

Author: Kyle Wood				woodky@onid.oregonstate.edu
CS372-400

Description:
A simple chatting server that works on a set port number specified by the user
on the command line. It then waits for a client to join the same port and then
establishes a line so that the two can send messages back and forth to each other.

References:
http://beej.us/guide/bgnet/output/html/multipage/index.html
http://stackoverflow.com/questions/655065/when-should-i-use-the-new-keyword-in-c
http://www.cplusplus.com/reference/cstdio/sprintf/
http://www.cplusplus.com/reference/cstdio/fgets/
http://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input

Tested on the OSU FLIP server.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#define MESSAGEMAXLEN 500
#define MAXPENDINGCONNECTIONS 1
#define HANDLEMAXLEN 10
#define TIMEOUT 180

int serverSocket;
int clientSocket;
int port;
struct sockaddr_in serverAddr;
struct sockaddr_in clientAddr;
time_t lastMessage;
char* quitter = "\\quit";
char* tmphandle;

int parsePort(int argc, char** argv);											//Parses the port number from the command line input, sends error if none is found
int createSocket();																//Creates the socket given the parsed info and specifies the relevant address data
int listenOnSocket();															//Binds the socket to an address then waits for connections
int clientConnected(unsigned int* clientLen);									//Checks to see if a client has connected to the same port #
void setHandle(char* clientHandle);												//Sets the handle to be displayed in front of all the connected client's messages
int messageClient(char* message, int messageLen);									//Sends initial message to client
int sendMessage(char* clientHandle, char* message);									//Parses the servers input and sends it to the client
int receiveMessage(char* clientHandle, char* receivedBuffer);						//Parses the message sent from the client and displays it on screen
int messageLoop(char* clientHandle, char* message, char* receivedBuffer);		//Controls the infinite loop for sending and receiving messages until timeout or interruption from a signal
void startup(int argc, char** argv);											//Runs the initial functions of parsePort, createSocket and listenOnSocket and checks them for errors

void signalTimeout(int sig);														//Causes the server to disconnect after 3 minutes of inactivity
void endConnection(int sig);													//Closes the connection between the client and server

int main(int argc, char** argv) {
	char* clientHandle;
	char* message;
	char* receivedBuffer;
	clientHandle = new char(HANDLEMAXLEN);
	tmphandle = new char(HANDLEMAXLEN);
	message = new char(MESSAGEMAXLEN);
	receivedBuffer = new char(MESSAGEMAXLEN);
	unsigned int clientLen;

	signal(SIGALRM, signalTimeout);
	signal(SIGINT, endConnection);

	startup(argc, argv);

	//server running
	printf("Waiting for clients to connect... (Press ctrl-c to quit out)\n");
	clientLen = sizeof(clientAddr);
	for (;;) {
		if (clientConnected(&clientLen) == 1) {
			//process this client

			printf("Connected to client %s\n", inet_ntoa(clientAddr.sin_addr));
			setHandle(clientHandle);

			//tell client their handle
			while (messageClient(clientHandle, strlen(clientHandle)) != 1) {
				printf("ERROR: Failed to tell the client their handle. Trying again...\n");
				sleep(2);
			}

			//client gets to send the first message
			printf("Now chatting with client %s. Max character length is %d (Type \\quit in chat to end the connection)\n", clientHandle, MESSAGEMAXLEN);
			messageLoop(clientHandle, message, receivedBuffer);
			printf("Client has disconnected. Waiting for another client...\n");
		}
	}

	return 0;
}

//Functions

int parsePort(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "ERROR: No port specified.\n");
		return 0;
	}
	else {
		sscanf(argv[1], "%d", &port);
		return 1;
	}
}

int createSocket() {
	if ((serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		fprintf(stderr, "ERROR: Problem creating the server socket.\n");
		return 0;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);

	return 1;
}

int listenOnSocket() {
	//bind to a local address
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		fprintf(stderr, "ERROR: Problem binding the server socket.\n");
		return 0;
	}

	if (listen(serverSocket, MAXPENDINGCONNECTIONS) < 0) {
		fprintf(stderr, "ERROR: Problem in listening for incoming connections.\n");
		return 0;
	}

	return 1;
}

void startup(int argc, char** argv) {
	if (parsePort(argc, argv) == 0) {
		exit(0);
	}

	if (createSocket() == 0) {
		exit(0);
	}

	if (listenOnSocket() == 0) {
		exit(0);
	}

}

int clientConnected(unsigned int* clientLen) {
	clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, clientLen);
	if (clientSocket < 0) {
		//no client connected
		return 0;
	}
	else {
		//client connected
		return 1;
	}
}

void setHandle(char* clientHandle) {
	int recVal = recv(clientSocket, tmphandle, MESSAGEMAXLEN, 0);
	
	sprintf(clientHandle, "%s>", tmphandle);
	return;
	
}

int messageClient(char* message, int messageLen) {
	if (send(clientSocket, message, messageLen, 0) < 0) {
		return 0;
	}
	else {
		return 1;
	}
}

int messageLoop(char* clientHandle, char* message, char* receivedBuffer) {
	for (;;) {
		if (receiveMessage(clientHandle, receivedBuffer) == -1) { return 0; }
		if (sendMessage(clientHandle, message) == 0){
			return 0;
		}
	}

	return 0;
}

int receiveMessage(char* clientHandle, char* receivedBuffer) {
	int recVal = recv(clientSocket, receivedBuffer, MESSAGEMAXLEN, 0);

	if (recVal == 0) {
		fprintf(stderr, "ERROR: Client has closed the connection\n");
		return -1;
	}
	else if (recVal == -1) {
		fprintf(stderr, "ERROR: Problem storing client message\n");
		return 0;
	}
	else {
		printf("%s %s\n", clientHandle, receivedBuffer);

		//clear received message
		memset(receivedBuffer, 0, MESSAGEMAXLEN);

		return 1;
	}
}

int sendMessage(char* clientHandle, char* message) {
	printf("You> ");
	lastMessage = time(NULL);
	fgets(message, MESSAGEMAXLEN, stdin);

	messageClient(message, strlen(message));

	if (strncmp(message, quitter, 5) == 0){
		printf("Quitting out of chat...\n");
		close(clientSocket);
		return 0;
	}

	lastMessage = time(NULL);
	memset(message, 0, MESSAGEMAXLEN);

	return 1;
}

void signalTimeout(int sig) {
	if ((time(NULL) - lastMessage) > TIMEOUT) {
		printf("\n");
		close(clientSocket);
	}

	alarm(1);
}

void endConnection(int sig) {
	printf("Manually shutting down the server...\n");
	exit(0);
}

