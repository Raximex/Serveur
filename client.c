#include "utils.h"
int main(int argc, char const* argv[])
{
    int sockfd;
    struct sockaddr_in addr;
    char buf[516];
    if(init_udp_client(&addr,&sockfd) == -1){
        printf("[init_udp_socket] : erreur");
    }
    request(RRQ,"hello world","netascii",sockfd,(struct sockaddr*)&addr);
    socklen_t len = sizeof(addr);
    recvfrom(sockfd,buf,516,0,(struct sockaddr *)&addr,&len);
    print_error_packet(buf);
    close(sockfd);
}

