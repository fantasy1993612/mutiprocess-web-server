#ifndef         SOCKET_H
#define         SOCKET_H
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<errno.h>
#include <sys/mman.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<pthread.h>
#include<signal.h>
#include<sys/select.h>
#include<sys/time.h>

extern char **environ;
#define RIO_BUFSIZE 	8192
#define LISTENQ 	1024
#define MAXLINE 	1024
#define MAXBUF 		8192
#define is_not_cgi 	1
#define is_cgi 		0


typedef struct
{
	int rio_fd;
        int rio_cnt;/*内部缓冲区中未读取的字节*/
        char *rio_bufptr;/*内部缓冲区中 下一个未读取的字节*/
        char rio_buf[RIO_BUFSIZE];
} rio_t;
typedef struct
{
	int maxfd; /*读描述集中最大的描述符是*/
	fd_set read;
	fd_set ready;
} poll;
 
typedef struct  sockaddr SA;
typedef void handler_t(int);

int open_listenfd(int port);
int open_clientfd(char *hostname,int port);
int parse_url(char *url,char *filename,char *cgiargs);

void do_http(int fd);
void get_filetype(char *filename,char *filetype);
void serve_dynamic(int fd,char *filename,char *cgiargs);
void serve_static(int fd,char * filename,int filesize);
void error(int fd,char *cause,char *errnum,char *errmsg,char *reasonmsg);
void read_requestignore(rio_t *rp);
void get_filetype(char *filename,char *filetype);

ssize_t rio_readn(int fd,void * usrbuf,size_t  n);
ssize_t rio_writen(int fd,void *usrbuf,size_t n);

handler_t  *signal_r(int signum, handler_t *handler);
void sigchild_handler( int sig );

#endif // SOCKET_H
