#include "utils.h"


int main(int argc, char**argv)
{   
    int sockfd;
    char buf[MAX_BLOCK_SIZE];
    struct sockaddr_in addr,client_addr;

    if(init_udp_server(&addr,&sockfd) == -1){
        printf("[init_udp_socket] : erreur\n");
    }
    socklen_t len = sizeof(addr);
    int n = recvfrom(sockfd,(char* )buf,512,0,(struct sockaddr *) &client_addr,&len);
    print_request(buf);
    close(sockfd);
}

