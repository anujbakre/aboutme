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
 #include <string.h>
 
 #define portAWS "25865"
 #define portAWS_UDP "24865"
 #define portA 21865
 #define portB 22865
 #define portC 23865
 
 #define MAX_LENGTH 1024
 
 int rec_no = 0;
 int flag = 0;
 long result[3];
 
 
 int createTCP()
 {
     int socket_fd;
     struct addrinfo info;
     struct addrinfo *servinfo,*p; 
     char yes = '1';
 
     
     memset(&info, 0, sizeof(info));			// From Beej guide
     info.ai_family =PF_UNSPEC;                 // not version specific
     info.ai_socktype = SOCK_STREAM;            // to create TCP socket
     info.ai_flags = AI_PASSIVE;                // gets IP address of local host
 
     int status =0;
     if((status = getaddrinfo("localhost",portAWS, &info, &servinfo)) != 0)
     {
         fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
         exit(1);
     }
	 // loop through all the results and bind to the first we can
     for( p = servinfo; p!=NULL; p->ai_next )		// From Beej guide
     {
         if ((socket_fd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 	// create socket
         {
                 perror("server: socket");
                 continue;
         }
         
         if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) 	// allow resue of socket
         {
             perror("AWS : setsockopt");
             exit(1);
         }
 
         if((bind(socket_fd, p->ai_addr, p->ai_addrlen)) == -1)			// bind the socket
         {
             close(socket_fd);
             perror("Server : bind");
             continue;
         }
             
         break;           
     }
 
     
 
     if (p == NULL)  
     {
         fprintf(stderr, "server: failed to bind\n");
         exit(1);
     }
 
     freeaddrinfo(servinfo);
 
    
 
      if (listen(socket_fd, 10) == -1) 					// listen on socket created
     {
         perror("server : listen");
         exit(1);
     }
     
      
 
     return socket_fd;
 }
 
 
 int listenUDP()
 {
     int status =0, socket_fd_AWS_UDP;
     struct addrinfo info, *servinfo, *p;
 
     memset(&info, 0, sizeof(info));        // from beej guide
     info.ai_family = PF_INET; 
     info.ai_socktype = SOCK_DGRAM;			// to create UDP socket
     info.ai_flags = AI_PASSIVE; 
 
     if((status = getaddrinfo("localhost", portAWS_UDP, &info, &servinfo)) != 0) 
     {
         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
         return 1;
     }
 
     // loop through all the results and bind to the first we can
     for(p = servinfo; p != NULL; p = p->ai_next) 		// From Beej guide
     {
         if ((socket_fd_AWS_UDP = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 		// create UDP socket
         {
             perror("AWS UDP listener: socket");
             continue;
         }
 
         if (bind(socket_fd_AWS_UDP, p->ai_addr, p->ai_addrlen) == -1) 			// Bind UDP socket
         {
             close(socket_fd_AWS_UDP);
             perror("AWS UDP listener: bind");
             continue;
         }
 
         break;
     }
 
     if (p == NULL) 
     {
         fprintf(stderr, "AWS UDP listener: failed to bind socket\n");
         return 2;
     }
 
     freeaddrinfo(servinfo);
 
     return socket_fd_AWS_UDP;
 }

/* This function take data from rec_buff and differntiates if the result is from server A, B or C.
It then stores the result at prope position in result[] and set the flag for which server it recived data from*/ 
 
 void parse_data_servername(char *rec_buff,int n, char *reduction, int socket_fd)
 {
     
     int i =0;
     char tmp[3*MAX_LENGTH];
     memset(tmp,0, sizeof tmp);
 
     struct sockaddr_in s;
     socklen_t l = sizeof(s);
     if (getsockname(socket_fd, (struct sockaddr *)&s, &l) == -1)
         perror("getsockname");
 
     
     for(i = 2; rec_buff[i] != '\0'; i++)
         tmp[i-2] = rec_buff[i];
 
     if(rec_buff[0] == 'A')
     {
         flag = flag | 0x4;					// set flag for server A
         result[0] = atol(tmp);				// save the result
         printf("The AWS received reduction result of %s from Backend­Server A using UDP over port %d and it is %ld \n", reduction,ntohs(s.sin_port), result[0]);
 
     }
         
 
     if(rec_buff[0] == 'B')
     {
         flag = flag | 0x2;				// set the flag for server B
         result[1] = atol(tmp);			// save the result
         printf("The AWS received reduction result of %s from Backend­Server B using UDP over port %d and it is %ld \n", reduction, ntohs(s.sin_port),result[1]);
     }
         
 
     if(rec_buff[0] == 'C')
     {
         flag = flag | 0x1;				// set the flag for server C
         result[2] = atol(tmp);			// save the result
         printf("The AWS received reduction result of %s from Backend­Server C using UDP over port %d and it is %ld \n", reduction,ntohs(s.sin_port), result[2]);
     }
       
 }
 
 
 /*
 
 This fucntion sperates data into 3 differnt buufer. These buffers are then used to send data to respective servers  */
 
 
 void divide_data(long f_len, long *data_array,long *buff_A, long *buff_B,long *buff_C)
 {
     int i, j;
     
     for(i=0,j =0;i<f_len/3;i++)
     {
         buff_A[i] = data_array[j++];
         
     }
       
     for(i = 0; i < f_len/3;i++)
     {
         buff_B[i] = data_array[j++];
        
     }
    
     
     for(i = 0; i < f_len/3;i++)
     {
         buff_C[i] = data_array[j++];
        
     }
 }
 
 
 /*
 
 this fucntion scaans data in buff, converts it to long data type and stores it in data_array
 
 */
 
 
 int data_parse(long *data_array, char *buff)
 {
     int i = 0;
     char tmp_buff[MAX_LENGTH];
	 
	      
     tmp_buff[MAX_LENGTH-1] = '\n';
    while(buff[i]!= '\0')
     {
         memset(tmp_buff, 0, sizeof(tmp_buff));
         while(buff[i] != '\n')
         {
             tmp_buff[i] = buff[i];
             i++;
         }
         tmp_buff[++i]='\0';
 
         data_array[rec_no] = atol(tmp_buff);
         rec_no++;
     }
 
     return rec_no;
 }
 
 /*
 
 This fucntion sends data to backend servers  */
 
 void sendUDP_backserver(int port,long *buff , int operation, int f_len, int socket_fd)
 {
     struct sockaddr_in addr;
     char operation_flen[MAX_LENGTH];
     char tmp_buff[MAX_LENGTH];
    
       
     operation_flen[0] = operation +'0';
     operation_flen[1] = '\t';
     operation_flen[2] = '\0';
 
     
     sprintf(tmp_buff, "%d", f_len);
     strcat(operation_flen,tmp_buff);
 
     memset(&addr, 0, sizeof(addr));					// from beej guide
     addr.sin_family = PF_INET;
     addr.sin_addr.s_addr = htonl(INADDR_ANY);
     addr.sin_port = htons(port);
 
     int n =0;
 
     if ((n = sendto(socket_fd, operation_flen, strlen(operation_flen),0,(struct sockaddr *)&addr, sizeof(addr))) == -1) 
     {
         perror("server AWS : operation sendto failed");
         exit(1);
     }
     if ((n = sendto(socket_fd, buff,3*MAX_LENGTH,0,(struct sockaddr *) &addr, sizeof(addr))) == -1) 
     {
         perror("server AWS : sendto  failed");
         exit(1);
     }
    
    
 }
 
 
 int main()
{
 	  int x =0;
      
  
     
     /* create TCP socket to communicate with client */
     int socket_fd = createTCP();
     int socket_fd_AWS_UDP = listenUDP();
     
 while(1)
{
      rec_no = 0;
      flag = 0;
     
	 if( x != 0)
         printf("\n\n\n");
     
     x = 1; 
     
     
     printf("The AWS is up and running\n");
         
     int socket_fd_accept =0;
     struct sockaddr_storage their_addr;             // after we accept connection to store info
     size_t addr_size = sizeof(their_addr);
 
     if ( (socket_fd_accept =accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size)) == -1) //accept the connection
     {
         perror("server : listen");
         exit(1);
     }
	 
 
         
     long f_len = 0, rec = 0;
     int n =0;
     char buff[MAX_LENGTH];
     int operation = 0;
    
     if((recv(socket_fd_accept,buff, MAX_LENGTH, 0)) < 0)			// recive operation and # of integers
     {
         perror("recv 1");
         exit(1);
     }
   
     
     char  tmp_buff[MAX_LENGTH];
     memset(tmp_buff, 0, sizeof(tmp_buff));
 
     /* spearate operation and file length from data recived*/
     for (n = 0; buff[n]!='\t' ; n++)
     {
         tmp_buff[n] = buff[n];
     }
     tmp_buff[++n] = '\0';
 
     char *reduction;       
     if (!strcmp("MIN", tmp_buff))
     {
         operation = 1;
         reduction = "MIN";
     }
     if(!strcmp("MAX", tmp_buff))
     {
         operation = 2;
         reduction = "MAX";
     }
     if (!strcmp("SUM", tmp_buff))
     {
         operation = 3;
         reduction = "SUM";
     }
     if (!strcmp("SOS", tmp_buff))
     {
         operation = 4;
         reduction = "SOS";
     }
 
 
 	memset(tmp_buff, 0, sizeof(tmp_buff));
    
 	int i =0;
     for (i = 0;n < MAX_LENGTH && buff[n] !='\0'; n++, i++)
     {
         tmp_buff[i] = buff[n];
     }
     i++;
     tmp_buff[i] = '\0';
 		
     f_len = atol(tmp_buff);
 	
 
     long *data_array =(long *)malloc(f_len * sizeof(long));
 
     while(rec < f_len)									// keep receiving data
     {
          if((n = recv(socket_fd_accept, buff, MAX_LENGTH,0)) < 0)
         {
             perror("recv");
             exit(1);
         }
        
        buff[n] = '\0';
		
		if(rec == f_len -1)
		{
			buff[n] ='\n';
			buff[n+1] = '\0';
		}
			 
		rec = data_parse(data_array,buff);                         
 
         //read more data untill all recived.
     }
 
     
 
 
     struct sockaddr_in s;
     socklen_t l = sizeof(s);
     if (getsockname(socket_fd_accept, (struct sockaddr *)&s, &l) == -1)
         perror("getsockname");
   
     printf("The AWS has received %ld numbers from the client using TCP over port %d\n",f_len, ntohs(s.sin_port) );
 
 
 
     long *buff_A = (long *)malloc((f_len/3) * sizeof(long));			// allocate memory dynamically to hold data to be send to server A, server B, server C
     if(buff_A == NULL)
         printf("memory not allocated\n");
     long *buff_B = (long *)malloc((f_len/3) * sizeof(long));
     if(buff_B == NULL)
         printf("memory not allocated\n");
     long *buff_C = (long *)malloc((f_len/3) * sizeof(long));
     if(buff_C == NULL)
         printf("memory not allocated\n");
 
     memset(buff_A, 0, (f_len/3) * sizeof(long));
     memset(buff_B, 0, (f_len/3) * sizeof(long));
     memset(buff_C, 0, (f_len/3) * sizeof(long));
 
 
     divide_data(f_len,data_array,buff_A, buff_B,buff_C);  
     
     /* all the data has been received now send data to server A, B, C using UDP */
 
     /* send operartion data to server A B C*/                                 
      
         
     sendUDP_backserver(portA, buff_A , operation, f_len/3, socket_fd_AWS_UDP);
     printf("The AWS sent %ld numbers to Backend­Server A\n",f_len/3);
 
     sendUDP_backserver(portB, buff_B , operation, f_len/3, socket_fd_AWS_UDP);
     printf("The AWS sent %ld numbers to Backend­Server B\n",f_len/3);
 
     sendUDP_backserver(portC, buff_C , operation, f_len/3, socket_fd_AWS_UDP);
     printf("The AWS sent %ld numbers to Backend­Server C\n",f_len/3);
 
     
     
     char rec_buff[3*MAX_LENGTH];
     memset(&rec_buff, 0, sizeof(rec_buff));
   
     size_t addr_len = sizeof(their_addr);       
 
     while(1)            //wait for server A B C to finish working on data ans send processed data  
     {
         
         if((n = recvfrom(socket_fd_AWS_UDP, rec_buff, MAX_LENGTH , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) 
         {
             perror("recvfrom");
             exit(1);
 	    }
         
 
         parse_data_servername(rec_buff, n, reduction, socket_fd_AWS_UDP);  // flags are 0x4 A, 0x2 B, 0x1 C. 
 
         if(flag == 0x7)            // all the backend servers have done processing 
             break;
 
                      
     
     }
 
 
 
     //all processing done, now proceess data at server and send to client back.
     long min = result[0], max = result[0];
     long send_result = 0;
    
     switch(operation)
     {
         case 1:  
                     for(i = 0; i < 3; i++)
                         if(result[i] < min)
                             min = result[i];
                     send_result = min;
                     break;
         case 2:
                     for(i = 0; i < 3; i++)
                         if(result[i] > max)
                             max = result[i];
                     send_result = max;
                     break;
         case 3: 
                     for(i = 0; i < 3; i++)
                         send_result += result[i];
                     break;
 
         case 4: 
                     for(i = 0; i < 3; i++)
                         send_result += result[i];
                     break;
 
         default: fprintf(stderr,"error recieving data from backend servers: invalid operartion");
                     exit(1);
 
     }
 
     printf("The AWS has successfully finished the reduction %s: %ld\n", reduction, send_result);
 
   
     
     if((send(socket_fd_accept,&send_result, sizeof(send_result),0)) < 0)			// send final result back to client
     {
         perror("send to client");
         exit(1);
     }
 
     printf("The AWS has successfully finished sending the reduction value to client.\n");
     
     free(buff_A);
     free(buff_B);
     free(buff_C);
     free(data_array);
    // close(socket_fd_accept);
     
     usleep(1000);
}
     
     shutdown(socket_fd,2);
     shutdown(socket_fd_AWS_UDP, 2);
    
     
    
     //free(send_buff);
 
     close(socket_fd);
     
     close(socket_fd_AWS_UDP);
     
     
 
     
     return 0;
 }
