#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "multiServer_socket" 
#define MAX_CLIENT_SUPPORTED 50 

int socket_table[MAX_CLIENT_SUPPORTED];


void reinitialize_readfds(fd_set *fd_set_ptr)
{
	FD_ZERO(fd_set_ptr);
	for(int i = 0 ; i < MAX_CLIENT_SUPPORTED ; i++)
	{
		
		if(socket_table[i] != -1)
		{
		    FD_SET(socket_table[i], fd_set_ptr);
		}
	}
}


static void initialize_socket_table()
{
	for(int i = 0; i< MAX_CLIENT_SUPPORTED ; i++) 
	{
		socket_table[i]=-1;
	}
}


void add_to_socket_table(int skt_fd)
{
	for(int i = 0; i < MAX_CLIENT_SUPPORTED ; i++) 
	{
		if(socket_table[i] != -1)
		{
			continue;
		}
		else
		{
			socket_table[i]=skt_fd;
			break;	
		}
	}
}


void remove_from_socket_table(int skt_fd)
{
	for(int i = 0; i < MAX_CLIENT_SUPPORTED ; i++) 
	{
		if(socket_table[i] != skt_fd)
		{
			continue;
		}
		else
		{
			socket_table[i] = -1;
			break;	
		}
	}
}


int maximum_fd()
{
    int maxfd = -1;
    for(int i = 0; i < MAX_CLIENT_SUPPORTED ; i++)
    {
        if(socket_table[i] > maxfd)
            maxfd = socket_table[i];
    }

    return maxfd;
}

int main(void)
{
	int master_socket; 
	int client_socket;
	int other_client_socket; 
	int t;
	int max;
	int n,len;
	struct sockaddr_un local, remote; 
	char str[100];
	fd_set readfds; 

	
	if ((master_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
	{
		perror("Master Socket Creation Error");
		exit(1);
	}

	
	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);

	
	if (bind(master_socket , (struct sockaddr *)&local, len) == -1) 
	{
		perror("Failed to bind the addr to the socket");
		exit(1);
	}
	

	if (listen(master_socket , 5) == -1) 
	{
		perror("Failed to listen From CLients");
		exit(1);
	}

	 
	initialize_socket_table();

	
	add_to_socket_table(master_socket);

	while(1)
	{
		
		reinitialize_readfds(&readfds);

		
		printf("__Waiting for any request .......\n");
		max = maximum_fd();

		
		select(max + 1, &readfds, NULL, NULL, NULL);

		
		if(FD_ISSET(master_socket , &readfds ))
		{
			printf("New Connection Request Arrived from Client. Naming it as : User%d\n", max+1 );
			
			t = sizeof(remote);
			
			if((client_socket = accept(master_socket, (struct sockaddr *)&remote, &t)) == -1)
			{
				perror("Failed to accept the connection");
				exit(1);
			}
			
			add_to_socket_table(client_socket);
			printf("Connected to the new User. \n");
			
			char str1[100];
			sprintf(str1,"\t\tWELCOME TO THE CHAT. USER%d\n",client_socket);
			str[1] = '\0';
			if (send(client_socket , str1, 100, 0) < 0) 
			{
				perror("send");
			}
		}

		
		else
		{
			int i;
			client_socket = -1;
			
			for (i = 0; i < MAX_CLIENT_SUPPORTED ; i++)
			{
				
				if(FD_ISSET(socket_table[i],&readfds))
				{
					
					client_socket = socket_table[i];

					
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
						if(choice==socket_table[x])
						{
							flag=1;
							break;
						}
					}


						
						if(choice==master_socket)
						{
							for(int i = 0 ; i < MAX_CLIENT_SUPPORTED ; i++)
							{
								
								int k=0;
							    int client = client_socket;
							    char buffer[100];
							    char s[7] = "Client";
							    k =  sprintf(buffer, "Chat Group Message :: Client_%d : %s", client,message);	

								
								if(socket_table[i] != -1 && socket_table[i] != client_socket && socket_table[i] != master_socket)
								{

								    if (send(socket_table[i] , buffer , 100, 0) < 0) 
								    {
								    	remove_from_socket_table(client_socket);
								    	close(client_socket);
								    	perror("send");
								    }
								}
								memset(buffer, 0, sizeof(buffer));
							}
						}

						
						else
						{
								
								int k=0;
							    int client = client_socket;
							    char buffer[100];
							    char s[7] = "Client";
							    k =  sprintf(buffer, "Client_%d : %s", client,message);

							    
							    {
								    if(choice != client_socket)
								    {
										if (send( choice , buffer , 100, 0) < 0) 
										{
											remove_from_socket_table(client_socket);
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
