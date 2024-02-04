#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#define PORT 999999
#define MAX_BLOCK_SIZE 516
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
 * Ack packets
 * 
*/





/**
 * @enum Operation codes for TFTP operations.
 */
enum{
    RRQ=1,
    WRQ,
    DATA,
    ACK,
    ERROR
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
int request(int type,const char* filename, const char* mode, int sockfd, struct sockaddr* addr);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Extracts the opcode from a TFTP packet.
 * 
 * @param packet The TFTP packet from which to extract the opcode.
 * @return The opcode as an integer.
 */
int get_opcode(char packet[512]);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Extracts the file name from a TFTP RRQ or WRQ packet.
 * 
 * @param packet The TFTP packet from which to extract the file name.
 * @return A pointer to a dynamically allocated string containing the file name. Caller must free this string.
 */
char* get_file_name(char packet[512]);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Extracts the transfer mode from a TFTP RRQ or WRQ packet.
 * 
 * @param packet The TFTP packet from which to extract the mode.
 * @return A pointer to a dynamically allocated string containing the mode. Caller must free this string.
 */
char* get_mode(char packet[512]);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Prints the request in the form of : opcode
 *                                            filename
 *                                            mode
 * 
 * @param packet The TFTP packet from which to extract the informations.
 * @return Nothing
 */
void print_request(char* packet);























#endif