#pragma once
#include<cstdio>
#include<iostream>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<cstring>
#include<cstdlib>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/sendfile.h>
#include<sys/wait.h>
#include<istream>
#include"serversocket.hpp"
#include"my_err.hpp"
using namespace std;

const char *path = "/home/cwh/WebServer/src/html";
const int BUFFER_SIZE = 4000;


class task
{
private:
	int connfd;
 
public:
	task(){}
    task(ServSocket &so)
    {
        connfd = so.getconnfd();
    }
	task(ServSocket * socket_object)
    {
        connfd = socket_object->getconnfd();
    }
	~task(){}
 
	void response(char *message, int status)  //错误响应函数，status是响应状态码
	{
		char buf[512];
		sprintf(buf, "HTTP/1.1 %d OK\r\nConnection: Close\r\n"  //响应头
		"content-length:%d\r\n\r\n", status, strlen(message));
 
		sprintf(buf, "%s%s", buf, message);
		write(connfd, buf, strlen(buf));
 
	}
	void response_file(int size, int status)  //请求静态文件响应函数，size为文件大小
	{
		char buf[128];
		sprintf(buf, "HTTP/1.1 %d OK\r\nConnection: Close\r\n"
		"content-length:%d\r\n\r\n", status, size);
		write(connfd, buf, strlen(buf));
	}
 
	void response_get(char *filename);   //Get函数
 
	void response_post(char *filename, char *argv);  //POST函数
 
	void operator()();
};
 
void task::operator()()
{
	char buffer[BUFFER_SIZE];
	int size;
    fprintf(stderr, "hello, ");//$$$$$$$$$$$
    size = read(connfd, buffer, BUFFER_SIZE - 1);  //读取Http请求报文
	fprintf(stderr, "%s", buffer);//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$4
	if(size > 0)
	{
		char method[5];
		char filename[50];
		int i, j;
		i = j = 0;
		while(buffer[j] != ' ' && buffer[j] != '\0')//获取请求方法
		{
			method[i++] = buffer[j++];
		}
		++j;
		method[i] = '\0';
		i = 0;
		while(buffer[j] != ' ' && buffer[j] != '\0')//获取请求文件
		{
			filename[i++] = buffer[j++];
		}
		filename[i] = '\0';
 
		if(strcasecmp(method, "GET") == 0)  //get method
		{
			response_get(filename);
		}
		else if(strcasecmp(method, "POST") == 0)  //post method
		{
			printf("Post\n");
			char argvs[100];
			memset(argvs, 0, sizeof(argvs));
			int k = 0;
			char *ch = NULL;
			++j;
			while((ch = strstr(argvs, "Content-Length")) == NULL) //查找请求头部中的Content-Length行
			{
				k = 0;
				memset(argvs, 0, sizeof(argvs));
				while(buffer[j] != '\r' && buffer[j] != '\0')
				{
					argvs[k++] = buffer[j++];
				}
				++j;
				printf("%s\n", argvs);///////////
			}
			int length;
			char *str = strchr(argvs, ':');  //获取POST请求数据的长度
			printf("%s\n", str);///////////
			++str;
			sscanf(str, "%d", &length);
			printf("length:%d\n", length);///////////
			j = strlen(buffer) - length;    //从请求报文的尾部获取请求数据
			k = 0;
			memset(argvs, 0, sizeof(argvs));
			while(buffer[j] != '\r' && buffer[j] != '\0')
				argvs[k++] = buffer[j++];
 
			argvs[k] = '\0';

			printf("%s\n", argvs);//////////
			response_post(filename, argvs);  //POST方法
		}
		else  //未知的方法
		{
			char message[512];
			sprintf(message, "<html><title>Tinyhttpd Error</title>");
			sprintf(message, "%s<body>\r\n", message);
			sprintf(message, "%s 501\r\n", message);
			sprintf(message, "%s <p>%s: Httpd does not implement this method", 
				message, method);
			sprintf(message, "%s<hr><h3>The Tiny Web Server<h3></body>", message);
			response(message, 501);
		}
 
 
		//response_error("404");
	}
	else if(size < 0)//读取失败，重新读取
	  my_err("读取失败!",__LINE__);
 
	sleep(1);  //wait for client close, avoid TIME_WAIT
    if (connfd > 0) close(connfd);
	// ServSocket(socket_object).Closeclient();
}
 
void task::response_get(char *filename)
{
	char file[100];
	strcpy(file, path);
 
	int i = 0;
	bool is_dynamic = false;
	char argv[20];
	//查找是否有？号
	while(filename[i] != '?' && filename[i] != '\0')
		    ++i;
	if(filename[i] == '?')
	{	//有?号，则是动态请求
		int j = i;
		++i;
		int k = 0;
		while(filename[i] != '\0')  //分离参数和文件名
			argv[k++] = filename[i++];
		argv[k] = '\0';
		filename[j] = '\0';
		is_dynamic = true;
	}
 
	if(strcmp(filename, "/") == 0)
		strcat(file, "/index.html");
	else
		strcat(file, filename);
 
 
	printf("response_get:%s\n", file);//////////////////
	struct stat filestat;
	int ret = stat(file, &filestat);
 
	if(ret < 0 || S_ISDIR(filestat.st_mode)) //file doesn't exits
	{
        fprintf(stderr, "filename:%s\n", file);//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		char message[512];
		sprintf(message, "<html><title>Tinyhttpd Error</title>");
		sprintf(message, "%s<body>\r\n", message);
		sprintf(message, "%s 404\r\n", message);
		sprintf(message, "%s <p>GET: Can't find the file", message);
		sprintf(message, "%s<hr><h3>The Tiny Web Server<h3></body>", 
			message);
		response(message, 404);
		return;
	}
 
	if(is_dynamic)
	{
		if(fork() == 0) 
		/*创建子进程执行对应的子程序，多线程中，创建子进程，
		只有当前线程会被复制，其他线程就“不见了”，这正符合我们的要求，
		而且fork后执行execl，程序被进程被重新加载*/
		{
			dup2(connfd, STDOUT_FILENO);  
			//将标准输出重定向到sockfd,将子程序输出内容写到客户端去。
	    		execl(file, argv); //执行子程序
		}
		wait(NULL);
	}
	else
	{
		int filefd = open(file, O_RDONLY);
		response_file(filestat.st_size, 200);
		//使用“零拷贝”发送文件
		sendfile(connfd, filefd, 0, filestat.st_size);
	}
}
 
 
void task::response_post(char *filename, char *argvs)
{
	char file[100];
	strcpy(file, path);
 
	strcat(file, filename);
 
	struct stat filestat;
	int ret = stat(file, &filestat);
	printf("%s\n", file);
	if(ret < 0 || S_ISDIR(filestat.st_mode)) //file doesn't exits
	{
		char message[512];
		sprintf(message, "<html><title>Tinyhttpd Error</title>");
		sprintf(message, "%s<body>\r\n", message);
		sprintf(message, "%s 404\r\n", message);
		sprintf(message, "%s <p>GET: Can't find the file", message);
		sprintf(message, "%s<hr><h3>The Tiny Web Server<h3></body>", 
			message);
		response(message, 404);
		return;
	}
 
	char argv[20];
	int a, b;
	ret = sscanf(argvs, "a=%d&b=%d", &a, &b);//判断参数是否正确
	if(ret < 0 || ret != 2)
	{
		char message[512];
		sprintf(message, "<html><title>Tinyhttpd Error</title>");
		sprintf(message, "%s<body>\r\n", message);
		sprintf(message, "%s 404\r\n", message);
		sprintf(message, "%s <p>GET: Parameter error", message);
		sprintf(message, "%s<hr><h3>The Tiny Web Server<h3></body>", 
			message);
		response(message, 404);
		return;
	}
	sprintf(argv, "%d&%d", a, b);
	if(fork() == 0) 
	/*创建子进程执行对应的子程序，多线程中，创建子进程，
	只有当前线程会被复制，其他线程就“不见了”，这正符合我们的要求，
	而且fork后执行execl，程序被进程被重新加载*/
	{
		dup2(connfd, STDOUT_FILENO);  
		//将标准输出重定向到sockfd,将子程序输出内容写到客户端去。
		execl(file, argv); //执行子程序
	}
	wait(NULL);
}
 
