#include "utils.h"
int request(uint16_t opcode, const char* filename, const char* mode, int sockfd, struct sockaddr* addr) {
    // l'octet à 0
    u_int8_t end_of_file = 0;
    int offset = 0;
    // Allocation mémoire
    char* packet = malloc(strlen(filename) + strlen(mode) + 4 + 2);
    if (packet == NULL) {
        perror("malloc");
        return -1;
    }

    // Convert to network byte
    uint16_t net_opcode = htons(opcode);
    memcpy(packet + offset, &net_opcode, sizeof(net_opcode));
    offset += sizeof(net_opcode);

    // Copy the filename and mode
    memcpy(packet + offset, filename, strlen(filename));
    offset += strlen(filename);
    //zero byte
    packet[offset++] = end_of_file;

    memcpy(packet + offset, mode, strlen(mode));
    offset += strlen(mode);
    //zero byte
    packet[offset++] = end_of_file; //Increment offset so we can get the total size of the packet

    // Send the packet
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
uint16_t get_block_number(char* packet){
    uint16_t block_number;
    memcpy(&block_number, packet + 2, sizeof(block_number)); // Copie correctement les 2 octets
    return ntohs(block_number); // Conversion nécessaire
}
char* get_data(char* packet){
    char* data = strdup(packet+4);
    return data;

}
void print_data_packet(char* packet){
    uint16_t opcode = get_opcode(packet);
    uint16_t block_number = get_block_number(packet);
    char* data = get_data(packet);
    printf("%u\n",opcode);
    printf("%u\n",block_number);
    printf("%s\n",data);
    free(data);
}
size_t convert_to_netascii(char* buffer) {
    size_t i, j;
    size_t input_length = strlen(buffer); 
    for (i = 0, j = 0; i < input_length; ++i, ++j) {
        if (buffer[i] == '\n') {
            buffer[j++] = '\r'; 
        }
        buffer[j] = buffer[i];
    }
    buffer[j] = '\0'; 
    return j; 
}
int send_ack(int sockfd, const struct sockaddr* dest_addr, socklen_t addrlen, uint16_t block_number) {
    size_t packet_size;
    char* ack_packet = build_ack_packet(block_number, &packet_size);

    if (!ack_packet) {
        return -1; // Failed to build ACK packet
    }

    size_t bytes_sent = sendto(sockfd, ack_packet, packet_size, 0, dest_addr, addrlen);
    free(ack_packet); // Free the dynamically allocated ACK packet

    if (bytes_sent < 0) {
        perror("Failed to send ACK packet");
        return -1;
    }

    return 0; // ACK packet sent successfully
}
void convert_netascii_to_native(char *data, int *length) {
    if(strcmp(PLATFORM_NAME, "windows") == 0) {
        return; // No conversion needed for Windows, as it already uses CR LF.
    }
    int i, j;
    // Convert CR LF to LF for Linux/Unix
    for (i = 0, j = 0; i < *length; ++i, ++j) {
        if (data[i] == '\r' && data[i + 1] == '\n') {
            i++; // Skip CR in CR LF
        }
        data[j] = data[i];
    }
    *length = j; // Update the length after conversion
    data[j] = '\0'; // Ensure the result is null-terminated
}
char* build_error_packet(uint16_t error_code, char* error_msg,size_t* packet_size) {
    //The zero byte
    u_int8_t end_of_error_msg = 0;
    //dynamically allocate the string
    char* error_packet = malloc(strlen(error_msg) + 5 + 1);
    if (error_packet == NULL) {
        perror("malloc");
        return NULL;
    }
    int offset = 0;
    // Conversion en network byte order
    uint16_t net_opcode = htons(ERROR); 
    uint16_t net_error_code = htons(error_code); 
    
    // copy of opcode
    memcpy(error_packet + offset, &net_opcode, sizeof(net_opcode));
    offset += sizeof(net_opcode);

    // copy of error code
    memcpy(error_packet + offset, &net_error_code, sizeof(net_error_code));
    offset += sizeof(net_error_code);

    // copy of error message 
    memcpy(error_packet + offset, error_msg, strlen(error_msg));
    offset += strlen(error_msg); 

    // adding the zero byte
    error_packet[offset++] = end_of_error_msg; //incrementing offset to get the real size
    *packet_size = offset; //assign the size
    return error_packet;
}
char* build_data_packet(uint16_t block_number, const char* data, size_t data_length, size_t* packet_size) {
    *packet_size = 4 + data_length; // Opcode (2 bytes) + Block number (2 bytes) + Data length
    char* packet = malloc(*packet_size);
    if (packet == NULL) {
        perror("malloc");
        return NULL;
    }

    uint16_t net_opcode = htons(DATA);
    uint16_t net_block_number = htons(block_number);

    int offset = 0;
    memcpy(packet + offset, &net_opcode, sizeof(net_opcode));
    offset += sizeof(net_opcode);

    memcpy(packet + offset, &net_block_number, sizeof(net_block_number));
    offset += sizeof(net_block_number);

    memcpy(packet + offset, data, data_length); // No need to adjust offset after this, as we're done

    return packet;
}
char* build_ack_packet(uint16_t block_number, size_t* packet_size) {
    *packet_size = 4; // Opcode (2 bytes) + Block number (2 bytes)
    char* packet = malloc(*packet_size);
    if (packet == NULL) {
        perror("malloc");
        return NULL;
    }

    uint16_t net_opcode = htons(ACK);
    uint16_t net_block_number = htons(block_number);

    int offset = 0;
    memcpy(packet + offset, &net_opcode, sizeof(net_opcode));
    offset += sizeof(net_opcode);

    memcpy(packet + offset, &net_block_number, sizeof(net_block_number));
    // No need to adjust offset after this, as we're done

    return packet;
}
