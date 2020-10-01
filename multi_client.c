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
	int s;
	int t;
	int len;

	struct sockaddr_un remote;

	char str[100];

	/*Creating the socket for the client.*/
	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
	{
		perror("Client socket creation Error");
		exit(1);
	}

	printf("Trying to Connect to the Server...\n");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	/*Trying to connect to the Server at the master socket*/
	if (connect(s, (struct sockaddr *)&remote, len) == -1) 
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
	    /*If flag is 1 then we don't have to wait.*/
	    if ( flag == 1 )
	    {
	    	/*Receiving the value from the server*/
	    	/*RECV here is of blocking nature as we are waitng for receiving the message.*/
	        int z = recv(s,str,sizeof(str),0);
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
	    	/*Receiving the value from the server*/
	    	/*RECV here is of non-blocking nature as we are waitng for receiving the message.*/
	    	/*DONTWAIT means we are not waiting to receive anything*/
	        int z = recv(s,str,sizeof(str),MSG_DONTWAIT);
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
	            //memset(str, 0, sizeof(str));
	        }

	    }

	    fd_set rfdset;
	    /*Clearing the fdset*/
	    FD_ZERO(&rfdset);
	    /*Adding STDIN to the fdset so that we can know if there is any message typed.*/
	    FD_SET(STDIN_FILENO, &rfdset); 

	    /*Creating the timeval attribute . Not used significantly here.*/
	    struct  timeval tv;
	    tv.tv_sec = 0;
	    tv.tv_usec = 0;
	    
	  	/*Now bloking the control at the select statement to check if anything has been typed into.*/
	    int bReady = select(STDIN_FILENO+1,&rfdset,NULL,NULL,&tv);
	    if (bReady > 0)
	    {
	        //  printf("Chat with Client Address: \n");
	        /*if( fgets (addr, 4096, stdin)!=NULL ) {
	        }*/
	        //printf(">>: ");

	        /*Getting the typed thing at the terminal*/
	        if( fgets (str, 100, stdin)!=NULL ) {
	        }
	        //strcat(addr,str);
	        send(s,str,strlen(str),0);
	    }
	    //memset(str, 0, sizeof(str));
	    flag=0;

	    /*Here I am not implementing the function that if the client says BYE he/she can exit*/
	    /*In order to exit the client just press the CTRL+C */
	}
	close(s);
	return 0;


}