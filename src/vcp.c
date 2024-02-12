/*
 * vcp.c
 *
 * Lecture: Network Embedded Systems
 * Authors: Giuseppe Boccia, Julio Cesar Espinoza Andrea, Tim Schmid
 *
 * This file contains the code with the functions of the virtual cord protocol
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
#include "vcp.h"

/* --------------------------------------------- variables and constants --------------------------------------------- */
float own_position;
int8_t i_successor; // index of the successor in the neighbors array, -1 if no successor
int8_t i_predecessor;
uint8_t neighbors_len;
vcp_neighbor_data_t neighbors[ESPNOW_MAX_PEERS];

// TODO virtual nodes

/* ----------------------------------------------- function definition ----------------------------------------------- */
static void vcp_task(void *);
static esp_err_t handle_vcp_message(esp_now_data_t);
static void join_virtual_cord(void);

/* Helpers for creating messages */
static esp_err_t new_hello_message(void);
static esp_err_t new_update_message(uint8_t, uint8_t[ESP_NOW_ETH_ALEN], float);
static esp_err_t new_data_message(float, char[]);
static esp_err_t create_message(vcp_message_data_t *msg, uint8_t args_length, uint8_t to[ESP_NOW_ETH_ALEN]);
static esp_err_t to_sender_queue(esp_now_data_t *);
static esp_err_t ack_message(uint8_t to[ESP_NOW_ETH_ALEN]);

/* Helpers for handling vcp functionality */
static int8_t find_neighbor_pos(float);
static int8_t find_neighbor_addr(uint8_t[ESP_NOW_ETH_ALEN]);
static int cmp_mac_addr(uint8_t[ESP_NOW_ETH_ALEN], uint8_t[ESP_NOW_ETH_ALEN]);
static float position(float, float);

/* ----------------------------------------------- MAIN VCP Algorithm ----------------------------------------------- */

/* This function is the main task for the vcp functionality - it gets scheduled by FreeRTOS
 *
 * Within the while loop the following stages will be processed:
 * - PHASE 1: Listening cycle --> Waits for x loop cycles for incoming hello messages and reacts accordingly (VCP_DISCOVERY_CYLCES)
 * - PHASE 2: Joins the cord
 * - Phase 3: Maintains cord position, sends/receives data, etc...
 *
 */
static void vcp_task(void *pvParameters)
{
    q_receive_data_t received_data;
    q_send_error_data_t send_error_data;

    uint8_t idle_count = VCP_DISCOVERY_CYCLES;
    uint16_t hello_message_period = 0;

    own_position = VCP_INITIAL;
    i_successor = -1;
    i_predecessor = -1;
    neighbors_len = 0;

    while (true)
    {

        vTaskDelay(VCP_TASK_DELAY_MS / portTICK_PERIOD_MS);
        hello_message_period += VCP_TASK_DELAY_MS;

        if (own_position == VCP_INITIAL && idle_count > 0)
        { // phase 1
            idle_count--;
        }
        else if (own_position == VCP_INITIAL && idle_count == 0)
        { // phase 2
            join_virtual_cord();
            printf("Trying to join virtual cord... %f\n", own_position);
        }

        // Phase 3 --> Uses an internal counter to send hello messages with a specific period
        if (own_position != VCP_INITIAL && hello_message_period > VCP_HELLO_MESSAGE_PERIOD)
        {
            if (new_hello_message() != ESP_OK)
            {
                ESP_LOGE(TAGS.send_tag, "Could not create hello message");
            }
            hello_message_period = 0;
        }

        // PHASE 3 --> Reacts to incoming messages
        if (uxQueueMessagesWaiting(receiver_queue) > 0)
        {
            if (xQueueReceive(receiver_queue, &received_data, portMAX_DELAY) == pdTRUE)
            {
                if (handle_vcp_message(parse_data(&received_data)) != ESP_OK)
                {
                    ESP_LOGE(TAGS.send_tag, "Handling message failed");
                }
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

/* Here the received message are being processed by a statemachine and depending on the message type an according action will be performed*/
static esp_err_t handle_vcp_message(esp_now_data_t msg)
{
    int8_t n;
    float recipient;

    switch (msg.payload->type)
    {
    case VCP_HELLO:
        n = find_neighbor_addr(msg.mac_addr);
        if (n == -1)
        {
            // add new neighbor
            neighbors[neighbors_len].position = ((float *)(msg.payload->args))[0];
            neighbors[neighbors_len].successor = ((float *)(msg.payload->args))[1];
            neighbors[neighbors_len].predecessor = ((float *)(msg.payload->args))[2];
            memcpy(neighbors[neighbors_len].mac_addr, msg.mac_addr, sizeof(msg.mac_addr));
        }
        else
        {
            // update existing neighbor
            neighbors[n].position = ((float *)(msg.payload->args))[0];
            neighbors[n].successor = ((float *)(msg.payload->args))[1];
            neighbors[n].predecessor = ((float *)(msg.payload->args))[2];
        }
        break;
    case VCP_UPDATE_SUCCESSOR:
        // update my successor and my cord position
        own_position = ((float *)(msg.payload->args))[0];
        n = find_neighbor_addr(msg.mac_addr);
        if (n != -1)
        {
            i_successor = n;
        }
        else
        {
            if (neighbors_len < ESPNOW_MAX_PEERS)
            {
                // Initialize new neighbor, its pos, succ and pred will be updated by an hello message in the future
                neighbors[neighbors_len].position = VCP_INITIAL;
                neighbors[neighbors_len].successor = VCP_INITIAL;
                neighbors[neighbors_len].predecessor = VCP_INITIAL;
                memcpy(neighbors[neighbors_len].mac_addr, msg.mac_addr, sizeof(msg.mac_addr));
                neighbors_len++;
            }
            else
            {
                ESP_LOGE(TAGS.receive_tag, "Can't add neighbor, max number of peers reached");
            }
        }
        break;
    case VCP_UPDATE_PREDECESSOR:
        // update my predecessor and my cord position
        own_position = ((float *)(msg.payload->args))[0];
        n = find_neighbor_addr(msg.mac_addr);
        if (n != -1)
        {
            i_predecessor = n;
        }
        else
        {
            if (neighbors_len < ESPNOW_MAX_PEERS)
            {
                // Initialize new neighbor, its pos, succ and pred will be updated by an hello message in the future
                neighbors[neighbors_len].position = VCP_INITIAL;
                neighbors[neighbors_len].successor = VCP_INITIAL;
                neighbors[neighbors_len].predecessor = VCP_INITIAL;
                memcpy(neighbors[neighbors_len].mac_addr, msg.mac_addr, sizeof(msg.mac_addr));
                neighbors_len++;
            }
            else
            {
                ESP_LOGE(TAGS.receive_tag, "Can't add neighbor, max number of peers reached");
            }
        }
        break;
    case VCP_CREATE_VIRTUAL_NODE:
        // TODO create virtual node, send update messages
        break;
    case VCP_DATA:
        // If message is for me, print, otherwise forward to successor
        recipient = ((float *)msg.payload->args)[0];
        if (recipient == own_position)
        {
            printf("Received data: %s\n", (char *)(msg.payload->args + 1));
        }
        else
        {
            new_data_message(recipient, (char *)(msg.payload->args + 1));
        }
        break;
    case VCP_ERR:
        printf("Received error message\n");
        break;
    default:
        printf("Received message of unknown type: %x\n", msg.payload->type);
        break;
    }
    return ESP_OK;
}

/*
 * ------------------------------------------------------------------
 * This function reviews the neighbors table to find its own cord position.
 * It also schedules update messages to be sent to the neighbors.
 * Adapted from slides "08-routing.pdf" and VCP paper (2008).
 * When joining the cord, 5 cases are possible:
 *   0. I have no neighbors
 *   A. I am neighbor with node 0.0
 *   B. I am neighbor with node 1.0
 *   C. I am neighbor with 2 nodes that are neighbor with each other
 *   D. None of the previous ones ---> create virtual node
 * ------------------------------------------------------------------
 */
static void join_virtual_cord()
{
    float new_neighbor_position;
    int8_t n;

    // CASE 0: I have no neighbors
    if (neighbors_len == 0)
    {
        own_position = VCP_START;
        return;
    }

    // CASE A: I am neighbor with node 0.0
    n = find_neighbor_pos(VCP_START);
    if (n != -1)
    {
        own_position = VCP_START;
        i_successor = n;
        i_predecessor = -1;
        if (neighbors[n].successor == VCP_INITIAL)
        {
            new_neighbor_position = VCP_END;
        }
        else
        {
            new_neighbor_position = position(own_position, neighbors[n].successor);
        }
        new_update_message(VCP_UPDATE_PREDECESSOR, neighbors[n].mac_addr, new_neighbor_position);
        return;
    }

    // CASE B: I am neighbor with node 1.0
    n = find_neighbor_pos(VCP_END);
    if (n != -1)
    {
        own_position = VCP_END;
        i_successor = -1;
        i_predecessor = n;
        new_neighbor_position = position(neighbors[n].predecessor, VCP_END);
        new_update_message(VCP_UPDATE_SUCCESSOR, neighbors[n].mac_addr, new_neighbor_position);
        return;
    }

    // CASE C: I am neighbor with 2 nodes that are neighbor with each other
    for (int i = 0; i < neighbors_len; i++)
    {
        for (int j = 0; j < neighbors_len; j++)
        {
            if ((i != j) && (neighbors[i].predecessor == neighbors[j].position))
            {
                // neighbor j is predecessor to neighbor i
                own_position = position(neighbors[j].position, neighbors[i].position);
                i_predecessor = j;
                i_successor = i;
                new_update_message(VCP_UPDATE_SUCCESSOR, neighbors[j].mac_addr, neighbors[j].position);
                new_update_message(VCP_UPDATE_PREDECESSOR, neighbors[i].mac_addr, neighbors[i].position);
                return;
            }
        }
    }

    // CASE D: create virtual node
    // TODO create virtual node. For the moment, own_position stays at VCP_INITIAL
    // new_create_virtual_node_message();
    own_position = VCP_INITIAL;
    return;
}

/* ----------------------------------------------- Helper functions ----------------------------------------------- */

/* Creates a the periodic hello message */
static esp_err_t new_hello_message()
{
    vcp_message_data_t *msg;
    uint8_t to[ESP_NOW_ETH_ALEN];

    uint8_t payload_length = sizeof(uint8_t) + 3 * sizeof(float);
    msg = (vcp_message_data_t *)malloc(payload_length);

    if (msg == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Could not allocate memory for hello message");
        return ESP_FAIL;
    }

    memset(msg, 0, payload_length);

    msg->type = VCP_HELLO;

    ((float *)msg->args)[0] = own_position;
    ((float *)msg->args)[1] = VCP_INITIAL;
    ((float *)msg->args)[2] = VCP_INITIAL;

    if (i_successor != -1)
    {
        ((float *)msg->args)[1] = neighbors[i_successor].position;
    }
    if (i_predecessor != -1)
    {
        ((float *)msg->args)[2] = neighbors[i_predecessor].position;
    }
    memcpy(to, broadcast_mac, sizeof(broadcast_mac));
    return create_message(msg, payload_length, to);
}

static esp_err_t ack_message(uint8_t to[ESP_NOW_ETH_ALEN])
{
    vcp_message_data_t *msg;
    msg = (vcp_message_data_t *)malloc(sizeof(uint8_t));

    if (msg == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Could not allocate memory for ack message");
        return ESP_FAIL;
    }

    memset(msg, 0, sizeof(uint8_t));
    msg->type = VCP_ACK;

    return create_message(msg, sizeof(uint8_t), to);
}

/* Creates a new update message */
static esp_err_t new_update_message(uint8_t type, uint8_t to[ESP_NOW_ETH_ALEN], float new_position)
{
    vcp_message_data_t *msg;
    uint8_t payload_length = sizeof(uint8_t) + (new_position);

    msg = (vcp_message_data_t *)malloc(payload_length);

    if (msg == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Could not allocate memory for update position message");
        return ESP_FAIL;
    }

    memset(msg, 0, payload_length);

    msg->type = type;
    ((float *)msg->args)[0] = new_position;

    return create_message(msg, sizeof(new_position), to);
}

/* Creates a new data message, this function contains the greedy routing mechanism */
static esp_err_t new_data_message(float to, char content[])
{
    vcp_message_data_t *msg;
    uint8_t payload_length = sizeof(float) + strlen(content);
    int8_t n;

    // ARGS: first 4 bytes are the float, the rest is the content (string)
    msg = (vcp_message_data_t *)malloc(payload_length);
    if (msg == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Could not allocate memory for data message");
        return ESP_FAIL;
    }

    memset(msg, 0, payload_length);

    msg->type = VCP_DATA;

    ((float *)msg->args)[0] = to;
    strcpy((char *)(((float *)(msg->args)) + 1), content); // pointer hell but should work :_)

    // if recipient is my neighbour, send it directly to him, otherwise send msg to successor
    n = find_neighbor_pos(to);
    if (n != -1)
    {
        return create_message(msg, payload_length, neighbors[n].mac_addr);
    }
    else
    {
        if (own_position > neighbors[n].position)
        {
            return create_message(msg, payload_length, neighbors[i_predecessor].mac_addr);
        }
        else
        {
            return create_message(msg, payload_length, neighbors[i_successor].mac_addr);
        }
    }
}

/* Converts the vcp_message_data_t to esp_now_data_t in order to be processed by the sender_task */
static esp_err_t create_message(vcp_message_data_t *msg, uint8_t args_length, uint8_t to[ESP_NOW_ETH_ALEN])
{
    esp_now_data_t *sender_queue_data;
    uint8_t payload_length = sizeof(esp_now_data_t);

    sender_queue_data = (esp_now_data_t *)malloc(payload_length);

    if (sender_queue_data == NULL)
    {
        ESP_LOGE(TAGS.send_tag, "Could not allocate memory for sender queue message");
        return ESP_FAIL;
    }

    sender_queue_data->payload_length = payload_length;
    sender_queue_data->payload = msg;

    if (cmp_mac_addr(to, broadcast_mac) == 0)
    {
        sender_queue_data->transmit_type = TRANSMIT_TYPE_BROADCAST;
    }
    else
    {
        sender_queue_data->transmit_type = TRANSMIT_TYPE_UNICAST;
    }

    memcpy(sender_queue_data->mac_addr, to, ESP_NOW_ETH_ALEN);
    return to_sender_queue(sender_queue_data);
}

/* Grabs the esp_now_data_t pointer and pushes it to the sender_queue */
static esp_err_t to_sender_queue(esp_now_data_t *esp_now_data)
{

    if (xQueueSend(sender_queue, esp_now_data, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAGS.send_tag, "Could not send hello message");
        return ESP_FAIL;
    }

    return ESP_OK;
}

/* Searches for a specific neighbor given its position. Returns its position in the neighbors array or -1 if not found */
static int8_t find_neighbor_pos(float p)
{
    for (int i = 0; i < neighbors_len; i++)
    {
        if (neighbors[i].position == p)
        {
            return i;
        }
    }

    return -1;
}

/* Searches for a specific neighbor given its mac address. Returns its position in the neighbors array or -1 if not found */
static int8_t find_neighbor_addr(uint8_t addr[ESP_NOW_ETH_ALEN])
{
    for (int i = 0; i < neighbors_len; i++)
    {
        if (!cmp_mac_addr(neighbors[i].mac_addr, addr))
        {
            return i;
        }
    }

    return -1;
}

/* Returns 0 if the two mac addresses are the same */
static int cmp_mac_addr(uint8_t a1[ESP_NOW_ETH_ALEN], uint8_t a2[ESP_NOW_ETH_ALEN])
{
    for (int i = 0; i < ESP_NOW_ETH_ALEN; i++)
    {
        if (a1[i] != a2[i])
        {
            return -1;
        }
    }
    return 0;
}

/* Returns a new position between p1 and p2 */
static float position(float p1, float p2)
{
    return (p1 + p2) / 2;
    // TODO rewrite using VCP_INTERVAL to avoid float hell
}

void init_vcp(void)
{
    xTaskCreate(vcp_task, "vcp_state_machine", 4096, NULL, 4, NULL);
}
