# Multithreading echo server

## How to use

There are two programs: server and client.

client:

call it as 
`$ ./client IP_ADDRESS PORT_NUMBER`

After running the program you can then type one line of text. After you enter an empty line, the program will send the full message you wrote to the socket (IP_ADDRESS, PORT_NUMBER) using UDP. It will print in the terminal all the data that it receives back from the server. 

- You can type another line of text to send another message to the server.

server:

call it as 
`$ ./server PORT_NUMBER`

It will start a server that will bind a socket to the port PORT_NUMBER. 

It will constantly wait for messages. 

Once it receives a message through this socket, it sends the same message back to the client (whoever sent the message in the first place) using UDP. Moreover, it will keep sending the same message to the same client every DELAY microseconds. We say that this client has been 'registered'.

It can handle MAXCLIENTS different registered clients at a time.

If one of the registered clients sends another message, it updates the message that it is sending to the client.
