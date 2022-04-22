#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

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

void do_client(int connectfd, sqlite3 *db);
void do_register(int connectfd, USR *usr, sqlite3 *db);
void do_login(int connectfd, USR *usr, sqlite3 *db);
void do_add(int connectfd, USR *usr, MSG *msg, sqlite3 *db);
void do_modify(int connectfd, USR *usr, MSG *msg, sqlite3 *db);
void do_delete(int connectfd, USR *usr, MSG *msg, sqlite3 *db);
void do_query(int connectfd, USR *usr, MSG *msg, sqlite3 *db);


void handler(int sig)
{
    wait(NULL);
}


int main(int argc, char const *argv[])
{
    int listenfd,connectfd;
    struct sockaddr_in server_addr,clientaddr;
    pid_t pid;
    sqlite3 *db;

    if(argc < 3)
    {
        printf("Usage : %s <ip> <port>\n", argv[0]);
		exit(-1);
    }

    if (sqlite3_open(DATABASE, &db) != SQLITE_OK)
	{
		printf("error : %s\n", sqlite3_errmsg(db));
		exit(-1);
	}

    sqlite3_exec(db, "create table usr(number text primary key, pass text)", NULL, NULL, NULL);
	sqlite3_exec(db, "create table msg(name text, sex text, age int, salary int, id text primary key,tel text, depatement text)", NULL, NULL, NULL);
    
    if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("fail to socket");
        exit(-1);
    }

    bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("fail to bind");
		exit(-1);
	}
	
    if (listen(listenfd, 5) < 0)
	{
		perror("fail to listen");
		exit(-1);
	}

    signal(SIGCHLD, handler);//处理僵尸进程

    while (1)
    {
        if ((connectfd = accept(listenfd, NULL, NULL)) < 0)
		{
			perror("fail to accept");
			exit(-1);
		}

        if ((pid = fork()) < 0)
		{
			perror("fail to fork");
			exit(-1);
		}
		else if(pid > 0)
		{
            close(connectfd);
		}
        else
        {
		    do_client(connectfd, db);
        }
    }
     
    return 0;
}

void do_client(int connectfd, sqlite3 *db)
{
	USR usr;
	MSG msg;
	while (recv(connectfd, &usr, sizeof(USR), 0) > 0)
	{
		printf("type = %d\n", usr.type);

		switch (usr.type)
		{
		case R :
			do_register(connectfd, &usr, db);
			break;
		case L :
			do_login(connectfd, &usr, db);
			break;
		case A :
			do_add(connectfd, &usr, &msg, db);
			break;
		case M :
			do_modify(connectfd, &usr, &msg, db);
			break;
		case D :
			do_delete(connectfd, &usr, &msg, db);
			break;
		case Q :
			do_query(connectfd, &usr, &msg, db);
			break;
		}
	}
	printf("client quit\n");
	exit(0);
	return;
}

void do_register(int connectfd, USR *usr, sqlite3 *db)
{
	char sqlstr[512] = {0};
	char *errmsg;
    int result;
    //使用sqlite3_exec函数调用插入函数判断是否能够插入成功
    //由于用户名设置为主键，所以如果用户名已经存在就会报错
	sprintf(sqlstr, "insert into usr values('%s', '%s')", usr->number, usr->data);
	if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		sprintf(usr->data, "user %s already exist!!!", usr->number);
	}
	else
	{
		result = strncmp(usr->number,"1000",4);
		if(result == 0)
		{
			strcpy(usr->data, "adminer OK");
		}
		else
		{
			strcpy(usr->data, "user OK");
		}
	}

	send(connectfd, usr, sizeof(USR), 0);

	return;
}
void do_login(int connectfd, USR *usr, sqlite3 *db)
{
	char sqlstr[512] = {0};
	char *errmsg, **result;
	int nrow, ncolumn, login;

    //通过sqlite3_get_table函数查询记录是否存在
	sprintf(sqlstr, "select * from usr where number = '%s' and pass = '%s'", usr->number, usr->data);
	if(sqlite3_get_table(db, sqlstr, &result, &nrow, &ncolumn, &errmsg) != SQLITE_OK)
	{
		printf("error : %s\n", errmsg);
	}
    //通过nrow参数判断是否能够查询到疾记录，如果值为0，则查询不到，如果值为非0，则查询到
	if(nrow == 0)
	{
		strcpy(usr->data, "number or password is wrony!!!");
	}
	else
	{
		login = strncmp(usr->number,"1000",4);
		if(login == 0)
		{
			strncpy(usr->data, "aOK",256);
		}
		else
		{
			strncpy(usr->data, "uOK",256);
		}
	}
	send(connectfd, usr, sizeof(USR), 0);

	return;
}

void do_add(int connectfd, USR *usr, MSG *msg, sqlite3 *db)
{	
	char sqlstr[512] = {0};
	char *errmsg;


	recv(connectfd, msg, sizeof(MSG), 0);
	sprintf(sqlstr, "insert into msg values('%s', '%s', '%d', '%d', '%s', '%s', '%s')", msg->name, msg->sex, msg->age, msg->salary, msg->id, msg->tel, msg->department);
	if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		sprintf(usr->data, "error = %s",errmsg);
	}
	else
	{
		strcpy(usr->data, "add OK"); 
	}
 
	send(connectfd, usr, sizeof(USR), 0);
	return;
}

void do_modify(int connectfd, USR *usr, MSG *msg, sqlite3 *db)
{
	char sqlstr[512] = {0};
	char *errmsg;
	char **ret;
    int nrow, ncolumn;

	recv(connectfd, msg, sizeof(MSG), 0);

	sprintf(sqlstr, "select * from msg where id = '%s'", msg->id);
    if(sqlite3_get_table(db, sqlstr, &ret, &nrow, &ncolumn, &errmsg) != SQLITE_OK)
    {
        sprintf(usr->data, "error = %s",errmsg);
    }

	if(nrow ==0)
	{
		strcpy(usr->data, "NO ID");
	}
	else
	{
		switch(atoi(usr->data))
		{
			case 1:
				sprintf(sqlstr, "update msg set name='%s' where id='%s'", msg->name, msg->id);
				break;
			case 2:
				sprintf(sqlstr, "update msg set sex='%s' where id='%s'", msg->sex, msg->id);
				break;
			case 3:
				sprintf(sqlstr, "update msg set age=%d where id='%s'", msg->age, msg->id);
				break;
			case 4:
				sprintf(sqlstr, "update msg set salary=%d where id='%s'", msg->salary, msg->id);
				break;
			case 5:
				sprintf(sqlstr, "update msg set tel='%s' where id='%s'", msg->tel, msg->id);
				break;
			case 6:
				sprintf(sqlstr, "update msg set department='%s' where id='%s'", msg->department, msg->id);
				break;
		}


		if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			sprintf(usr->data, "error = %s",errmsg);
		}
		else
		{
			strcpy(usr->data, "modify OK"); 
		}
	}
	send(connectfd, usr, sizeof(USR), 0);
	return;
}

void do_delete(int connectfd, USR *usr, MSG *msg, sqlite3 *db)
{
	char sqlstr[512] = {0};
	char **ret;
    int nrow, ncolumn;
	char *errmsg;

	recv(connectfd, msg, sizeof(MSG), 0);

	sprintf(sqlstr, "select * from msg where id = '%s'", msg->id);
    if(sqlite3_get_table(db, sqlstr, &ret, &nrow, &ncolumn, &errmsg) != SQLITE_OK)
    {
        sprintf(usr->data, "error = %s",errmsg);
    }

	if(nrow ==0)
	{
		strcpy(usr->data, "delete error!!!");
	}
	else
	{
		sprintf(sqlstr, "delete from msg where id='%s'",msg->id);
		if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			sprintf(usr->data, "error = %s",errmsg);
		}
		else
		{
			strcpy(usr->data, "delete OK"); 
		}
	}

	send(connectfd, usr, sizeof(USR), 0);
	return;
}

void do_query(int connectfd, USR *usr, MSG *msg, sqlite3 *db)
{
	recv(connectfd, msg, sizeof(MSG), 0);
	char sqlstr[512] = {0};
	char **ret;
    int nrow, ncolumn;
	char *errmsg;

	sprintf(sqlstr, "select * from msg where name = '%s'", msg->name);

    if(sqlite3_get_table(db, sqlstr, &ret, &nrow, &ncolumn, &errmsg) != SQLITE_OK)
    {
        sprintf(usr->data, "error = %s",errmsg);
    }
	
	if(nrow == 0)
	{
		strcpy(usr->data, "query error!!!");
	}
	else
	{
		if(!strncmp(usr->data,"1",1))
		{
			sprintf(usr->data,"%s:%s %s:%s %s:%s %s:%s %s:%s %s:%s %s:%s ",ret[0],ret[7],ret[1],ret[8],ret[2],ret[9],ret[3],ret[10],ret[4],ret[11],ret[5],ret[12],ret[6],ret[13]);
			printf("adminer query\n");
		}
		else
		{
			sprintf(usr->data,"%s:%s %s:%s %s:%s %s:%s ",ret[0],ret[7],ret[1],ret[8],ret[5],ret[12],ret[6],ret[13]);
			printf("usr query\n");
		}
	}
	
	send(connectfd, usr, sizeof(USR), 0);

}

