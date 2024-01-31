#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

char transfer_mode[10]="NETASCII";

int main(int argc, char const* argv[])
{
    int status, valread, client_fd;
    char buf[2048]="Salut";
    struct sockaddr_in serv_addr;
    socklen_t addrlen = sizeof(serv_addr);


    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((status
         = connect(client_fd, (struct sockaddr*)&serv_addr,
                   addrlen))< 0) 
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    /*if(send(client_fd,buf,strlen(buf),0)<0)
    {
        printf("Erreur envoi message");
        return -1;
    }*/
    send_RRQ(client_fd,"Test.txt");
    // closing the connected socket
    close(client_fd);
    return 0;
}


void send_RRQ(int client_fd, char *file_name)
{

    int len=0;
    char buffer[512];// le buffer
    int bloc=1;
    uint16_t opcode = htons(1); // RRQ

    uint8_t octet = 0; // octet différenciant le mode et le nom de fichier

    memcpy(buffer, &opcode,2);//copie du code opérateur
    len+=2;//déplacement du pointeur
    memcpy(buffer+len,file_name,strlen(file_name));//Nom de fichier
    len+=strlen(file_name);
    memcpy(buffer+len,&octet,1);//Octet
    len+=1;
    memcpy(buffer,transfer_mode,strlen(transfer_mode));//copie de mode de transfert
    len+=strlen(transfer_mode);
    memcpy(buffer,&octet,1);


    send(client_fd,buffer,strlen(buffer),0);
}




void send_WRQ(int client_fd,char *file_name)
{


    int len=0;
    char buffer[512];// le buffer
    int bloc=1;
    uint16_t opcode = htons(2); // RRQ

    uint8_t octet = 0;

    memcpy(buffer, &opcode,2);
    len+=2;
    memcpy(buffer+len,file_name,strlen(file_name));
    len+=strlen(file_name);
    memcpy(buffer+len,&octet,1);
    len+=1;
    memcpy(buffer,transfer_mode,strlen(transfer_mode));
    len+=strlen(transfer_mode);
    memcpy(buffer,&octet,1);


    send(client_fd,buffer,strlen(buffer),0);
}