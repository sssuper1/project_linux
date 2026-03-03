#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	system("/usr/sbin/flashcp -v /home/root/BOOT.BIN /dev/mtd0");
	system("/usr/sbin/flashcp -v /home/root/image.ub /dev/mtd2");
	system("echo 0 > /www/test");
 //http协议
    printf("Content-Type:text/html;charset=utf-8\r\n");
    printf("\r\n");

 	
   return 0;
}

