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
static vcp_cord_positions_t cord_positions = {0, 0, 0};

/* ----------------------------------------------- function definition ----------------------------------------------- */
static void cord_maintenance(void);
static void check_sending_error(void);
static void greedy_routing(void);
static void vcp_state_machine(void);

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
static esp_err_t create_hello_message(esp_now_send_param_t *esp_now_param)
{
    esp_now_param = malloc(sizeof(esp_now_send_param_t));

    // This is just an example, you can change the payload to your needs
    uint8_t hello_message_payload[3] = {0x01, 0x02, 0x03};
    size_t payload_length = sizeof(hello_message_payload);

    if (esp_now_param == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Could not allocate memory for esp_now_param");
        return ESP_FAIL;
    }

    memset(esp_now_param, 0, sizeof(esp_now_send_param_t));

    esp_now_param->len = payload_length + sizeof(esp_now_data_t);
    esp_now_param->buffer = malloc(esp_now_param->len);

    if (esp_now_param->buffer == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Could not allocate memory for esp_now_param->buffer");
        free(esp_now_param);
        return ESP_FAIL;
    }

    esp_now_data_t *buffer = (esp_now_data_t *)esp_now_param->buffer;
    buffer->transmit_type = TRANSMIT_TYPE_BROADCAST;

    memcpy(buffer->payload, hello_message_payload, payload_length);

    if (xQueueSend(sender_queue, esp_now_param, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAGS.send_tag, "Could not send hello message");
        free(esp_now_param->buffer);
        free(esp_now_param);
        return ESP_FAIL;
    }

    return ESP_OK;
}

/*
 * ------------------------------------------------------------------
 * This function is the main state machine of the VCP
 * ------------------------------------------------------------------
 */
static void vcp_state_machine(void)
{

    /* On startup - do variable initialization and send an initial hello message through ESP-NOW */

    esp_now_send_param_t esp_now_param;
    esp_now_data_t esp_now_data;

    if (create_hello_message(&esp_now_param) != ESP_OK)
    {
        ESP_LOGE(TAGS.send_tag, "Could not create hello message");

        // TODO react to the error
    }

    /* startup done - start the main loop and react on incoming messages, do the cord maintenance */

    while (1)
    {
        // TODO react on incoming messages
        // TODO do the cord maintenance
        // TODO check if the sending was successful
        // TODO do the greedy routing
    }
}

esp_err_t init_vcp(void)
{
    // TODO
    return ESP_OK;
}

void deinit_vcp(void)
{
    // TODO
}
