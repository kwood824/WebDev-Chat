Compiling:

	Make sure to unzip the contents of Project1.zip all to the same directory (chatclient.c, chatserve.cpp, makefile).
	Simply run the command "make all" to compile both chatclient and chatserve.

Running:
	
	The chat server is run by typing "./chatserve (Port Number)" where Port Number is the port you wish to use for the server.
	The chat client is run by typing "./chatclient (Host) (Port Number)" where Host is the host you want to connect to and Port Number is the port you wish to use.
	
Controlling the Chat Server:

	If no clients are available to connect the server will wait for them. 
	Press Ctrl-C to kill the server program at any time.
	When connected to a client and it is your turn to send a message you will see a prompt saying "You>" this is when you enter text you wish to send to the client.
	You can type "\quit" whenever it is your turn to chat to close the connection with the current client and return to waiting for another.
	Again, Ctrl-C is used to end the program entirely.
	
Controlling the Chat Client:

	Once connected you will be prompted to type in a handle that is 10 or less characters.
	When it is your turn to chat you will see a prompt saying "(Your Handle)>" where Your Handle is the handle you chose in the previous step.
	You can type "\quit" whenever it is your turn to chat to terminate the program.
	You will be timed out and disconnected after around 3 minutes of inactivity.