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
vcp_cord_positions_t vcp_state;

/* ----------------------------------------------- function definition ----------------------------------------------- */
static void cord_maintenance(void);
static void check_sending_error(void);
static void greedy_routing(void);
static void update_cord(void);
static void vcp_state_machine(void *pvParameters);
static esp_err_t handle_vcp_message(q_receive_data_t received_data);
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
 * This function reviews the neighbors table to find its own cord position.
 * It also schedules update messages to be sent to the neighbors.
 * Adapted from slides "08-routing.pdf".
 * ------------------------------------------------------------------
 */
static esp_err_t set_my_position(void)
{
    float p_temp;
    bool found;

    for (int i = 0; i < vcp_state.neighbors_len; i++)
    {
        if (vcp_state.neighbors[i].position == VCP_START)
        {
            if (vcp_state.neighbors[i].successor < VCP_START)
            {
                p_temp = VCP_END;
            }
            else if (vcp_state.neighbors[i].successor == VCP_END)
            {
                p_temp = (VCP_START + VCP_END) / 2;
            }
            else   
            {
                p_temp = vcp_state.neighbors[i].successor - VCP_INTERVAL * (vcp_state.neighbors[i].successor - vcp_state.neighbors[i].position)
            }
            vcp_state.own_position = p_temp;
            if(send_new_position_to_neighbor(i, p_temp) != ESP_OK)          // send JOIN msg to neighbor i
            {
                ESP_LOGE(TAGS.send_tag, "Could not send join message");
                return ESP_FAIL;
            }

        }
        else if (vcp_state.neighbors[i].position == VCP_END)
        {
            if (vcp_state.neighbors[i].successor == VCP_START)
            {
                p_temp = (VCP_START + VCP_END) / 2;
            }
            else
            {
                p_temp = vcp_state.neighbors[i].predecessor - VCP_INTERVAL * (vcp_state.neighbors[i].predecessor - vcp_state.neighbors[i].position)
            }
            // TODO update current pos with p_temp and send new position to neighbour i
        }
        else
        {
            found = false;
            for (int j = 0; j < vcp_state.neighbors_len; j++)
            {
                if ((i != j) && (vcp_state.neighbors[i].predecessor == vcp_state.neighbors[j].position))
                {
                    found = true;
                    p_temp = (vcp_state.neighbors[i].position + vcp_state.neighbors[j].position) / 2
                    // TODO temporally store positions of i and j; send block req to j with p_temp (????)
                }
            }
            if (found == 0) {
                // TODO create virtual node
            }
        }
    }

    return ESP_OK
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



static esp_err_t send_new_position_to_neighbor(int i_neighbor, float p_temp)
{
    // TODO create JOIN message and add to send queue
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
    memcpy(esp_now_data->destination_mac, broadcast_mac, sizeof(broadcast_mac));
    esp_now_data->payload_length = 1;   // message contains only 1 byte, no args
    esp_now_data->payload = malloc(esp_now_data->payload_length);
    esp_now_data->payload[0] = VCP_HELLO;
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

    /* Initialize vcp_state variable */
    vcp_state.own_position = VCP_INITIAL;
    vcp_state.i_successor = -1;
    vcp_state.i_predecessor = -1;
    vcp_state.neighbors_len = 0;

    /* startup done - start the main loop and react on incoming messages, do the cord maintenance */

    while (1)
    {
        vTaskDelay(VCP_TASK_DELAY_MS / portTICK_PERIOD_MS);

        if (create_hello_message() != ESP_OK)
        {
            ESP_LOGE(TAGS.send_tag, "Could not create hello message");
        }

        if (uxQueueMessagesWaiting(receiver_queue) > 0)
        {
            if (xQueueReceive(receiver_queue, &received_data, portMAX_DELAY) == pdTRUE)
            {
                printf("Received data from: " MACSTR " with length: %d\n", MAC2STR(received_data.mac_addr), received_data.data_len);
                handle_vcp_message(received_data);
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

/*
 * -------------------------------------------------------------------------------------
 * This function handles a received message and updates the vcp_state global variable
 * -------------------------------------------------------------------------------------
 */
static esp_err_t handle_vcp_message(q_receive_data_t received_data)
{
    vcp_message_data_t msg_data;
    msg_data.msg_type = received_data.data[0];  // first byte is the message type
    msg_data.args = received_data.data + 1;     // the rest of the data is arguments
    switch (msg_data.msg_type)
    {
    case VCP_HELLO:
        // TODO update neighbor list with new node address and position, maybe send "update" to create virtual node
        break;
    case VCP_JOIN:
        // TODO update neighbor list with new info
        break;
    case VCP_DATA:
        // TODO if i'm the receiver, print content, otherwise apply greedy routing
        break;  
    case VCP_ERROR:
        // TODO print error
        break;
    default:
        break;
    }
    return ESP_OK;
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
