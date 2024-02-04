#include "utils.h"
int main(int argc, char const* argv[])
{
    int sockfd;
    struct sockaddr_in addr;

    if(init_udp_client(&addr,&sockfd) == -1){
        printf("[init_udp_socket] : erreur");
    }
    request(RRQ,"hello world","netascii",sockfd,(struct sockaddr*)&addr);
    close(sockfd);
}

