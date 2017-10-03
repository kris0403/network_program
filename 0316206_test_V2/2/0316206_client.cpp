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
using namespace std;
bool check_exit(char* a)
{
    bool check = false;
    for(int i = 0;i < COMMAND_MAX;i++)
    {
        if(a[i] == '\n')
        {
            break;
        }
        else if(a[i] == 'E' && a[i+1] == 'X' && a[i+2] == 'I' && a[i+3] == 'T' )
        {
            check = true;
            break;
        }
        return check;
    }
}
int main(int argc,char ** argv)
{
    struct sockaddr_in server_addr;
    int socket_fd;
    int n;
    if(argc != 3)
    {
        cout<<"a"<<endl;
        return 0;
    }
    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        cout<<"b"<<endl;
        return 0;
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET,argv[1],&server_addr.sin_addr);
    if(connect(socket_fd, (struct sockaddr *)&server_addr,sizeof(server_addr))<0)
    {
        cout<<"connect fail"<<endl;
        return 0;
    }
    int maxfdp1;
    fd_set r_set;
    char send_line[COMMAND_MAX],recv_line[COMMAND_MAX];
    FD_ZERO(&r_set);
	 char* opcode ; 
	char full_sent_line[COMMAND_MAX];
	char data_1[COMMAND_MAX];
   	char data_2[COMMAND_MAX];
    while(1)
    {
	
		FD_SET(fileno(stdin),&r_set);
        FD_SET(socket_fd,&r_set);
        maxfdp1 = max(fileno(stdin),socket_fd)+1;
        select(maxfdp1,&r_set,NULL,NULL,NULL);
        if(FD_ISSET(socket_fd,&r_set))
        {
            int leng;
            if((leng = read(socket_fd,recv_line,COMMAND_MAX)) == 0)
            {
                cout<<"the server has closed the connection."<<endl;
                close(socket_fd);
                return 0;
            }
            		recv_line[leng] = '\0';
            		cout<<recv_line;
        }
        if(FD_ISSET(fileno(stdin),&r_set))
        {
            if(fgets(send_line,COMMAND_MAX,stdin)==NULL || check_exit(send_line) == true)
            {
                write(socket_fd,send_line,strlen(send_line));
				close(socket_fd);
                return 0;
            }
			write(socket_fd,send_line,strlen(send_line));		
			for(int i=0;i<=sizeof(send_line);i++)
    		{
       			full_sent_line[i]=send_line[i];
    		}
    		opcode = strtok(full_sent_line," \n");  		
		      
		
		if(strcmp(opcode,"PUT") == 0)
		{
				for(int i = 0;i < COMMAND_MAX;i++)
				{
			
					data_1[i] = 0;
					data_2[i] = 0;
				}
        		int space_2;
        		int counter = 0;
       			 for(space_2 = 4;send_line[space_2]!=' ';space_2++)
        		{
               			 data_1[counter] = send_line[space_2];
                		counter++;
        		}
        		counter =0;
        		space_2+=1;
       			 for(;send_line[space_2]!='\n';space_2++)
        		{
                		data_2[counter] = send_line[space_2];
                		counter++;
       			 } 
       			 
       		//	cout<<data_1<<" "<<data_2<<endl;
			char line[COMMAND_MAX];
			int fd;
			FILE* fp;
			
			fp = fopen(data_1,"r");
			fd = fileno(fp);
			int total=0;		
			while(n = read(fd,line,COMMAND_MAX))
			{
				total += n;
			}
			char buffer[COMMAND_MAX];
			sprintf(buffer,"%d", total);
			write(socket_fd,buffer,15);
			fclose(fp);
			
			fp = fopen(data_1,"r");
			fd = fileno(fp);
			while(n = read(fd,line,COMMAND_MAX))
			{
				write(socket_fd,line,n);
			}
			printf("%s %s %s succeeded.\n",opcode,data_1,data_2);
			fclose(fp);
		}
		
		else if(strcmp(opcode,"GET") == 0)
		{
				for(int i = 0;i < COMMAND_MAX;i++)
				{
			
					data_1[i] = 0;
					data_2[i] = 0;
				}
        		int space_2;
        		int counter = 0;
       			 for(space_2 = 4;send_line[space_2]!=' ';space_2++)
        		{
               			 data_1[counter] = send_line[space_2];
                		counter++;
        		}
        		counter =0;
        		space_2+=1;
       			 for(;send_line[space_2]!='\n';space_2++)
        		{
                		data_2[counter] = send_line[space_2];
                		counter++;
       			 } 
       			 
       		//	cout<<data_1<<" "<<data_2<<endl;
			char line[COMMAND_MAX];					
			FILE* fp;
			int fd;
			fp = fopen(data_2,"w");
			fd = fileno(fp);
			
			
			read(socket_fd,line,15);
			//cout<<line<<endl;
			int file_size = atoi(line);	
			fclose(fp);
		
			fp = fopen(data_2,"w");
			fd = fileno(fp);	
			int n;
			while(n = read(socket_fd,line,COMMAND_MAX))
			{
				file_size -= n;
				write(fd,line,n);
				if(file_size == 0)break;
			}
			printf("%s %s %s succeeded.\n",opcode,data_1,data_2);
			fclose(fp);
			
		}
		
		
		
        }
    }



	return 0;
}


