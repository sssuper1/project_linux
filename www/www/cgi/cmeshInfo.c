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




typedef struct {
    char pwname[20];
    char pwvalue[10];
    char pwstate[5];
    char fqname[20];
    char fqvalue[10];
    char fqstate[5];
    char bwname[20];
    char bwvalue[10];
    char bwstate[5];
    char rtname[20];
    char rtvalue[10];
    char rtstate[5];
    char dtname[20];
    char dtvalue[10];
    char dtstate[5];
    char sdname[20];
    char sdvalue[10];
    char sdstate[5];
    char mdname[20];
    char mdvalue[10];
    char mdstate[5];
    char wmvalue[10];
    char wmstate[5];
    char routename[20];
    char routevalue[10];
    char routestate[5];
    char slotname[20];
    char slotvalue[10];
    char slotstate[5];
    //char filtername[20];
    // char filtervalue[10];
    // char filterstate[5];
    char selectfreq1name[20];
    char selectfreq1value[10];
    char selectfreq1state[5];
    char selectfreq2name[20];
    char selectfreq2value[10];
    char selectfreq2state[5];
    char selectfreq3name[20];
    char selectfreq3value[10];
    char selectfreq3state[5];
    char selectfreq4name[20];
    char selectfreq4value[10];
    char selectfreq4state[5];

    char transmodename[20];
    char transmodevalue[10];
    char transmodestate[5];

    char powerlevelname[20];
    char powerlevelvalue[10];
    char powerlevelstate[5];

    char powerattenuationname[20];
    char powerattenuationvalue[10];
    char powerattenuationstate[5];

    char rxchannelmodename[20];
    char rxchannelmodevalue[10];
    char rxchannelmodestate[5];

} User;

//判断新数据与旧数据是否一直，若一直，状态位0保持不变
static int callbacktest(void* data, int argc, char** argv, char** azColName) {
    User* user = (User*)data;
	if(strcmp(argv[0], "m_txpower") == 0){
        if (strcmp(argv[1], user->pwvalue) == 0) {
            strcpy(user->pwstate, "0");
	//printf("%s\n%s\n",user->pwvalue,user->pwstate);
        }
	}
	
	if(strcmp(argv[0], "rf_freq") == 0){
        if (strcmp(argv[1], user->fqvalue) == 0) {
            strcpy(user->fqstate, "0");
        }
	}

	if(strcmp(argv[0], "m_chanbw") == 0){
        if (strcmp(argv[1], user->bwvalue) == 0) {
            strcpy(user->bwstate, "0");
        }
	}

	if(strcmp(argv[0], "m_rate") == 0){
        if (strcmp(argv[1], user->rtvalue) == 0) {
            strcpy(user->rtstate, "0");
        }
	}
	
	if(strcmp(argv[0], "m_distance") == 0){
        if (strcmp(argv[1], user->dtvalue) == 0) {
            strcpy(user->dtstate, "0");
        }
	}

	if(strcmp(argv[0], "m_ssid") == 0){
        if (strcmp(argv[1], user->sdvalue) == 0) {
            strcpy(user->sdstate, "0");
        }
	}

	if(strcmp(argv[0], "m_bcastmode") == 0){
        if (strcmp(argv[1], user->mdvalue) == 0) {
            strcpy(user->mdstate, "0");
        }
	}

	if(strcmp(argv[0], "workmode") == 0){
        if (strcmp(argv[1], user->wmvalue) == 0) {
            strcpy(user->wmstate, "0");
        }
	}

 /* add by sdg    增加路由切换，空滤开关，时隙长度 自适应选频频率 */
	if(strcmp(argv[0], "m_route") == 0){
        if (strcmp(argv[1], user->routevalue) == 0) {
            strcpy(user->routestate, "0");
        }
	}  


	if(strcmp(argv[0], "m_slot_len") == 0){
        if (strcmp(argv[1], user->slotvalue) == 0) {
            strcpy(user->slotstate, "0");
        }
	}
 


	if(strcmp(argv[0], "m_trans_mode") == 0){
        if (strcmp(argv[1], user->transmodevalue) == 0) {
            strcpy(user->transmodestate, "0");
        }
	}  

 	if(strcmp(argv[0], "m_select_freq1") == 0){
        if (strcmp(argv[1], user->selectfreq1value) == 0) {
            strcpy(user->selectfreq1state, "0");
        }
	}  
 	if(strcmp(argv[0], "m_select_freq2") == 0){
        if (strcmp(argv[1], user->selectfreq2value) == 0) {
            strcpy(user->selectfreq2state, "0");
        }
	}  
 	if(strcmp(argv[0], "m_select_freq3") == 0){
        if (strcmp(argv[1], user->selectfreq3value) == 0) {
            strcpy(user->selectfreq3state, "0");
        }
	}  
 	if(strcmp(argv[0], "m_select_freq4") == 0){
        if (strcmp(argv[1], user->selectfreq4value) == 0) {
            strcpy(user->selectfreq4state, "0");
        }
	}

    if(strcmp(argv[0], "power_level") == 0){
        if (strcmp(argv[1], user->powerlevelvalue) == 0) {
            strcpy(user->powerlevelstate, "0");
        }
	}

    if(strcmp(argv[0], "power_attenuation") == 0){
        if (strcmp(argv[1], user->powerattenuationvalue) == 0) {
            strcpy(user->powerattenuationstate, "0");
        }
	}

    if(strcmp(argv[0], "rx_channel_mode") == 0){
        if (strcmp(argv[1], user->rxchannelmodevalue) == 0) {
            strcpy(user->rxchannelmodestate, "0");
        }
	}


   return 0;
}


static int callback(void *data, int argc, char **argv, char **azColName){

	printf("<data>%s</data>\n",argv[1]);//输出值

   return 0;
}

static int callbackstate(void *data, int argc, char **argv, char **azColName){

	printf("<data>%s</data>\n",argv[4]);//输出状态

   return 0;
}

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
    User user;
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
char value[20][30];
int j;
int l=0,m=0,n=1;

memset(value,0,sizeof(value));

for(j=0;j<len;j++){

	if(info[j]==38){
	n=1;
	l++;
	}

	if(n==0){
        if(m < 29){ // Prevent buffer overflow
            value[l][m]=info[j];
            m++;
        }
    }

	if(info[j]==61){   //  " 61 代表 = "
		if(info[j+1]==38||j==(len-1)){  // 如果没有值（=后面是&或者字符串结尾），手动设为 "NULL"
		value[l][0]=78;
		value[l][1]=85;
		value[l][2]=76;
		value[l][3]=76;
		}
	n=0;   // 开始收集 value 字符
	m=0;   // value 字符数组位置重置
	}

}
//初始化状态
strcpy(user.wmvalue, value[0]);     //workmode
strcpy(user.pwvalue, value[1]);      //m_txpower
strcpy(user.fqvalue, value[2]);      // rf_freq
strcpy(user.bwvalue, value[3]);      // m_chanbw
strcpy(user.rtvalue, value[4]);      //m_rate
strcpy(user.dtvalue, value[5]);     //m_distance
strcpy(user.sdvalue, value[6]);     // m_ssid
strcpy(user.mdvalue, value[7]);     //m_bcastmode
strcpy(user.routevalue, value[8]);     //m_route
strcpy(user.slotvalue, value[9]);      //m_slot_len
//strcpy(user.filtervalue, value[10]);   //m_filter
strcpy(user.transmodevalue, value[10]);   //m_trans_mode

strcpy(user.selectfreq1value, value[11]);   //sleect_freq_1
strcpy(user.selectfreq2value, value[12]);   //sleect_freq_2
strcpy(user.selectfreq3value, value[13]);   //sleect_freq_3
strcpy(user.selectfreq4value, value[14]);   //sleect_freq_4

strcpy(user.powerlevelvalue, value[15]);     //power_level
strcpy(user.powerattenuationvalue, value[16]); //power_attenuation
strcpy(user.rxchannelmodevalue, value[17]); //rx_channel_mode

strcpy(user.pwstate, "1");
strcpy(user.fqstate, "1");
strcpy(user.bwstate, "1");
strcpy(user.rtstate, "1");
strcpy(user.dtstate, "1");
strcpy(user.sdstate, "1");
strcpy(user.mdstate, "1");
strcpy(user.wmstate, "1");
strcpy(user.routestate, "1");
strcpy(user.slotstate, "1");
//strcpy(user.filterstate, "1");
strcpy(user.transmodestate, "1");
strcpy(user.selectfreq1state, "1");
strcpy(user.selectfreq2state, "1");
strcpy(user.selectfreq3state, "1");
strcpy(user.selectfreq4state, "1");
strcpy(user.powerlevelstate, "1");
strcpy(user.powerattenuationstate, "1");
strcpy(user.rxchannelmodestate, "1");


//回调判断状态函数
rc = sqlite3_exec(db, "Select * from meshInfo", callbacktest, &user, &zErrMsg);
//设置数据库命令
sql= (char*)malloc(sizeof(char)*2048);
  /* Create merged SQL statement */
   sprintf(sql,
            "UPDATE meshInfo set value = '%s' where name= 'm_txpower'; "
            "UPDATE meshInfo set value = '%s' where name= 'rf_freq'; "
            "UPDATE meshInfo set value = '%s' where name= 'm_chanbw'; "
            "UPDATE meshInfo set value = '%s' where name= 'm_rate'; "
            "UPDATE meshInfo set value = '%s' where name= 'm_distance'; "
            "UPDATE meshInfo set value = '%s' where name= 'm_ssid'; "
            "UPDATE meshInfo set value = '%s' where name= 'm_bcastmode'; "
            "UPDATE meshInfo set value = '%s' where name= 'workmode'; "
            "UPDATE meshInfo set value = '%s' where name= 'm_route'; "
            "UPDATE meshInfo set value = '%s' where name= 'm_slot_len'; "
            
            "UPDATE meshInfo set value = '%s' where name= 'm_trans_mode';"
            "UPDATE meshInfo set value = '%s' where name= 'm_select_freq1';"
            "UPDATE meshInfo set value = '%s' where name= 'm_select_freq2';"
            "UPDATE meshInfo set value = '%s' where name= 'm_select_freq3';"  
            "UPDATE meshInfo set value = '%s' where name= 'm_select_freq4';"
            "UPDATE meshInfo set value = '%s' where name= 'power_level';"
            "UPDATE meshInfo set value = '%s' where name= 'power_attenuation';"
            "UPDATE meshInfo set value = '%s' where name= 'rx_channel_mode';",
               user.pwvalue,user.fqvalue,user.bwvalue,user.rtvalue,user.dtvalue,
               user.sdvalue,user.mdvalue,user.wmvalue,user.routevalue,user.slotvalue,
               user.transmodevalue,user.selectfreq1value,user.selectfreq2value,
               user.selectfreq3value,user.selectfreq4value,
               user.powerlevelvalue,user.powerattenuationvalue,user.rxchannelmodevalue);
	sqlite3_busy_handler(db,busyHandle,NULL);
   
//"UPDATE meshInfo set value = '%s' where name= 'm_filter';"   user.filtervalue,

   /* Execute SQL statement */
//回调输出函数
   rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if( rc != SQLITE_OK ){
      //printf("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      //printf("Operation done successfully\n");
   }

sprintf(sql,
   "UPDATE meshInfo set state = '%s' where name= 'm_txpower'; "
    "UPDATE meshInfo set state = '%s' where name= 'rf_freq'; "
    "UPDATE meshInfo set state = '%s' where name= 'm_chanbw'; "
    "UPDATE meshInfo set state = '%s' where name= 'm_rate'; "
    "UPDATE meshInfo set state = '%s' where name= 'm_distance'; "
    "UPDATE meshInfo set state = '%s' where name= 'm_ssid'; "
    "UPDATE meshInfo set state = '%s' where name= 'm_bcastmode'; "
    "UPDATE meshInfo set state = '%s' where name= 'workmode'; "
    "UPDATE meshInfo set state = '%s' where name= 'm_route'; "
    "UPDATE meshInfo set state = '%s' where name= 'm_slot_len'; "
    "UPDATE meshInfo set state = '%s' where name= 'm_trans_mode';"
    "UPDATE meshInfo set state = '%s' where name= 'm_select_freq1';"
    "UPDATE meshInfo set state = '%s' where name= 'm_select_freq2';"
    "UPDATE meshInfo set state = '%s' where name= 'm_select_freq3';"
    "UPDATE meshInfo set state = '%s' where name= 'm_select_freq4';"
    "UPDATE meshInfo set state = '%s' where name= 'power_level';"
    "UPDATE meshInfo set state = '%s' where name= 'power_attenuation';"
    "UPDATE meshInfo set state = '%s' where name= 'rx_channel_mode';"
    ,
      user.pwstate,user.fqstate,user.bwstate,user.rtstate,user.dtstate,user.sdstate,
      user.mdstate,user.wmstate,user.routestate,user.slotstate,user.transmodestate,
      user.selectfreq1state,user.selectfreq2state,user.selectfreq3state,user.selectfreq4state,
      user.powerlevelstate,user.powerattenuationstate,user.rxchannelmodestate);
	sqlite3_busy_handler(db,busyHandle,NULL);

    // "UPDATE meshInfo set state = '%s' where name= 'm_filter';"  user.filterstate,


   /* Execute SQL statement */
//回调状态输出函数
   rc = sqlite3_exec(db, sql, callbackstate, (void*)data, &zErrMsg);
   if( rc != SQLITE_OK ){
      //printf("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      //printf("Operation done successfully\n");
   }

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
printf("<script>location.assign('http://%s:100/meshInfo.html')</script>",inet_ntoa(addr->sin_addr));
//printf("<script>location.assign('http://192.168.1.10/meshInfo1.html')</script>");

	}
	if (strncmp(ret,"GET",3) == 0){
    //http协议
    printf("Content-Type:text/xml;charset=utf-8\r\n");
    printf("\r\n");
	printf("\n");
	printf("<root>\n");//开始XML文档 建立一个根标志root


   /* Create SQL statement */
   sql = "SELECT * from meshInfo";

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
