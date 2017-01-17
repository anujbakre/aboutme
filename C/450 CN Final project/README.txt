A. Name
B. Student ID


C & D.	DESCRIPTION
	
	The project implements a simple client server architecture which 
	can perform computational offloading over 3 backend servers to reduce computation 
	time. 
	
	serverA.c / serverB.c/ serverC.c :
		These are the 3 backend servers. They receive data from AWS, compute results
		send it back to aws server.

	aws.c :
		This is the aws server. It receives data from client and operation to be performed. 
		The data is then distributed among 3 backend servers by aws. when aws receives results 
		from backend servers, it performs some additional operations on them and send final 
		result back to client.

	client.c : 
		This is client which requests aws server do perform operation. It reads data from a file
		named "nums.csv" and sends to aws server. It then displays the final result obtained from 
		aws.
		
	
E.	EXECUTING STEPS

	1. make all
	2. make serverA
	3. make serverB
	4. make serverC
	5. make aws
	6. ./client <function>

*note: replace <function> with operation to be performed(min,max,sum,sos) 


F.	FORMAT OF MESSAGES EXCHANGED

	1. client to aws:
		First packet from client to aws contains the function name and number of integers is file
		as a string separated by a 	'\t': "SUM	300" and the next packets carry integers cast into 
		a char array.
		
	2. aws to backend servers:
		First packet from aws to backend server contains the function number and number of integers 
		on to perform operation as a string separated by a 	'\t': "1	100" and the next packets carry 
		integers cast into a long array.
		
	3. backend servers to aws:
		Only one packet is sent from backend server to aws. It consists of server identifier (A/B/C) and the 
		result separated by '\t': "A	1000" in char array format. 
		
	4. aws to client:
		only one packet is sent from aws to client. It consists of final computed result in long format
		: 3000
		
		
G.	IDIOSYNCRASY IN PROJECT
	
	If the final result of any computation exceeds range of size32_t data type, 
	overflow can cause unpredictable results.



REFERENCES 

1. Beej's Guide to Network Programming - http://beej.us/guide/bgnet/output/html/multipage/index.html









 








