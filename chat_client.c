#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "multiServer_socket"

int main(void)
{
	int socket_creation;
	int t;
	int len;

	struct sockaddr_un remote;

	char str[100];

	
	if ((socket_creation = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
	{
		perror("Client socket creation Error");
		exit(1);
	}

	printf("Trying to Connect to the Server...\n");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	
	if (connect(socket_creation, (struct sockaddr *)&remote, len) == -1) 
	{
		perror("connect");
		exit(1);
	}

	printf("Connected. To The Server\n");

	int flag = 1;
	for (;;) 
	{
	    char str[100];
	    char addr[100];
	    
	    if ( flag == 1 )
	    {
	    	
	        int z = recv(socket_creation,str,sizeof(str),0);
	        if ( z == -1 )
	        {
	        	printf("Did not receive anything , error in receiving from server ....\n");
	        }
	        else if ( z == 0 )
	        {
	            printf("....\n");
	            break;
	        }
	        else
	        {
	            str[z] = 0;
	            printf("%s",str);
	        }
	    }
	    else
	    {
	    	
	        int z = recv(socket_creation,str,sizeof(str),MSG_DONTWAIT);
	        if ( z == -1 )
	        {
	        }
	        else if ( z == 0 )
	        {
	            printf("XX Server Closed the Connection. XX\n");
	            break;
	        }
	        else
	        {
	            str[z] = 0;
	            printf("%s",str);
	            
	        }

	    }

	    fd_set rfdset;
	    
	    FD_ZERO(&rfdset);
	    
	    FD_SET(STDIN_FILENO, &rfdset); 

	    
	    struct  timeval tv;
	    tv.tv_sec = 0;
	    tv.tv_usec = 0;
	    
	  	
	    int bReady = select(STDIN_FILENO+1,&rfdset,NULL,NULL,&tv);
	    if (bReady > 0)
	    {
	        
	        if( fgets (str, 100, stdin)!=NULL ) {
	        }
	        
	        send(socket_creation,str,strlen(str),0);
	    }
	    
	    flag=0;

	    
	}
	close(socket_creation);
	return 0;


}
