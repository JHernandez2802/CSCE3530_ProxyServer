// Compile with: gcc createProxyDB.c -l sqlite3 -o db

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main(int argc, char *argv[])
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql;
	const char* data = "Callback function called";
	
	// Opens DB "group3ProxyDB"
	rc = sqlite3_open("group3ProxyDB.db", &db);

	if( rc )
	{	// Checks for error in sql code
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}
	
	// Create customer table, first deletes if already exists
	sql = "DROP TABLE IF EXISTS `cache`;" \
		  "CREATE TABLE IF NOT EXISTS `cache` (" \
		  "`IP` TEXT PRIMARY KEY NOT NULL," \
		  "data TEXT NOT NULL);";
		  
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg); // Executes SQL statement
	if( rc != SQLITE_OK )
	{	// Checks for error in sql code
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		exit(0);
	}
	else
	{
		fprintf(stdout, "Table created successfully\n");
	}
	
	// Inserts values into both tables
	sql = "INSERT INTO cache (IP,data) " \
		  "VALUES ('1.1.1.1','hfkdfjhsdkjfhsdkfhskjdh');";
		
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg); // Executes SQL statement
	if( rc != SQLITE_OK )
	{	// Checks for error in sql code
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Records created successfully\n");
	}
	
	// Checks cache table by printing records
	sql = "SELECT * from cache";

	printf("\n");
	
	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg); // Executes SQL statement
		
	if( rc != SQLITE_OK )
	{	// Checks for error in sql code
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Operation done successfully\n");
	}
	
	// closes database
	sqlite3_close(db);
	return 0;
}
