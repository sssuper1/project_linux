#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>





static int busyHandle(void* ptr,int retry_times)
{
	sqlite3_sleep(10);
	return 1;
}


int main(int argc, char* argv[])
{

    char *ret = NULL;
    char *info = NULL;
    int len,i;
    //获取请求类型 get或者post
    ret = getenv("REQUEST_METHOD");
 


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
//	printf("获取%s",argv[1]);
//html通过post请求
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

//切割成不同的value分别存储到相应的变量中
//printf("%s %d\n",info,len);
        char delfreq[10];
        char addfreq[10];
        int j;
        int m=0,n=0;

        memset(delfreq,0,sizeof(delfreq));
        memset(addfreq,0,sizeof(addfreq));

        for(j=1;j<len;j++){

            if(n==1&&info[j]!=0){
	            delfreq[m]=info[j];
	            m++;
            }

            if(n==2&&info[j]!=0){
	            addfreq[m]=info[j];
	            m++;
            }


	        if(info[j]==61&&info[j-1]==100){
	            n=1;
	        }

            if(info[j]==61&&info[j-1]==97){
	            n=2;
	        }

	    }

    


        //回调判断状态函数
        rc = sqlite3_exec(db, "Select * from freqInfo", 0,0, &zErrMsg);
        //设置数据库命令
        sql= (char*)malloc(sizeof(char)*1024);
        /* Create merged SQL statement */
        if(n==1){
            sprintf(sql,"delete from freqInfo where freq=%s;",delfreq);
        }
        if(n==2){
            sprintf(sql,"insert into freqInfo values(%s);",addfreq);
        }
   
	    sqlite3_busy_handler(db,busyHandle,NULL);
        /* Execute SQL statement */
        //回调输出函数
        rc = sqlite3_exec(db, sql, 0, (void*)data, &zErrMsg);

	if(n==1){
		sql = "update systemInfo set value = '0' where name LIKE 'freq%';";
   		sqlite3_busy_handler(db,busyHandle,NULL);
   		rc = sqlite3_exec(db, sql, 0, (void*)data, &zErrMsg);
	}

	}
	if (strncmp(ret,"GET",3) == 0){
   /* Create SQL statement */
   sql = "DELETE FROM freqInfo;";

   /* Execute SQL statement */
   sqlite3_busy_handler(db,busyHandle,NULL);
   rc = sqlite3_exec(db, sql, 0, (void*)data, &zErrMsg);

   sql = "update systemInfo set value = '0' where name LIKE 'freq%';";
   sqlite3_busy_handler(db,busyHandle,NULL);
   rc = sqlite3_exec(db, sql, 0, (void*)data, &zErrMsg);

   sqlite3_close(db);
    }


   return 0;
}

