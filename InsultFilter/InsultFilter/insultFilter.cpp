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
void replaceInsults(string mystr[],int, s_insults insults[] );

int main(){
	string mystr;
	string buff[5]={"Fuck","slap","bastard","Alcoholic","master"};

	int buffSize=sizeof(buff)/sizeof(buff[0]);

	//Add the insults from the text file to 
	//the array of structs
	addInsults(insults);

	//Prints insult list out for verification purposes
	//printInsults(insults);
	
	//Prints out words before replacing them
	cout<<"Words in buff array before replacement"<<endl;
	for(int i=0;i<5;i++)
		cout<<buff[i]<<" ";
	cout<<"\n"<<endl;

	//Replaces insults in test string
	replaceInsults(buff, buffSize, insults);

	//Verifies insults have been replaced
	cout<<"Words in buff array after replacement"<<endl;
	for(int i=0;i<5;i++)
		cout<<buff[i]<<" ";
	cout<<"\n"<<endl;

	//Used to keep command prompt open in Visual Studio
	getline(cin,mystr);

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

 /*    
	getline(fin,data);
	insults[x].insult = data;
	x++;
*/

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
 *  @param buff     Array of strings that may contain insults
 *
 *  @param insults  Array of struct s_insults that contains the insults
 */
void replaceInsults(string buff[], int buffSize, s_insults insults[] ){
	//Variables
	string temp;	//Used to convert text in buff to lowercase
	locale loc;
	
	//for loop for the buffer list size
	for(int i=0; i<buffSize; i++){

		//Converts text in buff to lowercase
		temp = buff[i];
		for (int k=0; k<temp.length(); k++)
			temp[k] = tolower(temp[k],loc);

		//for loop for the insult list size
		for(int j=0; j<SIZE; j++){ 
			if( temp.compare(insults[j].insult) == 0){
				buff[i]="BLEEP";
				break; //break to move on to the next word in the buffer
			}
		}//end inner forloop
	}//end outer forloop
}
