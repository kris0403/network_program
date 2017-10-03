#include <errno.h>
#include <signal.h>
#include <sys/time.h>
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
#define COMMAND_MAX 1400
using namespace std;
struct SEND_LINE
{
    char temp_line[COMMAND_MAX];
    int send_id;
    int size;
    int total_size;
};
int main(int argc, char **argv)
{
    struct sockaddr_in server_addr;
    int socket_fd;
    struct sockaddr cliaddr;
    socklen_t 		clilen;
    clilen = sizeof(cliaddr);
    socklen_t 		servlen;
    servlen = sizeof(server_addr);
    if(argc != 4)
    {
        cout<<"ERROR : ./sender <receiver_ip> <port_number> <Data_name>"<<endl;
        return 0;
    }
    
    bzero(&server_addr, sizeof(server_addr));//initial server_addr
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));//host to network long (2 bytes)
    inet_pton(AF_INET,argv[1],&server_addr.sin_addr);
    if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)//SOCK_DGRAM for UDP
    {
        return 0;
    }
    
    char line[COMMAND_MAX];
    int fd;
    FILE* fp;
    int n;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    SEND_LINE  sender[1];
    sender[0].send_id = 0;
    
    fp = fopen(argv[3],"r");
    fd = fileno(fp);
    int total=0;
    while((n = read(fd,line,COMMAND_MAX)))
    {
        total += n;
    }
    sender[0].total_size = total;
    char buffer[COMMAND_MAX];
    //sprintf(buffer,"%d", total);
    
    char* ack = new char[1];
    
    fclose(fp);
    fp = fopen(argv[3],"r");
    fd = fileno(fp);
    while(n = read(fd,sender[0].temp_line,COMMAND_MAX))//read(fd,line,COMMAND_MAX)
    {
        sender[0].send_id++;
        sender[0].size = n;
        sender[0].total_size = total;
        while(1)
        {
            sendto(socket_fd, sender, sizeof(sender), 0, (struct sockaddr *) &server_addr, servlen);//write(socket_fd,line,n);
            
            n = recvfrom(socket_fd,ack,1,0,  &cliaddr, &clilen);
            if(n < 0)
            {
                //if(errno == EINTR)
                //{
                cout<<"timeout!!"<<endl;
                continue;
                //}
            }
            else
            {
                //cout<<ack<<endl;
                break;
            }
            
        }
    }
    cout<<"successed"<<endl;
    fclose(fp);
    return 0;
}
