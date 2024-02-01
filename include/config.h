/*
 * config.h
 *
 * Lecture: Network Embedded Systems
 * Authors: Giuseppe Boccia, Julio Cesar Espinoza Andrea, Tim Schmid
 *
 * This file contains the necessary configuration for the project
 */

#ifndef CONFIG_H
#define CONFIG_H

/* -------- wifi constants -------- */
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF ESP_IF_WIFI_STA
#define ESPNOW_WIFI_CHANNEL 1

/* -------- queue constants -------- */
#define SENDER_QUEUE_SIZE 10
#define RECEIVER_QUEUE_SIZE 5
#define SENDER_ERROR_QUEUE_SIZE 5

#endif