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

/* -------------- functions --------------- */
void sender_callback(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // TODO
}

void receiver_callback(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    // TODO
}

void deinit_sender_receiver(void) {
    // TODO
}



esp_err_t init_sender_receiver(void) {
    /* WiFi should start before using ESPNOW */

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(ESPNOW_WIFI_MODE));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

    return ESP_OK;
}