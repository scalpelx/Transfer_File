#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "transfer.h"

void sendfile(FILE *fp, int sockfd);

int main(int argc, char* argv[])
{
    //判断参数
    if (argc != 3) 
    {
        perror("usage:send_file filepath <IPaddress>");
        exit(1);
    }

    //创建TCP套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("Can't allocate sockfd");
        exit(1);
    }

    //设置传输对端套接字地址
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVERPORT);
    if (inet_pton(AF_INET, argv[2], &serveraddr.sin_addr) < 0) //将IP地址格式从字符串转换为二进制
    {
        perror("IPaddress Convert Error");
        exit(1);
    }

    //建立连接
    if (connect(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("Connect Error");
        exit(1);
    }
    
    //获取文件名
    char *filename = basename(argv[1]); //文件名
    if (filename == NULL)
    {
        perror("Can't get filename");
        exit(1);
    }
    
    /*发送文件名
      为了将文件名一次发送出去，而不是暂存到TCP发送缓冲区中，避免对方收到多余的数据，不好解析正确的文件名，
      需要将要发送的数据大小设置为缓冲区大小*/
    char buff[BUFFSIZE] = {0};
    strncpy(buff, filename, strlen(filename));
    if (send(sockfd, buff, BUFFSIZE, 0) == -1)
    {
        perror("Can't send filename");
        exit(1);
    }
    
    //打开要发送的文件
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) 
    {
        perror("Can't open file");
        exit(1);
    }

    //读取并发送文件
    sendfile(fp, sockfd);
    puts("Send Success");

    //关闭文件和套接字
    fclose(fp);
    close(sockfd);
    return 0;
}

void sendfile(FILE *fp, int sockfd) 
{
    int n; //每次读取数据数量
    char sendline[MAX_LINE] = {0}; //暂存每次读取的数据
    while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0) 
    {
        if (n != MAX_LINE && ferror(fp)) //读取出错并且没有到达文件结尾
        {
            perror("Read File Error");
            exit(1);
        }
        
        //将读取的数据发送到TCP发送缓冲区
        if (send(sockfd, sendline, n, 0) == -1)
        {
            perror("Can't send file");
            exit(1);
        }
        memset(sendline, 0, MAX_LINE); //清空暂存字符串
    }
}