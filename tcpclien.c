#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include "font.h"


int x,y,x2,y2;

struct LcdDevice *init_lcd(const char *device)
{
	//申请空间
	struct LcdDevice* lcd = malloc(sizeof(struct LcdDevice));
	if(lcd == NULL)
	{
		return NULL;
	} 

	//1打开设备
	lcd->fd = open(device, O_RDWR);
	if(lcd->fd < 0)
	{
		perror("open lcd fail");
		free(lcd);
		return NULL;
	}
	
	//映射
	lcd->mp = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd->fd,0);

	return lcd;
}

int get_xy()//读取坐标
{
	
	int pressure;
	// 1》打开触摸屏文件
	int ts_fd = open("/dev/input/event0", O_RDONLY);
	if(ts_fd == -1)
	{
		perror("open /dev/input/event0 failed!");
		return -1;
	}
	
	struct input_event ts_buf;
	bzero(&ts_buf, sizeof(ts_buf));
	
	int flag1=1, flag2=1;
	
	while(1)
	{
// 2》读取触摸屏文件数据-->阻塞等待手指触摸屏幕，如果没有点击屏幕是会一直等待（在驱动里面设置）
		read(ts_fd, &ts_buf, sizeof(ts_buf));
		
		
		// 3》把触摸屏文件数据转化具体坐标和压力值
		//判断是否为绝对位移坐标事件
		if(ts_buf.type == EV_ABS)
		{
			//进一步判断是否为x轴事件
			if(ts_buf.code == ABS_X )
			{
				
				//保存具体的x轴坐标
				x2 = ts_buf.value;

			    x2 = x2*800/1024;
			
		      
				
				
				
				
				
			}
			//进一步判断是否为y轴事件
			else if(ts_buf.code == ABS_Y )
			{
				
				//保存具体的y轴坐标
				y2 = ts_buf.value;

			    y2 = y2*480/600;
			
					
			
				
			}
			
		}
		 //判断按键事件
		if(ts_buf.type == EV_KEY)
		{
			//进一步判断是否为压力值事件
			if(ts_buf.code == BTN_TOUCH)
			{
				if(ts_buf.value == 0)
				{
					break;
				}
				//手指按下 ，记录按下的值 
				if(ts_buf.value == 1)
				{
					x = x2;
					y = y2;
					
				}
			}

		} 
	}
	// 4》关闭文件
	close(ts_fd);
	
	
	
}

int ziku(char *buf)
{
	printf("zikutest\n");
    //初始化Lcd
	struct LcdDevice* lcd = init_lcd("/dev/fb0");
			
	//打开字体	
	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");
	  
	//字体大小的设置
	fontSetSize(f,30);
	
	
	//创建一个画板（点阵图）         //长 //宽
	bitmap *bm = createBitmapWithInit(800,375,4,getColor(0,255,255,255)); //也可使用createBitmapWithInit函数，改变画板颜色
	//bitmap *bm = createBitmap(288, 100, 4);
	
	
	
	
	
	//scanf("%d", &a[0]);
	
	
	
	//将字体写到点阵图上
	fontPrint(f,bm,0,0,buf,getColor(0,255,0,0),0);
	
	//把字体框输出到LCD屏幕上
	show_font_to_lcd(lcd->mp,0,0,bm);

	//把字体框输出到LCD屏幕上
	//show_font_to_lcd(lcd->mp,200,200,bm);
	/*bzero(buf, 12);
	destroyBitmap(bm);
	bitmap *bm = createBitmapWithInit(800,375,4,getColor(0,255,255,255)); //也可使用createBitmapWithInit函数，改变画板颜色
	
	//关闭字体，关闭画板
	fontUnload(f); */ //字库不需要每次都关闭 
	destroyBitmap(bm);//画板需要每次都销毁 
	
}

void *funa(void *arg)
{
	int sockfd = *(int*)arg;
	while(1)
	{
		get_xy();
		 if(y2>375&&x2<255)
		  {
			  printf("y2=%d x2=%d\n",y2,x2);
			  char buf[15]={"weather:"};
			  write(sockfd,buf,strlen(buf));  
		  }
	}
	
	return 0;
	
}

void *funb(void *arg)
{
	int sockfd = *(int*)arg;
	//printf("testfunb\n");
	while(1)
	{
		char buf[4096]={0};
		read(sockfd,buf,strlen(buf));
		//printf("%s\n",buf);
		if(strstr(buf,"usr_msg")!=0)//读私发信息
			{
		
				printf("私发：%s\n",buf);//字库
				ziku(buf);
			}
	}
	
	return 0;
	
}

int set_bmp(int bmp_w,int bmp_h,int m,int n,const char *bmp_name,int c)//bmp设置
{
	
	// 1》打开bmp文件
	
	FILE *fp = fopen(bmp_name,"r");
	if(fp == NULL)
	{
		perror("bmp open failed!\n");
		return -1;
	}
	printf("showing %s \n", bmp_name);
	// 2》跳过54个字节文件头
	int ret=lseek(fp->_fileno, 54, SEEK_SET);
	if( ret == -1)
	{
		perror("lseek failed!\n");
		return -1;
	}
	else 
	{
		printf("ret: %d \n",ret);
	}
	
	int ret2 = (4-bmp_w*3%4)%4;
	
	// 3》读取bmp文件的数据
	unsigned char bmp_buf[bmp_h*bmp_w*3+ret2*bmp_h];//图片缓冲区
	bzero(bmp_buf, bmp_h*bmp_w*3+ret2*bmp_h);
	int ret1 = fread(bmp_buf, bmp_h*bmp_w*3+ret2*bmp_h, 1, fp);
	if(ret1 == 1)
	{
		printf("fread %d nmemb data!\n", ret1);
		
	}
	else if(ret1 < 1)
	{
		if(feof(fp))
		{
			printf("end !\n");
		}
		if(ferror(fp))
		{
			printf("error !\n");
		}
	}
	// 4》关闭文件
	fclose(fp);
	
	
	// 5》合成LCD类型像素点数据
	unsigned int lcd_buf[bmp_h*bmp_w];//屏幕缓冲区
	bzero(lcd_buf,bmp_h*bmp_w*4);
	
	int i, j;
	for(j=0; j<bmp_h; j++)
	{
		for(i=0; i<bmp_w; i++)
		{
		
			lcd_buf[j*bmp_w+i] = (bmp_buf[3*i+3*j*bmp_w+0]<<0) | (bmp_buf[3*i+3*j*bmp_w+1]<<8) | (bmp_buf[3*i+3*j*bmp_w+2]<<16) ;
			
		}
	}
	unsigned int show_buf[bmp_h*bmp_w];//上下颠倒
	bzero(show_buf,bmp_h*bmp_w*4);
	
	int x1,y1;
	for(y1=0;y1<bmp_h;y1++)
	{
		for(x1=0;x1<bmp_w;x1++)
		{
			show_buf[(bmp_h-1-y1)*bmp_w+x1]=lcd_buf[y1*bmp_w+x1];
		}
	}
			
	// 6》打开LCD屏幕文件
	int lcd_fd = open("/dev/fb0", O_RDWR);
	if(lcd_fd == -1)
	{
		perror("open /dev/fb0 failed!");
		return -1;
	}
	
	// 7》内存映射
	unsigned int *addr = mmap(NULL, 800*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
	if(addr == MAP_FAILED)
	{
		perror("mmap failed!");
		close(lcd_fd);
		return -1;
	}
	
	// 8》把数据通过指针赋值的方式拷贝到内存
	
	int a,b;
	
	if(c == 1)//从上往下
	{
		for(a=0; a<bmp_h; a++)
		{	
			for(b=0; b<bmp_w; b++)
			{
			
				*(addr+800*a+b+800*n+m) = show_buf[a*bmp_w+b];
			}
			
		}
	}
	else//从左往右
	{
		for(b=0; b<bmp_w; b++)
		{
			for(a=0;a<bmp_h; a++)
			{
				*(addr+800*a+b+800*n+m) = show_buf[a*bmp_w+b];
			}
			usleep(1000);
		}
		
	}
	
	// 9》关闭文件和解除映射
	
	close(lcd_fd);
	munmap(addr, 800*480*4);
	printf("show %s success\n", bmp_name);
	return 0;
}



int main(int argc,char **argv)
{
	
	set_bmp(800,480,0,0,"11.bmp",1);
	
	if(argc!=2)
	{
		printf("请输入正确的参数\n");
	}

		//1.创建 TCP  通信协议
       int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd < 0)
			{
				perror("");
				return 0; 
			}
			else
			{
				printf("创建成功\n");
			}
	
		//设置链接的服务器地址信息 
		struct sockaddr_in  addr;  
		addr.sin_family   = AF_INET; //IPV4 协议  
		addr.sin_port     = htons(1218); //端口
		addr.sin_addr.s_addr = inet_addr("192.168.22.100"); //服务器的IP 地址
		//2.链接服务器 
		int ret=connect(sockfd,(struct sockaddr *)&addr,sizeof(addr));
		if(ret < 0)
		{
			perror("");
			return 0;
		}
		else
		{
			printf("链接服务器成功\n");
			char buff[1024]={"name:"};
			char buf[1024]={0};
			sprintf(buf,"%s%s",buff,argv[1]);
			write(sockfd,buf,strlen(buf));
		}
		
	pthread_t  tid;
	pthread_create(&tid,NULL,funa,&sockfd); 
	pthread_t  tid1;
	pthread_create(&tid1,NULL,funb,&sockfd); 
	while(1)
	{
		
	    
		//1.清空文件描述符集合 
		   fd_set set;
		   FD_ZERO(&set);
		
		 //2.把需要监听文件描述符 添加到集合中 
		   FD_SET(0,&set);
		
		
		 //3.把服务器文件描述符添加到集合中 
		  FD_SET(sockfd,&set);
		  
		  int ret=select(sockfd+1,&set,NULL,NULL,NULL);
		  if(ret < 0)
		  {
			  perror("");
			  return 0;
		  }
		  
		  if(FD_ISSET(sockfd,&set))//读信息
		  {
			 char   buf[4096]={0}; 
			 read(sockfd,buf,1024);
			if(strstr(buf,"该用户不存在或未上线")!=0)//读取到对方不在线
			 {
				 printf("%s\n",buf);
			 }
		 
			else if(strstr(buf,"已建立与该用户的聊天窗口")!=0)//读取到对方在线并输入聊天内容
			{
				printf("%s\n",buf);
				while(1)
				{
					char buff[4096]={0};
					printf("请输入聊天内容:\n");
					scanf("%s",buff);
					if(strstr(buff,"over")!=0)
					{
						break;
					}
					write(sockfd,buff,strlen(buff));
				}
			}
			
			else if(strstr(buf,"为您播报未来四天的天气")!=0)//读天气
			{
		
				printf("%s\n",buf);//字库
				ziku(buf);
			}
			
			/*  else if(strstr(buf,"usr_msg")!=0)//读私发信息
			{
		
				printf("%s\n",buf);//字库
				ziku(buf);
			}  */
			
			else if(strstr(buf,"广播")!=0)
			{
			 printf("广播通知:%s\n",buf); 
			 ziku(buf);
			}
			
		  }
		  
		  if(FD_ISSET(0,&set))	//住户私聊 发送接收方信息的住户名
		  {
			  
			char buf[1024]={0}; 
			char buff[4069]={0};
			read(0,buf,1024);
			
		/* 	if(strstr(buf,"weather:")!=0)
			{
				write(sockfd,buf,strlen(buf));
			} */
			
			
			
				sprintf(buff,"%s%s","revusr_name:",buf);
				printf("buf=%s\n",buff);
				//发送 
				write(sockfd,buff,strlen(buff));
		
		  }
		  
		 
		  
		  
		  
		  
		  
		  
		  
		  
		  
		  
		  
		  
		  
		  
		  
	}
	
	//关闭通信 
	close(sockfd);
}