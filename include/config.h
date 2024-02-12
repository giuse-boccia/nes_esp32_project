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
#define ESPNOW_MAX_PEERS 20

#define SENDER_TASK_DELAY_MS 10
#define VCP_TASK_DELAY_MS 100

#define ESPNOW_PMK "pmk1234567890123"
#define ESPNOW_LMK "lmk1234567890123"

/*
 * MESSAGE TYPES used in vcp_message_data_t
 * - HELLO (0x00) + float + float + float                   ----> 13 bytes
 * - UPDATE_SUCCESSOR (0x01) + float                        ----> 5 bytes
 * - UPDATE_PREDECESSOR (0x02) + float                      ----> 5 bytes
 * - DATA (0x04) + float(receiver) + char[]                 ----> ? bytes (at least 5)
 */
#define VCP_HELLO 0x00
#define VCP_UPDATE_SUCCESSOR 0x01
#define VCP_UPDATE_PREDECESSOR 0x02
#define VCP_CREATE_VIRTUAL_NODE 0x03
#define VCP_DATA 0x04
#define VCP_ERR 0x05
#define VCP_ACK 0x06

/* VCP parameters */
#define VCP_START 0.0
#define VCP_END 1.0
#define VCP_INITIAL -1.0
#define VCP_INTERVAL 0.1
#define VCP_VIRT_INTERVAL 0.9
#define VCP_DISCOVERY_CYCLES 3 // number of "cycles" before joining the cord
#define VCP_HELLO_MESSAGE_PERIOD (10 * VCP_TASK_DELAY_MS)

typedef struct
{
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    uint8_t *data;
    uint8_t data_len;
    int8_t rssi;
} q_receive_data_t;

typedef struct
{
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
} q_send_error_data_t;

typedef struct
{
    uint8_t type; // es: VCP_HELLO, VCP_DATA
    void *args;   // args can be of any type and dimension, depending on type
} vcp_message_data_t;

typedef struct
{
    uint8_t transmit_type;              // 0: unicast, 1: broadcast
    uint8_t mac_addr[ESP_NOW_ETH_ALEN]; // addr of sender OR receiver (depending on which queue struct is in)
    uint8_t payload_length;             // length of the data
    vcp_message_data_t *payload;        // data
} esp_now_data_t;

typedef struct
{
    char *receive_tag;
    char *send_tag;
} error_tags_t;

enum
{
    TRANSMIT_TYPE_UNICAST = 0,
    TRANSMIT_TYPE_BROADCAST = 1,
};

#endif
