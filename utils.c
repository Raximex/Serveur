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
int request(uint16_t opcode, const char* filename, const char* mode, int sockfd, struct sockaddr* addr) {
    u_int8_t end_of_file = 0;
    int offset = 0;
    // Calcul de la taille nécessaire pour le paquet
    char* packet = malloc(strlen(filename) + strlen(mode) + 4 + 2);
    if (packet == NULL) {
        perror("malloc");
        return -1;
    }

    // Convertir l'opcode en network byte order avant de le copier
    uint16_t net_opcode = htons(opcode);
    memcpy(packet + offset, &net_opcode, sizeof(net_opcode));
    offset += sizeof(net_opcode);

    // Copie du nom de fichier et du mode dans le paquet, avec terminaison nulle
    memcpy(packet + offset, filename, strlen(filename));
    offset += strlen(filename);
    packet[offset++] = end_of_file; // Ajouter un octet de fin pour le nom de fichier

    memcpy(packet + offset, mode, strlen(mode));
    offset += strlen(mode);
    packet[offset++] = end_of_file; // Ajouter un octet de fin pour le mode

    // Envoyer le paquet; utiliser offset comme la longueur réelle du paquet
    if (sendto(sockfd, packet, offset, 0, addr, sizeof(*addr)) == -1) {
        perror("[sendto]");
        free(packet); 
        return -1;
    }

    free(packet); 
    return 1;
}
uint16_t get_opcode(char* packet){
    uint16_t opcode;
    memcpy(&opcode, packet, sizeof(opcode));
    return ntohs(opcode); // Convertit de network byte order à host byte order
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
void print_request_packet(char* packet){
    uint16_t opcode = get_opcode(packet);
    char* filename = get_file_name(packet);
    char* mode = get_mode(packet);
    printf("%u\n",opcode);
    printf("%s\n",filename);
    printf("%s\n",mode);
    free(filename);
}
void print_error_packet(char* packet){
    uint16_t opcode = get_opcode(packet);
    uint16_t error_code = get_error_code(packet);
    char* error_message = get_error_message(packet);
    printf("%u\n",opcode);
    printf("%u\n",error_code);
    printf("%s\n",error_message);
    free(error_message);
}
uint16_t get_error_code(char* packet){
    uint16_t error_code;
    memcpy(&error_code, packet + 2, sizeof(error_code)); // Copie correctement les 2 octets
    return ntohs(error_code); // Conversion nécessaire
}
char* get_error_message(char* packet){
    char* error_msg = strdup(packet+4);
    return error_msg;

}

char* build_error_packet(uint16_t error_code, char* error_msg) {
    u_int8_t end_of_error_msg = 0;
    // Ajustez la taille du paquet pour inclure le byte de terminaison nulle
    size_t packet_size = strlen(error_msg) + 5 + 1; // +1 pour le byte de terminaison nulle
    char* error_packet = malloc(packet_size);
    if (error_packet == NULL) {
        perror("malloc");
        return NULL;
    }
    int offset = 0;
    uint16_t net_opcode = htons(ERROR); // Assurez-vous que ERROR est défini à 5
    uint16_t net_error_code = htons(error_code); // Conversion en network byte order
    
    // Copie de l'opcode en network byte order
    memcpy(error_packet + offset, &net_opcode, sizeof(net_opcode));
    offset += sizeof(net_opcode);

    // Copie du code d'erreur en network byte order
    memcpy(error_packet + offset, &net_error_code, sizeof(net_error_code));
    offset += sizeof(net_error_code);

    // Copie du message d'erreur et ajout du byte de terminaison nulle
    memcpy(error_packet + offset, error_msg, strlen(error_msg));
    offset += strlen(error_msg); // Pas besoin d'incrémenter offset ici pour le byte nul

    // Ajout du byte de terminaison nulle
    error_packet[offset] = end_of_error_msg; 
    
    return error_packet;
}
int handle_request(char* packet, struct sockaddr_in* client_addr,int sockfd){
    uint16_t opcode = get_opcode(packet);
    switch (opcode)
    {
    case RRQ:
        handle_rrq(packet,client_addr,sockfd);
        break;
    default:
        break;
    }    

}
int handle_rrq(char* packet,struct sockaddr_in* client_addr,int sockfd){
    char* filename = get_file_name(packet);
    char* mode = get_mode(packet);
    FILE * requested_file = NULL;
    if(strcasecmp("netascii",mode)){
        requested_file = fopen(filename,"r");
    }
    else{
        requested_file = fopen(filename,"rb");
    }
    if(requested_file == NULL){
        // le fichier n'existe pas on crée le Error packet
        char* error_packet = build_error_packet(FILE_NOT_FOUND,"le fichier est introuvable\n");
        if (sendto(sockfd, error_packet, strlen(error_packet), 0, (struct sockaddr*)client_addr, sizeof(*client_addr)) == -1) {
            perror("[sendto]");
            free(error_packet); 
        return -1;
    }

    free(error_packet); 
    return 1;
    }
    
}