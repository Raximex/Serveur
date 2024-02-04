#include "utils.h"


int init_udp_server(struct sockaddr_in* addr,int* sockfd){
    if ((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[socket]");
        return -1;
    }
    (*addr).sin_family=AF_INET;
    (*addr).sin_port=htons(8080);
    (*addr).sin_addr.s_addr= INADDR_ANY;

     if (bind(*sockfd,(struct sockaddr*)addr,sizeof(*addr))< 0) {
        perror("[bind]");
        return -1;
    }
    return 0;

}
int init_udp_client(struct sockaddr_in* addr,int* sockfd){
    if ((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[socket]");
        return -1;
    }
    (*addr).sin_family=AF_INET;
    (*addr).sin_port=htons(8080);
    (*addr).sin_addr.s_addr= INADDR_ANY;
    return 0;

}
int rrq(const char* filename,const char* mode,int sockfd,struct sockaddr* addr){
    u_int16_t opcode = 1;
    u_int8_t end_of_file = 0;
    int offset = 0;
    char packet[512];
    memcpy(packet,&opcode,2);
    offset+=2;
    memcpy(packet+offset,filename,strlen(filename));
    offset+=strlen(filename);
    memcpy(packet+offset,&end_of_file,1);
    offset+=1;
    memcpy(packet+offset,mode,strlen(mode));
    offset+=strlen(mode);
    memcpy(packet+offset,&end_of_file,1);
    if(sendto(sockfd,packet,512,MSG_WAITALL,addr,sizeof(*addr))==-1){
        perror("[sendto]");
        return -1;
    }
    return 1;
}
int wrq(const char* filename,const char* mode,int sockfd,struct sockaddr* addr){
    u_int16_t opcode = 2;
    u_int8_t end_of_file = 0;
    int offset = 0;
    char packet[512];
    memcpy(packet,&opcode,2);
    offset+=2;
    memcpy(packet+offset,filename,strlen(filename));
    offset+=strlen(filename);
    memcpy(packet+offset,&end_of_file,1);
    offset+=1;
    memcpy(packet+offset,mode,strlen(mode));
    offset+=strlen(mode);
    memcpy(packet+offset,&end_of_file,1);
    if(sendto(sockfd,packet,512,MSG_WAITALL,addr,sizeof(*addr))==-1){
        perror("[sendto]");
        return -1;
    }
    return 1;
}
int get_opcode(char packet[512]){
    return (uint16_t) packet[0];
}
char* get_file_name(char packet[512]){
    char* buffer=strdup(packet+2);
    return buffer;
}
char* get_mode(char packet[512]){
    char* filename = get_file_name(packet);
    char* mode = strdup(packet+strlen(filename)+3);
    free(filename);
    return mode;
}
int handle_request(char packet[512]){
    int opcode = get_opcode(packet);
    switch (opcode)
    {
    case RRQ:
        /* code */

        break;
    case WRQ:
        /* code */
        break;
    case DATA:
        /* code */
        break;
    case ACK:
        /* code */
        break;            
    case ERROR:
        /* code */
        break;
    default:
        break;
    }    



}