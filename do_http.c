#include "socket.h"

void get_filetype(char *filename,char *filetype)
{
        if(strstr(filename,".html"))
            strcpy(filetype,"text/html");
        else if(strstr(filename,".gif"))
            strcpy(filetype,"image/gif");
        else if(strstr(filename,".jpg"))
            strcpy(filetype,"image/jpeg");
        else if(strstr(filename,".bmp"))
            strcpy(filetype,"image/bmp");
        else if(strstr(filename,".mpg"))
            strcpy(filetype,"video/mpeg");
        else if(strstr(filetype,".png"))
            strcpy(filetype,"image/png");
            else if(strstr(filename,".mp4"))
            strcpy(filetype,"video/mp4");

}
void serve_dynamic(int fd,char *filename,char *cgiargs)
{
        char buf[MAXLINE],*list[] = {NULL};

        sprintf(buf,"HTTP/1.0 200 OK\r\n");
        rio_writen(fd,buf,strlen(buf));
        sprintf(buf,"Server: Tiny Web Server\r\n");
        rio_writen(fd,buf,strlen(buf));

        if(fork() == 0)
        {
            setenv("QUERY_STRING",cgiargs,1);/*设置环境变量*/
            dup2(fd,STDOUT_FILENO);/*标准输出重定向到客户端*/
            execve(filename,list,environ);
        }
        wait(NULL);
}
/*读哪些文件*/
void serve_static(int fd,char * filename,int filesize)
{
        int srcfd;
        char *srcp,filetype[MAXLINE],buf[MAXLINE];

        /*把头部信息发送给客户端*/
        get_filetype(filename,filetype);
        sprintf(buf,"HTTP/1.0 200 OK\r\n");
        sprintf(buf,"%s Server: Tiny Web Server\r\n",buf);
        sprintf(buf,"%s Content-length: %d\r\n",buf,filesize);
        sprintf(buf,"%s Content-type: %s\r\n\r\n",buf,filetype);
        rio_writen(fd,buf,strlen(buf));

        /*把发送的文件内容给客户端*/
        srcfd =open(filename,O_RDONLY,0);
        srcp = mmap(0,filesize,PROT_READ,MAP_PRIVATE,srcfd,0);/*将文件映射到虚拟存储器里面*/
        close(srcfd);/*映射到内存就可以把原来的文件描述符关了*/
        rio_writen(fd,srcp,filesize);/*从虚拟存储器里面去读*/
        munmap(srcp,filesize);
}

void error(int fd,char *cause,char *errnum,char *errmsg,char *reasonmsg )/*把错误号,原因,那个文件出错都显示出来*/
{

    char buf[MAXLINE],body[MAXBUF];

    sprintf(body,"<html><title> Fantasy Web Error</title>");/*html 体部标记*/
    sprintf(body,"%s <body bgcolor=""ffffff"">\r\n",body);
    sprintf(body,"%s<p><h1>%s: %s</h1></p>\r\n",body,errnum,errmsg);
    //sprintf(body,"%s<p>%s: %s\r\n",body,reasonmsg,cause);
    sprintf(body,"%s<p><h2><hr><em> From Fantasy Web Server </em></h2></p>\r\n",body);

    sprintf(buf,"HTTP/1.0 %s %s\r\n",errnum,errmsg);
    rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Content-type: text/html\r\n");
    rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Content-length: %d\r\n\r\n",(int)strlen(body));
    rio_writen(fd,buf,strlen(buf));
    rio_writen(fd,body,strlen(body));

}
/*读报头的所有信息,web服务器不处理报头
    过滤,这个时候第一个请求已经读取了
*/
void read_requestignore(rio_t *rp)
{
        char buf[MAXLINE];
        int fd;

        rio_readlineb(rp,buf,MAXLINE);

        while(strcmp(buf,"\r\n"))
        {
            rio_readlineb(rp,buf,MAXLINE);
            FILE *out = fopen("log.txt","w+");
            if(out == NULL)
            {
                perror("cannot open the file ");
                exit(-1);
            }
            fprintf(out,"%s",buf);
            fflush(out);
            fclose(out);
        }
        return;
}
/*解析字符串是静态的还是动态的*/
int parse_url(char *url,char *filename,char *cgiargs)
{
       char *ptr;

        if(!strstr(url,"cgi-bin"))/*找有没有cgi-bin表示是一个静态文件*/
        {
            strcpy(cgiargs,"");/*标志是静态文件*/
            strcpy(filename,".");
            strcat(filename,url);
            if(url[strlen(url)-1] == '/')/*这种情况取一个默认的文件index.html*/
                strcat(filename,"index.html");
            return  is_not_cgi;
        }
        else  /*是动态的*/
        {
                ptr = index(url,'?');
                if(ptr)
                {
                    strcpy(cgiargs,ptr+1);
                    *ptr = '\0';
                }
                else
                    strcpy(cgiargs ,"");
        strcpy(filename,".");
        strcat(filename,url);
        return is_cgi;
        }
}

/* 处理http事物
调用uri_parse函数解析请求
并且serve_static函数处理静态内容
serve_dynamic 运行cgi程序
*/
void  do_http(int fd)
{
    int is_static;
    struct stat sbuf;
   char  buf[MAXLINE],method[MAXLINE],version[MAXLINE],url[MAXLINE];
   char  filename[MAXLINE],cgiargs[MAXLINE];
   rio_t rio;


   rio_readinitb(&rio,fd);/*读函数初始化*/
   rio_readlineb(&rio,buf,MAXLINE);/*读函数和描述符连接起来*/
   sscanf(buf,"%s %s %s",method,url,version);/*注意到字符串遇到空格停顿*/

   if(strcasecmp(method,"GET"))/*不区分大小写的比较两个字符串,没有get 就不行*/
    {
       error(fd,method,"501","Not Implemented","web is not implemnt this method");
        return  ;
    }

    read_requestignore(&rio);/*读取并忽略报头*/

    is_static  = parse_url(url,filename,cgiargs);/*解析字符串看是静态还是非静态,若果有找到了返回*/
      if(stat(filename,&sbuf) < 0)
      {
            error(fd,filename,"404","Not Found","web server cannot find file");
            return;
      }

      if(is_static)/*判断一下读取的是静态的还是动态的*/
      {
            if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR &sbuf.st_mode))/*判断一下是不是普通文件还有这个文件可不可读*/
            {
                    error(fd,filename,"403","Forbidden","web server cannot read the file");
                    return;
            }
            serve_static(fd,filename,sbuf.st_size);/*读文件*/
      }
      else /*动态的就去调执行的那个文件*/
      {
            if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR &sbuf.st_mode ))/*判断文件是不是普通文件,再看看能不能读*/
            {
                        error(fd,filename,"403","Forbidden","web server cannot run the cgi program");
                        exit(-1);
            }
            printf("%s\n",cgiargs);
            serve_dynamic(fd,filename,cgiargs);
      }
}
