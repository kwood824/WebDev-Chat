/*
TITLE: Simple Chat Client		(chatclient.c)

Author: Kyle Wood				woodky@onid.oregonstate.edu
CS372-400

Description:
A simple chatting client that will connect with the chat server built in C++
and allows the exchange of messages one at a time. Requires the user to input
host and port # on command line and then choose a handle once the program begins.

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
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define MESSAGEMAXLEN 500
#define HANDLEMAXLEN 10
#define TIMEOUT 180

int serverSocket;
struct sockaddr_in serverAddr;
int port;
char* serverAddress;
char* message;
char* handle;
char* receivedBuffer;
time_t lastMessage;
char* quitter = "\\quit";
char* tmphandle;

int parsePortAddress(int argc, char** argv);		//Reads in the address and port # from the command line, produces error if they are not found
int createSocket();									//Reads the socket and fills in the relevant address info
int connectToServer();								//Runs the function to actually connect to the chat server
int sendMessage();									//Reads in the user's input and sends it to the chat server, also checks for the \quit option
int receiveMesssage();								//Parses incomping messages from the chat server and displays them
int MessageLoop();									//Controls the loop of sending and receiving messages until timeout or one of the users closes the connection
void startContact(int argc, char** argv);			//Runs the parsePortAddress, createSocket, and connectToServer functions and checks them for errors

void signalTimeout(int sig);						//Controls the timeout for the chat program that happens around 3 minutes of no messages being sent

int main(int argc, char** argv) {
	message = malloc(MESSAGEMAXLEN * sizeof(char));
	receivedBuffer = malloc(MESSAGEMAXLEN * sizeof(char));
	handle = malloc(HANDLEMAXLEN * sizeof(char));

	signal(SIGALRM, signalTimeout);					//Signal for the timeout

	startContact(argc, argv);

	//Sets the handle

	tmphandle = malloc(HANDLEMAXLEN * sizeof(char));
	printf("Please choose a handle (10 characters max)\n");

	fgets(tmphandle, HANDLEMAXLEN, stdin);
	strtok(tmphandle, "\n");

	if (send(serverSocket, tmphandle, strlen(tmphandle), 0) == -1) {
		fprintf(stderr, "ERROR: Handle couldn't be sent\n");
	}

	if (getHandle(handle) == 0) {
		printf("ERROR: getHandle returned 0\n");
		return 0;
	}

	//talk to the server
	printf("Connected to the server! Max message length is %d characters (Type \\quit to end connection)\n", MESSAGEMAXLEN);
	MessageLoop();

	return 0;
}

int parsePortAddress(int argc, char** argv) {
	if (argc < 3) {
		printf("ERROR: Missing address, port or both!\n");
		return 0;
	}

	sscanf(argv[2], "%d", &port);
	serverAddress = argv[1];
	return 1;
}

int createSocket() {
	if ((serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		fprintf(stderr, "Problem creating the server socket.\n");
		return 0;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);

	return 1;
}

int connectToServer() {
	if (connect(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		fprintf(stderr, "Problem connecting to server %s on port %d\n", serverAddress, port);
		return 0;
	}
	return 1;
}

void startContact(int argc, char** argv) {
	if (parsePortAddress(argc, argv) == 0) {
		printf("ERROR: parsePortAddress returned 0\n");
		exit(0);
	}

	if (createSocket() == 0) {
		printf("ERROR: Create socket returned 0\n");
		exit(0);
	}

	if (connectToServer() == 0) {
		exit(0);
	}
}

int getHandle(char* handle) {
	int recvVal;
	while ((recvVal = recv(serverSocket, handle, HANDLEMAXLEN, 0)) == -1) {
		fprintf(stderr, "Problem retrieving the handle from the server. Retrying...\n");
	}
	if (recvVal == 0) {
		fprintf(stderr, "The server has closed the connection\n");
		return 0;
	}
	else {
		return 1;
	}
}

int MessageLoop() {
	lastMessage = time(NULL);
	alarm(180);
	for (;;) {
		sendMessage();
		if (receiveMesssage() == -1) { return 0; }
	}

	return 0;
}

void signalTimeout(int sig) {
	if ((time(NULL) - lastMessage) > TIMEOUT) {
		printf("\nTimed out. You are no longer connected to the server\n"); //clear to newline
		exit(0);
	}

	alarm(1);
}

int sendMessage() {
	printf("%s ", handle);
	fgets(message, MESSAGEMAXLEN, stdin);
	lastMessage = time(NULL);

	if (send(serverSocket, message, strlen(message), 0) == -1) {
		fprintf(stderr, "ERROR: Message couldn't be sent!\n");
	}

	if (strncmp (message, quitter, 5) == 0){
		printf("Quitting out of chat...\n");
		exit(0);
	}

	//clear message
	memset(message, 0, MESSAGEMAXLEN);

	return 0;
}

int receiveMesssage() {
	int recVal = recv(serverSocket, receivedBuffer, MESSAGEMAXLEN, 0);

	if (recVal == 0) {
		fprintf(stderr, "ERROR: The server has closed the connection!\n");
		return -1;
	}
	else if (recVal == -1) {
		fprintf(stderr, "ERROR: Problem storing client message!\n");
		return 0;
	}
	else {
		printf("Server> %s\n", receivedBuffer);
		lastMessage = time(NULL);

		//clear received message
		memset(receivedBuffer, 0, MESSAGEMAXLEN);

		return 1;
	}
}