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
    printf("Content-TypeS:text/html;charset=utf-8\r\n");
    printf("\r\n");
        //查看post的请求
        //CONTENT_LENGTH 客户端向标准输入设备发送的数据长度，单位为字节
 len = atoi(getenv("CONTENT_LENGTH"));//获取字符串长度
        info = (char *) malloc(len+1);//设置info为比字符串长度加1大小的地址指针
//依次将字符串每个字符写入info数组内
        for(i=0;i<len;i++){
            info[i] = (char)fgetc(stdin);
        }
        info[i] = 0;//post传递的表单数据，即在字符串后加上空作为结束符

//切割成不同的value分别存储
printf("%s %d",info,len);
char value[4][30];
int j;
int l=0,m=0,n=1;

memset(value,0,sizeof(value));//初始化数组value为空

/*
字符串格式为id1=value1&id2=value2
从第一位字符开始辨识，初始n=1不进行录入
当识别到“=”（ascii值为61）时，判断下一位字符
如果下一位是“&”（ascii值为38）即录入的内容为空，将数据内容写入NULL，避免输入数据库中相应数据时为空，再次调用该数据时导致报错
如果不是，将n置于0（即下一位起开始录入value内容）,二维数组第二维脚标m置于初始位0,进入下一轮循环识别下一位
当识别到“&”（ascii值为38）时，将n置于1,即停止录入id内容，当前组数据录入结束即将进入下组数据，二维数组第一维脚标l加1

*/
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


printf("%s",value[0]);
printf("%s",value[1]);
printf("%s",value[2]);
printf("%s",value[3]);

sql= (char*)malloc(sizeof(char)*1024);//为sql设置地址内存空间，这里设置1024个字符
  /* Create merged SQL statement */
   sprintf(sql,"UPDATE userInfo set value = '%s' where name= 'm_ip';UPDATE userInfo set value = '%s' where name= 'm_dhcpStart';UPDATE userInfo set value = '%s' where name= 'm_dhcpGateway';UPDATE userInfo set value = '%s' where name= 'm_dhcpDns';",value[0],value[1],value[2],value[3]);
printf("%s",sql);


   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if( rc != SQLITE_OK ){
      //printf("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      //printf("Operation done successfully\n");
   }
free(sql);//释放sql地址内存，防止内存溢出
   sqlite3_close(db);
printf("<script>location.assign('http://192.168.56.147/userInfo.html')</script>");//跳转回原网址
	}
	if (strncmp(ret,"GET",3) == 0){
    //http协议
    printf("Content-Type:text/xml;charset=utf-8\r\n");
    printf("\r\n");
	printf("\n");
	printf("<root>\n");//开始XML文档 建立一个根标志root


   /* Create SQL statement */
   sql = "SELECT * from userInfo";

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

