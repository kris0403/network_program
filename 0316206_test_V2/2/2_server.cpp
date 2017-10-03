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
#include <fstream>
#define COMMAND_MAX 1000
#define SERVER_PORT 1111
#define LISTENQ 1024
char ip[FD_SETSIZE][30];
unsigned short port[FD_SETSIZE];
char data_1[FD_SETSIZE][COMMAND_MAX];
char data_2[FD_SETSIZE][COMMAND_MAX];
struct fi{
	char file_name[COMMAND_MAX][COMMAND_MAX];
};
fi file_n[FD_SETSIZE];
int op;
char opcode[COMMAND_MAX];
int list_count[COMMAND_MAX] = {0};
using namespace std;
void command(int* client,int id,char *recv_line,int sockfd)
{
    char buffer[COMMAND_MAX]; 
    char full_recv_line[COMMAND_MAX];
    for(int i=0;i<=sizeof(recv_line);i++)
    {
        full_recv_line[i]=recv_line[i];
    }
    char* opcode = strtok(full_recv_line," \n");
    if(strcmp(opcode,"PUT") == 0)//op 0
    {
		op = 0;
		for(int i=0;i<COMMAND_MAX;i++)
		{
			data_1[id][i] = 0;
			data_2[id][i] = 0;
		}
		
        int space_2;
        int counter = 0;
        for(space_2 = 4;recv_line[space_2]!=' ';space_2++)
        {
                data_1[id][counter] = recv_line[space_2];
                counter++;
        }
        counter =0;
        space_2+=1;
        for(;recv_line[space_2]!='\n';space_2++)
        {
                data_2[id][counter] = recv_line[space_2];
                counter++;
        }
		char line[COMMAND_MAX];					
		FILE* fp;
		int fd;
		fp = fopen(data_2[id],"w");
		fd = fileno(fp);
		
		read(sockfd,line,15);
		int file_size = atoi(line);	
		fclose(fp);
		
		fp = fopen(data_2[id],"w");
		fd = fileno(fp);	
		int n;
		while(n = read(sockfd,line,COMMAND_MAX))
		{
			file_size -= n;
			write(fd,line,n);
			if(file_size == 0)break;
		}
		fclose(fp);
		strcpy(file_n[id].file_name[list_count[id]],data_2[id]);
		list_count[id]++;
    }
    else if(strcmp(opcode,"LIST")== 0)//op 1
    {
		op = 1; 
		for(int i=0;i<list_count[id];i++)
		{
			sprintf(buffer,"%s\n",file_n[id].file_name[i]);
       		write(sockfd,buffer,strlen(buffer));
     	}
		sprintf(buffer,"%s succeeded.\n",opcode);
        write(sockfd,buffer,strlen(buffer));
    }
    else if(strcmp(opcode,"GET")== 0)//op 2
    {
		op = 2;        
		int space_2;
        int counter = 0;
        for(int i=0;i<COMMAND_MAX;i++)
		{
			data_1[id][i] = 0;
			data_2[id][i] = 0;
		}
        for(space_2 = 4;recv_line[space_2]!=' ';space_2++)
        {
            data_1[id][counter] = recv_line[space_2];
            counter++;
        }
        counter =0;
        space_2+=1;
        for(;recv_line[space_2]!='\n';space_2++)
        {
            data_2[id][counter] = recv_line[space_2];
        	counter++;
        }
		
			int n;
			char line[COMMAND_MAX];
			int fd;
			FILE* fp;
			
			fp = fopen(data_1[id],"r");
			fd = fileno(fp);
			int total=0;
			//cout<<data_1[id]<<endl;		
			while(n = read(fd,line,COMMAND_MAX))
			{
				//cout<<n<<endl;
				total += n;
			}
			char buffer[COMMAND_MAX];
			sprintf(buffer,"%d", total);
			//cout<<buffer<<endl;
			write(sockfd,buffer,15);
			fclose(fp);
			
			fp = fopen(data_1[id],"r");
			fd = fileno(fp);
			while(n = read(fd,line,COMMAND_MAX))
			{
				write(sockfd,line,n);
			}
			
			fclose(fp);
    }
    else if(strcmp(opcode,"EXIT")== 0)
    {
    	list_count[id] = 0;
	}
	else
	{
		char buffer[COMMAND_MAX];
		sprintf(buffer,"command erorr!\n");
		write(sockfd,buffer,30);
	}
}
int main(int argc, char** argv)
{
    int                 listening_fd;
    struct sockaddr_in  listening_addr;
    int                 maxi;
    int                 client[FD_SETSIZE];
    int                 max_fd;
    int                 nready,connfd,sockfd;
    fd_set              all_set,r_set;
    socklen_t           clilen;
    struct sockaddr_in  cliaddr,servaddr;
    char                recv_line[COMMAND_MAX];
    int                 ii;
    ssize_t             n;
	char buffer[COMMAND_MAX];


    memset(&listening_addr, 0 ,sizeof(listening_addr));
    listening_addr.sin_family = AF_INET;
    listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listening_addr.sin_port = htons(SERVER_PORT);

    listening_fd = socket(AF_INET, SOCK_STREAM,0);
    if(listening_fd == -1)
    {
        cout<<"cannot create socket."<<endl;
        exit(0);
    }
    if((bind(listening_fd, (struct sockaddr *)&listening_addr,sizeof(listening_addr)) == -1))
    {
        cout<<"cannot bind."<<endl;
        exit(0);
    }
    if(listen(listening_fd,LISTENQ) == -1)
    {
        cout<<"cannot listen."<<endl;
        exit(0);
    }

    /*FD_SET(listening_fd,&all_set);
    max_fd = listening_fd;
    maxi = -1;
    for(int i = 0;i< FD_SETSIZE;i++)
    {
        client[i] = -1;
    }
    FD_ZERO(&all_set);
    FD_SET(listening_fd,&all_set);*/
    int j=0;
    while(1)
    {
        clilen = sizeof(cliaddr);
        connfd = accept(listening_fd,(struct sockaddr *)&cliaddr,&clilen);
		
		if(fork() == 0) 
		{
			close(listening_fd);
			while(1)
			{
				int n = read(connfd,recv_line,COMMAND_MAX);
				if(n == 0)
				{
					close(connfd);
					break;
				}
				command(client, j,recv_line,connfd);
			}
			j++;	
			exit(0);
		}

		close(connfd);
		/*r_set = all_set;
        nready = select(max_fd + 1,&r_set,NULL,NULL,NULL);
        if(FD_ISSET(listening_fd,&r_set))
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listening_fd,(struct sockaddr *)&cliaddr,&clilen);
            for(ii = 0;ii< FD_SETSIZE;ii++)
            {
                if(client[ii]<0)
                {
                    client[ii] = connfd;
                    break;
                }
            }
            inet_ntop(AF_INET,&cliaddr.sin_addr,ip[0],sizeof(ip[0]));
            port[0] = ntohs(cliaddr.sin_port);
            ////hello(client,ii,&cliaddr,maxi);
            if(ii == FD_SETSIZE)
            {
                cout<<"too many"<<endl;
                exit(0);
            }
            FD_SET(connfd,&all_set);
            if(connfd > max_fd)
            {
                max_fd = connfd;
            }
            if(ii > maxi)
            {
                maxi = ii;
            }
            if(--nready <= 0)
            {
                continue;
            }
        }
        for(int j= 0;j<=maxi;j++)
        {
            if((sockfd = client[j])<0)
            {
                continue;
            }
            if(FD_ISSET(sockfd,&r_set))
            {
                if((n = read(sockfd,recv_line,COMMAND_MAX)) == 0)
                {
                    for(int i=0;i<=maxi;i++)
                    {
                            if(i==j)
                            {
                            continue;
                            }
                            else
                            {//client[i]leva
							
                            }
                    }
                    close(sockfd);
                    FD_CLR(sockfd,&all_set);
                    client[j] = -1;
					
                }
                else
                {
			                  
			command(client, j,recv_line,sockfd);
				
                }
                if(--nready <= 0)
                {
                        break;
                }

            }
        }*/


    }

   

}


