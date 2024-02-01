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
#include "vcp.h"
/* ------------- own includes -------------- */

/* -------- variables and constants -------- */
QueueHandle_t receiver_queue;
/* -------- variables and constants -------- */

/* ---- definition of internal fucntions --- */
static void sender_callback(const uint8_t *mac_addr, esp_now_send_status_t status);
static void receiver_callback(const esp_now_recv_info_t *info, const uint8_t *data, int len);
static void deinit_sender_receiver(void);
/* ---- definition of internal fucntions --- */

/* -------------- functions --------------- */
static void sender_callback(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    // TODO
}

static void receiver_callback(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    // TODO
}

static void deinit_sender_receiver(void)
{
    // TODO
}

static esp_err_t init_sender_receiver(void)
{
    // TODO
    return ESP_OK;
}