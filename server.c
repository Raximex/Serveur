#include "utils.h"


int main(int argc, char**argv)
{   
    int sockfd;
    char buf[512];
    struct sockaddr_in addr;

    if(init_udp_server(&addr,&sockfd) == -1){
        printf("[init_udp_socket] : erreur\n");
    }
    socklen_t len = 0;
    int n = recvfrom(sockfd,(char* )buf,512,MSG_WAITALL,0,&len);
    uint16_t opcode = get_opcode(buf);
    char* filename = get_file_name(buf);
    char* mode = get_mode(buf);
    printf("%u\n",opcode);
    printf("%s\n",filename);
    printf("%s\n",mode);
    close(sockfd);
}

