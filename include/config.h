/*
 * config.h
 *
 * Lecture: Network Embedded Systems
 * Authors: Giuseppe Boccia, Julio Cesar Espinoza Andrea, Tim Schmid
 *
 * This file contains the necessary configuration for the project
 */

#ifndef CONFIG_H
#define CONFIG_H

 /* --------------------------------------------- variables and constants --------------------------------------------- */

#define ESPNOW_QUEUE_TIMEOUT 512

#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF ESP_IF_WIFI_AP
#define ESPNOW_WIFI_CHANNEL 1

#define SENDER_QUEUE_SIZE 10
#define RECEIVER_QUEUE_SIZE 5
#define SENDER_ERROR_QUEUE_SIZE 5
#define ESP_NOW_MAX_PEERS 20

#define ESPNOW_SENDING_DELAY_MS 100
#define VCP_TASK_DELAY_MS 1000

#define ESP_NOW_PMK "pmk1234567890123"
#define ESP_NOW_LMK "lmk1234567890123"

#define PHYSICAL_NEIGHBOURS ;

/* Message types used in vcp_message_data_t */
#define VCP_HELLO 0x00
#define VCP_UPDATE_SUCCESSOR 0x01
#define VCP_UPDATE_PREDECESSOR 0x02
#define VCP_CREATE_VIRTUAL_NODE 0x03
#define VCP_DATA 0x04
#define VCP_ERR 0x05


/*
 * MESSAGE TYPES
 * - HELLO (0x00) + float + float + float  ----> 13 bytes
 * - UPDATE_SUCCESSOR (0x01) + float       ----> 5 bytes
 * - UPDATE_PREDECESSOR (0x02) + float     ----> 5 bytes
 * - DATA (0x04) + char[]                  ----> ? bytes
 * 

*/

/* VCP parameters */
#define VCP_START 0.0
#define VCP_END 1.0
#define VCP_INITIAL -1.0
#define VCP_INTERVAL 0.1
#define VCP_VIRT_INTERVAL 0.9
#define VCP_DISCOVERY_CYCLES 3     // number of "cycles" before joining the cord


typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    uint8_t *data;
    uint8_t data_len;
    int8_t rssi;
} q_receive_data_t;

typedef struct {
    uint8_t msg_type;   // es: VCP_HELLO, VCP_DATA
    void *args;         // args can be of any type and dimension, depending on msg_type
} vcp_message_data_t;

typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
} q_send_error_data_t;

typedef struct {
    uint8_t transmit_type; // 0: unicast, 1: broadcast
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];         // addr of sender OR receiver (depending on which queue struct is in)
    uint8_t payload_length; // length of the data
    vcp_message_data_t *payload;     // data
} esp_now_data_t;

enum {
    TRANSMIT_TYPE_UNICAST = 0,
    TRANSMIT_TYPE_BROADCAST = 1,
};

typedef struct {
    int len;                                   // length of the ESP_NOW vendor specific content
    uint8_t *buffer;                           // pointer to the buffer of ESP_NOW vendor specific content
    uint8_t destination_mac[ESP_NOW_ETH_ALEN]; // destination MAC address
} esp_now_send_param_t;

typedef struct {
    char *receive_tag;
    char *send_tag;
} error_tags_t;

#endif
