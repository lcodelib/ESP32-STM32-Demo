#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f1xx_hal.h"

#define USART2_MAX_RECV_LEN		512

#define MAX_BUF_SIZE 1024
#define TCP_MAX_SEND_SIZE 64

uint8_t* esp32_check_cmd(uint8_t *str);
int esp32_send_cmd(uint8_t *cmd,char *ack,uint16_t waittime);
int esp32_send_data(uint8_t *data);
int esp32_debug_printf(char *data);
int esp32_mod_init(void);
int esp_set_wifi_mode(int mode);
int esp_set_wifi_connect(char *ssid,char *pwd,char *mac,int macen);
int esp_set_wifi_disconnect(void);
int esp_set_wifi_autocon(int stat);
int esp_set_wifi_cipmode(int mode);
int esp_set_wifi_tcp(char* ip,int port);
int esp_set_wifi_opmode(int mode);
int esp_send_tcp_data(uint8_t *data,int len);
void esp32_init(void);
