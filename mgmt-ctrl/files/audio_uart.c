
#include "audio_uart.h"
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include "socketUDP.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include "ui_get.h"
/* global */
int serial_port;
int audio_read;
struct sockaddr_in S_GROUND_AUDIO;
int SOCKET_AUDIO;

int audio_cnt_send;    //话音发送计数
int audio_cnt_recv;	   //话音接收计数

extern uint8_t SELFID;

extern Info_0x06_Statistics stat_info;

//#define TEST_AUDIO

int audio_uart_init(void)
{
	int i;
	uint8_t cnt=0;

	while(cnt<MAX_RETRY_COUNT)
	{
		serial_port = open("dev/ttyS0", O_RDWR);  //打开串口屏串口   |O_NOCTTY
		if(serial_port!=-1)
			break;
		cnt++;
		if(cnt<MAX_RETRY_COUNT)
			sleep(1);		
	}
	if(serial_port==-1)
	{
		printf("ERROR:open ui uart fail\r\n");
		return -1;
	}
    printf("[AUDIO DEBUG] audio fd :%d \r\n",serial_port);

	struct termios tty;
	if(tcgetattr(serial_port,&tty) !=0)
	{
		perror("获取串口配置失败");
		close(serial_port);
		return 1;
	}


	cfsetispeed(&tty,B460800);
	cfsetospeed(&tty,B460800);
	// cfsetispeed(&tty,B115200);
	// cfsetospeed(&tty,B115200);


	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;
//	tty.c_cflag |= CREAD | CLOCAL;

	tty.c_lflag &= ~(ICANON | ECHO | ECHOE |ISIG);
	tty.c_oflag &= ~OPOST;

	tty.c_cflag &= ~CRTSCTS;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);

	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);



//音频数据
	tty.c_cc[VMIN] = 15;
	tty.c_cc[VTIME] = 0;

	printf("tty.c_cc[VMIN]=%d\n",tty.c_cc[VMIN]);
	printf("tty.c_cc[VTIME]=%d\n",tty.c_cc[VTIME]);

	if(tcsetattr(serial_port,TCSANOW,&tty)!=0)
	{
		perror("设置串口属性失败");
		close(serial_port);
		return NULL;
	}

	printf("[audio uart thread]:audio uart init complete, ready to send\n");



    

/* test audio    */   

}




void send_audio_broadcaset_packet(int socket_fd,void *buf,int size)
{

    if(buf==NULL||size<=0)
    {
        printf("[AUDIO DEBUG] audio buf is null\r\n");
        return;
    }

	if(SendUDPClient(socket_fd,buf,size,&S_GROUND_AUDIO)<0)
	{
		printf("[AUDIO DEBUG] send audio  packet fail\r\n");
		return ;
	}


} 

void write_audio_info(char* buf,int len)
{
	int write_len;
	int i;
	uint8_t play_cnt=0;
	if(buf==NULL||len<=0)
	{
		printf("[AUDIO DEBUG] write buf is null \r\n");
		return;
	}
	Audio_Aggregated_Packet *packet=(Audio_Aggregated_Packet*)buf;

	//printf("[AUDIO DEBUG] packet seq %d \r\n",packet->seq);
	while(play_cnt<PACKET_AGGREGATED_NUM)
	{
		write_len=write(serial_port,packet->packet[play_cnt].data,AUDIO_PACKET_SIZE);
		if(packet->packet[play_cnt].data[0]!=0x61)
			continue;
		// for(i=0;i<15;i++)
		// {
		// 	printf(" %#x",packet->packet[play_cnt].data[i]);
		// }
		// printf("\r\n");
		play_cnt++;
		usleep(20000);
	}

	//write_len=write(serial_port,buf,len);
	//printf("[AUDIO DEBUG] write len %d \r\n",write_len);
}



// 添加话音包到发送缓冲区
// int voice_sender_add_packet(voice_sender_t* sender, const uint8_t* voice_data) {
//     if (sender->current_packet_count >= PACKET_AGGREGATED_NUM) {
//         return -1; // 缓冲区已满
//     }
    
//     // 复制话音数据
//     memcpy(sender->aggregated_packet.packet[sender->current_packet_count].data, 
//            voice_data, VOICE_PACKET_SIZE);
    
//     sender->current_packet_count++;
    
//     // 如果是第一个包，设置序列号
//     if (sender->current_packet_count == 1) {
//         sender->aggregated_packet.seq = sender->next_sequence;
//     }
    
//     return sender->current_packet_count;
// }

void process_audio_info(int fd)
{
		FILE* file = NULL;

    char audio_buf[MAX_AUDIO_SIZE];
    // int len=0;
    if(fd<=0)
    {
		printf("[AUDIO DEBUG] fd is nulls\r\n");
        return;
    }


	Audio_Aggregated_Packet send_packet;
	memset(&send_packet,0,sizeof(Audio_Aggregated_Packet));

    SOCKET_AUDIO=createUdpClient((void*)&S_GROUND_AUDIO,"192.168.2.255",AUDIO_BROADCAST_PORT);

   	int opt=1;

    setsockopt(SOCKET_AUDIO,SOL_SOCKET,SO_BROADCAST,&opt,sizeof(opt));//


#ifdef TEST_AUDIO    
/* test audio    */   
    char line[64]={0};
	file = fopen("/home/root/AMBE.txt","r");
	if(file == NULL)
	{
		printf("error opening file\n");
	}
	printf("[AUDIO DEBUG]test AMBE \r\n");

	
	while(1)
	{
	//需要把文件重定位到开头
    rewind(file);
	while(fgets(line,sizeof(line)-1,file) != NULL){
		uint8_t WT3081B_message[15]={0};

		WT3081B_message[0] = 0x61;
		WT3081B_message[1] = 0x00;
		WT3081B_message[2] = 0x09;
		WT3081B_message[3] = 0x01;
		WT3081B_message[4] = 0x01;
		WT3081B_message[5] = 0x48;
		uint8_t WT3081B_num = 6;

		char* split	= strtok(line,",");
		while(split != NULL && *split != '\r'){
			char* endptr = NULL;
			long hex_num = strtol(split,&endptr,0);
			WT3081B_message[WT3081B_num++] = hex_num;
			split = strtok(NULL,",");
		}

		send_audio_broadcaset_packet(SOCKET_AUDIO,WT3081B_message,sizeof(WT3081B_message));
		//ssize_t bytes_written = write(serial_port,WT3081B_message,sizeof(WT3081B_message));
		// for(i=0;i<bytes_written;i++)
		// {
		// 	printf(" %#x",WT3081B_message[i]);
		// }
		// printf("\r\n");
		//printf("[AUDIO DEBUG] write len %d \r\n",bytes_written);
		usleep(20000);
	}

}

#endif


    while(1)
    {
        memset(audio_buf,0,MAX_AUDIO_SIZE);
        audio_read=read(fd,audio_buf,MAX_AUDIO_SIZE-1);
        if(audio_read>0)
        {
			if(send_packet.current_cnt<PACKET_AGGREGATED_NUM)
			{
				memcpy(send_packet.packet[send_packet.current_cnt].data,audio_buf,audio_read);
				send_packet.current_cnt++;
			}
			else
			{
				//聚包完成
				send_packet.current_cnt=0;
				send_packet.seq++;
				send_audio_broadcaset_packet(SOCKET_AUDIO,(void*)&send_packet,sizeof(Audio_Aggregated_Packet));
				stat_info.audio_tx_packets++;
			}

			//audio_buf[2]=0x09; 
			//usleep(20000);
            //send_audio_broadcaset_packet(SOCKET_AUDIO,audio_buf,audio_read);
			
			//write_audio_info(audio_buf,audio_read);
			
        }
    }
}


void play_audio(void)
{
	int buflen=0;
	char buffer[1024];
	char ifname[] = "br0";
	struct sockaddr_in from;
	uint32_t selfip;
	char selfAddr[4] = {0xc0,0xa8,0x02,0x01};

	selfAddr[3]=SELFID;
	// printf("seifid:%d\r\n",selfAddr[3]);
	memcpy(&selfip,selfAddr,sizeof(uint32_t));
	// struct in_addr selfIP;
	// selfIP.s_addr = selfip;

	int audio_s = CreateUDPServer(AUDIO_BROADCAST_PORT);
	if (audio_s <= 0)
	{
		printf("ERROR: create socket_audio \n");
		exit(1);
	}
	if (setsockopt(audio_s, SOL_SOCKET, SO_BROADCAST, ifname, 4) < 0) {
		printf("socket_audio opt error\n");
		exit(1);
	}
		int socklen=0;
	while(1)
	{
		buflen=recvfrom(audio_s, buffer, 1000, 0, &from, &socklen);
		if(selfip!=inet_addr(inet_ntoa(from.sin_addr))&&(buflen>0))
		{
			//printf("[AUDIO DEBUG] recv audio socket from %s  \r\n",inet_ntoa(from.sin_addr));
				
			//
			write_audio_info((char*)buffer,buflen);
			stat_info.audio_rx_packets++;
		}


	}
}

void audio_thread(void)
{
    printf("create thread : get audio \r\n");
    audio_uart_init();
    process_audio_info(serial_port);
}

void play_audio_thread(void)
{
	printf("create thread : send audio \r\n");
	play_audio();
}
