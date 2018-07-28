#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

#define SER_PORT 80
#define LISTEN_MAX 5
#define METHOD_LEN 32
#define NAME_MAX_LEN 128
#define IP_ADDR "10.0.2.15"
#define SER_HOME "/home/Ricky/WorkSpace/web_service"

int create_sockfd(short port,char *ipstr);
void * client_thread(void * arg);
int get_req_str(char data[],int data_len,char buff[],int curr_index);
void http_res_succ(char http_head[],int filesize,int c);
void http_res_fail(char http_head[],int c);

int main()
{
	
	int sockfd = create_sockfd(SER_PORT,IP_ADDR);
	if(-1 == sockfd)
	{
		printf("create sockfd failed\n");
		return 1;
	}

	while(1)
	{
		struct sockaddr_in caddr;
		int len = sizeof(caddr);
		int c = accept(sockfd,(struct sockaddr*)&caddr,&len);
		if(c < 0)
		{
			perror("accept error");
			continue;
		}
		pthread_t id;
		pthread_create(&id,NULL,client_thread,(void *)c);
	}
	return 0;
}

int get_req_str(char data[],int data_len,char buff[],int curr_index)
{
	int i = 0;
	for(;i < data_len;++i)
	{
		if(buff[curr_index] == ' ')
		  break;
		data[i] = buff[curr_index++];
	}
	return curr_index + 1;
}

void http_res_succ(char http_head[],int filesize,int c)
{
	strcpy(http_head,"HTTP/1.1 200 ok\r\n");
	strcat(http_head,"Server: ser_http\r\n");
	sprintf(http_head + strlen(http_head),"Content-Length: %d\r\n",filesize);
	strcat(http_head,"\r\n");
	send(c,http_head,strlen(http_head),0);
}

void http_res_fail(char http_head[],int c)
{
	strcpy(http_head,"HTTP/1.1 404 faile\r\n");
	strcat(http_head,"Server: ser_http\r\n");
	strcat(http_head,"\r\n");
	send(c,http_head,strlen(http_head),0);
}

int create_sockfd(short port,char *ipstr)
{
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sockfd)
	  return -1;
	struct sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ipstr);

	int res = bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	if(-1 == res)
	  return -1;
	listen(sockfd,LISTEN_MAX);
	return sockfd;
}

void *client_thread(void *arg)
{
	int c = (int)arg;
	char request_buff[1024] = {0};

	while(1)
	{
		int n = recv(c,request_buff,1024,0);
		if(n <= 0)
		{
			printf("client close or err\n");
			break;
		}
		printf("read:\n%s\n",request_buff);
		char Method[METHOD_LEN] = {0};
		int buff_index = get_req_str(Method,METHOD_LEN - 1,request_buff,0);

		if(strcmp(Method,"GET") == 0)
		{
			char filename[NAME_MAX_LEN] = {0};
			buff_index = get_req_str(filename,NAME_MAX_LEN - 1,request_buff,buff_index);
			char path[256] = {SER_HOME};
			if(strcmp(filename,"/") == 0)
			  strcat(path,"/index.html");
			else
			  strcat(path,filename);

			int fd = open(path,O_RDONLY);
			char http_head[512] = {0};
			if(-1 == fd)
			{
				http_res_fail(http_head,c);
				break;
			}

			int filesize = lseek(fd,0,SEEK_END);
			lseek(fd,0,SEEK_SET);
			http_res_succ(http_head,filesize,c);
			while((sendfile(c,fd,0,512)) > 0);
			close(fd);
		}
		else if(strcmp(Method,"POST") == 0)
		{
			char func[NAME_MAX_LEN] = {0};
			buff_index = get_req_str(func,NAME_MAX_LEN - 1,request_buff,buff_index);
			char path[128] = {"."};
			strcat(path,func);

			char data[100]={0};
			strcpy(data,strstr(request_buff,"a1="));
			int fd[2];
			assert(pipe(fd) != -1);
			pid_t pid = fork();
			if(pid == 0)
			{
				close(fd[0]);
				assert(dup2(fd[1],1) != -1);
				execl(path,func,data,(char*)0);
				exit(0);
			}
			close(fd[1]);
			wait(NULL);
			char http_head[512] = {0};
			char buff[4096] = {0};
			while(read(fd[0],buff,1024) > 0);
			int size = strlen(buff);
			http_res_succ(http_head,size,c);
			send(c,buff,size,0);
			close(fd[0]);
		}
	}
	close(c);
}
