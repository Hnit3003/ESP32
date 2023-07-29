#ifndef HTTP_SERVER_H
#define HTTP_SEVER_H

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
// #include "esp_tls_crypto.h"
#include "esp_http_server.h"

void start_webserver(void);
void stop_webserver(void);

#endif
