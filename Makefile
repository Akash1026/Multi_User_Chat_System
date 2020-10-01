all: chat_client chat_server
multi_client: multi_client.c
	gcc -o client  chat_client.c
multi_server: multi_server.c 
	gcc  -o server chat_server.c




