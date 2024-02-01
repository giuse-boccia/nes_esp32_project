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
/* -------- variables and constants -------- */

static esp_err_t init_sender_receiver(void);

#endif