#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

static int callback(void *data, int argc, char **argv, char **azColName){

	printf("<data>%s</data>\n",argv[1]);//输出值

   return 0;
}




int main(int argc, char* argv[])
{

    char *ret = NULL;
    char *info = NULL;
    int len,i;
    //获取请求类型 get或者post
    ret = getenv("REQUEST_METHOD");
    if(ret == NULL){
        printf("获取method失败！");
        return -1;
    }

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

     if(strncmp(ret,"POST",4) == 0){
    //http协议
    printf("Content-Type:text/html;charset=utf-8\r\n");
    printf("\r\n");
        //查看post的请求
        //CONTENT_LENGTH 客户端向标准输入设备发送的数据长度，单位为字节
 len = atoi(getenv("CONTENT_LENGTH"));
        info = (char *) malloc(len+1);

        for(i=0;i<len;i++){
            info[i] = (char)fgetc(stdin);
        }
        info[i] = 0;//post传递的表单数据

//切割成不同的value分别存储
printf("%s %d",info,len);
char value[8][30];
int j;
int l=0,m=0,n=1;

memset(value,0,sizeof(value));

for(j=0;j<len;j++){

	if(info[j]==38){
	n=1;
	l++;
	}

	if(n==0){
	value[l][m]=info[j];
	m++;}

	if(info[j]==61){
		if(info[j+1]==38||j==(len-1)){
		value[l][0]=78;
		value[l][1]=85;
		value[l][2]=76;
		value[l][3]=76;
		}
	n=0;
	m=0;
	}

}




sql= (char*)malloc(sizeof(char)*1024);
  /* Create merged SQL statement */
   sprintf(sql,"UPDATE serialInfo set value = '%s' where name= 'r_baudrate0';UPDATE serialInfo set value = '%s' where name= 'r_dID0';UPDATE serialInfo set value = '%s' where name= 'r_dport0';UPDATE serialInfo set value = '%s' where name= 'r_lport0';UPDATE serialInfo set value = '%s' where name= 'r_baudrate1';UPDATE serialInfo set value = '%s' where name= 'r_dID1';UPDATE serialInfo set value = '%s' where name= 'r_dport1';UPDATE serialInfo set value = '%s' where name= 'r_lport1';",value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7]);



   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if( rc != SQLITE_OK ){
      //printf("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      //printf("Operation done successfully\n");
   }
free(sql);
   sqlite3_close(db);
printf("<script>location.assign('http://192.168.2.2/serialInfo.html')</script>");
	}
	if (strncmp(ret,"GET",3) == 0){
    //http协议
    printf("Content-Type:text/xml;charset=utf-8\r\n");
    printf("\r\n");
	printf("\n");
	printf("<root>\n");//开始XML文档 建立一个根标志root


   /* Create SQL statement */
   sql = "SELECT * from serialInfo";

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
}


   return 0;
}

