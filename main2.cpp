//file main.cpp

using namespace std;

#include <fstream>
#include <locale>
#include <iostream>
#include <sstream>
#include <string>
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
#include <sqlite3.h>
#include <assert.h>

#define SIZE 246

struct s_insults{
	string insult;
	int pos;
	int strSize;
} insults[SIZE];

//predicates
char* getDestination(char Buffer[1000]);
char* getName(char Bufferr[1000]);
char* getData(char Buffer[1000]);
int hostname_to_ip(char *name , char *ip);
char* make_initial_request(char* destination, char* data);
char* make_successive_request(char* data);
void *client_handler(void *args);
char* handleResponse(int, int, int);
char* getName(char Bufferr[1000]);
static int getRecord(void *NotUsed, int argc, char **argv, char **azColName);
void addInsults(s_insults insults[]);
void printInsults(s_insults insults[]);
string replaceInsults(string data, s_insults insults[] );
bool checkInsults (string data, s_insults[]);
int addBlacklist(char strArr[50][50], int total);

char curHost[50] = "";
int threadCount = 0;
char IP[20] = "";
char data2[100000] = "";
bool newEntry = true;
bool blacklisted = false;
char blacklist[50][50];

struct Arguments {
    char ip[INET_ADDRSTRLEN];
    int sock;
};

int main(){
	//Add the insults from the text file to 
	//the array of structs
	addInsults(insults);

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
    socketAddress.sin_port = htons(8888);

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

        //set up a structure to transfer data to thread
        struct Arguments args;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddress.sin_addr.s_addr, ip, INET_ADDRSTRLEN);
        strcpy(args.ip, ip);
        args.sock = connfd;
        //print to log
		fprintf(stderr, "Client %d connected (%s). Creating new thread.\n", threadCount, ip);
        //spawn new thread
		status = pthread_create(&a_thread, NULL, client_handler, (void *)&args);
	}
	return 0;
}

//get the destination from a request packet
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
      		char *temp = NULL;
            char* placeholder;
            //covers www.s and http://www.s
            if((placeholder = strstr(pci, "www.")) != NULL){
                //comment to filter out www. (not necessary)
                //placeholder = placeholder + 4;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder))+1);
                memcpy(temp, placeholder, (strlen(placeholder)));
                int pos = strlen(placeholder);
                temp[pos] = '\0';
            }
            //covers just http://
            else if((placeholder = strstr(pci, "http://")) != NULL){
                placeholder = placeholder + 7;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder))+1);
                memcpy(temp, placeholder, (strlen(placeholder)));
                int pos = strlen(placeholder);
                temp[pos] = '\0';
            }
            else if((placeholder = strstr(pci, "https://")) != NULL){
                placeholder = placeholder + 8;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder))+1);
                memcpy(temp, placeholder, (strlen(placeholder)));
                int pos = strlen(placeholder);
                temp[pos] = '\0';
            }
      		else{
                placeholder = pci + 1;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder))+1);
                memcpy(temp, placeholder, (strlen(placeholder)));
                int pos = strlen(placeholder);
                temp[pos] = '\0';
      			//memcpy(temp, &pci[1], (strlen(pci)-1));      			
      		}
            char *ip = (char*)malloc(sizeof(char)*100);
            fprintf(stderr, "Destination: %s\n", temp);

            char* token2 = strtok(temp, "/");
            temp = token2;

            if(hostname_to_ip(temp , ip) != 1)
                return ip;
            else
                return NULL;
        }
        //get next chunk
    	pci = strtok (NULL, " \n");
  	}
}

char* getData(char Buffer[1000]){
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
            char *temp = NULL;
            char* placeholder;
            //covers www.s and http://www.s
            if((placeholder = strstr(pci, "www.")) != NULL){
                //comment to filter out www. (not necessary)
                //placeholder = placeholder + 4;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder))+1);
                memcpy(temp, placeholder, (strlen(placeholder)));
                int pos = strlen(placeholder);
                temp[pos] = '\0';
            }
            //covers just http://
            else if((placeholder = strstr(pci, "http://")) != NULL){
                placeholder = placeholder + 7;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder))+1);
                memcpy(temp, placeholder, (strlen(placeholder)));
                int pos = strlen(placeholder);
                temp[pos] = '\0';
            }
            else if((placeholder = strstr(pci, "https://")) != NULL){
                placeholder = placeholder + 8;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder))+1);
                memcpy(temp, placeholder, (strlen(placeholder)));
                int pos = strlen(placeholder);
                temp[pos] = '\0';
            }
            else{
                placeholder = pci + 1;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder))+1);
                memcpy(temp, placeholder, (strlen(placeholder)));
                int pos = strlen(placeholder);
                temp[pos] = '\0';
                //memcpy(temp, &pci[1], (strlen(pci)-1));               
            }
            char *ip = (char*)malloc(sizeof(char)*100);

            char* token2 = strtok(temp, "/");
            token2 = strtok(NULL, "/");
            
            char *data = (char*)malloc(sizeof(char)*200);
            data[0] = '/';

            while(token2 != NULL){
                strcat(data, token2);
                strcat(data, "/");
                token2 = strtok(NULL, "/");
            }
            int pos = strlen(data);
            pos--;
            data[pos] = '\0';

            temp = data;
            if(temp != NULL)
                return temp;
            else
                return NULL;
        }
        //get next chunk
        pci = strtok (NULL, " \n");
    }
}

//get the name of the destination (not ip) from request
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
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)+1));
                strncpy(temp, placeholder, (strlen(placeholder)));
                return temp;
            }
            //covers just http://
            else if((placeholder = strstr(pci, "http://")) != NULL){
                placeholder = placeholder + 7;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)+1));
                strncpy(temp, placeholder, (strlen(placeholder)));
                return temp;
            }
            else if((placeholder = strstr(pci, "https://")) != NULL){
                placeholder = placeholder + 8;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)+1));
                strncpy(temp, placeholder, (strlen(placeholder)));
                return temp;
            }
            else{
                placeholder = pci + 1;
                temp = (char*)malloc(sizeof(char)*(strlen(placeholder)+1));
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
char* make_initial_request(char* destination, char* data){
	char* request = (char*)malloc(sizeof(char)*400);
	sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nContent-Type: text/html\r\n\r\n", data, destination);
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
void *client_handler(void *args){

    //make variable to hold arguments received at thread creation
    struct Arguments *arguments = (Arguments*)args;
    int sock = arguments->sock;
	char temp[5000];
	memset(temp, '\0', 5000);
	int user = threadCount;
	
	// Used for SQLite
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql, strSql[110000];
	const char* data = "Callback function called";
	int blacklistTotal = 0;
	
	// resets data
	strcpy(IP, "\0");
	strcpy(data2, "\0");
	newEntry = true;
	blacklisted = false;

	//db setup and blacklist init
	rc = sqlite3_open("group3ProxyDB.db", &db);
	blacklistTotal = addBlacklist(blacklist, blacklistTotal);
	
    //recv the local response
	if(recv(sock, temp, 5000, 0) > 100){

        //turn destination name into ip address
    	char *destination = getDestination(temp);

        char* data = getData(temp);
        if (data == NULL || strcmp(data, "") == 0){
            data = (char*)malloc(sizeof(char)*2);
            strcpy(data, "/");
        }



        //add a check to cancel out multiple requests with no destination
        if(destination == NULL){
            destination = (char*)malloc(sizeof(char)*strlen(curHost));
            strcpy(destination, curHost);
        }
        fprintf(stderr, "Client %d: Destination is '%s', Data is '%s', Source was '%s'\n", user, destination, data, arguments->ip);
		
    	// Checks blacklist
		for(int i = 0; i < blacklistTotal; i++){
            //site was found on blacklist
			if (strcmp(blacklist[i],destination) == 0 || strcmp(blacklist[i],getName(temp)) == 0){
				blacklisted = true;
                fprintf(stderr, "Destination, %s, is blacklisted, exiting thread.\n", destination);
                char* response = (char*)malloc(sizeof(char)*400);
                sprintf(response, "HTTP/1.1 200 OK\r\n\r\n<!DOCTYPE html><body>Sorry, the site you tried to access is blocked.</body></html>");
                send(sock, response, strlen(response), 0);
                close(sock);
                pthread_exit(0);
				break;
			}
		}
		
		// Checks if the IP is already in the cache DB
		sprintf(strSql, "SELECT * FROM cache where IP='%s'", destination);
		sql = &strSql[0];
		rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg);
		// Flags newEntry that the IP is already in the DB
		if (strlen(IP) < 3)
			newEntry = true;

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
        }  
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(80);
        //attempt connection with assigned socket
        if(connect(serverDesc,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        {
           	fprintf(stderr, "Error: Connect Failed \n");
        }
        fprintf(stderr, "Client %d: Proxy connected to destination server.\n", user);

        
        //generate request based on passed destination
        char *request;
        if(strstr(getName(temp),".com") || strstr(getName(temp),".edu") || strstr(getName(temp),".net") || strstr(getName(temp),".org")){
            char *host = (char*)malloc(sizeof(char)*100);
            strcpy(host, getName(temp));
            char* token3 = strtok(host, "/");
            request = make_initial_request(token3, data);
        }
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

        //receive and handle response from destination		
        char *response = handleResponse(serverDesc, 3, sock);
		
        //close the stream to the destination server and flush stderr
        close(serverDesc);
        fflush(stderr);  

		// Checks if the IP accessed it new or not
		if (newEntry)
		{// Saves IP and response to DB if it is new
			strcpy(IP, destination);
			strcpy(data2, response);
			sprintf(strSql, "INSERT INTO cache VALUES('%s','%s');", IP, data2);	
			sql = &strSql[0];
			rc = sqlite3_exec(db, sql, getRecord, (void*)data, &zErrMsg);
		}
        //entry is already saved in cache and stored in data2 variable
        else{
            fprintf(stderr, "%s was found in the cache.\n", destination);
            //send cached data to local server
        }
		sqlite3_close(db);     
    }
    //close up stream to local system and free up other assets
   	close(sock);
    fflush(stderr);
    pthread_exit(0);
}

char *handleResponse(int sock, int timeout, int localSock){
    //init variables
    int bytesReceived = 0;
    int totalBytes= 0;
    struct timeval start, now;
    char recvChunk[512];
    double difference; 

    //start time
    gettimeofday(&start , NULL);
     
    //string to accumulate chunks
    string responseCatch;

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
        memset(recvChunk, '\0', 512);
        if((bytesReceived = recv(sock, recvChunk , 512 , 0)) < 0){
            //delay if nothing was received, just for 0.1 seconds
            //in case more is on the way or being processed
            usleep(100000);
        }
        else{
            //reset start time
            gettimeofday(&start , NULL);
            //add received bytes to total
            totalBytes += bytesReceived;
            if(responseCatch.empty())
                    responseCatch = recvChunk;
            else
                responseCatch += recvChunk;
        }
    }

	//filters insults out of string
	responseCatch=replaceInsults(responseCatch, insults);
	
    //convert string back to c string
    char *response = (char*)malloc(sizeof(char)*responseCatch.length());
    strcpy(response, responseCatch.c_str());
    strcat(response, "\0");

    //change total number of bytes received
    fprintf(stderr, "**Received %d bytes**\n", totalBytes);

    send(localSock, response, totalBytes, 0);
    return response;
}

// Gets record from database
static int getRecord(void *NotUsed, int argc, char **argv, char **azColName)
{
	strcpy(IP, argv[0]);
	strcpy(data2, argv[1]);
		
	return 0;
}

/** @brief          Adds insults to struct array
 *
 *  @details        Grabs the insults from "insults.txt" and adds
 *                  them to the insults struct array
 *  @param insults  Array of struct s_insults
 */
void addInsults(s_insults insults[]){
	//Variables
	int x=0;
	string data;
	ifstream fin;

	//Opens file
    fin.open("insults.txt",ios::in);
    assert (!fin.fail( ));    

	//Reads from the file until the end of file
    while ( !fin.eof( ) ){
		getline(fin,data);
		for(int i=0;i<data.length()-1;i++)
			insults[x].insult += data[i];
		insults[x].pos = 0;
		insults[x].strSize = data.length() -1;
		x++;
     }
	//Close the file
     fin.close( );
     assert(!fin.fail( ));
}

/** @brief          Replaces words that are insults
 * 
 *  @details        Compares words in the buffer with a predetermined 
 *                  list of insults. 
 *    
 *  @param data     String of data that may contain insults
 *
 *  @param insults  Array of struct s_insults that contains the insults
 */
string replaceInsults(string data, s_insults insults[] ){
 	//Variables
	string goodStr, compareStr, newData, blank;
	locale loc;
	int counter=0;

	for(int i=0; i<data.length(); i++){
		//Checks to see if character is NOT a new line 
		//character or white-space
		if(data[i]!=' ' || data[i]!='\0' 
		|| data[i]!='\r'|| data[i]!='\n'){
			//Creates compareStr string character by character
			//and makes them lower-case to prevent filter bypass
			compareStr+=data[i];
			goodStr+=data[i];
			compareStr[counter] = tolower(compareStr[counter],loc);
			counter++;
			//Looks ahead for new line characters and
			//white-spaces to know that a word is complete
 			if(data[i+1]==' ' || data[i+1]=='\0' 
			|| data[i+1]=='\r'|| data[i+1]=='\n'
			|| data[i+1]=='<'){
				//Runs if a word has been completed
				if(checkInsults(compareStr,insults)){
					//Replaces word and resets strings and counter
					compareStr="*****";
					newData+=compareStr;
					compareStr.clear();
					goodStr.clear();
					counter=0;
				}
				else{
					//Adds word to return string and resets strings and counter
					newData+=goodStr;
					compareStr.clear();
					goodStr.clear();
					counter=0;
				}
			}
		}
		//Checks to see if character IS a new line 
		//character or white-space
		if(data[i]==' ' || data[i]=='\0' 
		|| data[i]=='\r'|| data[i]=='\n'){
			//Adds white-spaces and new lines to return string
			//and resets strings and counters
			newData+=data[i];
			compareStr.clear();
			goodStr.clear();
			counter=0;
		}
	}
	
	return newData;
}

/** @brief          Compares word against insults list
 * 
 *  @details        Compares words in the buffer with a predetermined 
 *                  list of insults. 
 *    
 *  @param data     String to be compared against insults list
 *
 *  @param insults  Array of struct s_insults that contains the insults
 * 
 *  @return         Returns boolean value TRUE if an insult is matched
 *                  and a boolean value of FALSE if no insult is matched
 */
bool checkInsults(string data, s_insults insults[]){
	 for(int j=0; j<SIZE; j++){ 
		//Moves to next word if the two strings are of different length
		if(insults[j].strSize !=  data.length() && (j+1) != SIZE )
			j++;
		//Compares the two words and returns true if they match
		if( data.compare(insults[j].insult) == 0){
			return true;
		}
	}
	//If no insult has been found return false
	return false;
}

// Adds IP's from blacklist.txt
int addBlacklist(char strArr[50][50], int total){
	//Variables
	int x=0;
	string data;
	ifstream fin;

	//Opens file
    fin.open("blacklist.txt",ios::in);
    assert (!fin.fail( ));    

	//Reads from the file until the end of file
    while ( !fin.eof( ) ){
		getline(fin,data);
        strcpy(strArr[x], data.c_str());
		total++;
		x++;
     }
	//Close the file
     fin.close( );
     assert(!fin.fail( ));
	 return total;
}
