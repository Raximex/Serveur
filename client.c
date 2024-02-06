#include "utils.h"
#include "client.h"
static char transfer_mode[9] = "netascii";
static int sockfd;
static struct sockaddr_in addr;

static int connect_to_tftp_server(const char* server_ip, int server_port) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[socket]");
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port); // Specify the server's port
    addr.sin_addr.s_addr = inet_addr(server_ip); // Specify the server's IP address

    return 0;
}
static void set_file_transfer_mode(const char* mode){
        strcpy(transfer_mode,mode);
}
static void send_file(const char* filename) {
    FILE* requested_file = fopen(filename, "rb");
    if (requested_file == NULL) {
        perror("Failed to open file");
        return; 
    }

    request(WRQ, filename, transfer_mode, sockfd, (struct sockaddr*)&addr);

    char buf[516];
    size_t packet_size, bytes_read;
    socklen_t len = sizeof(addr);
    char* data_packet;
    int block_number = 0;

    // Wait for the initial ACK for the WRQ
    size_t bytes_received = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);
    if (bytes_received == -1) {
        perror("[recvfrom]");
        fclose(requested_file);
        return;
    }

    if (get_opcode(buf) != ACK || get_block_number(buf) != 0) {
        printf("Unexpected response or error after WRQ\n");
        fclose(requested_file);
        return;
    }

    while ((bytes_read = fread(buf, 1, 512, requested_file)) > 0) {
        block_number++;
        data_packet = build_data_packet(block_number, buf, bytes_read, &packet_size);
        if (sendto(sockfd, data_packet, packet_size, 0, (struct sockaddr*)&addr, len) == -1) {
            perror("[sendto]");
            free(data_packet); // Free the allocated memory
            fclose(requested_file); // Close the file
            return;
        }
        free(data_packet); // Free the allocated memory after sending

        // Receive the ACK packet
        bytes_received = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);
        if (bytes_received == -1) {
            perror("[recvfrom]");
            break;
        }

        if (get_opcode(buf) == ACK && get_block_number(buf) == block_number) {
            printf("Received ACK for block %d\n", block_number);
        } else if (get_opcode(buf) == ERROR) {
            printf("[ERROR] Code = %d : Message = %s\n", get_error_code(buf), get_error_message(buf));
            break; // Exit the loop on error
        }
    }

    fclose(requested_file); // Close the file after sending all data
}
void receive_file(const char* filename) {
    request(RRQ, filename, transfer_mode, sockfd, (struct sockaddr*)&addr);
    char buf[516];
    size_t packet_size;
    FILE* requested_file = fopen("new.txt", "wb");//pour test : il faut le remplacer par filename
    if (!requested_file) {
        perror("Failed to open file");
        return;
    }
    socklen_t len = sizeof(addr);
    int block_number = 1;

    while (1) {
        // Receive a data packet
        size_t bytes_received = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);
        if (bytes_received == -1) {
            perror("[recvfrom]");
            break;
        }
        if(get_opcode(buf) == ERROR){
            printf("[get] : error code = %d : error message : %s\n",get_error_code(buf),get_error_message(buf));
            if(remove(filename)){
                perror("[remove]");
            }
            break;
        }
        fwrite(get_data(buf), 1, bytes_received - 4, requested_file); // 
        send_ack(sockfd,(struct sockaddr*)&addr,len,block_number);

        block_number++;
        // Check if this is the last data block
        if (bytes_received < 516) {
            printf("[get] : SUCCESS\n");
            break;
        }
    }
    fclose(requested_file);
}







































int main(int argc, char const* argv[]) {
    if (connect_to_tftp_server("127.0.0.1",8080) == -1) {
        printf("[connect_to_tftp_server] : erreur");
    }
    receive_file("dir/file.txt");
    
    close(sockfd);
    return 0;
}
