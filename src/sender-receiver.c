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
#include "sender-receiver.h"
/* ------------- own includes -------------- */

/* -------- variables and constants -------- */
QueueHandle_t receiver_queue;
QueueHandle_t sender_queue;
QueueHandle_t sender_error_queue;
/* -------- variables and constants -------- */

/* ---- definition of internal fucntions --- */
static void sender_callback(const uint8_t *mac_addr, esp_now_send_status_t status);
static void receiver_callback(const esp_now_recv_info_t *info, const uint8_t *data, int len);
static esp_err_t encode_data(esp_now_send_param_t *esp_now_param, esp_now_data_t *esp_now_data, uint8_t *payload, uint8_t transmit_type);
/* ---- definition of internal fucntions --- */

/* -------------- functions --------------- */
static void sender_callback(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    // TODO
}

static void receiver_callback(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    // TODO decode the received data and write on the queue
}

esp_err_t parse_data(void)
{
    // TODO
    return ESP_OK;
}

/* This functions is used to configure the vendor-specific content which contains the payload and some extra meta-data
 * TODO:    Add a proper CRC calculation
 *          Add and adapt relevant data-structure need for VCP
 */
static esp_err_t encode_data(esp_now_send_param_t *esp_now_param, esp_now_data_t *esp_now_data, uint8_t *payload, uint8_t transmit_type)
{
    esp_now_data->transmit_type = transmit_type;
    esp_now_data->seq_num = transmitter_sequence_numbers[transmit_type]++;
    esp_now_data->payload = payload;

<<<<<<< HEAD
    // TODO --> do a propper CRC calculation with the data
    esp_now_data->crc = 0;

    esp_now_param->len = sizeof(esp_now_data_t);
    esp_now_param->buffer = (uint8_t *)esp_now_data;
    memcpy(esp_now_param->destination_mac, broadcast_mac, ESP_NOW_ETH_ALEN);

=======
esp_err_t send_broadcast(void)
{
    // TODO --> Add appropriate data-structure to the parameter of the function
    // TODO --> encode the data and send the message
>>>>>>> parent of 18401d9 (AD callback implemtation)
    return ESP_OK;
}

esp_err_t send_data(uint16_t sending_delay_ms, uint8_t *payload, uint8_t transmit_type, esp_now_send_param_t *esp_now_param)
{
    // User-specific dealy bevor sending the next message
    vTaskDelay(sending_delay_ms / portTICK_PERIOD_MS);

    esp_now_data_t esp_now_data;

    if (encode_data(esp_now_param, esp_now_data, payload, transmit_type) != ESP_OK)
    {
        ESP_LOGE(TAGS.send_tag, "Error encoding data");
        return ESP_FAIL;
    }

    if (esp_now_send(esp_now_param->destination_mac, esp_now_param->buffer, esp_now_param->len) != ESP_OK)
    {
        ESP_LOGE(TAGS.send_tag, "Error sending message");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t init_sender_receiver(void)
{
    // TODO
    return ESP_OK;
<<<<<<< HEAD
}

esp_err_t deinit_sender_receiver(void)
{
    // TODO
    return ESP_OK;
=======
>>>>>>> parent of 18401d9 (AD callback implemtation)
}