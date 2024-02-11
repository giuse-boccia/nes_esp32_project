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

/* --------------------------------------------- variables and constants --------------------------------------------- */
extern QueueHandle_t receiver_queue;
extern QueueHandle_t sender_queue;
extern QueueHandle_t sender_error_queue;

extern const uint8_t broadcast_mac[ESP_NOW_ETH_ALEN];
extern const error_tags_t TAGS;

/* ----------------------------------------------- function definition ----------------------------------------------- */
esp_err_t init_sender_receiver(void);
esp_now_data_t parse_data(q_receive_data_t *);
void deinit_sender_receiver(void);
void print_esp_now_data_t(esp_now_data_t *);

#endif