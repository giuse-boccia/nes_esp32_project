/*
    * sender-receiver.c

    * Lecture: Network Embedded Systems
    * Authors: Giuseppe Boccia, Julio Cesar Espinoza Andrea, Tim Schmid
    *
    * This file contains the code for sending and receiving data via ESP-NOW.
    * The data will be but in a queue and processed by another task
    *
*/
/* ------------- external libs-------------- */
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
/* ------------- external libs-------------- */

/* ------------- own includes -------------- */
#include "config.h"
#include "sender-receiver.h"
/* ------------- own includes -------------- */

/* -------- variables and constants -------- */
QueueHandle_t receiver_queue;
QueueHandle_t sender_queue;
QueueHandle_t sender_error_queue;

const uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const error_tags_t TAGS = {"espnow_receiver", "espnow_sender"};
static uint16_t transmitter_sequence_numbers[2] = {0, 0};

/* -------- variables and constants -------- */

/* ---- definition of internal fucntions --- */
static void sender_error_callback(const uint8_t *mac_addr, esp_now_send_status_t status);
static void receiver_callback(const esp_now_recv_info_t *info, const uint8_t *data, int len);
static void deinit_sender_receiver(void);
/* ---- definition of internal fucntions --- */

/* -------------- functions --------------- */
static void sender_error_callback(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    q_send_error_data_t sender_error_data;

    if (mac_addr == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "MAC address is NULL");
        return;
    }
    memcpy(sender_error_data.mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    sender_error_data.status = status;

    if (xQueueSend(sender_error_queue, &sender_error_data, ESPNOW_QUEUE_TIMEOUT) != pdTRUE)
    {
        ESP_LOGE(TAGS.send_tag, "Queue send error");
    }
}

static void receiver_callback(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    q_receive_data_t receive_data;
    uint8_t *sender_mac = info->src_addr;

    if (sender_mac == NULL || data == NULL || len == 0)
    {
        ESP_LOGE(TAGS.receive_tag, "Received data argument error - nothing received");
        return;
    }

    memcpy(receive_data.mac_addr, sender_mac, ESP_NOW_ETH_ALEN);
    receive_data.data = (uint8_t *)malloc(len);

    if (receive_data.data == NULL)
    {
        ESP_LOGE(TAGS.receive_tag, "Memory allocation error");
        return;
    }

    memcpy(receive_data.data, data, len);
    receive_data.data_len = len;

    if (xQueueSend(receiver_queue, &receive_data, ESPNOW_QUEUE_TIMEOUT) != pdTRUE)
    {
        ESP_LOGE(TAGS.receive_tag, "Queue send error");
        free(receive_data.data);
    }
}

static void encode_data(void)
{
    //
}

static void decode_data(void)
{
    // TODO
}

static void deinit_sender_receiver(void)
{
    // TODO
}

esp_err_t send_broadcast(uint16_t sending_delay_ms, uint8_t *data)
{

    // User-specific dealy bevor sending the next message
    vTaskDelay(sending_delay_ms / portTICK_PERIOD_MS);

    esp_now_data_t data_buffer;
    esp_now_send_param_t send_param;

    data_buffer.transmit_type = TRANSMIT_TYPE_BROADCAST;
    data_buffer.seq_num = transmitter_sequence_numbers[TRANSMIT_TYPE_BROADCAST]++;
    data_buffer.payload = data;
    // TODO --> do a propper CRC calculation with the data
    data_buffer.crc = 0;

    send_param.len = sizeof(esp_now_data_t);
    send_param.buffer = (uint8_t *)&data_buffer;

    memcpy(send_param.destination_mac, broadcast_mac, ESP_NOW_ETH_ALEN);

    return ESP_OK;
}

esp_err_t send_unicast(void)
{
    // TODO --> Add appropriate data-structure to the parameter of the function
    // TODO --> encode the data and send the message
    return ESP_OK;
}

esp_err_t init_sender_receiver(void)
{
    // TODO
    return ESP_OK;
}
