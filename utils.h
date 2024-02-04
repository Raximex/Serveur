#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h> 
#define PORT 999999
#define MAX_BLOCK_SIZE 516
//Remarques : htons ? 

/**
 * important informations about packets :
 * 
 * RRQ / WWRQ packets : has a NON fixed size :
 *          
 *          2 bytes     string    1 byte     string   1 byte
 *           ------------------------------------------------
 *          | Opcode |  Filename  |   0  |    Mode    |   0  |
 *           ------------------------------------------------
 * 
 * Data packets : has maximum of 516 bytes : 
 *              - if the Data size is < 512     ->  signals the ebd of the transfer 
 *              - block numbers on data packets begins with 1 and increases by 1     
 *                  
 *                  2 bytes     2 bytes      n bytes
 *                  ----------------------------------
 *                | Opcode |   Block #  |   Data     |
 *                ----------------------------------
 *  
 * 
 * Ack packets : 
 *               - block number -> the block that is acknowledged
 *               - WRQ is acknowledged with block number = 0
 * 
 *                          2 bytes     2 bytes
 *                        ---------------------
 *                       | Opcode |   Block #  |
 *                        ---------------------
 * 
 * 
 * Error packets : 
 *              - can acknowledge all other packets.
 *              - error message is in netascii
 *              
 *                  2 bytes     2 bytes      string     1 byte
 *               ----------------------------------------
 *               | opcode    |  ErrorCode |   ErrMsg   |   0  |
 *               ---------------------------------------- 
 * 
 * 
 * Error Codes

   Value     Meaning

   0         Not defined, see error message (if any).
   1         File not found.
   2         Access violation.
   3         Disk full or allocation exceeded.
   4         Illegal TFTP operation.
   5         Unknown transfer ID.
   6         File already exists.
   7         No such user.
*/





/**
 * @enum Operation codes for TFTP operations.
 */
enum opcode{
    RRQ = 1,
    WRQ,
    DATA,
    ACK,
    ERROR
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @enum Codes of errors in the ERROR packet.
 */
enum error {
    NOT_DEFINED,
    FILE_NOT_FOUND,
    ACCESS_VIOLATION,
    MEMORY,
    ILLEGAL_OPERATION,
    UKNOWN_ID,
    FILE_ALREADY_EXISTS,
    NO_SUCH_USER
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initializes a UDP server socket.
 * 
 * This function creates a UDP socket and binds it to the specified address and port.
 * 
 * @param addr Pointer to struct sockaddr_in where server address and port are specified.
 * @param sockfd Pointer to an integer where the socket file descriptor will be stored.
 * @return Returns 0 on success, -1 on failure with an error message printed to stderr.
 */
int init_udp_client(struct sockaddr_in*,int* sockfd);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initializes a UDP client socket.
 * 
 * This function creates a UDP socket for the client. The socket is not bound to a specific address or port.
 * 
 * @param addr Pointer to struct sockaddr_in where client address and port are specified. Not used in this function but included for symmetry with server initialization.
 * @param sockfd Pointer to an integer where the socket file descriptor will be stored.
 * @return Returns 0 on success, -1 on failure with an error message printed to stderr.
 */
int init_udp_server(struct sockaddr_in*,int* sockfd);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Sends a request packet over UDP.
 * 
 * Constructs and sends a TFTP request packet to the server specified by addr.
 * 
 * @param type The type of request ( RRQ or WRQ )
 * @param filename The name of the file to read.
 * @param mode The transfer mode ("netascii", "octet", or "mail").
 * @param sockfd The socket file descriptor used to send the packet.
 * @param addr The address of the server to which the request is sent.
 * @return Returns 1 on success, -1 on failure with an error message printed to stderr.
 */
int request(uint16_t opcode,const char* filename, const char* mode, int sockfd, struct sockaddr* addr);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Extracts the opcode from a TFTP packet.
 * 
 * @param packet The TFTP packet from which to extract the opcode.
 * @return The opcode as an integer.
 */
uint16_t get_opcode(char* packet);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Extracts the file name from a TFTP RRQ or WRQ packet.
 * 
 * @param packet The TFTP packet from which to extract the file name.
 * @return A pointer to a dynamically allocated string containing the file name. Caller must free this string.
 */
char* get_file_name(char* packet);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Extracts the transfer mode from a TFTP RRQ or WRQ packet.
 * 
 * @param packet The TFTP packet from which to extract the mode.
 * @return A pointer to a dynamically allocated string containing the mode. Caller must free this string.
 */
char* get_mode(char* packet);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Prints the request in the form of : opcode
 *                                            filename
 *                                            mode
 * 
 * @param packet The TFTP packet from which to extract the informations.
 * @return Nothing
 */
void print_request_packet(char* packet);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Extract error code from a TFTP ERROR packet.
 * 
 * @param packet The TFTP packet from which to extract the error code.
 * @return The error code as integer.
 */
uint16_t get_error_code(char* packet);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Extracts the error message from a TFTP ERROR packet.
 * 
 * @param packet The TFTP packet from which to extract the error code.
 * @return A pointer to a dynamically allocated string containing the error message. Caller must free this string.
 */
char* get_error_message(char* packet);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Builds an error packet
 * 
 * @param error_code the error code.
 * @param error_msg the error message.
 * @return A pointer to a dynamically allocated string containing the error packet. Caller must free this string.
 */
void print_error_packet(char* packet);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Prints the error packet in the form of : opcode
 *                                                 error code
 *                                                 error message
 * 
 * @param packet The TFTP packet from which to extract the informations.
 * @return Nothing
 */
char* build_error_packet(uint16_t error_code, char* error_msg,size_t* packet_size);

//TODO
int handle_request(char* packet, struct sockaddr_in* client_addr,int sockfd);

//TODO : gérer les modes netascii , octet et email
int handle_rrq(char* packet,struct sockaddr_in* client_addr,int sockfd);























#endif