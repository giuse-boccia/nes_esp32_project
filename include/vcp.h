/*
    * vcp.h

    * Lecture: Network Embedded Systems
    * Authors: Giuseppe Boccia, Julio Cesar Espinoza Andrea, Tim Schmid
    *
    * This file contains the code with the functions of the virtucal cord protocol
    *
*/

#ifndef VCP_H
#define VCP_H

/* --------------------------------------------- variables and constants --------------------------------------------- */

/* 
 * Data structure which stores the virtual and physical positions relevant for routing.
 * The successor and predecessor neighbors are saved as an index from 0 to ESP_NOW_MAX_PEERS - 1. For example the physical
 * address of the successor neighbor can be accessed with neighbors[i_successor].mac_addr.
*/
typedef struct
{
    float own_position;
    int8_t i_successor;     // index of the successor in the neighbors array, -1 if no successor
    int8_t i_predecessor;
    uint8_t neighbors_len;
    vcp_neighbor_data_t neighbors[ESP_NOW_MAX_PEERS];
} vcp_cord_positions_t;

typedef struct
{
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    float position;
    float successor;
    float predecessor;
} vcp_neighbor_data_t;

/* ----------------------------------------------- function definition ----------------------------------------------- */
esp_err_t init_vcp(void);

#endif