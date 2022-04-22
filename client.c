#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  N  32
#define  R  1   //  register
#define  L  2   //  login
#define  Q  3   //  query
#define  M  4   //  modify
#define  A  5   //  add
#define  D  6   //  delete		

#define DATABASE "my.db"

typedef struct 
{
	int type;
	char number[N];  //工号
	char data[256];   //密码或消息
} USR;

typedef struct 
{
	char name[N]; //姓名
	char sex[N]; //性别
	int age;     //年龄
	int salary;  //薪水
	char id[N];	//身份证号
	char tel[N]; //手机号
	char department[N]; //部门
} MSG;

void do_register(int socketfd, USR *usr);
int do_login(int socketfd, USR *usr);
void do_msg(int socketfd, int result);
void do_add(int socketfd, MSG *msg, USR *usr);
void do_modify(int socketfd, MSG *msg, USR *usr);
void do_delete(int socketfd, MSG *msg, USR *usr);
void do_query(int socketfd, MSG *msg, USR *usr, int result);

int main(int argc, char *argv[])
{
	MSG msg;
	USR usr;
	int socketfd ;
	struct sockaddr_in server_addr;
	if (argc < 3)
	{
		printf("Usage : %s <serv_ip> <serv_port>\n", argv[0]);
		exit(-1);
	}
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("fail to connect");
		exit(-1);
	}
    int n;
	while(1)
	{
		printf("************************************\n");
		printf("* 1: register   2: login   3: quit *\n");
		printf("************************************\n");
		printf("please choose : ");

		int result;

		if(scanf("%d", &n) <= 0)
		{
			perror("scanf");
			exit(-1);
		}

		switch(n)
		{
			case 1:
				do_register(socketfd, &usr);
				break;
			case 2:
                //执行登录函数，执行完毕后通过返回值决定是否要跳转到下一个菜单
				if((result = do_login(socketfd, &usr)) != 0)					
					do_msg(socketfd, result);
				break;
			case 3:
				close(socketfd);
				exit(0);
		}
	}
	return 0;
}


void do_register(int socketfd, USR *usr)
{
	//指定操作码
	usr->type = R;
    //输入工号
	printf("input your number:");
	scanf("%s", usr->number);
	//输入密码
    printf("input your password:");
	scanf("%s", usr->data);

	send(socketfd, usr, sizeof(USR), 0);

    recv(socketfd, usr, sizeof(USR), 0);
	printf("register : %s\n", usr->data);
	
    return;
}

int do_login(int socketfd, USR *usr)
{
	usr->type = L;
    //输入工号
	printf("input your number:");
	scanf("%s", usr->number);
	//输入密码
    printf("input your password:");
	scanf("%s", usr->data);

	send(socketfd, usr, sizeof(USR), 0);
	recv(socketfd, usr, sizeof(USR), 0);
	//判断是否登录成功
    if(strncmp(usr->data, "aOK", 4) == 0)
	{
        //登录成功返回1
		printf("login : adminer OK\n");
		return 1;
	}
	else if (strncmp(usr->data, "uOK", 4) == 0)
	{
		//登录成功返回2
		printf("login : user OK\n");
		return 2;
	}
	else
	{
		//登录失败返回0
		printf("login : %s\n", usr->data);
		return 0;
	}
}


void do_msg(int socketfd,int result)
{
	MSG msg;
	USR usr;
	while(1)
	{
		int n;
		printf("************************************\n");
		printf("* 1: query   2: modify   3:add   4:delete   5: quit   6.up *\n");
		printf("************************************\n");
		printf("please choose : ");

		if(scanf("%d", &n) <= 0)
		{
			perror("scanf");
			exit(-1);
		}
		if(n == 6)
		{
			break;
		}
		switch(n)
		{
			case 1:
				if(result == 2)
				{
					printf("有访问权限\n");
					do_query(socketfd, &msg, &usr,result);
				}
				else
				{
					printf("有访问权限\n");
					do_query(socketfd, &msg, &usr,result);
				}
				break;
			case 2:
				if(result == 2)
				{
					printf("没有访问权限\n");
				}
				else
				{
					printf("有访问权限\n");
					do_modify(socketfd, &msg, &usr);
				}
				break;
			case 3:
				if(result == 2)
				{
					printf("没有访问权限\n");
				}
				else
				{
					printf("有访问权限\n");
					do_add(socketfd, &msg, &usr);
				}
				break;
			case 4:
				if(result == 2)
				{
					printf("没有访问权限\n");
				}
				else
				{
					printf("有访问权限\n");
					do_delete(socketfd, &msg, &usr);
				}
				break;
			
			case 5:
				close(socketfd);
				exit(0);
		}
	}
	return ;
}

void do_add(int socketfd, MSG *msg, USR *usr)
{	
	usr->type = A;

	printf("input name:");
	scanf("%s", msg->name);

    printf("input sex:");
	scanf("%s", msg->sex);

	printf("input age:");
	scanf("%d", &msg->age);

    printf("input salary:");
	scanf("%d", &msg->salary);

	printf("input id:");
	scanf("%s", msg->id);

    printf("input tel:");
	scanf("%s", msg->tel);

	printf("input department:");
	scanf("%s", msg->department);

	send(socketfd, usr, sizeof(USR), 0);
	send(socketfd, msg, sizeof(MSG), 0);


	recv(socketfd, usr, sizeof(USR), 0);
	printf(">>> %s\n", usr->data);
	return;
}
void do_modify(int socketfd, MSG *msg, USR *usr)
{
	int n;
	usr->type = M;
	memset(usr->data,0,256);

NEXT:
    printf("请输入要修改员工的身份证号\n");
	scanf("%s", msg->id);
	while (1)
	{
		printf("请输入要修改的内容\n");
		printf("* 1: name   2: sex   3:age   4:salary   5: tel   6.department  7.up*\n");
		printf("************************************\n");

		if(scanf("%d", &n) <= 0)
		{
			perror("scanf");
			exit(-1);
		}
		if(n == 7)
		{
			break;
		}

		switch(n)
		{
			case 1:
				printf("请输入修改员工的姓名为\n");
				scanf("%s", msg->name);
				if(strcmp(msg->name, "#") == 0)
					break;
				sprintf(usr->data,"%d",n);
				send(socketfd, usr, sizeof(USR), 0);
				send(socketfd, msg, sizeof(MSG), 0);
				recv(socketfd, usr, sizeof(USR), 0);
				printf(">>> %s\n", usr->data);
				if(strncmp(usr->data, "NO ID",6) == 0)
					goto NEXT;	
				break;
			case 2:
				printf("请输入修改员工的性别为\n");
				scanf("%s", msg->sex);
				if(strcmp(msg->sex, "#") == 0)
					break;
				sprintf(usr->data,"%d",n);
				send(socketfd, usr, sizeof(USR), 0);
				send(socketfd, msg, sizeof(MSG), 0);
				recv(socketfd, usr, sizeof(USR), 0);
				printf(">>> %s\n", usr->data);
				if(strncmp(usr->data, "NO ID",6) == 0)
					goto NEXT;				
				break;
			case 3:
				printf("请输入修改员工的年龄为\n");
				scanf("%d", &msg->age);
				if(msg->age == 0)
					break;
				sprintf(usr->data,"%d",n);
				send(socketfd, usr, sizeof(USR), 0);
				send(socketfd, msg, sizeof(MSG), 0);
				recv(socketfd, usr, sizeof(USR), 0);
				printf(">>> %s\n", usr->data);
				if(strncmp(usr->data, "NO ID",6) == 0)
					goto NEXT;		
				break;
			case 4:
				printf("请输入修改员工的薪水为\n");
				scanf("%d", &msg->salary);
				if(msg->salary == 0)
					break;
				sprintf(usr->data,"%d",n);
				send(socketfd, usr, sizeof(USR), 0);
				send(socketfd, msg, sizeof(MSG), 0);
				recv(socketfd, usr, sizeof(USR), 0);
				printf(">>> %s\n", usr->data);
				if(strncmp(usr->data, "NO ID",6) == 0)
					goto NEXT;						
				break;
			case 5:
				printf("请输入修改员工的电话为\n");
				scanf("%s", msg->tel);
				if(strcmp(msg->tel, "#") == 0)
					break;
				sprintf(usr->data,"%d",n);
				send(socketfd, usr, sizeof(USR), 0);
				send(socketfd, msg, sizeof(MSG), 0);
				recv(socketfd, usr, sizeof(USR), 0);
				printf(">>> %s\n", usr->data);
				if(strncmp(usr->data, "NO ID",6) == 0)
					goto NEXT;		
				break;
			case 6:
				printf("请输入修改员工的部门为\n");
				scanf("%s", msg->department);
				if(strcmp(msg->department, "#") == 0)
					break;
				sprintf(usr->data,"%d",n);
				send(socketfd, usr, sizeof(USR), 0);
				send(socketfd, msg, sizeof(MSG), 0);
				recv(socketfd, usr, sizeof(USR), 0);
				printf(">>> %s\n", usr->data);	
				if(strncmp(usr->data, "NO ID",6) == 0)
					goto NEXT;	
				break;
		}
	}
	return;
}

void do_delete(int socketfd, MSG *msg, USR *usr)
{
	usr->type = D;

	printf("请输入要删除员工的身份证号：\n");
	scanf("%s", msg->id);

	send(socketfd, usr, sizeof(USR), 0);
	send(socketfd, msg, sizeof(MSG), 0);
	
	recv(socketfd, usr, sizeof(USR), 0);
	printf(">>> %s\n", usr->data);
	return;
}


void do_query(int socketfd, MSG *msg, USR *usr, int result)
{
	
	usr->type = Q;
	sprintf(usr->data,"%d",result);
	//保存result不被更改
	char temp[N];
	strcpy(temp,usr->data);
	while (1)
	{
		strcpy(usr->data,temp);
		printf("请输入要查询的员工的姓名：\n");
		scanf("%s", msg->name);
		//返回上一级菜单
		if(strcmp(msg->name, "#") == 0)
			break;

		send(socketfd, usr, sizeof(USR), 0);
		send(socketfd, msg, sizeof(MSG), 0);
	
		recv(socketfd, usr, sizeof(USR), 0);

		printf(">>> %s\n", usr->data);


	}

	return;
}