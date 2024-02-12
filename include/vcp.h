/*
    * vcp.h

    * Lecture: Network Embedded Systems
    * Authors: Giuseppe Boccia, Julio Cesar Espinoza Andrea, Tim Schmid
    *
    * This file contains the code with the functions of the virtual cord protocol
    *
*/

#ifndef VCP_H
#define VCP_H

/* --------------------------------------------- variables and constants --------------------------------------------- */

/*
 * Data structure which stores the virtual and physical positions relevant for routing.
 * The successor and predecessor neighbors are saved as an index from 0 to ESPNOW_MAX_PEERS - 1. For example the physical
 * address of the successor neighbor can be accessed with neighbors[i_successor].mac_addr.
 */
typedef struct
{
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    float position;
    float successor;
    float predecessor;
} vcp_neighbor_data_t;

/* ----------------------------------------------- function definition ----------------------------------------------- */
void init_vcp(void);

#endif