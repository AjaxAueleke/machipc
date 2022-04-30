#ifndef CONVERSIONS_HPP
#define CONVERSIONS_HPP

#include <netinet/in.h>

/**
 * @brief presentation to numeric : converts a string representation of ipv4 address into a network byte value
 * @param ipv4_addr the string representing IPv4 address
 * @return the converted numeric value on success
 * @throw returns -1 and error msg on error
 */
in_addr_t pton(const char *ipv4_addr);
/**
 * @brief numeric to presentation : converts a numeric network address into a string ipv4-address
 * @param numeric_addr the numeric address value
 * @param buffer pointer to buffer - the destination string
 * @return pointer to the same buffer on success
 * @throw returns NULL and error msg on error
 */
const char *ntop(in_addr_t numeric_addr, char *buffer);

#endif