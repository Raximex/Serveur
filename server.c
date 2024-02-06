#include "utils.h"


int main(int argc, char**argv)
{   
    int sockfd;
    char buf[MAX_BLOCK_SIZE];
    struct sockaddr_in addr,client_addr;

    if(init_udp_server(&addr,&sockfd,8080) == -1){
        printf("[init_udp_socket] : erreur\n");
    }
    socklen_t len = sizeof(addr);
    recvfrom(sockfd,(char* )buf,516,0,(struct sockaddr *) &client_addr,&len);
    print_request_packet(buf);
    handle_request(buf,&client_addr,sockfd);
    close(sockfd);
}

