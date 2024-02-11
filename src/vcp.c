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
int8_t i_successor;     // index of the successor in the neighbors array, -1 if no successor
int8_t i_predecessor;
uint8_t neighbors_len;
vcp_neighbor_data_t neighbors[ESP_NOW_MAX_PEERS];
// TODO virtual nodes ??

/* ----------------------------------------------- function definition ----------------------------------------------- */
static void vcp_state_machine(void *);
static esp_err_t handle_vcp_message(esp_now_data_t);
static void join_virtual_cord(void);
static esp_err_t new_hello_message(void);
static esp_err_t new_update_message(uint8_t, uint8_t[ESP_NOW_ETH_ALEN], float);
static esp_err_t new_data_message(float, char[]);
static esp_err_t create_message(vcp_message_data_t, uint8_t[ESP_NOW_ETH_ALEN]);
static esp_err_t to_sender_queue(esp_now_data_t *);
static int8_t find_neighbor_pos(float);
static int8_t find_neighbor_addr(uint8_t[ESP_NOW_ETH_ALEN]);
static int cmp_mac_addr(uint8_t[ESP_NOW_ETH_ALEN], uint8_t[ESP_NOW_ETH_ALEN]);
static float position(float, float);


/*
 * ------------------------------------------------------------------
 * This function is the main state machine of the VCP
 * ------------------------------------------------------------------
 */
static void vcp_state_machine(void *pvParameters) {
    q_receive_data_t received_data;
    q_send_error_data_t send_error_data;

    int idle_count = VCP_DISCOVERY_CYCLES;  // number of "listening" cycles before joining the network

    /* Initialize variable */
    own_position = VCP_INITIAL;
    i_successor = -1;
    i_predecessor = -1;
    neighbors_len = 0;

    /*
    * Main loop after initialization
    * PHASE 1: Discovery cycles ---> doesn't send hello msg, reacts to incoming msg
    * PHASE 2: Join cord
    * PHASE 3: Joined -------------> send hello msg, react to incoming msg
    */
    while (1) {

        vTaskDelay(VCP_TASK_DELAY_MS / portTICK_PERIOD_MS);

        if (own_position == VCP_INITIAL && idle_count > 0) {                // phase 1
            idle_count--;
        } else if (own_position == VCP_INITIAL && idle_count == 0) {        // phase 2
            join_virtual_cord();
            printf("Trying to join virtual cord... %f\n", own_position);
        }

        if (own_position != VCP_INITIAL) {                                  // phase 3
            if (new_hello_message() != ESP_OK) {
                ESP_LOGE(TAGS.send_tag, "Could not create hello message");
            }
        }

        while (uxQueueMessagesWaiting(receiver_queue) > 0) {
            if (xQueueReceive(receiver_queue, &received_data, portMAX_DELAY) == pdTRUE) {
                handle_vcp_message(parse_data(&received_data));
            }
        }

        if (uxQueueMessagesWaiting(sender_error_queue) > 0) {
            if (xQueueReceive(sender_error_queue, &send_error_data, portMAX_DELAY) != pdTRUE) {
                ESP_LOGE(TAGS.receive_tag, "Sending error status: %d\n", send_error_data.status);
            }
        }
    }
}

/*
 * -------------------------------------------------------------------------------------
 * This function handles a received message and updates the vcp state
 * -------------------------------------------------------------------------------------
*/
static esp_err_t handle_vcp_message(esp_now_data_t msg) {
    int8_t n;
    float recipient;


    switch (msg.payload->type) {
    case VCP_HELLO:
        n = find_neighbor_addr(msg.mac_addr);
        if (n == -1) {
            // add new neighbor
            neighbors[neighbors_len].position = ((float *)(msg.payload->args))[0];
            neighbors[neighbors_len].successor = ((float *)(msg.payload->args))[1];
            neighbors[neighbors_len].predecessor = ((float *)(msg.payload->args))[2];
            memcpy(neighbors[neighbors_len].mac_addr, msg.mac_addr, sizeof(msg.mac_addr));
        } else {
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
        if (n != -1) {
            i_successor = n;
        } else {
            if (neighbors_len < ESP_NOW_MAX_PEERS) {
                // Initialize new neighbor, its pos, succ and pred will be updated by an hello message in the future
                neighbors[neighbors_len].position = VCP_INITIAL;
                neighbors[neighbors_len].successor = VCP_INITIAL;
                neighbors[neighbors_len].predecessor = VCP_INITIAL;
                memcpy(neighbors[neighbors_len].mac_addr, msg.mac_addr, sizeof(msg.mac_addr));
                neighbors_len++;
            } else {
                ESP_LOGE(TAGS.receive_tag, "Can't add neighbor, max number of peers reached");
            }
        }
        break;
    case VCP_UPDATE_PREDECESSOR:
        // update my predecessor and my cord position
        own_position = ((float *)(msg.payload->args))[0];
        n = find_neighbor_addr(msg.mac_addr);
        if (n != -1) {
            i_predecessor = n;
        } else {
            if (neighbors_len < ESP_NOW_MAX_PEERS) {
                // Initialize new neighbor, its pos, succ and pred will be updated by an hello message in the future
                neighbors[neighbors_len].position = VCP_INITIAL;
                neighbors[neighbors_len].successor = VCP_INITIAL;
                neighbors[neighbors_len].predecessor = VCP_INITIAL;
                memcpy(neighbors[neighbors_len].mac_addr, msg.mac_addr, sizeof(msg.mac_addr));
                neighbors_len++;
            } else {
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
        if (recipient == own_position) {
            printf("Received data: %s\n", (char *)(msg.payload->args + 1));
        } else {
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
static void join_virtual_cord() {
    float new_neighbor_position;
    int8_t n;

    // CASE 0: I have no neighbors
    if (neighbors_len == 0) {
        own_position = VCP_START;
        return;
    }

    // CASE A: I am neighbor with node 0.0
    n = find_neighbor_pos(VCP_START);
    if (n != -1) {
        own_position = VCP_START;
        i_successor = n;
        i_predecessor = -1;
        if (neighbors[n].successor == VCP_INITIAL) {
            new_neighbor_position = VCP_END;
        } else {
            new_neighbor_position = position(own_position, neighbors[n].successor);
        }
        new_update_message(VCP_UPDATE_PREDECESSOR, neighbors[n].mac_addr, new_neighbor_position);
        return;
    }

    // CASE B: I am neighbor with node 1.0
    n = find_neighbor_pos(VCP_END);
    if (n != -1) {
        own_position = VCP_END;
        i_successor = -1;
        i_predecessor = n;
        new_neighbor_position = position(neighbors[n].predecessor, VCP_END);
        new_update_message(VCP_UPDATE_SUCCESSOR, neighbors[n].mac_addr, new_neighbor_position);
        return;
    }

    // CASE C: I am neighbor with 2 nodes that are neighbor with each other
    for (int i = 0; i < neighbors_len; i++) {
        for (int j = 0; j < neighbors_len; j++) {
            if ((i != j) && (neighbors[i].predecessor == neighbors[j].position)) {
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

/* Creates a new hello message */
static esp_err_t new_hello_message() {
    vcp_message_data_t msg;
    uint8_t to[ESP_NOW_ETH_ALEN];

    float args[3] = { own_position, VCP_INITIAL, VCP_INITIAL };

    if (i_successor != -1) { args[1] = neighbors[i_successor].position; }
    if (i_predecessor != -1) { args[2] = neighbors[i_predecessor].position; }
    msg.type = VCP_HELLO;
    msg.args = args;

    memcpy(to, broadcast_mac, sizeof(broadcast_mac));
    return create_message(msg, to);
}

/* Creates a new update message */
static esp_err_t new_update_message(uint8_t type, uint8_t to[ESP_NOW_ETH_ALEN], float new_position) {
    vcp_message_data_t msg;
    float args[1] = { new_position };

    msg.type = type;
    msg.args = args;

    return create_message(msg, to);
}

/* Creates a new data message */
static esp_err_t new_data_message(float to, char content[]) {
    vcp_message_data_t msg;
    int8_t n;

    msg.type = VCP_DATA;

    // ARGS: first 4 bytes are the float, the rest is the content (string)
    msg.args = malloc(sizeof(float) + strlen(content));
    if (msg.args == NULL) {
        ESP_LOGE(TAGS.send_tag, "Could not allocate memory for data message");
        return ESP_FAIL;
    }
    ((float *)msg.args)[0] = to;
    strcpy((char *)(((float *)(msg.args)) + 1), content);       // pointer hell but should work :_)

    // if recipient is my neighbour, send it directly to him, otherwise send msg to successor
    n = find_neighbor_pos(to);
    if (n != -1) {
        return create_message(msg, neighbors[n].mac_addr);
    }

    return create_message(msg, neighbors[i_successor].mac_addr);

}

/*
 * ------------------------------------------------------------------
 * This function creates a message and pushes it on the sender_queue
 * ------------------------------------------------------------------
 */
static esp_err_t create_message(vcp_message_data_t msg, uint8_t to[ESP_NOW_ETH_ALEN]) {


    return ESP_OK;
}

/*
 * ------------------------------------------------------------------
 * This function pushes a message to the sender queue
 * ------------------------------------------------------------------
 */
static esp_err_t to_sender_queue(esp_now_data_t *esp_now_data) {

    if (xQueueSend(sender_queue, esp_now_data, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAGS.send_tag, "Could not send hello message");
        return ESP_FAIL;
    }

    return ESP_OK;
}

/* Searches for a specific neighbor given its position. Returns its position in the neighbors array or -1 if not found */
static int8_t find_neighbor_pos(float p) {
    for (int i = 0; i < neighbors_len; i++) {
        if (neighbors[i].position == p) {
            return i;
        }
    }

    return -1;
}

/* Searches for a specific neighbor given its mac address. Returns its position in the neighbors array or -1 if not found */
static int8_t find_neighbor_addr(uint8_t addr[ESP_NOW_ETH_ALEN]) {
    for (int i = 0; i < neighbors_len; i++) {
        if (!cmp_mac_addr(neighbors[i].mac_addr, addr)) {
            return i;
        }
    }

    return -1;
}

/* Returns 0 if the two mac addresses are the same */
static int cmp_mac_addr(uint8_t a1[ESP_NOW_ETH_ALEN], uint8_t a2[ESP_NOW_ETH_ALEN]) {
    for (int i = 0; i < ESP_NOW_ETH_ALEN; i++) {
        if (a1[i] != a2[i]) {
            return -1;
        }
    }
    return 0;
}

/* Returns a new position between p1 and p2 */
static float position(float p1, float p2) {
    return (p1 + p2) / 2;
    // TODO rewrite using VCP_INTERVAL to avoid float hell
}

esp_err_t init_vcp(void) {
    xTaskCreate(vcp_state_machine, "vcp_state_machine", 4096, NULL, 4, NULL);

    return ESP_OK;
}
