#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

static int callback(void *data, int argc, char **argv, char **azColName){

int i;
for(i=0;i<302;i++){
	printf("<data>%s</data>\n",argv[i]);//输出值
};

   return 0;
}

int main(int argc, char* argv[])
{
    //http协议
    printf("Content-Type:text/xml;charset=utf-8\r\n");
    printf("\r\n");
	printf("\n");
	printf("<root>\n");//开始XML文档 建立一个根标志root
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   const char* data = "Callback function called";

   /* Open database */
   rc = sqlite3_open("test.db", &db);
   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
     // printf("Opened database successfully");
   }

   /* Create SQL statement */
   sql = "SELECT * from surveyInfo";

   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if( rc != SQLITE_OK ){
      //printf("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      //printf("Operation done successfully\n");
   }
	printf("</root>\n");//结束XML文档
   sqlite3_close(db);
   return 0;
}

