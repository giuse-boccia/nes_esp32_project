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

/* data structure which stores the virtual and physical positions relevante for routing
 * The physical neighbors array is sized to 20, which is the maximum amount of peers that can be comupted by the ESP-NOW
 * protocol
 */

typedef struct
{
    float lower_neighbor_cord_position;
    float upper_neighbor_cord_position;
    float own_cord_position;
    float physical_neighbors[20];
} vcp_cord_positions_t;

/* ----------------------------------------------- function definition ----------------------------------------------- */
esp_err_t init_vcp(void);

#endif