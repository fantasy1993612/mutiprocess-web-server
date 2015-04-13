
/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
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
#define MAXLINE 1024
/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */

int main(void)
{
char *buf, *p;
char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
int n1 = 0, n2 = 0;
buf = getenv("QUERY_STRING");
if (buf == NULL || strlen(buf) == 0) {
snprintf(content, sizeof(content),
"<form name=\"input\" action=\"http://localhost:2000/cgi-bin/addr\" method=\"get\">"
"<h1 align=\"center\">"
"<input type=\"text\" name=\"num1\" />"
"<br />"
"+"
"<br />"
"<input type=\"text\" name=\"num2\" />"
"<br />"
"<br />"
"<p>"
"<input type=\"submit\" value=\"submit\" />"
"</h1>"
"</form>");
} else {
p = strchr(buf, '&');
*p = '\0';
char *index = strchr(buf, '=');
if (index == NULL) {
strcpy(arg1, buf);
} else {
strcpy(arg1, index+1);
}
n1 = atoi(arg1);
index = strchr(p+1, '=');
if (index == NULL) {
strcpy(arg2, p+1);
} else {
strcpy(arg2, index+1);
}
n2 = atoi(arg2);
/* Make the response body */
snprintf(content, sizeof(content),
"Welcome to add.com: "
"THE Internet addition portal.\r\n<p>"
"The answer is: %d + %d = %d\r\n<p>"
"Thanks for visiting!\r\n",
n1, n2, n1+n2);
}
/* Generate the HTTP response */
printf("Content-length: %d\r\n", (int)strlen(content));
printf("Content-type: text/html\r\n\r\n");
printf("%s", content);
fflush(stdout);
exit(0);
}
