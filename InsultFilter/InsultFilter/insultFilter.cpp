using namespace std;

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <assert.h>

//Size of insult list
#define SIZE 256

struct s_insults{
	string insult;
	int pos;
} insults[SIZE];

//Function declarations
void addInsults(s_insults insults[]);
void printInsults(s_insults insults[]);
void replaceInsults(string mystr[], s_insults insults[] );

int main(){
	string mystr;
	string testStr[5]={"Fuck","slap","bastard","Alcoholic","master"};

	//strSize=testStr.size();
	//Add the insults from the text file to 
	//the array of structs
	addInsults(insults);

	//Prints Insults out for verification
	//printInsults(insults);

	//Prints out words before replacing them
	cout<<"Words in string array before replacement"<<endl;
	for(int i=0;i<5;i++)
		cout<<testStr[i]<<" ";
	cout<<"\n"<<endl;

	//Replaces insults in test string
	replaceInsults(testStr, insults);

	//Verifies insults have been replaced
	cout<<"Words in string array after replacement"<<endl;
	for(int i=0;i<5;i++)
		cout<<testStr[i]<<" ";
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

/** @brief          Replaces the word in a string if it is an insult
 *    
 *  @param mystr    Array of strings that may contain insults
 *
 *  @param insults  Array of struct s_insults
 */
void replaceInsults(string mystr[], s_insults insults[] ){
	//cout<<"string size is "<<mystr[0].size()<<endl;
	//string test="Alcoholic";
	//int x=0;

	//for loop for the insult list size
	for(int i=0; i<SIZE; i++){
		//for loop for the string size
		for(int j=0; j<5; j++){
			if( mystr[j].compare(insults[i].insult) == 0 )
				mystr[j]="******";
		}//end inner forloop
	}//end outer forloop
}
