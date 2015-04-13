#include "socket.h"
/*
open_clientfd函数和运行在主机hostname上的服务器建立一个连接
并在知名端口port上监听连接请求 。
*/
int open_clientfd(char *hostname,int port)
{
        int clientfd;
        struct hostent *hp;
        struct sockaddr_in serveraddr;

        if((clientfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
        {
            perror("socket error");
            exit(-1);
        }

        if((hp = gethostbyname(hostname))==NULL)
        {
            perror("get host name error");
            exit(-2);
        }

        bzero(&serveraddr,sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_port = htons(port);/*主机字节序转换网络字节序*/
        bcopy(hp->h_addr_list[0],&serveraddr.sin_addr.s_addr,hp->h_length);/*把hp->h_addr_list[0]中的ip地址复制到serveraddr中*/

      if(connect(clientfd,(SA*)&serveraddr,sizeof(serveraddr)) < 0)
      {
            perror("connect error");
            exit(-1);
      }

      return clientfd;
}

/* 打开监听套接字，这个描述符准备在知名端口上准备接受请求*/
int open_listenfd(int port)
{

        int listenfd,optval = 1;
        struct sockaddr_in  serveraddr;

        if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
        {
            perror("socket error");
            exit(-1);
        }

       if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&optval,sizeof(int)) < 0)/*这个套接字的地址可以重用，重复绑定 */
       {
            perror("adress already in use error");
            exit(-1);
       }

       bzero(&serveraddr,sizeof(serveraddr));
       serveraddr.sin_family = AF_INET;
       serveraddr.sin_port = htons(port);
       serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if(bind(listenfd,(SA*)&serveraddr,sizeof(serveraddr)))
        {
            perror("bind error");
            exit(-1);
        }

        if(listen(listenfd,LISTENQ) < 0)
        {
            perror("listen error");
            exit(-1);
        }

        return listenfd;
}
handler_t  *signal_r(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART; 

    if (sigaction(signum, &action, &old_action) < 0)
	perror("Signal error");
    return (old_action.sa_handler);
}
void sigchild_handler( int sig )
{
    int stat;
	while(waitpid(-1,&stat,WNOHANG)>0);
	return;
}
