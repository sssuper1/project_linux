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

static int callback(void *data, int argc, char **argv, char **azColName){

	printf("<data>%s</data>\n",argv[1]);

   return 0;
}

static int callbackstate(void *data, int argc, char **argv, char **azColName){

	printf("<data>%s</data>\n",argv[2]);

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
char value[4][30];
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


printf("%s",value[0]);
printf("%s",value[1]);
printf("%s",value[2]);
printf("%s",value[3]);

sql= (char*)malloc(sizeof(char)*1024);
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

	sprintf(sql,"UPDATE userInfo set state = '1';");
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callbackstate, (void*)data, &zErrMsg);

free(sql);
   sqlite3_close(db);

//取IP地址
    struct ifreq ifr;
    struct sockaddr_in *addr;
    int fd;
 
    char *iface = "br0"; // 替换为你的网络接口名
 
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }
 
    strncpy(ifr.ifr_name, iface, IFNAMSIZ);
 
    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        perror("ioctl");
        close(fd);
        exit(1);
    }
 
    addr = (struct sockaddr_in *)&ifr.ifr_addr;
 //   printf("IP Address: %s\n", inet_ntoa(addr->sin_addr));
 
    close(fd);
printf("<script>location.assign('http://%s:100/userInfo.html')</script>",inet_ntoa(addr->sin_addr));
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
	rc = sqlite3_exec(db, sql, callbackstate, (void*)data, &zErrMsg);
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

