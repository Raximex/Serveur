#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, char**argv)
{   
    int sockfd, new_socket, recvfd;
    char buf[2048];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {//
        perror("socket échoué");
        return 1;
    }

    addr.sin_family=AF_INET;//
    addr.sin_port=htons(8080);
    addr.sin_addr.s_addr= INADDR_ANY;

     if (bind(sockfd,(struct sockaddr*)&addr,
             addrlen)
        < 0) {
        perror("bind échoué");
        return 1;
    }
    if(listen(sockfd, 3)<0)
    {
        perror("Listen échoué");
        return 1;
    }
    if((new_socket = accept(sockfd,(struct sockaddr*)&addr, &addrlen))<0)
    {
        perror("Nouveau socket échoué");
        return 1;
    }
    else{
        printf("client connecté !!\n");
    }
   // recvfd = read(new_socket,buf,strlen(buf)-1);
   // printf("%s", buf);
    accuseRecep(&new_socket);
    close(sockfd);
    close(new_socket);
}

void accuseRecep(int new_socket)
{
    char buffer[512];

    int accuse;
    accuse=read(new_socket,buffer,511);
    printf("%s",buffer);
}
