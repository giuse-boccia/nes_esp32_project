/*
 * sender-receiver.h
 *
 * Lecture: Network Embedded Systems
 * Authors: Giuseppe Boccia, Julio Cesar Espinoza Andrea, Tim Schmid
 *
 * This file contains the code for sending and receiving data via ESP-NOW.
 * The data will be but in a queue and processed by another task
 *
 */

#ifndef SENDER_RECEIVER_H
#define SENDER_RECEIVER_H

#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define CONFIG_ESPNOW_CHANNEL 1     // TODO: this shouldn't be forced here but set using esp-idf config.py

/* -------- variables and constants -------- */
extern QueueHandle_t receiver_queue;
/* -------- variables and constants -------- */

void sender_callback(const uint8_t *mac_addr, esp_now_send_status_t status);
void receiver_callback(const esp_now_recv_info_t *info, const uint8_t *data, int len);
void deinit_sender_receiver(void);
esp_err_t init_sender_receiver(void);

#endif