all: chat_client chat_server
chat_client: chat_client.c
	gcc -o client  chat_client.c
chat_server: chat_server.c 
	gcc  -o server chat_server.c




