#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "multiServer_socket" /*Server Process is running on this special file path. Clients need to connect to this*/
#define MAX_CLIENT_SUPPORTED 50 /*MAximum number of clients supported on my chat MultiChat platform*/

int fd_table[MAX_CLIENT_SUPPORTED];

/*Function to reinitialize the readfds */
void reinitialize_readfds(fd_set *fd_set_ptr)
{
	FD_ZERO(fd_set_ptr);
	for(int i = 0 ; i < MAX_CLIENT_SUPPORTED ; i++)
	{
		/*Copying each Socket present in fd_table to readfds one by one*/
		if(fd_table[i] != -1)
		{
		    FD_SET(fd_table[i], fd_set_ptr);
		}
	}
}

/*Function to initialize all elements of fd_table to be -1 */
static void initialize_fd_table()
{
	for(int i = 0; i< MAX_CLIENT_SUPPORTED ; i++) 
	{
		fd_table[i]=-1;
	}
}

/*Function to add a new socket connection's sock_fd to fd_table */
void add_to_fd_table(int skt_fd)
{
	for(int i = 0; i < MAX_CLIENT_SUPPORTED ; i++) 
	{
		if(fd_table[i] != -1)
		{
			continue;
		}
		else
		{
			fd_table[i]=skt_fd;
			break;	
		}
	}
}

/*Function to remove an old socket connection's sock_fd from fd_table */
void remove_from_fd_table(int skt_fd)
{
	for(int i = 0; i < MAX_CLIENT_SUPPORTED ; i++) 
	{
		if(fd_table[i] != skt_fd)
		{
			continue;
		}
		else
		{
			fd_table[i] = -1;
			break;	
		}
	}
}

/*Returns the maximum of the file descreptors to be used in select function*/
int maximum_fd()
{
    int maxfd = -1;
    for(int i = 0; i < MAX_CLIENT_SUPPORTED ; i++)
    {
        if(fd_table[i] > maxfd)
            maxfd = fd_table[i];
    }

    return maxfd;
}

int main(void)
{
	int master_socket; 
	int client_socket;
	int other_client_socket; /*The other clients to send data to*/
	int t;
	int max;
	int n,len;
	struct sockaddr_un local, remote; 
	char str[100];
	fd_set readfds; /* Set of fds*/

	/*Creating the socket for the Server. All clients need to bind to it.*/
	if ((master_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
	{
		perror("Master Socket Creation Error");
		exit(1);
	}

	/*Defining the parameters for the sock_addr*/
	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);

	/*Binding the address for the Server socket*/
	if (bind(master_socket , (struct sockaddr *)&local, len) == -1) 
	{
		perror("Failed to bind the addr to the socket");
		exit(1);
	}
	

	/*Server is listening now at the master_socket*/
	if (listen(master_socket , 5) == -1) 
	{
		perror("Failed to listen From CLients");
		exit(1);
	}

	/*Function to initialize all elements of fd_table to be -1 */ 
	initialize_fd_table();

	/*Adding the master_socket to fd_table of FDs*/
	add_to_fd_table(master_socket);

	while(1)
	{
		/*Function to reinitialize the readfds */
		reinitialize_readfds(&readfds);

		/*Now the server is waiting for the new requests either new or data requests*/
		printf("__Waiting for any request .......\n");
		max = maximum_fd();

		/*Checking on several different sockets and do something based on whichever one is ready first*/
		select(max + 1, &readfds, NULL, NULL, NULL);

		/*If the select function shows the connection is on master_socket*/
		/*If the connection is on master socket. It is a request for new connection*/
		/*We assign the new fd to the new connecting client.*/
		if(FD_ISSET(master_socket , &readfds ))
		{
			printf("New Connection Request Arrived from Client. Naming it as : User%d\n", max+1 );
			/*Getting the size of remote client*/
			t = sizeof(remote);
			/*Accepting the new request from the client.*/
			if((client_socket = accept(master_socket, (struct sockaddr *)&remote, &t)) == -1)
			{
				perror("Failed to accept the connection");
				exit(1);
			}
			/*Adding the new client to the fd_table*/
			add_to_fd_table(client_socket);
			printf("Connected to the new User. \n");
			/*	printf("Hello Welcome to the chat system %s\n", str);*/
			/*Sending initial Message to the Client.*/
			char str1[100];
			sprintf(str1,"\t\tWELCOME TO THE CHAT. USER%d\n",client_socket);
			str[1] = '\0';
			if (send(client_socket , str1, 100, 0) < 0) 
			{
				perror("send");
			}
		}

		/*If the select function shows that the request is from The existing user. */
		else
		{
			int i;
			client_socket = -1;
			/*Iterating over all the clients to check which has placed a request.*/
			for (i = 0; i < MAX_CLIENT_SUPPORTED ; i++)
			{
				/*Checking if the client at fd_table[i] has placed a request.*/
				if(FD_ISSET(fd_table[i],&readfds))
				{
					/*Marking the client socket.*/
					client_socket = fd_table[i];

					/*GETTING MESSAGE FROM CLIENT PART*/
					n = recv(client_socket, str , 100, 0);
					if (n <= 0) 
					{
						if (n < 0)
						{
							perror("recv");
						}						
					}


					str[n] = '\0';
					printf("client%d> %s", client_socket,str);
					other_client_socket = client_socket;

					char *ptr;
					long choice;
					char message[100];

					choice = strtol(str, &ptr, 10); 
					strcpy(message,ptr);

					int flag = 0;

					for(int x = 0 ; x < MAX_CLIENT_SUPPORTED ; x++)
					{
						if(choice==fd_table[x])
						{
							flag=1;
							break;
						}
					}


						/*SENDING TO OTHER PART*/
						if(choice==master_socket)
						{
							for(int i = 0 ; i < MAX_CLIENT_SUPPORTED ; i++)
							{
								/*Preparing the message for sending it to the group chat*/
								int k=0;
							    int client = client_socket;
							    char buffer[100];
							    char s[7] = "Client";
							    k =  sprintf(buffer, "Chat Group Message :: Client_%d : %s", client,message);	

								/*Sending to all code.*/
								if(fd_table[i] != -1 && fd_table[i] != client_socket && fd_table[i] != master_socket)
								{

								    if (send(fd_table[i] , buffer , 100, 0) < 0) 
								    {
								    	remove_from_fd_table(client_socket);
								    	close(client_socket);
								    	perror("send");
								    }
								}
								memset(buffer, 0, sizeof(buffer));
							}
						}

						/*SEND TO ALL CODE*/
						/*Sending to each Socket present in fd_table to readfds one by one*/
						else
						{
								/*Preparing the message for sending it to the other client.*/
								int k=0;
							    int client = client_socket;
							    char buffer[100];
							    char s[7] = "Client";
							    k =  sprintf(buffer, "Client_%d : %s", client,message);

							    //if(flag==1)
							    {
								    if(choice != client_socket)
								    {
										if (send( choice , buffer , 100, 0) < 0) 
										{
											remove_from_fd_table(client_socket);
											close(client_socket);
											perror("send");
										}
										memset(buffer, 0, sizeof(buffer));
									}
								}
						}
						printf("Msg sent \n");
				}
			}
		}
	}
	close(master_socket);
	return 0;
}