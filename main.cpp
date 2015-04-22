using namespace std;
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
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
char* make_initial_request(char* destination);
char* make_successive_request(char* data);
int getResponseSize(int sock, int timeout);
char* handleResponse(int, int, int);
char* getName(char Bufferr[1000]);

char curHost[50] = "";
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
    listen(listenfd, 100);


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
		fprintf(stderr, "Client %d connected. Creating new thread.\n", threadCount);
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
        //chunk with '/' as first character holds address
        if(pci[0] == '/'){
      		char *temp;
            char* placeholder;
            //covers www.s and http://www.s
            if((placeholder = strstr(pci, "www.")) != NULL){
                //comment to filter out www. (not necessary)
                //placeholder = placeholder + 4;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)+1));
                memcpy(temp, placeholder, (strlen(placeholder)));
            }
            //covers just http://
            else if((placeholder = strstr(pci, "http://")) != NULL){
                placeholder = placeholder + 7;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)+1));
                memcpy(temp, placeholder, (strlen(placeholder)));
            }
            else if((placeholder = strstr(pci, "https://")) != NULL){
                placeholder = placeholder + 8;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)+1));
                memcpy(temp, placeholder, (strlen(placeholder)));
            }
      		else{
                placeholder = pci + 1;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)+1));
                memcpy(temp, placeholder, (strlen(placeholder)));
      			//memcpy(temp, &pci[1], (strlen(pci)-1));      			
      		}
            char *ip = (char*)malloc(sizeof(char)*100);
            fprintf(stderr, "Destination: %s\n", temp);
            if(hostname_to_ip(temp , ip) != 1)
                return ip;
            else
                return NULL;
        }
        //get next chunk
    	pci = strtok (NULL, " \n");
  	}
}

char* getName(char Buffer[1000]){
    char tempBuff[1000];
    strcpy(tempBuff, Buffer);

    //pull the server name
    char * pch;
    pch = strtok (tempBuff,"\n");

    char * pci;
    pci = strtok (pch, " ");

    while (pci != NULL){
        if(pci[0] == '/'){
            char *temp;
            char* placeholder;
            //covers www.s and http://www.s
            if((placeholder = strstr(pci, "www.")) != NULL){
                if(strstr(pci, "http://") != NULL)
                    placeholder = placeholder + 7;
                //placeholder = placeholder + 4;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)));
                strncpy(temp, placeholder, (strlen(placeholder)));
                return temp;
            }
            //covers just http://
            else if((placeholder = strstr(pci, "http://")) != NULL){
                placeholder = placeholder + 7;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)));
                strncpy(temp, placeholder, (strlen(placeholder)));
                return temp;
            }
            else if((placeholder = strstr(pci, "https://")) != NULL){
                placeholder = placeholder + 8;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)));
                strncpy(temp, placeholder, (strlen(placeholder)));
                return temp;
            }
            else{
                placeholder = pci + 1;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)));
                strncpy(temp, placeholder, (strlen(placeholder)));
                return temp;           
            }
        }
        pci = strtok (NULL, " \n");
    }

    return NULL;
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
char* make_initial_request(char* destination){
	char* request = (char*)malloc(sizeof(char)*400);
	sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", "/", destination);
    strcpy(curHost, destination);
    fprintf(stderr, "Changed current host to: %s\n", curHost);
	return request;
}

char* make_successive_request(char* data){
    char* request = (char*)malloc(sizeof(char)*400);
    sprintf(request, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", data, curHost);
    return request;
}

//handler for each thread
void *client_handler(void *sock_desc){
	int sock = *(int*)sock_desc;
	char temp[5000];
	memset(temp, '\0', 5000);
	int user = threadCount;

    //recv the local response
	if(recv(sock, temp, 5000, 0) > 100){
        //turn destination name into ip address
    	char *destination = getDestination(temp);
        //exit(EXIT_FAILURE);
        //add a check to cancel out multiple requests with no destination
        if(destination == NULL){
            destination = (char*)malloc(sizeof(char)*strlen(curHost));
            strcpy(destination, curHost);
        }
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
            fprintf(stderr, "Client %d: Proxy connected to destination server.\n", user);

            char *request;
            //generate request based on passed destination
            if(strstr(getName(temp),".com") || strstr(getName(temp),".edu") || strstr(getName(temp),".net") || strstr(getName(temp),".org"))
                request = make_initial_request(destination);
            else{
                fprintf(stderr,"DATA REQUESTED: %s\n", getName(temp));
                request = make_successive_request(getName(temp));
            }
            fprintf(stderr, "Client %d: Request:\n%s", user, request);

            //send request to socket
            if(send(serverDesc,request,strlen(request),0) < 0)
                fprintf(stderr, "Error sending request.\n");

            //change socket settings so it is non-blocking
            fcntl(serverDesc, F_SETFL, O_NONBLOCK);

            char *response = handleResponse(serverDesc, 4, sock);

            close(serverDesc);       
    }

   	close(sock);

}

int getResponseSize(int sock, int timeout){
    int bytesReceived = 0;
    int totalBytes = 0;
    struct timeval start, now;
    char recvChunk[1024];
    double difference; 

    //start time
    gettimeofday(&start , NULL);

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
            //stop when we receive last portion of html
            if(strstr(recvChunk, "</html>"))
                break;
        }
    }

    //fprintf(stderr, "%s\n", response);
    fprintf(stderr, "Received %d bytes\n", totalBytes);

    return totalBytes;
}

char *handleResponse(int sock, int timeout, int localSock)
{
    int bytesReceived = 0;
    int totalBytes= 0;
    struct timeval start, now;
    char recvChunk[1025];
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
        memset(recvChunk, '\0', 1025);
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
            //stop when we receive last portion of html
            if(strstr(recvChunk, "</html>"))
                break;
        }
    }

    //convert string back to c string
    char *response = (char*)malloc(sizeof(char)*responseCatch.length());
    strcpy(response, responseCatch.c_str());
    strcat(response, "\0");
    //fprintf(stderr, "%s\n", response);
    fprintf(stderr, "**Received %d bytes**\n", totalBytes);

    //return converted response

    //change socket settings so it is non-blocking
    fcntl(localSock, F_SETFL, O_NONBLOCK);

    send(localSock, response, totalBytes, 0);
    return response;
}