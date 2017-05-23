#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "transfer.h"

void writefile(int sockfd, FILE *fp);

int main(int argc, char *argv[]) 
{
    //创建TCP套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) 
    {
        perror("Can't allocate sockfd");
        exit(1);
    }
    
    //配置服务器套接字地址
    struct sockaddr_in clientaddr, serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);

    //绑定套接字与地址
    if (bind(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) == -1) 
    {
        perror("Bind Error");
        exit(1);
    }

    //转换为监听套接字
    if (listen(sockfd, LINSTENPORT) == -1) 
    {
        perror("Listen Error");
        exit(1);
    }

    //等待连接完成
    socklen_t addrlen = sizeof(clientaddr);
    int connfd = accept(sockfd, (struct sockaddr *) &clientaddr, &addrlen); //已连接套接字
    if (connfd == -1) 
    {
        perror("Connect Error");
        exit(1);
    }
    close(sockfd); //关闭监听套接字

    //获取文件名
    char filename[BUFFSIZE] = {0}; //文件名
    if (recv(connfd, filename, BUFFSIZE, 0) == -1) 
    {
        perror("Can't receive filename");
        exit(1);
    }

    //创建文件
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) 
    {
        perror("Can't open file");
        exit(1);
    }
    
    //把数据写入文件
    char addr[INET_ADDRSTRLEN];
    printf("Start receive file: %s from %s\n", filename, inet_ntop(AF_INET, &clientaddr.sin_addr, addr, INET_ADDRSTRLEN));
    writefile(connfd, fp);
    puts("Receive Success");

    //关闭文件和已连接套接字
    fclose(fp);
    close(connfd);
    return 0;
}

void writefile(int sockfd, FILE *fp)
{
    ssize_t n; //每次接受数据数量
    char buff[MAX_LINE] = {0}; //数据缓存
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0) 
    {
        if (n == -1)
        {
            perror("Receive File Error");
            exit(1);
        }
        
        //将接受的数据写入文件
        if (fwrite(buff, sizeof(char), n, fp) != n)
        {
            perror("Write File Error");
            exit(1);
        }
        memset(buff, 0, MAX_LINE); //清空缓存
    }
}