/*
 * vcp.c
 *
 * Lecture: Network Embedded Systems
 * Authors: Giuseppe Boccia, Julio Cesar Espinoza Andrea, Tim Schmid
 *
 * This file contains the code with the functions of the virtucal cord protocol
 */

/* --------------------------------------------------- external libs --------------------------------------------------- */
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

/* -------------------------------------------------- own includes --------------------------------------------------- */
#include "config.h"
#include "sender-receiver.h"
#include "vcp.h"

/* --------------------------------------------- variables and constants --------------------------------------------- */

/* ----------------------------------------------- function definition ----------------------------------------------- */
static void
cord_maintenance(void);
static void check_sending_error(void);
static void greedy_routing(void);
static void vcp_state_machine(void *pvParameters);
static esp_err_t create_hello_message();

static void cord_maintenance(void)
{
    // TODO Manage sending periodic "hello messages" and react to the received ones
    // TODO Update the peers list
}

static void check_sending_error(void)
{
    // TODO check the sender_error_queue and react of the target is not reachable
}

static void greedy_routing(void)
{
    // TODO If data is not for this node, decide where to send it
}

/*
 * ------------------------------------------------------------------
 * This function creates a hello message and pushes it on the
 * sender_queue
 * ------------------------------------------------------------------
 */
static esp_err_t to_sender_queue(esp_now_data_t *esp_now_data)
{

    if (xQueueSend(sender_queue, esp_now_data, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAGS.send_tag, "Could not send hello message");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static esp_err_t create_hello_message()
{

    uint8_t total_length = sizeof(esp_now_data_t) + sizeof(broadcast_mac);
    /* Allocate memory on the heap */
    esp_now_data_t *esp_now_data = (esp_now_data_t *)malloc(total_length);

    if (esp_now_data == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Could not allocate memory for hello message");
        return ESP_FAIL;
    }
    /* Fill the esp_now_data_t struct with the data */
    memset(esp_now_data, 0, total_length);
    esp_now_data->transmit_type = TRANSMIT_TYPE_BROADCAST;
    esp_now_data->payload_length = sizeof(broadcast_mac);
    memcpy(esp_now_data->payload, broadcast_mac, sizeof(broadcast_mac));

    /* Send the pointer to the data-struct to the sender_queue using the to_sender_queue function*/

    return to_sender_queue(esp_now_data);
}

/*
 * ------------------------------------------------------------------
 * This function is the main state machine of the VCP
 * ------------------------------------------------------------------
 */
static void vcp_state_machine(void *pvParameters)
{
    /* On startup - do variable initialization and send an initial hello message through ESP-NOW */

    q_receive_data_t received_data;
    q_send_error_data_t send_error_data;

    /* startup done - start the main loop and react on incoming messages, do the cord maintenance */

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        if (create_hello_message() != ESP_OK)
        {
            ESP_LOGE(TAGS.send_tag, "Could not create hello message");
        }

        if (uxQueueMessagesWaiting(receiver_queue) > 0)
        {
            if (xQueueReceive(receiver_queue, &received_data, portMAX_DELAY) == pdTRUE)
            {
                printf("Received data from: " MACSTR " with length: %d\n", MAC2STR(received_data.mac_addr), received_data.data_len);
            }
        }

        if (uxQueueMessagesWaiting(sender_error_queue) > 0)
        {
            if (xQueueReceive(sender_error_queue, &send_error_data, portMAX_DELAY) != pdTRUE)
            {
                ESP_LOGE(TAGS.receive_tag, "Sending error status: %d\n", send_error_data.status);
            }
        }
    }
}

esp_err_t init_vcp(void)
{

    xTaskCreate(vcp_state_machine, "vcp_state_machine", 4096, NULL, 4, NULL);

    return ESP_OK;
}

void deinit_vcp(void)
{
    // TODO
}
