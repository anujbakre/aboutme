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


#define portB_UDP "22865"
#define MAX_LENGTH 1024
#define portAWS_UDP "24865"

long f_len =0;
int rec_no =0;

void data_parse(long *data_array, long *buff)			// store recived data in array
{
    int i = 0;
        
    while( i < (3*MAX_LENGTH - 1)/4 && rec_no <= f_len)
    {
        
        data_array[i] = buff[rec_no];
         i++;
        rec_no++;
    }

    
}

void sendUDP_backserver(char *port, struct sockaddr *p, char *buff, int socket_fd )				// send result to aws server
{
    int status =0;
    
    int n=0;
 
       
    if ((n = sendto(socket_fd, buff, strlen(buff),0,(const struct sockaddr *)p, sizeof(struct sockaddr))) == -1)
    {
        perror("server B : sendto  failed");
        exit(1);
    }

}


int main()
{
    int x =0;
    int status =0;
	  int socket_fd_B;
    struct addrinfo info;
    struct addrinfo *servinfo,*p; 
    struct sockaddr_in s;
    socklen_t l;
    struct sockaddr_storage their_addr;
    size_t addr_len;
    int n;
    char rec_buff[3*MAX_LENGTH], tmp_buff[3*MAX_LENGTH];
    int operation;
    long min, max;
    long send_result = 0;
    char *reduction;
    char send_data[3*MAX_LENGTH];
    char tmp_buff_B[3*MAX_LENGTH];
    
while(1)
{
    
	rec_no = 0;
     f_len =0;
     memset(&info, 0, sizeof(info));        // from beej
    info.ai_family = PF_INET; 
    info.ai_socktype = SOCK_DGRAM;
    info.ai_flags = AI_PASSIVE; 

	
	 if( x != 0)
         printf("\n\n\n");
     
     x = 1; 
   
    if((status = getaddrinfo("localhost", portB_UDP, &info, &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 					// From Beej guide
    {
        if ((socket_fd_B = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 		// create socket
        {
            perror("backend server B UDP listener: socket");
            continue;
        }

        if (bind(socket_fd_B, p->ai_addr, p->ai_addrlen) == -1) 				// bind the socket
        {
            close(socket_fd_B);
            perror("backend server B UDP listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) 												// From Beej guide
    {
        fprintf(stderr, "backend server B UDP listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);
    
    addr_len = sizeof(their_addr);
    n =0;
    
    
    
    


   
  
     l = sizeof(s);
    if (getsockname(socket_fd_B, (struct sockaddr *)&s, &l) == -1)
        perror("getsockname");

    printf("The Server B is up and running using UDP on port %d\n", ntohs(s.sin_port)); 
    
   
    
   
    memset(rec_buff, 0, sizeof(rec_buff));
    rec_buff[MAX_LENGTH-1] = '\0';

    operation = 0;

    if ((n = recvfrom(socket_fd_B, rec_buff, MAX_LENGTH -1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1)		// recieve opeartion and # of integers from aws
    {
        perror("backend server B : recvfrom");
        exit(1);
    }
   
    memset(tmp_buff, 0, sizeof(tmp_buff));
    for (n = 0; rec_buff[n]!='\t' && n < MAX_LENGTH && rec_buff[n] !='\0'; n++)
        tmp_buff[n] = rec_buff[n];
    
    operation = rec_buff[0] -'0';
   
   
    
    n++;
    memset(tmp_buff, 0, sizeof(tmp_buff));
    int i =0;
    for (i =0 ;n < MAX_LENGTH && rec_buff[n] !='\0'; n++, i++)
        tmp_buff[i] = rec_buff[n];
    
    tmp_buff[++i] = '\0';

    f_len = atol(tmp_buff);

    

    memset(rec_buff,0,sizeof(rec_buff));
    long *data_array =(long *)malloc((f_len + 1)* sizeof(long));
    memset(data_array,0,sizeof(data_array));

    long rec_buff_new[3*MAX_LENGTH];
    memset(rec_buff_new,0,sizeof(rec_buff));

    while(rec_no <= f_len)									// recieve data from aws
    {
        memset(rec_buff_new,0,sizeof(rec_buff));
         if ((n = recvfrom(socket_fd_B, rec_buff_new, 3*MAX_LENGTH , 0,(struct sockaddr *)&their_addr, &addr_len)) < 0 )
        {
            perror("backend server B : recvfrom");
            exit(1);
        }
         else if(n == 0)
        {
            /* clinet closed the socket*/
            fprintf(stderr, "less number data from clinet\n");
           
        }
      

        data_parse(data_array,rec_buff_new);                         

      //read more data untill all recieved
    }

    printf("The Server B has received %ld numbers\n", f_len);

    min = data_array[0];
    max = data_array[0];
    send_result = 0;
    
    switch(operation)						// perfrom operation on data recieved
    {
        case 1:  
                    for(i = 0; i < f_len; i++)
                        if(data_array[i] < min)
                            min = data_array[i];
                        send_result = min;
                    break;
        case 2:
                    for(i = 0; i < f_len; i++)
                        if(data_array[i] > max)
                            max = data_array[i];
                    send_result = max;
                    break;
        case 3: 
                    for(i = 0; i < f_len; i++)
                        send_result += data_array[i];
                    break;

        case 4: 
                    for(i = 0; i < f_len; i++)
                        send_result += data_array[i] * data_array[i] ;
                    break;

        default: fprintf(stderr,"error recieving data from aws servers: invalid operation");
                    exit(1);
                    break;

    }

        
    switch(operation)
    {
        case 1: reduction = "MIN";
            break;
        case 2: reduction = "MAX";
            break;
        case 3: reduction = "SUM";
            break;
        case 4: reduction = "SOS";
            break;
    }

    printf("The Server B has successfully finished the reduction %s: %ld \n",reduction,send_result);

    memset(&info, 0, sizeof(info));        // from beej
    info.ai_family = PF_UNSPEC; 
    info.ai_socktype = SOCK_DGRAM;
    info.ai_flags = AI_PASSIVE; 

   
    memset(tmp_buff_B, 0, sizeof(tmp_buff_B)); 
    memset(send_data, 0, sizeof(send_data));
    
    sprintf(tmp_buff_B, "%ld" , send_result);

    send_data[0] = 'B';
    send_data[1] = '\t';
    strcat(send_data,tmp_buff_B);

    //printf("Send data : %s \n", send_data);
    
     sendUDP_backserver(portAWS_UDP, (struct sockaddr *)&their_addr, send_data, socket_fd_B);				// send result to aws`
   

    printf("The Server B has successfully finished sending the reduction value to AWS server\n");
    
    
    
    shutdown(socket_fd_B, 2);
     free(data_array);
    
  
    
    
   
    close(socket_fd_B);
    
    usleep(1000);
} 
}
