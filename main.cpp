using namespace std;
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h> 
#include <iostream>

//predicates
int hostname_to_ip(char *name , char *ip);
char* getDestination(char Buffer[1000]);
void *client_handler(void *sock_desc);
char* make_request(char* destination);
char* handleResponse(int, int);

int threadCount = 0;

int main(){

	//create and initialize socket address structrure
    struct sockaddr_in socketAddress;
    memset(&socketAddress, '0', sizeof(socketAddress));

    struct sockaddr_in clientAddress;  
    memset(&clientAddress, '0', sizeof(clientAddress));

    //create and initialze server buffer space
    char serverBuff[1000];
    memset(serverBuff, '0', sizeof(serverBuff)); 

    int listenfd = 0, connfd = 0;

    //create socket descriptor, using the IPv4 protocol, a socket stream
    // and default protocol '0'
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //set socket address parameters.
    socketAddress.sin_family = AF_INET;
    //used INADDR_ANY for simplicity, allows server to run regardless of IP
    socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    //user port 65001
    socketAddress.sin_port = htons(65001);

	//bind socket address parameters to socket descriptor created above
    bind(listenfd, (struct sockaddr*)&socketAddress, sizeof(socketAddress)); 

    //mark the socket descriptor as passive, so that it can continually allow
    //accept() calls. set the backlog queue to 10.
    listen(listenfd, 10);


    while(1){
	    //create c-friendly variable to signify length
	    socklen_t cliLength = sizeof(clientAddress);
	    //wait for connection
	    connfd = accept(listenfd, (struct sockaddr*)&clientAddress, &cliLength);

	    //thread set-ups
	    pthread_t a_thread;
	    int *sock_tmp, status;
	    sock_tmp = (int*)malloc(sizeof(int));
	    *sock_tmp = connfd; 
		threadCount++;
		printf("Client %d connected. Creating new thread.\n", threadCount);
		status = pthread_create(&a_thread, NULL, client_handler, (void *)sock_tmp);
	}

	return 0;
}

char* getDestination(char Buffer[1000]){
	char tempBuff[1000];
	strcpy(tempBuff, Buffer);

    //pull the server name
    char * pch;
    pch = strtok (tempBuff,"\n");

    char * pci;
    pci = strtok (pch, " ");

    while (pci != NULL){
  		char *temp = (char*)malloc(sizeof(char)*(strlen(pci)-1));
  		if(pci[0] == '/'){
  			memcpy(temp, &pci[1], (strlen(pci)-1));
  			char *ip = (char*)malloc(sizeof(char)*100);
  			if(hostname_to_ip(temp , ip) != 1)
  			   return ip;
            else
                return NULL;
  		}
    	pci = strtok (NULL, " \n");
  	}
}

//use c library functions to query dns servers
int hostname_to_ip(char *name , char *ip){
    struct hostent *host;
    struct in_addr **addr_list;
    int i;
         
    if ((host = gethostbyname(name)) == NULL){
        // get the host info
        herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) host->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++){
        strcpy(ip , inet_ntoa(*addr_list[i]));
        //return after finding one entry
        return 0;
    }
    //no match, return fail
    return 1;
}

//make a request to the provided destination
char* make_request(char* destination){
	char* request = (char*)malloc(sizeof(char)*100);
	sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", "/", destination);
	return request;
}

//handler for each thread
void *client_handler(void *sock_desc){
	int sock = *(int*)sock_desc;
	char temp[1000];
	bzero(temp, 1000);
	int user = threadCount;

    //recv the local response
	if(recv(sock, temp, 1000, 0) > 100){
        //turn destination name into ip address
    	char *destination = getDestination(temp);

        //add a check to cancel out multiple requests with no destination
        if(destination != NULL){
        //if(strlen(destination) > 1){
            fprintf(stderr, "Client %d: Destination is '%s'\n", user, destination);

           	//create and initialize server connection
        	struct sockaddr_in serverAddress;
        	memset(&serverAddress, '0', sizeof(serverAddress)); 

        	int serverDesc = 0;

        	//create socket based on basic and ambigious parameters
        	if((serverDesc = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                fprintf(stderr, "Error: Could not create socket \n");
            } 
            //set socket address attributes
            //tests to make sure second user string is an ip address
            if(inet_pton(AF_INET, destination, &serverAddress.sin_addr)<=0)
            {
                fprintf(stderr, "Please pass an ip address.\n");
                //exit(EXIT_FAILURE);
            }  
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(80);
            //attempt connection with assigned socket
            if(connect(serverDesc,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
            {
               	fprintf(stderr, "Error: Connect Failed \n");
            }
            printf("Client %d: Proxy connected to destination server.\n", user);

            //generate request based on passed destination
            char *request = make_request(destination);
            fprintf(stderr, "Client %d: Request:\n%s", user, request);

            //send request to socket
            if(send(serverDesc,request,strlen(request),0) < 0)
                fprintf(stderr, "Error sending request.\n");

            //change socket settings so it is non-blocking
            fcntl(serverDesc, F_SETFL, O_NONBLOCK);

            //handle response from the server
            char *response = handleResponse(serverDesc, 4);

            close(serverDesc);

           	send(sock, response, strlen(response), 0);
       }
    }

   	close(sock);

}

char *handleResponse(int sock, int timeout)
{
    int bytesReceived = 0;
    int totalBytes= 0;
    struct timeval start, now;
    char recvChunk[1024];
    double difference; 

    //start time
    gettimeofday(&start , NULL);
     
    //string to accumulate chunks
    string responseCatch = "";

    while(1){
        //get the current time
        gettimeofday(&now , NULL);
        //now calculate the time elapsed in seconds
        difference = (now.tv_sec - start.tv_sec) + 1e-6 * (now.tv_usec - start.tv_usec);
        //if no data is received at all, and we have passed twice
        //the timeout limit, we're done
        if((timeout * 2) < difference){
            break;
        }
        //if we have passed timeout limit, AND we have some bytes,
        //then we pulled a chunk that was not 1024 bits, and are
        //done receiving more
        else if((difference > timeout) && (totalBytes > 0)){
            break;
        }
        //reset the chunk variable
        memset(recvChunk, 0 , 1024);
        if((bytesReceived = recv(sock, recvChunk , 1024 , 0)) < 0){
            //delay if nothing was received, just for 0.1 seconds
            //in case more is on the way or being processed
            usleep(100000);
        }
        else{
            //reset start time
            gettimeofday(&start , NULL);
            //add received bytes to total
            totalBytes += bytesReceived;
            responseCatch += recvChunk;
            fprintf(stderr,"%s",recvChunk);
            //stop when we receive last portion of html
            if(strstr(recvChunk, "</html>"))
                break;
        }
    }

    //convert string back to c string
    char *response = (char*)malloc(sizeof(char)*responseCatch.length());
    strcpy(response, responseCatch.c_str());
    fprintf(stderr, "%s\n", response);
    
    //return converted response
    return response;
}