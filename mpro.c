#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

struct node 
{
char name[100];
int  socket; 
struct node *next;  
struct node *prev;

};

int max=0;

void  inser_node(struct node  *head,int socket,char name[100])
{
	//1.新建结点  
	struct node  *new =  malloc(sizeof(struct node));
				  new->socket = socket;  
				  
	//2.插入  
	strcpy(new->name,name);
	new->prev = head->prev; 
	new->next = head;  
	head->prev->next = new;  
	head->prev = new; 
	
}

//显示好友列表
void show(struct node  *head)
{
	//指向第一个结点  
	struct node  *pos = head->next; 
	
	while(pos != head)
	{
		//通过 socket 获取网络描述符的信息  
		struct sockaddr_in  clien_addr;
		int addr_len = sizeof(clien_addr); 
		

		getpeername(pos->socket,(struct sockaddr *)&clien_addr,&addr_len);
		char  *ip  = inet_ntoa(clien_addr.sin_addr);
		int port = ntohs(clien_addr.sin_port);
		
		
		printf("name: %s\tsocket=%d  ip:%s port:%d\n",pos->name,pos->socket,ip,port);
		
		pos = pos->next;
		
	}
	
}

//发布公告
void anoucmt(struct node  *head,char buf[1024])
{
	//指向第一个结点  
	struct node  *pos = head->next; 
 
	while(pos != head)
	{
	 
		write(pos->socket,buf,strlen(buf));
		pos = pos->next; 	
	} 	 
}
void send_list(struct node  *head,int send_socket )
{
	//指向第一个结点  
	struct node  *pos = head->next; 
 
 
	while(pos != head)
	{
		char buf[100] = {0};
		sprintf(buf,"allusrname:%s",pos->name);
		
		printf("%s\n",buf);
		
		write(send_socket,buf,strlen(buf));
		sleep(1);
		pos = pos->next;
	}
 
 
}


//查找用户 发送信息
struct node  * findusr(struct node  *head,char name[100],int send_socket )
{
	//指向第一个结点  
	struct node  *pos = head->next; 
	//char buf[1024] = {"已建立与该用户的聊天窗口"};
 
	
	while(pos != head)
	{
		if(strcmp(pos->name,name) == 0)
		{
			
			//write(send_socket,buf,strlen(buf));
			return pos;
		}
		
		pos = pos->next;
	}
	
	char buf[1024] = {"该用户不存在或未上线"};
	 
	write(send_socket,buf,strlen(buf));
	return NULL;
}

//返回指针
struct node  *usr_p = NULL;
struct node  *usr_p1 = NULL;
//http请求天气
void http_api(int send_socket, char function[100])
{
	//1.新建TCP 通信对象 
	int http_socket = socket(AF_INET, SOCK_STREAM, 0);

	//2.链接服务器  
	//设置服务器的IP地址信息  
	struct sockaddr_in  http_addr;  
	http_addr.sin_family   = AF_INET; //IPV4 协议  
	http_addr.sin_port     = htons(80); //端口 80  ,所有的HTTP 服务器端口都是  80  
	
	if(strstr(function,"weather"))
	{
		http_addr.sin_addr.s_addr = inet_addr("47.107.155.132"); //服务器的IP 地址信息
		int ret=connect(http_socket,(struct sockaddr *)&http_addr,sizeof(http_addr));
			if(ret < 0)
			{
				perror("");
				return ; 
			}
		//天狗日志https://cloud.qqshabi.cn/api/tiangou/api.php
		char *http = "GET /api.php?key=free&appid=0&msg=广州天气 HTTP/1.1\r\nHost:api.qingyunke.com\r\n\r\n";

		write(http_socket,http,strlen(http));
		
		char buf[100] = "为您播报未来四天的天气";
		//头数据切割
		char head[1024] = {0};
		int size = read(http_socket,head,1024);
		char *end =  strstr(head,"\r\n\r\n") + 32; 
		//char *end_e = strstr(head,"\"}")+1;
		
		char *tmp =  strstr(end,"广州天气 ：");
		char end_buf[1024]= {0};
		sscanf(tmp,"%[^{]",end_buf);			
		
		//分行 切割不需要的数据
		char buf2[1024]= {0};
		char *sp =  strstr(end,"{br}");
		while(sp!= NULL)
		{
			char buf1[1024]= {0};
			sscanf(sp,"%*[^}]}%[^{]",buf1);	
			sprintf(buf2,"%s\n%s",buf2,buf1);
			sp++;
			sp = strstr(sp,"{br}");
		}
		sprintf(end_buf,"%s%s",end_buf,buf2);
		
		
		//查找末尾不需要的字符地址
		char *end_e = strstr(end_buf,"\"}")+1;
		char new_head[1024] = {0};
		sprintf(new_head,"%s\n%s",buf,end_buf);
		//printf("%s\n",new_head);
		
		//write(send_socket,new_head,size-(int)(end - head)+strlen(buf));
		write(send_socket,new_head,(int)(end_e - end_buf)+strlen(buf));
	}
	else if(strstr(function,"licked beast:"))
	{
		http_addr.sin_addr.s_addr = inet_addr("113.96.178.42"); //服务器的IP 地址信息
		int ret=connect(http_socket,(struct sockaddr *)&http_addr,sizeof(http_addr));
			if(ret < 0)
			{
				perror("");
				return ; 
			}
		char *http = "GET /api/dog?format=text HTTP/1.1\r\nHost:v1.alapi.cn\r\n\r\n";
		write(http_socket,http,strlen(http));
		
		char buf[100] = "为您播报天狗兽@手记";
		
		char head[1024] = {0};
		int size = read(http_socket,head,1024);
		char *end =  strstr(head,"\r\n\r\n") + 8;
		 
		char *end_e = strstr(head,"\n0")-1;
		if( end_e == NULL)
		{
			printf("NULL\n" );
		}
		
		printf("end_e: %s\n",end_e);
		char new_head[1024] = {0};
		sprintf(new_head,"%s\n%s",buf,end);
		
		printf("%s\n",new_head);
		write(send_socket,new_head,size-(int)(end_e - end)+strlen(buf));
 		
	}
 
}

int main()
{
	
	//创建头结点 
	struct node  *head =  malloc(sizeof(struct node));
				  head->next = head; 
				  head->prev = head;  
				  head->socket = 0; 
	
	
	
	//1.新建TCP 通信对象 
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_socket==0)
	{
		perror("");
		return -1;
	}
	
	//SO_REUSEADDR
	//解决端口复用
	int on=1;
	setsockopt(tcp_socket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	on=1;
	setsockopt(tcp_socket,SOL_SOCKET,SO_REUSEPORT,&on,sizeof(on));
	//2.绑定服务器信息
	//设置服务器的IP地址信息  
		struct sockaddr_in  addr;  
		addr.sin_family   = AF_INET; //IPV4 协议  
		addr.sin_port     = htons(1218);
		addr.sin_addr.s_addr = INADDR_ANY; //服务器的IP 地址信息
	int ret=bind(tcp_socket,(struct sockaddr *)&addr,sizeof(addr));
	if(ret < 0)
	{
		perror("bind");
		return -1; 
	}
	else
	{
		printf("绑定网络服务器成功\n");
	}
	
	//3.设置服务器为监听模式
	listen(tcp_socket,5);
	
	
	//开启多路复用功能，监听多个文件描述符集合  
	
	while(1)
	{
		printf("1.显示用户列表  2.发布公告\n");
		
		 //1.清空文件描述符集合 
		   fd_set set;
		   FD_ZERO(&set);
		
		 //2.把需要监听文件描述符 添加到集合中 
		   FD_SET(0,&set);
		
		
		 //3.把服务器文件描述符添加到集合中 
		  FD_SET(tcp_socket,&set);
		
		
		 //4.把新链接进来的文件描述符添加到集合中
		 for(struct node  *pos=head->next;pos != head;pos=pos->next)
		 {
			FD_SET(pos->socket,&set); 
		 }
		
		
		 //4.开始监听  
		 if(max == 0)
		 {
			 max = tcp_socket;
		 }
		 else
		 {
			//把最大的描述符赋值给 max  
			//指向第一个结点  
			struct node  *pos = head->next; 
			while(pos != head)
			{
		
			//printf("socket=%d\n",pos->socket);
		    if(pos->socket > max)
			{
				 max = pos->socket;
			}
			
			pos = pos->next;
		
			} 
		 }
		 
	   int ret=select(max+1,&set,NULL,NULL,NULL);
		   if(ret < 0)
		   {
			   perror("");
			   return 0; 
		   }
		
		//判断是否为 输入信息  
		if(FD_ISSET(0,&set))
		{
			int  a=0;
			scanf("%d",&a);
			printf("a=%d\n",a);
			if(a == 1)
			{
				show(head); 
			}
			else if(a == 2)
			{
				char buf[1024] = {0};
				char buf2[1024] = {0};
				scanf("%s",buf);
				sprintf(buf2,"广播通知:%s",buf);
				anoucmt(head,buf2);
			}
			else{
				continue;
			}
		}
		
		//判断是否需要接受链接请求 
		if(FD_ISSET(tcp_socket,&set))
		{
			int new_fd=accept(tcp_socket,NULL,NULL);
				if(new_fd < 0)
				{
					perror("");
				}
				else
				{	
					//读取新用户名
					char buf[100]={0};
					char usr_name[50]={0};
					read(new_fd,buf,100);
					if(strstr(buf,"name:"))
					{
						char *tmp = strstr(buf,"name:");
						sscanf(tmp,"name:%s",usr_name);
					
						printf("name:%s\n",usr_name);
						
						//新用户描述符
						printf("new_fd=%d\n",new_fd);
						inser_node(head,new_fd,usr_name);	
					}
					else 
					printf("非本地用户\n");
				}
		
		}
 
	
		//判断是否客户端有信息发送过来   
		for(struct node  *pos=head->next;pos != head;pos=pos->next)
		{
			if(FD_ISSET(pos->socket,&set))
			{
				char buf[1024]={0}; 
				int size=read(pos->socket,buf,1024); //读取数据
				
					if(size <= 0)
					{
						//printf("%s已经下线了\n",pos->name);
						char exit[1024] = {0};
						sprintf(exit,"%s已经下线了\n",pos->name); 
						printf("%s",exit); 
						anoucmt(head,exit);
						
						//删除结点 
						struct node  *del = pos; 
						pos = pos->prev; 
						
						del->prev->next = del->next;  
						del->next->prev = del->prev;  
						del->next = NULL; 
						del->prev = NULL;  
						close(del->socket);
						free(del); 
					}
					else
					{
					 
						if(strstr(buf,"revallusr:"))
						{
							send_list(head,pos->socket);
							break;
						}
						
						//收取天气信息
						else if(strstr(buf,"weather:"))
						{
							printf("weather:\n");
							
							http_api(pos->socket,buf);
							break;
						}
						
						else if(strstr(buf,"licked beast:"))
						{
							printf("licked beast:\n");
							http_api(pos->socket,buf);
							break;
						}
						
						
						else  
						{
							char sendto[1024]={0};
							strcpy(sendto,buf);
							char * pp = strtok(sendto,":");
							if(pp  == NULL)
							{
								printf("格式不对\n");
								break;
							}
						 
							char *p =strstr(buf,":")+1;
							if(p  == NULL)
							{
								printf("格式不对\n");
								break;
							}
						  
							//查找此用户
							usr_p = findusr(head,sendto,pos->socket);
							if(usr_p == NULL)
							{
								printf("该用户%s不存在或未上线\n",sendto);
								break;
							}
							
							//获取发件人名字，作为接收方消息头缀
							char send_buf[100] = {0};
							sprintf(send_buf,"usr_%s:%s\n",pos->name,p);
							write(usr_p->socket,send_buf,strlen(send_buf));
						} 
					 
						
					}	
			}
			
		}
	}

}