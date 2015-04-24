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
bool checkInsults (string data, s_insults[]);

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
			|| data[i+1]=='\r'|| data[i+1]=='\n'){
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
