#include "socket.h"
int main(int argc,char  **argv)
{
    int listenfd,connfd,clilen,port;
    struct sockaddr_in clientaddr;
    
    if(argc != 2){
            fprintf(stderr,"usage:%s  <port>\n",argv[0]);
    }
    port =atoi( argv[1]);

    listenfd = open_listenfd(port);

    signal_r(SIGCHLD,sigchild_handler);

  while(1){
        clilen = sizeof(clientaddr);
       if( (connfd = accept(listenfd,(SA*)&clientaddr,&clilen)) < 0){
                if(errno == EINTR)
                    continue;
                else
                    perror("accept error");
       }
       if(fork() == 0){
            close(listenfd);/*avoid the waste of the fd*/
            do_http(connfd);
            exit(0);
       }
       close(connfd);
  }
    return 0;
}
