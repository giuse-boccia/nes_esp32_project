/*
 * vcp.c
 *
 * Lecture: Network Embedded Systems
 * Authors: Giuseppe Boccia, Julio Cesar Espinoza Andrea, Tim Schmid
 *
 * This file contains the code with the functions of the virtucal cord protocol
 */

/* --------- external libs ---------- */
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
/* --------- external libs ---------- */

/* ------------- own includes -------------- */
#include "config.h"
#include "sender-receiver.h"
#include "vcp.h"
/* ------------- own includes -------------- */

/* -------- variables and constants -------- */

/* -------- variables and constants -------- */

/* ---- definition of internal fucntions --- */
static void cord_maintenance(void);
static void check_sending_error(void);
static void greedy_routing(void);
static void vcp_state_machine(void);
/* ---- definition of internal fucntions --- */

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

static void vcp_state_machine(void)
{
    // TODO react to incoming messages which are on the sender_queue
}

esp_err_t init_vcp(void)
{
    // TODO
    return ESP_OK;
}