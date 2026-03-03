#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() 
{
   pid_t pid;
    int status;

    // 创建一个新的进程
    pid = fork();

    if (pid == -1) {
        // fork失败
        printf("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 子进程
        // 使用execl()函数执行/home/目录下的flash.cp脚本
        // 注意：execl()的第二个参数是脚本名称，之后的参数是传递给脚本的参数（如果有的话），最后以NULL结束
        execl("/www/cgi-bin/flashcp.sh", "flashcp.sh", (char *)NULL);
        // 如果execl()执行成功，则不会返回；如果返回，说明执行失败
        printf("execl failed");
        exit(EXIT_FAILURE);
    } else {
        // 父进程
        // 等待子进程结束
        waitpid(pid, &status, 0);

 //http协议
    printf("Content-Type:text/html;charset=utf-8\r\n");
    printf("\r\n");

 


 
        if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child killed by signal %d\n", WTERMSIG(status));
        }
    }

    return 0;
}


