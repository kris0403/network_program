#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#define COMMAND_MAX 1000
using namespace std;
bool check_exit(char * a)
{
	bool check = false;
	for(int i=0;i<COMMAND_MAX;i++)
	{
		if(a[i] == '\n')
		{
			break;
		}
		else if(a[i] == 'e' && a[i+1] == 'x' && a[i+2] == 'i' && a[i+3]=='t')
		{
			check = true;
			break;
		}
	}
	return check;
}
int main(int argc, char **argv)
{
	struct sockaddr_in server_addr;
	int socket_fd;
	int n;
	
	if(argc != 3)
	{
		cout<<"usage: tcpcli <IPaddress>"<<endl;
		return 0;
	}
	
	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return 0;
	}
	bzero(&server_addr, sizeof(server_addr));//initial server_addr
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));//host to network long (2 bytes)
	//memcpy(&server_addr.sin_addr,argv[1] , sizeof(argv[1]));//一定讀 host->h_length的長度才停止 
	inet_pton(AF_INET,argv[1],&server_addr.sin_addr);
	if(connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		cout<<"connect fail"<<endl;
		return 0;
	} 
	int maxfdp1;
	fd_set r_set;
	char send_line[COMMAND_MAX],recv_line[COMMAND_MAX];
	
	
	FD_ZERO(&r_set);
	while(1)
	{
		FD_SET(fileno(stdin),&r_set);
		FD_SET(socket_fd,&r_set);
		maxfdp1 = max(fileno(stdin), socket_fd) + 1;
		select(maxfdp1, &r_set, NULL, NULL, NULL);
		
		
		if(FD_ISSET(socket_fd,&r_set))
		{   
		  int leng;
			if((leng=read(socket_fd, recv_line, COMMAND_MAX))==0)
			{
				close(socket_fd);
				return 0;
			}
			recv_line[leng] = '\0';
			cout<<recv_line<<endl;
		}
		if(FD_ISSET(fileno(stdin),&r_set))/* input is readable */
		{
			if(fgets(send_line,COMMAND_MAX,stdin)==NULL || check_exit(send_line)==true)
			{
				
				//FD_CLR(fileno(stdin),&r_set);
				close(socket_fd);
				return 0;
			}
			write(socket_fd,send_line,strlen(send_line));
		}
	}
		
	/*pid_t pid;
	pid = fork();
	
	if(pid == 0)//child write
	{
		while(1)
		{
			char* recv_line;
			recv_line = new char[COMMAND_MAX];
			
			fgets(recv_line, COMMAND_MAX, stdin);
			if(check_exit(recv_line)==true)//exit
			{
				exit(0);
			}
			write(socket_fd,recv_line , strlen(recv_line));
		}
	}
	else if(pid > 0)//parent read
	{
		int s;
		while(1)
		{
			char* get_line;
			get_line = new char[COMMAND_MAX];
			int n = read(socket_fd, get_line, COMMAND_MAX);
			//fputs(get_line, stdout);
			cout<<get_line<<endl;
			if(n==0) {
					close(socket_fd);
					//return 0;
					exit(0);
				}
				
				//recv_line[n] = '\0';
				
		}
	}
	else
	{
		exit(0);
	}*/
	return 0;
}
