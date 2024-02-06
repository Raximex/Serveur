#include "server.h"
#define DEBUG 0
void send_error_packet(int error_code,char* error_msg, const struct sockaddr_in* client_addr, int sockfd){
    size_t packet_size;
    char* error_packet = build_error_packet(error_code, error_msg, &packet_size);
        if (sendto(sockfd, error_packet, packet_size, 0, (struct sockaddr*)client_addr, sizeof(*client_addr)) == -1) {
            perror("[sendto]");
        }
        free(error_packet); 
}
static int init_tftp_server(int port,int* sockfd,struct sockaddr_in* addr) {
    if ((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[socket]");
        return -1;
    }
    (*addr).sin_family = AF_INET;
    (*addr).sin_port = htons(port); // Specify the port as an argument
    (*addr).sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces

    if (bind(*sockfd, (struct sockaddr*)addr, sizeof(*addr)) < 0) {
        perror("[bind]");
        return -1;
    }
    return 0;
}
static void handle_client_requests(int sockfd,struct sockaddr_in* addr,struct sockaddr_in* client_addr){
    char buf[MAX_BLOCK_SIZE];
    socklen_t len = sizeof(addr);
    recvfrom(sockfd,(char* )buf,516,0,(struct sockaddr *) client_addr,&len);
    uint16_t opcode = get_opcode(buf);
    char* filename ;
    char* mode;
    switch (opcode)
    {
    case RRQ:
        filename = get_file_name(buf);
        mode = get_mode(buf);
        process_rrq(filename,mode,client_addr,sockfd);
        free(filename);
        free(mode);
        break;
    case WRQ :
        filename = get_file_name(buf);
        mode = get_mode(buf);
        process_wrq(filename,mode,client_addr,sockfd);
        free(filename);
        free(mode);
    default:
        break;
    }    
}
static int process_rrq(char* filename,char* mode, const struct sockaddr_in* client_addr, int sockfd){
    char buf[516]; 
    size_t packet_size;
    FILE* requested_file = fopen(filename, "rb");

    if (requested_file == NULL) {
        
       send_error_packet(FILE_NOT_FOUND,"Le fichier est introuvable",client_addr,sockfd);
        return -1;
    }

    char data[512];
    size_t bytes_read = 0;
    int block_number = 1;
    socklen_t len = sizeof(*client_addr);

    while ((bytes_read = fread(data, 1, 512, requested_file)) > 0) {
        char* data_packet = build_data_packet(block_number++, data, bytes_read, &packet_size);
        if (sendto(sockfd, data_packet, packet_size, 0, (struct sockaddr*)client_addr, len) == -1) {
            perror("[sendto]");
            free(data_packet);
            fclose(requested_file); 
            return -1;
        }
        free(data_packet);

        // Receive the ACK packet for the current block_number
        if (recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)client_addr, &len) == -1) {
            perror("[recvfrom]");
            fclose(requested_file); 
            return -1;
        }

        if (get_opcode(buf) != ACK || get_block_number(buf) != block_number - 1) {
            printf("Unexpected ACK received.\n");
            fclose(requested_file); 
            return -1;
        }
    }

    fclose(requested_file); 
    if(DEBUG){
        printf("FILE TRANSFERRED WITH SUCCESS\n");
    }

    return 1;
}
static int process_wrq(char* filename, char* mode, const struct sockaddr_in* client_addr, int sockfd) {
    FILE* received_file = fopen("new.txt", "wb"); // à modifier filename
    if (received_file == NULL) {
        send_error_packet(ERROR, "Cannot open file for writing", client_addr, sockfd);
        return -1;
    }

    // Send initial ACK for WRQ to start receiving data
    size_t ack_packet_size;
    char* ack_packet = build_ack_packet(0, &ack_packet_size); // ACK for block 0
    socklen_t len = sizeof(*client_addr);
    if (sendto(sockfd, ack_packet, ack_packet_size, 0, (const struct sockaddr*)client_addr, len) < 0) {
        perror("Failed to send ACK");
        free(ack_packet);
        fclose(received_file);
        return -1;
    }
    free(ack_packet);

    char buf[516];
    uint16_t block_number = 0;
    ssize_t bytes_received = 516; // Initialize to enter the loop

    // Use while loop instead of do-while
    while (bytes_received == 516) { // Loop as long as a full-sized packet is received
        bytes_received = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)client_addr, &len);
        if (bytes_received < 4) { // Packet too small to be valid
            perror("Received packet too small");
            fclose(received_file);
            return -1;
        }

        if (get_opcode(buf) != DATA) {
            printf("Unexpected opcode. Expected DATA.\n");
            fclose(received_file);
            return -1;
        }

        uint16_t received_block_number = get_block_number(buf);
        if (received_block_number != block_number + 1) {
            printf("Unexpected block number.\n");
            fclose(received_file);
            return -1;
        }

        // Write received data to file
        fwrite(buf + 4, 1, bytes_received - 4, received_file);

        // Send ACK for received block
        ack_packet = build_ack_packet(received_block_number, &ack_packet_size);
        if (sendto(sockfd, ack_packet, ack_packet_size, 0, (const struct sockaddr*)client_addr, len) < 0) {
            perror("Failed to send ACK");
            free(ack_packet);
            fclose(received_file);
            return -1;
        }
        free(ack_packet);
        block_number = received_block_number; // Update block number for next iteration
    }

    fclose(received_file);
    if(DEBUG){
        printf("File received successfully.\n");
    }
    return 0;
}

int main(int argc, char**argv)
{   
    struct sockaddr_in addr,client_addr;
    int sockfd;
    if(init_tftp_server(8080,&sockfd,&addr) == -1){
        printf("[init_udp_socket] : erreur\n");
    }
    while (1)
    {
     handle_client_requests(sockfd,&addr,&client_addr);
    }
    
}

