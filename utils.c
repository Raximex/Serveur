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
int request(int type,const char* filename, const char* mode, int sockfd, struct sockaddr* addr) {
    u_int16_t opcode = htons(type); // Convert opcode to network byte order
    u_int8_t end_of_file = 0;
    int offset = 0;
    // Allocate enough memory to include the opcode, filename, mode, and zero terminators
    char* packet = malloc(strlen(filename) + strlen(mode) + 4 + 2);
    if (packet == NULL) {
        perror("malloc");
        return -1;
    }

    memcpy(packet + offset, &opcode, 2);
    offset += 2;
    memcpy(packet + offset, filename, strlen(filename));
    offset += strlen(filename);
    packet[offset++] = end_of_file; // Add zero byte to terminate filename
    memcpy(packet + offset, mode, strlen(mode));
    offset += strlen(mode);
    packet[offset++] = end_of_file; // Add zero byte to terminate mode

    // Send the packet; use offset as the actual packet length
    if (sendto(sockfd, packet, offset, 0, addr, sizeof(*addr)) == -1) {
        perror("[sendto]");
        free(packet); 
        return -1;
    }

    free(packet); 
    return 1;
}
int get_opcode(char* packet){
    return (uint16_t) packet[0];
}
char* get_file_name(char* packet){
    char* buffer=strdup(packet+2);
    return buffer;
}
char* get_mode(char* packet){
    char* filename = get_file_name(packet);
    char* mode = strdup(packet+strlen(filename)+3);
    free(filename);
    return mode;
}
void print_request(char* packet){
    uint16_t opcode = get_opcode(packet);
    char* filename = get_file_name(packet);
    char* mode = get_mode(packet);
    printf("%u\n",opcode);
    printf("%s\n",filename);
    printf("%s\n",mode);
}
int handle_request(char* packet){
    int opcode = get_opcode(packet);
    switch (opcode)
    {
    case RRQ:
        /* code */
        handle_rrq(packet);
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
int handle_rrq(char* packet){

}