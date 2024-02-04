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

/* -------- variables and constants -------- */
extern QueueHandle_t receiver_queue;
extern QueueHandle_t sender_queue;
extern QueueHandle_t sender_error_queue;
<<<<<<< HEAD

extern const uint8_t broadcast_mac[ESP_NOW_ETH_ALEN];

=======
>>>>>>> parent of 18401d9 (AD callback implemtation)
/* -------- variables and constants -------- */

esp_err_t init_sender_receiver(void);
esp_err_t deinit_sender_receiver(void);
esp_err_t send_data(uint16_t sending_delay_ms, uint8_t *payload, uint8_t transmit_type, esp_now_send_param_t *esp_now_param);
esp_err_t parse_data(void);

#endif