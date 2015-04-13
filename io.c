#include "socket.h"

/* 每打开一个描述符就会调用一次readinitb,fd与地址rp处所指向的缓冲区关联
  * rio_readinit函数从rp读出一个文本行(包括结尾的换行符)把它拷贝到存储器位置usrbuf
  *并用‘\0’结束
  */
void rio_readinitb(rio_t *rp,int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}
/*带缓冲区的读函数*/
static ssize_t rio_read(rio_t *rp,char *usrbuf,size_t n)
{
        int cnt;

        while(rp->rio_cnt <= 0)
        {
             rp->rio_cnt = read(rp->rio_fd,rp->rio_bufptr,sizeof(rp->rio_buf));

             if(rp->rio_cnt < 0)
             {
                    if(errno != EINTR )
                        return -1;
             }
             else if(rp->rio_cnt == 0)/*EOF*/
             {
                return 0;
             }
            else
                rp->rio_bufptr = rp->rio_buf;
        }

        cnt = n;
        if(rp->rio_cnt < n)
            cnt = rp->rio_cnt;/*要读的字符的数比缓冲区多,就只读缓冲区的*/
        memcpy(usrbuf,rp->rio_bufptr,cnt);
        rp->rio_bufptr  +=  cnt;
        rp->rio_cnt -= cnt;

        return cnt;
}
/*读取一行,带缓冲*/
ssize_t rio_readlineb(rio_t *rp,void *usrbuf,size_t maxlen)
{
        int rc,n;
        char c,*bufp = usrbuf;

        for(n = 1;n < maxlen;n++)
        {
                if((rc = rio_read(rp,&c,1)) == 1)/*用带缓冲的rio_read*/
                {
                        *bufp ++= c;
			if(c == '\n')
				break;
                }else if(rc == 0)
                {
                    if(n == 1)
                        return 0;/*没数据读*/
                    else
                        break;/*数据读完了*/
                }else
                    return -1;
        }

        *bufp = 0;
        return n;
}
/*带缓冲的读*/
ssize_t rio_readnb(rio_t *rp,void *usrbuf,size_t n)
{
    char *bufp = usrbuf;
    size_t nleft = n;
    ssize_t nread;

        while(nleft > 0)
        {
                if((nread = rio_read(rp,bufp,nleft)) <0)
                {
                        if(errno == EINTR)
                            nread = 0;
                        else
                            return -1;
                }
                else if(nread == 0)
                {
                    break;
                }
                bufp += nread;
                nleft  -= nread;
        }

        return (n -nleft);
}
/* rio_readn这个函数是一个健壮的可以多次读的函数，而且对中断返回再次读,size_t是 int ,ssize_t 是unsigned int*/
ssize_t rio_readn(int fd,void * usrbuf,size_t  n)
{
        char *pbuf = usrbuf;
        size_t  nleft = n;/*还剩下多少字节没有读完*/
        ssize_t nread;

        while(nleft > n)
        {
            if((nread = read(fd,pbuf,nleft))<0)
            {
                if(errno == EINTR)
                     nread = 0;/*发生中断的话就重读*/
            }
            else if(nread == 0)
            {
                break;/*读到文件末尾EOF*/
            }

            pbuf += nread;
            nleft -= nread;
        }

        return (n-nleft);
}

/*rio_write函数可以多次写*/
ssize_t rio_writen(int fd,void *usrbuf,size_t n)
{
        size_t  nleft = n;
        ssize_t nwritten;
        char *pbuf = usrbuf;

        while(nleft > 0)
        {
            if((nwritten = write(fd,pbuf,n)) <=0)
            {
                if(errno == EINTR)
                    nwritten = 0;/*中断返回后重写*/
                else
                   return  -1;
            }

            pbuf += nwritten;
            nleft -= nwritten;
        }

        return n;
}




