/*
    * sender-receiver.c

    * Lecture: Network Embedded Systems
    * Authors: Giuseppe Boccia, Julio Cesar Espinoza Andrea, Tim Schmid
    *
    * This file contains the code for sending and receiving data via ESP-NOW.
    * The data will be but in a queue and processed by another task
    *
*/
/* --------------------------------------------------- external libs --------------------------------------------------- */
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
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

/* -------------------------------------------------- own includes --------------------------------------------------- */
#include "config.h"
#include "sender-receiver.h"

/* --------------------------------------------- variables and constants --------------------------------------------- */
QueueHandle_t receiver_queue;
QueueHandle_t sender_queue;
QueueHandle_t sender_error_queue;

const uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const error_tags_t TAGS = {"espnow_receiver", "espnow_sender"};
static uint16_t transmitter_sequence_numbers[2] = {0, 0};

/* ----------------------------------------------- function definition ----------------------------------------------- */
static void sender_error_callback(const uint8_t *mac_addr, esp_now_send_status_t status);
static void receiver_callback(const esp_now_recv_info_t *info, const uint8_t *data, int len);
static esp_err_t encode_data(esp_now_send_param_t *esp_now_param, esp_now_data_t *esp_now_data);
static esp_err_t send_data(esp_now_data_t *esp_now_data);

/*
 * ------------------------------------------------------------------
 * Callback function which is called everytime data is sent via
 * ESP-NOW, the function puts the data into a queue for further
 * processing
 * ------------------------------------------------------------------
 */
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

/*
 * ------------------------------------------------------------------
 * Callback function for receiving data via ESP-NOW, the data is
 * put into a queue for further processing
 * ------------------------------------------------------------------
 */
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
    receive_data.rssi = info->rx_ctrl->rssi;

    if (xQueueSend(receiver_queue, &receive_data, ESPNOW_QUEUE_TIMEOUT) != pdTRUE)
    {
        ESP_LOGE(TAGS.receive_tag, "Receiving data queue error");
        free(receive_data.data);
    }
}

/*
 * ------------------------------------------------------------------
 * Function which decodes the data sent over ESP-NOW and checks if
 * the data is valid using CRC algorithm
 * ------------------------------------------------------------------
 */
// esp_now_data_t parse_data(q_receive_data_t *received_message)
// {
//     //
// }

void print_esp_now_data_t(esp_now_data_t *data)
{
    printf("\n-----------------------------------------------\n");
    printf("Transmit Type: %d\n", data->transmit_type);
    printf("Destination MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
           data->destination_mac[0], data->destination_mac[1], data->destination_mac[2],
           data->destination_mac[3], data->destination_mac[4], data->destination_mac[5]);
    printf("Sequence Number: %d\n", data->seq_num);
    printf("CRC: %d\n", data->crc);
    printf("Payload Length: %d\n", data->payload_length);
    // Print the payload as a string
    printf("Payload: ");
    for (int i = 0; i < data->payload_length; i++)
    {
        printf("0x%x", data->payload[i]);
    }
    printf("\n-----------------------------------------------\n");
}

/*
 * ------------------------------------------------------------------
 * Function which configures the vendor-specific content, the payload
 * and the meta-data
 *
 * TODO:
 * - Add a proper CRC calculation
 * - Add and adapt relevant data-structure need for VCP
 * ------------------------------------------------------------------
 */
static esp_err_t encode_data(esp_now_send_param_t *esp_now_param, esp_now_data_t *esp_now_data)
{
    esp_now_param->len = sizeof(esp_now_data_t) + esp_now_data->payload_length;
    esp_now_param->buffer = (uint8_t *)malloc(esp_now_param->len);

    if (esp_now_param->buffer == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Memory allocation error");
        return ESP_FAIL;
    }

    esp_now_data->seq_num = transmitter_sequence_numbers[esp_now_data->transmit_type]++;
    esp_now_data->crc = 1234;

    memset(esp_now_param->buffer, 0, esp_now_param->len);
    memcpy(esp_now_param->buffer, esp_now_data, esp_now_param->len);

    // TODO --> do a propper CRC calculation with the dat

    if (esp_now_data->transmit_type == TRANSMIT_TYPE_BROADCAST)
    {
        memcpy(esp_now_param->destination_mac, broadcast_mac, ESP_NOW_ETH_ALEN);
    }
    else if (esp_now_data->transmit_type == TRANSMIT_TYPE_UNICAST)
    {
        memcpy(esp_now_param->destination_mac, esp_now_data->destination_mac, ESP_NOW_ETH_ALEN);
    }

    return ESP_OK;
}

static esp_err_t send_data(esp_now_data_t *esp_now_data)
{
    esp_now_send_param_t *esp_now_param = malloc(sizeof(esp_now_send_param_t));
    memset(esp_now_param, 0, sizeof(esp_now_send_param_t));

    if (encode_data(esp_now_param, esp_now_data) != ESP_OK)
    {
        ESP_LOGE(TAGS.send_tag, "Error encoding data");
        free(esp_now_param);
        free(esp_now_data);
        return ESP_FAIL;
    }

    uint8_t *own_mac = (uint8_t *)malloc(ESP_NOW_ETH_ALEN);
    esp_wifi_get_mac(ESPNOW_WIFI_IF, own_mac);

    printf("Sending data from: " MACSTR " with length: %d\n", MAC2STR(own_mac), esp_now_param->len);

    if (esp_now_send(esp_now_param->destination_mac, esp_now_param->buffer, esp_now_param->len) != ESP_OK)
    {
        ESP_LOGE(TAGS.send_tag, "Error sending message");
        free(esp_now_param);
        free(esp_now_data);
        return ESP_FAIL;
    }

    return ESP_OK;
}

/*
 * ------------------------------------------------------------------
 * Task to send data via ESP-NOW
 * Grabs data from the sender_queue and sends it via ESP-NOW
 * ------------------------------------------------------------------
 */
static void send_data_task(void *pvParameters)
{

    esp_now_data_t esp_now_data;

    while (1)
    {
        // User-specific dealy bevor sending the next message
        vTaskDelay(100 / portTICK_PERIOD_MS);

        if (uxQueueMessagesWaiting(sender_queue) > 0)
        {
            if (xQueueReceive(sender_queue, &esp_now_data, portMAX_DELAY) == pdPASS)
            {

                if (send_data(&esp_now_data) != ESP_OK)
                {
                    ESP_LOGE(TAGS.send_tag, "Error sending data");
                }
            }
        }
    }
}

esp_err_t init_sender_receiver(void)
{

    /*
     * ------------------------------------------------------------------
     * setup of all required queues for async communication between tasks
     * ------------------------------------------------------------------
     */

    receiver_queue = xQueueCreate(RECEIVER_QUEUE_SIZE, sizeof(q_receive_data_t));

    if (receiver_queue == NULL)
    {
        ESP_LOGE(TAGS.receive_tag, "Error creating receiver queue");
        return ESP_FAIL;
    }

    sender_queue = xQueueCreate(SENDER_QUEUE_SIZE, sizeof(esp_now_data_t));

    if (sender_queue == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Error creating sender queue");
        return ESP_FAIL;
    }

    sender_error_queue = xQueueCreate(SENDER_ERROR_QUEUE_SIZE, sizeof(q_send_error_data_t));

    if (sender_error_queue == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Error creating sender error queue");
        return ESP_FAIL;
    }

    /*
     * ------------------------------------------------------------------
     * Register all required callbacks and initialize the ESP-NOW module
     * ------------------------------------------------------------------
     */

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(sender_error_callback));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(receiver_callback));
    ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)ESP_NOW_PMK));

    /*
     * ------------------------------------------------------------------
     * Register the broadcast MAC address as a peer and start the sender
     * task
     * ------------------------------------------------------------------
     */

    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));

    if (peer == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Memory allocation error");
        deinit_sender_receiver();
        return ESP_FAIL;
    }

    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = ESPNOW_WIFI_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);

    ESP_ERROR_CHECK(esp_now_add_peer(peer));
    free(peer);

    xTaskCreate(send_data_task, "send_data_task", 2048, NULL, 5, NULL);

    return ESP_OK;
}

void deinit_sender_receiver(void)
{
    vSemaphoreDelete(sender_queue);
    vSemaphoreDelete(sender_error_queue);
    vSemaphoreDelete(receiver_queue);
    esp_now_deinit();
}