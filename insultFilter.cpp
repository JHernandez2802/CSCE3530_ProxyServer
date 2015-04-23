using namespace std;

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <locale>
#include <assert.h>

//Size of insult list
#define SIZE 246

struct s_insults{
	string insult;
	int pos;
	int strSize;
} insults[SIZE];

//Function declarations
void addInsults(s_insults insults[]);
void printInsults(s_insults insults[]);
string replaceInsults(string data, s_insults insults[] );

int main(){
	string data;
	
	cout<<"input> ";
		getline(cin,data);
		

	//Add the insults from the text file to 
	//the array of structs
	addInsults(insults);

	//Prints insult list out for verification purposes
	//printInsults(insults);

	//Prints out words before replacing them
	cout<<"You input "<<data<<endl;

	//Replaces insults in test string
	data = replaceInsults(data, insults);

	//Verifies insults have been replaced
	cout<<"This is now "<<data<<endl;

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
        insults[x].insult = data;
		insults[x].pos = 0;
		insults[x].strSize = data.length();
		x++;
     }
	//Close the file
     fin.close( );
     assert(!fin.fail( ));
}

/** @brief          Prints out the insults in the struct array
 *    
 *  @param insults  Array of struct s_insults
 */
void printInsults(s_insults insults[]){
	cout<<"The insults in the array are:"<<endl;
	for(int i=0;i<SIZE;i++)
		cout<<insults[i].insult<<endl;
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
	string temp, buffer, newData;	//Used to convert text in buff to lowercase
	locale loc;
	
	istringstream stream(data);

	while(stream>>buffer){
		//converts buffer to lower case to test for insults
		temp = buffer;
		for(int k=0; k<temp.length(); k++)
			temp[k] = tolower(temp[k],loc);
		temp = temp + "\r";
		
		for(int j=0; j<SIZE; j++){ 
			//Moves to next word if the two strings are of different length
			if(insults[j].strSize !=  temp.length() && (j+1) != SIZE )
				j++;
			if( temp.compare(insults[j].insult) == 0){
				buffer="*****";
				break;
			}
		}
		newData+=" "+buffer;
	}
	cout<<"New data is "<<newData<<endl;
	return newData;

/* 	for(int i=0; i<data.length(); i++){

		//Converts data to lowercase
		temp = data[i];
		for(int k=0; k<temp.length(); k++)
			temp[i] = tolower(temp[i],loc);
		temp = temp + "\r";
		//for loop for the insult list size
		for(int j=0; j<SIZE; j++){ 
			//Moves to next word if the two strings are of different length
			if(insults[j].strSize !=  temp.length() && (j+1) != SIZE )
				j++;
			if( temp.compare(insults[j].insult) == 0){
				data="*****";
				break; //break to move on to the next word in the buffer
			}
			
		}//end inner forloop
	}//end while  */
}

