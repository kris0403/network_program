#include <errno.h>
#include <signal.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h> 
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
#define COMMAND_MAX 1400
#define SERV_PORT 1111
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
    if(argc != 3) {
        printf("Usage: %s <SERVER PORT>\n", argv[0]);
        return 0;
    }
    
    
    int     sockfd;
    struct sockaddr_in servaddr;
    struct sockaddr cliaddr;
    socklen_t 		clilen;
    socklen_t 		servlen;
    clilen = sizeof(cliaddr);
    servlen = sizeof(servaddr);
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1)
    {
        cout<<"cannot create socket."<<endl;
        exit(0);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    
	if((bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1))
	{
		cout<<"cannot bind."<<endl;
		exit(0);
	}
    char line[COMMAND_MAX];
    FILE* fp;
    int fd;
    char* ack = new char[1];
    SEND_LINE  sender[1];
    
    
   fp = fopen(argv[2],"w");
    fd = fileno(fp);

    
    int file_size = 50000000;
    int n;
    int check[5000] = {0};
    int file_size_count = 0;
    
    //cout<<"1"<<ack<<endl;
    
    while(file_size > 0)
    {
        //cout<<"receive"<<endl;
        if((n = recvfrom(sockfd,sender,sizeof(sender),0, &cliaddr, &clilen))>= 0){
            if(file_size_count == 0)
            {
                file_size = sender[0].total_size;
            }
            file_size_count++;
            sendto(sockfd, ack, 1, 0, &cliaddr, clilen);
            if( check[sender[0].send_id] == 0)
            {
                cout<<file_size<<endl;
                write(fd,sender[0].temp_line,sender[0].size);
                file_size -= sender[0].size;
            }
        }
        check[sender[0].send_id] = 1;
    }
    sendto(sockfd, ack, 1, 0, &cliaddr, clilen);
    fclose(fp);
    cout<<"successed"<<endl;
    return 0;
}
