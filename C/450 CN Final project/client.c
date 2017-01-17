#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define portAWS "25865"
#define MAX_LENGTH 1024

char operation[3];
int count = 0;

void printerror(char * error)			// to print error messages if any on stderr
{
	fprintf(stderr,"%s\n",error);
	exit(1);	
}

int data_parse(char *buff)				// count the number of int in file
{
    int i = 0;
    
    while(buff[i] != '\0')
    {
       
       while(buff[i] != '\n')            
            i++;
       
       count++;
       i++;
    }
    
    return count;
}


/* this fucntion check command line arrugments and sets operation accordingly */

int CheckCMDLine(int argc, const char *argv[])		/* from csci 402 warmup1 assignment*/
{
	if(argc > 2)
		printerror("More than 2 arguments passed");
	if(argc < 1)
		printerror("Less arguments entered");
	
	if(strcmp(argv[1],"max") == 0)
		strcpy(operation, "MAX");
	else if(strcmp(argv[1],"min") == 0)
		strcpy(operation, "MIN");
	else if(strcmp(argv[1],"sum") == 0)
		strcpy(operation, "SUM");
	else if(strcmp(argv[1],"sos") == 0)
		strcpy(operation, "SOS");
	else
		printerror("Incorrect Command");
		
	return 0;
}

int main(int argc, char const *argv[])
{
    CheckCMDLine(argc, argv);


	int socket_fd, n;
    struct addrinfo info;
    struct addrinfo *servinfo,*p; 

    struct sockaddr_storage their_addr;             // after we accept connection to store info

    memset(&info, 0, sizeof(info));				// From Beej guide
    info.ai_family = PF_UNSPEC;					// not version specific
    info.ai_socktype = SOCK_STREAM;				// create TCP socket
    info.ai_flags = AI_PASSIVE; 				// gets IP address of local host

    printf("The client is up and running.\n");

    int status =0;
    if((status = getaddrinfo("localhost",portAWS, &info, &servinfo)) != 0)
    {
    	fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    	exit(1);
	}

	 // loop through all the results and bind to the first we can
	for( p = servinfo; p!=NULL; p->ai_next )			// From Beej guide
	{
		if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)		// create socket
		{
           	 perror("Client: socket");
           	 continue;
        }

        if((connect(socket_fd, p->ai_addr, p->ai_addrlen)) == -1)			// connect to aws on created socket
        {
           	close(socket_fd);
           	perror("Client : connect");
           	continue;
       	}
        break;
	}

    freeaddrinfo(servinfo);

    if (p == NULL)  
    {
        fprintf(stderr, "Client: failed to bind\n");
        exit(1);
    }

    
    char send_buff[3*MAX_LENGTH];
    char buff[3*MAX_LENGTH];
    memset(send_buff,0,sizeof(send_buff));
    memset(buff,0,sizeof(buff));

    strcpy(send_buff, operation);

    FILE *fp = fopen("nums.csv","rb");			// open the file & count # of integers

     while(!feof(fp))
     {
        fgets(buff,MAX_LENGTH,fp);
        
		count++;
     }
     fclose(fp);
	 if(count % 3 != 0)
		count--;

    send_buff[3] = '\t';
    send_buff[4] = '\0';
    
    memset(buff,0,sizeof(buff));
    sprintf(buff,"%d", count);

    strcat(send_buff, buff);

    if((send(socket_fd, send_buff, sizeof(send_buff), 0)) < 0)			// send operation and # of integers to aws
    {
            perror("send to client");
            exit(1);
    }
    printf("The client has sent the reduction type %s to AWS\n", operation);

  
    
    memset(buff,0,sizeof(buff));
	  fp = fopen("nums.csv","rb");
    int	i =0;
    while(!feof(fp))							// send data to aws server
    {
    
		memset(send_buff,0,sizeof(send_buff));
		fgets(send_buff,MAX_LENGTH,fp);
		
		i++;
	
    	if((send(socket_fd, send_buff, sizeof(send_buff), 0)) < 0)
    	{
	        perror("send to client");
	        exit(1);
                 
                 
    	}
	}
    fclose(fp);
    printf("The client has sent %d numbers to AWS\n",count);
    
    long rec_buff[3*MAX_LENGTH];
	  memset(rec_buff,0,sizeof(rec_buff));

	if((n = recv(socket_fd, rec_buff, MAX_LENGTH, 0)) < 0)				// recieve final result from aws 
    {
        perror("recv");
        exit(1);
    }
        
        
    long result = rec_buff[0];
    printf("The client has received reduction %s: %ld\n", operation, result );
    
  //  shutdown(socket_fd,2);
    close(socket_fd);

  usleep(1000);
	return 0;
}
