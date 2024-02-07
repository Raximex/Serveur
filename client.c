#include "client.h"
#define DEBUG 0
static char transfer_mode[9] = "netascii";
static int sockfd;
static struct sockaddr_in addr;
static void exit_tftp_client(){
    close(sockfd);
    exit(0);
}
static void print_help_information() {
    printf(
        "connect \tconnect to remote tftp server\n"
        "mode    \tset file transfer mode (binary or ascii)\n"
        "put     \tsend file to the server\n"
        "get     \treceive file from the server\n"
        "quit    \texit tftp client\n"
        "verbose \ttoggle verbose mode (detailed output)\n"
        "trace   \ttoggle packet tracing (show packet details)\n"
        "status  \tshow current status (server, mode, etc.)\n"
        "binary  \tset mode to octet (binary file transfer)\n"
        "ascii   \tset mode to netascii (text file transfer)\n"
        "rexmt   \tset per-packet retransmission timeout (in seconds)\n"
        "timeout \tset total retransmission timeout (in seconds)\n"
        "?       \tprint help information\n"
        "\n"
    );
}
static void process_command(const char* command) {
    if (strcmp(command, "put") == 0) {
        handle_put_command();
    } else if (strcmp(command, "get") == 0) {
        handle_get_command();
    } else if (strcmp(command, "?") == 0) {
        print_help();
    } else if (strcmp(command, "quit") == 0) {
        printf("Exiting TFTP client.\n");
        exit(0); // Exit the program
    } else {
        printf("Error: Unrecognized command. Type '?' for help.\n");
    }
}

static void handle_put_command() {
    char filename[100];
    printf("Enter filename to send: ");
    scanf("%99s", filename);
    // Assuming send_file is implemented to handle file transmission
    send_file(filename);
}

static void handle_get_command() {
    char filename[100];
    printf("Enter filename to receive: ");
    scanf("%99s", filename);
    // Assuming receive_file is implemented to handle file reception
    receive_file(filename);
}

static void print_help() {
    printf(
        "Commands available:\n"
        "  put     \tSend file to the server\n"
        "  get     \tReceive file from the server\n"
        "  quit    \tExit TFTP client\n"
        "  ?       \tPrint this help information\n"
    );
}
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

        if (get_opcode(buf) == ACK && get_block_number(buf) == block_number && DEBUG) {
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
    char cwd[100];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/");
    strcat(cwd, filename);

    FILE *requested_file = fopen(cwd, "wb"); // pour test : il faut le remplacer par filename
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
            if(DEBUG){
                printf("[get] : SUCCESS\n");
            }
            break;
        }
    }
    fclose(requested_file);
}







































int main(int argc, char const* argv[]) {
    if (connect_to_tftp_server("127.0.0.1",8080) == -1) {
        printf("[connect_to_tftp_server] : erreur");
    }
   char command[100];

   printf("tftp> ");
   if (scanf("%99s", command) == EOF)
   {
       return 1; // Exit loop if end of input is reached
   }
   process_command(command);
}
