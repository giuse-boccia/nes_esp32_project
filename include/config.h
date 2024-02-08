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

#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF ESP_IF_WIFI_STA
#define ESPNOW_WIFI_CHANNEL 1

#define SENDER_QUEUE_SIZE 10
#define RECEIVER_QUEUE_SIZE 5
#define SENDER_ERROR_QUEUE_SIZE 5

#define ESPNOW_SENDING_DELAY_MS 10

#define ESPNOW_PMK "pmk1234567890123"
#define ESPNOW_LMK "lmk1234567890123"

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
    uint8_t transmit_type; // 0: unicast, 1: multicast
    uint16_t seq_num;      // sequence number
    uint16_t crc;          // crc16
    uint8_t *payload;      // data
} esp_now_data_t;

enum
{
    TRANSMIT_TYPE_UNICAST = 0,
    TRANSMIT_TYPE_BROADCAST = 1,
};

typedef struct
{
    int len;                                   // length of the ESP_NOW vendor specific content
    uint8_t *buffer;                           // pointer to the buffer of ESP_NOW vendor specific content
    uint8_t destination_mac[ESP_NOW_ETH_ALEN]; // destination MAC address
} esp_now_send_param_t;

typedef struct
{
    char *receive_tag;
    char *send_tag;
} error_tags_t;

#endif