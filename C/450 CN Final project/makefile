all:
	gcc -o AWS aws.c -lnsl -lsocket -lresolv
	gcc -o server_A serverA.c -lnsl -lsocket -lresolv
	gcc -o server_B serverB.c -lnsl -lsocket -lresolv
	gcc -o server_C serverC.c -lnsl -lsocket -lresolv
	gcc -o client client.c -lnsl -lsocket -lresolv

serverA: 
	./server_A

serverB: 
	./server_B

serverC: 
	./server_C

aws: 
	./AWS

clean : 
	rm AWS server_A server_B server_C client
