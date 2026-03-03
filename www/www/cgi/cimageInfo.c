


#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "cgic.h"

#define DEBUG_ON
#ifdef DEBUG_ON
void PrintMessage(const char *str)
{
    int fd;
    fd =  open("/home/vmuser/mycgi_log",O_WRONLY|O_CREAT|O_APPEND);
    if(fd < 0)
        return;

    time_t the_time;

    struct tm *info;
    time(&the_time);
    info = gmtime(&the_time );

    dprintf(fd,"[%2d:%02d]\n", (info->tm_hour)%24, info->tm_min);

    write(fd,str,strlen(str));

    close(fd);

}
#endif


enum ErrLog
{
    ErrSucceed = 0x00,
    ErrOpenFile,
    ErrNoFile,
    ErrNonePath
};

int cgiMain() {

        cgiFilePtr file;
        FILE *fd;
        char name[512];
        char path[128];
        char contentType[1024];
        int size = 0;
        int got = 0;
        int t = 0;
        char *tmp = NULL;

        //设置类型文件
        cgiHeaderContentType("text/html; charset=utf-8");
        if (cgiFormFileName("updatafile", name, sizeof(name)) != cgiFormSuccess) //获取客户端pathname
        {
           fprintf(cgiOut,"<p> 文件上传失败. </p>\n");
           return ErrNoFile;
        }

        //显示上传文件内容
        fprintf(cgiOut, "提交上传文件名称: ");
        cgiHtmlEscape(name);//虽然已经获取到名称，如果文件名中有特殊的名称，将会被转换，总结：从html获取的字符串需要显示到网页的用这个比较好，用fprintf也可以。
        fprintf(cgiOut, "<br>\n");

        //获取文件大小
        cgiFormFileSize("updatafile", &size);
        fprintf(cgiOut, "文件大小为: %d 字节<br>\n", size);

        //上传文件内容类型
        cgiFormFileContentType("updatafile", contentType, sizeof(contentType));
        fprintf(cgiOut, "文件的内容类型为: ");
        cgiHtmlEscape(contentType);
        fprintf(cgiOut, "<br>\n");

        if (cgiFormString("updatapath", path, sizeof (path)) != cgiFormSuccess)
        {
            fprintf(cgiOut, "<p> Could not open the file. </p>\n");
            return ErrNonePath;
        }

        //上传文件内容类型
        fprintf(cgiOut, "文件的路径: ");
        cgiHtmlEscape(path);
        fprintf(cgiOut, "<br>\n");

        //尝试打开上传的，并存放在系统中的临时文件
        if (cgiFormFileOpen("updatafile", &file) != cgiFormSuccess)
        {
           fprintf(cgiOut, "<p> Could not open the file. </p>\n");
           return ErrOpenFile;
        }

        t = -1;
        while (1)
        {
           tmp = strstr(name+t+1, "\\");  // 从pathname解析出filename
           if (NULL == tmp)
               tmp = strstr(name+t+1, "/");
           if (NULL != tmp)
               t = (int)(tmp-name);
           else
               break;
        }
		//动态内存分配
        tmp = (char *)malloc(size * sizeof(char)); 
        strcat(path, name+t+1);

        //上传文件内容类型
        fprintf(cgiOut, "最终生成文件: ");
        cgiHtmlEscape(path);
        fprintf(cgiOut, "<br>\n");


        //创建文件，以字节流的方式打开
        fd = fopen(path, "wb+");
        if (fd == NULL)
        {
           return ErrOpenFile;
        }

        // 从临时文件读出content
        while (cgiFormFileRead(file, tmp, size, &got) == cgiFormSuccess)
        {
           fwrite(tmp, size, sizeof(char), fd);  //把读出的content写入新文件
        }

        

        //关闭文件
        cgiFormFileClose(file);
        free(tmp);
        fclose(fd);
	//BOOT.BIN写入flash
	//system("flashcp -v /home/root/image.ub /dev/mtd2");
	//打印输出
        fprintf(cgiOut, "<p> 上传文件成功. </p>\n");
	

		//跳转回到主页面，这个需要浏览器html代码功能来实现
        fprintf(cgiOut,"<meta http-equiv=\"Refresh\" content=\"3;URL=/index.html\">");
        return ErrSucceed;


}

