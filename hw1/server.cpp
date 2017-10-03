#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#define COMMAND_MAX 1000
#define SERVER_PORT 1111
#define LISTENQ 1024
using namespace std;
char name[FD_SETSIZE][30];
char ip[FD_SETSIZE][30];
unsigned short port[FD_SETSIZE];
int have_name[FD_SETSIZE] = {0};
bool some_come = false;
int first_space(char* recv_line,int i)
{
	for(;recv_line[i] == ' ';i++);
	return i;
}
void hello_user(int* client,int id,const struct sockaddr_in * cli_addr,int maxi)
{
	char buffer[COMMAND_MAX];
	//int n = read(client[id],buffer,COMMAND_MAX);
//	buffer[n] = '\0';
	inet_ntop(AF_INET, &cli_addr->sin_addr, ip[id], sizeof(ip[id]));
	port[id] = ntohs(cli_addr->sin_port);
	if(have_name[id] == 0)
	{
		
		sprintf(buffer,"[Server] Hello, anonymous! From: %s/%hu",ip[id],port[id]);
		write(client[id],buffer,strlen(buffer));
		strcpy(name[id],"anonymous");
		
		
		for(int i=0;i<=maxi;i++)
		{
			if(i == id)
			{
				continue;
			}
			else
			{
				sprintf(buffer,"[Server] Someone is coming!");
				write(client[i],buffer,strlen(buffer));
			}
		}
			
	
		
	}
	else if(have_name[id] == 1)
	{
		sprintf(buffer,"[Server] Hello, %s! From: %s/%hu",name[id],ip[id],port[id]);
		write(client[id],buffer,strlen(buffer));
		for(int i=0;i<=maxi;i++)
		{
			if(i == id)
			{
				continue;
			}
			else
			{
				sprintf(buffer,"[Server] Someone is coming!");
				write(client[i],buffer,strlen(buffer));
			}
		}
	}
}
bool check_name_unique(int id,char* recv_line)//if not unique return false
{
	bool check = true;
	for(int i = 0;i < FD_SETSIZE;i++)
	{
		if(strcmp(name[i],recv_line) == 0)
		{
			if(i == id)
			{
				continue;
			}
			else
			{
				check = false;
				break;
			}
			
		}
	}
	return check;
}
bool check_name_avaliable(char* recv_line,int buffer_counter)
{
	bool check = true;
	if(buffer_counter < 2 || buffer_counter >12)
	{
		check = false;
		return check;
	}
	for(int i = 0;i < buffer_counter;i++)
	{
		if(!((recv_line[i] >= 'A' && recv_line[i] <= 'Z') || (recv_line[i] >= 'a' && recv_line[i] <= 'z'))) 
		{
			check = false;
			break;
		}
	}
	return check;
}
void change_name(int* client,int id,char* recv_line,int maxi)
{
	char buffer[COMMAND_MAX];
	char new_name[100];
	int k = first_space(recv_line,0);//command name
	k+=4;
	int counter = 0;
	for(int i = first_space(recv_line,k);recv_line[i] != '\n';i++)
	{
		new_name[counter] = recv_line[i];
		counter++;
	}
	new_name[counter]='\0';
	if(strcmp(new_name,"anonymous") == 0)
	{
		sprintf(buffer,"[Server] ERROR: Username cannot be anonymous.");
		write(client[id],buffer,strlen(buffer));
	}
	else if(check_name_avaliable(new_name,counter) == false)
	{
		sprintf(buffer,"[Server] ERROR: Username can only consists of 2~12 English letters.");
		write(client[id],buffer,strlen(buffer));
	} 
	else if(check_name_unique(id,new_name) == false)
	{
		for(int i=0;i<=maxi;i++)
		{
			if(i == id)
			{
				sprintf(buffer,"[Server] ERROR: %s has been used by others.",new_name);
				write(client[i],buffer,strlen(buffer));
			}
			else
			{
				continue;
			}
			
		}
		
	}
	
	else//change name success
	{
		sprintf(buffer,"[Server] You're now known as %s.",new_name);
		write(client[id],buffer,strlen(buffer));
		
		for(int i = 0;i < FD_SETSIZE;i++)
		{
			if(i == id)
			{
				
				continue;
			}
			else
			{
				sprintf(buffer,"[Server] %s is now known as %s.",name[id],new_name);
				write(client[i],buffer,strlen(buffer));
			}
		}
		strcpy(name[id],new_name);
		have_name[id] = 1; 
	}
	
}
void who_are(int* client,int id,int maxi)
{
	char* buffer;
	buffer = new char[COMMAND_MAX];
		for(int j = 0;j <= maxi;j++)
		{
			if(j==id)
			{
				sprintf(buffer,"[Server] %s %s/%hu ->me\n",name[id],ip[id],port[id]);
				write(client[id],buffer,strlen(buffer));				
			}
			else if(client[j] > 0)
			{
				sprintf(buffer,"[Server] %s %s/%hu\n",name[j],ip[j],port[j]);
				write(client[id],buffer,strlen(buffer));		
			}	
		}	
}
void broadcast(int* client,int id,char* recv_line,int maxi)
{
	char* buffer;
	buffer = new char[COMMAND_MAX];
	char yell_mes[1000];
	int k = first_space(recv_line,0);//command name
	k+=4;
	int counter = 0;
	for(int i = first_space(recv_line,k);recv_line[i] != '\n';i++)
	{
		yell_mes[counter] = recv_line[i];
		counter++;
	}
	yell_mes[counter]='\0';
	for(int i = 0;i <= maxi;i++)
	{
		sprintf(buffer,"[Server] %s yell %s\n",name[id],yell_mes);
		write(client[i],buffer,strlen(buffer));
	}	
}
void private_mess(int* client,int id,char* recv_line,int maxi)
{
	char sent_name[100];
	int k = first_space(recv_line,0);//command name
	k+=4;
	int counter = 0;
	int com_counter;
	char sent_mes[COMMAND_MAX];
	for(com_counter = first_space(recv_line,k);recv_line[com_counter] != ' ';com_counter++)
	{
		sent_name[counter] = recv_line[com_counter];
		counter++;
	}
	sent_name[counter]='\0';
	int mes_counter = 0;
	
	for(int i = first_space(recv_line,com_counter);recv_line[i] != '\n';i++)
	{
		sent_mes[mes_counter] = recv_line[i];
		mes_counter++;
	}
	sent_mes[mes_counter]='\0';
	
	
	char* buffer;
	buffer = new char[COMMAND_MAX];
	
	bool exist = false;
	for(int i = 0;i <= maxi;i++)
	{
		if(strcmp(sent_name,name[i]) == 0)
		{
			exist = true;
		}
	}
	if(strcmp(name[id],"anonymous") == 0)
	{
		sprintf(buffer,"[Server] ERROR: You are anonymous.\n");
		write(client[id],buffer,strlen(buffer));
	}
	else if(strcmp(sent_name,"anonymous") == 0)
	{
		sprintf(buffer,"[Server] ERROR: The client to which you sent is anonymous.\n");
		write(client[id],buffer,strlen(buffer));
	}
	else if(exist == false)
	{
		sprintf(buffer,"[Server] ERROR: The receiver doesn't exist.\n");
		write(client[id],buffer,strlen(buffer));
	}
	else
	{
		for(int i = 0;i <= maxi;i++)
		{
			if(i == id)
			{
				sprintf(buffer,"[Server] SUCCESS: Your message has been sent.\n");
				write(client[id],buffer,strlen(buffer));
			}
			else
			{
				sprintf(buffer,"[Server] %s tell you %s\n",name[id],sent_mes);
				write(client[i],buffer,strlen(buffer));
			}
		}
	}
}
void command(int* client, int maxi, int id, char* recv_line)
{
	int counter = 0;
	char* buffer;
	buffer = new char[COMMAND_MAX];
	char* com;
	com = new char[100];
	int com_counter=0;
	for(int i = first_space(recv_line,0);recv_line[i] != ' '&&recv_line[i] != '\n';i++)
	{
		com[com_counter] = recv_line[i];
		com_counter++;
	}
	com[com_counter] = '\0';
	if(strcmp(com,"name") == 0)
	{
		change_name(client,id,recv_line,maxi);
	}
	else if(strcmp(com,"who") == 0)
	{
		who_are(client,id,maxi);
	}
	else if(strcmp(com,"tell") == 0)
	{
		private_mess(client,id,recv_line,maxi);
	}
	else if(strcmp(com,"yell") == 0)
	{
		broadcast(client,id,recv_line,maxi);
	}
	else
	{
		sprintf(buffer,"[Server] ERROR: Error command.\n");
		write(client[id],buffer,strlen(buffer));
	}
	/*while(recv_line[counter] != '\n')
	{
		if(recv_line[counter] == ' ')// && yell == false && tell == false)
		{
			counter++;
		}
		else if(recv_line[counter] == 'n' && recv_line[counter + 1] == 'a' && recv_line[counter + 2] == 'm' && recv_line[counter + 3] == 'e')
		{
			counter += 5;
			do_name = true;
		}
		else if(recv_line[counter] == 'w' && recv_line[counter + 1] == 'h' && recv_line[counter + 2] == 'o')
		{
			counter+=3;
			who = true;
		}
		else if(recv_line[counter] == 'y' && recv_line[counter + 1] == 'e' && recv_line[counter + 2] == 'l'&& recv_line[counter + 3] == 'l')
		{
			counter += 5;
			yell = true;
		}
		else if(recv_line[counter] == 't' && recv_line[counter + 1] == 'e' && recv_line[counter + 2] == 'l'&& recv_line[counter + 3] == 'l')
		{
			counter += 4;
			tell = true;
		}
		else
		{
			buffer[buffer_counter] = recv_line[counter];
			//cout<<"1 "<<buffer[buffer_counter]<<endl;
			buffer_counter++;
			counter++;
			//cout<<"2 "<<recv_line[counter]<<endl;
		}
	}*/
	
		//buffer[buffer_counter] = '\0';
	
	
}
int main(int argc, char **argv)
{
	int						 listening_fd;
	struct sockaddr_in 		 listening_addr;
	int 					 maxi;
	int 					client[FD_SETSIZE];
	int 					max_fd;
	int 					nready,connfd,sockfd;	
	fd_set 					all_set, r_set;
	socklen_t 				clilen;
	struct sockaddr_in 		cliaddr, servaddr;
	char 					recv_line[COMMAND_MAX];
	int 					ii;
	ssize_t 				n;
	
	
	memset(&listening_addr, 0, sizeof(listening_addr));//initial listening_addr
	listening_addr.sin_family = AF_INET;//IPV4
	listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listening_addr.sin_port = htons(SERVER_PORT);
	
	listening_fd = socket(AF_INET, SOCK_STREAM, 0);//return one socket descripter,AF_INET = IPv4,SOCK_STREAM = TCP socket ,0==自動尋找適合的 protocol
	if(listening_fd == -1)
	{
		cout<<"cannot create socket."<<endl;
		exit(0);
	}
	if((bind(listening_fd, (struct sockaddr *) &listening_addr, sizeof(listening_addr)) == -1))
	{
		cout<<"cannot bind."<<endl;
		exit(0);
	}
	if(listen(listening_fd, LISTENQ) == -1)//LISTENQ->incoming queuem允許多少人進來 
	{
		cout<<"cannot listen."<<endl;
		exit(0);
	}
	
	
	FD_SET(listening_fd, &all_set);
	max_fd = listening_fd;
	maxi = -1;
	for(int i = 0;i < FD_SETSIZE;i++)
	{
		client[i] = -1;//-1 indicates available entry
	}
	FD_ZERO(&all_set);//set all_set = 0
	FD_SET(listening_fd, &all_set);//set listening's all_set = 1
	while(1)
	{
		
		r_set = all_set;//struct assign
		nready = select(max_fd + 1, &r_set, NULL, NULL, NULL);
		if(FD_ISSET(listening_fd, &r_set))//listening's r_set 是否為1
		{
			clilen = sizeof(cliaddr);
			connfd = accept(listening_fd, (struct sockaddr *)&cliaddr, &clilen); //return new socket file descriptor after you connect
			//new client come
			//to do
			
			
			for(ii = 0;ii < FD_SETSIZE;ii++)
			{
				if(client[ii] < 0)
				{
				//	some_come = true;
					client[ii] = connfd;//save descripteor
					break;
				}
			}
			hello_user(client,ii,&cliaddr,maxi);
			
			if(ii == FD_SETSIZE)
			{
				cout<<"too many clients"<<endl;
				exit(0);
			}
			
			FD_SET(connfd, &all_set);//add new descriptor to set
			if(connfd > max_fd)//for select
			{
				max_fd = connfd;
			}
			if(ii > maxi)//max index in client[] array
			{
				maxi = ii;
			}
			if(--nready <= 0)//no more readable descriptors
			{
				continue;
			}
		}
		for(int j = 0;j <= maxi;j++)
		{
			if((sockfd = client[j]) < 0)
			{
				continue;
			}
			if(FD_ISSET(sockfd, &r_set))
			{
				if((n = read(sockfd, recv_line, COMMAND_MAX)) == 0)//client[j]leave
				{
					
					for(int i=0;i<=maxi;i++)
					{
						if(i == j)
						{
							continue;
						}
						else
						{
							char* buffer;
							buffer = new char[COMMAND_MAX];
							sprintf(buffer,"[Server] %s is offline.",name[j]);
							write(client[i],buffer,strlen(buffer));
						}
						
					}
					
					close(sockfd);
					FD_CLR(sockfd, &all_set);
					client[j] = -1;
				}
				else
				{
					command(client, maxi, j, recv_line);
				}
				if(--nready <= 0)//no more readable descriptors
				{
					break;
				}
			}
		}		
	}
	return 0;
}
