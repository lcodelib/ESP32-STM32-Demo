#include "ESP32.h"

static uint8_t esp32_write_buf[MAX_BUF_SIZE];

UART_HandleTypeDef huart2; //ESP32模组串口

UART_HandleTypeDef huart1; //Debug串口

uint8_t  USART2_RX_BUF[USART2_MAX_RECV_LEN];

static void MX_USART2_UART_Init(void);

static void MX_USART1_UART_Init(void);

/* ============================================================
函数名：esp32_check_ack
作用：检查执行结果是否正确
形参：需要检查的预期结果
返回值：返回0不为预期结果，返回1为达到预期
=============================================================== */

uint8_t* esp32_check_ack(char *str)
{
		char *strx=0;
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
		return (uint8_t*)strx;
}

/* ============================================================
函数名：esp32_send_cmd
作用：发送AT指令并返回执行结果
形参：待发指令，预期结果，等待时间
返回值：返回0为发送数据出错，返回1为正常
=============================================================== */

int esp32_send_cmd(uint8_t *cmd,char *ack,uint16_t waittime)
{
		int res = 0;
		int len = strlen((const char*)cmd);
		int acklen = strlen((const char*)ack);
		memset(USART2_RX_BUF,0,USART2_MAX_RECV_LEN);
		HAL_UART_Transmit(&huart2,cmd,len,200);
		HAL_UART_Receive(&huart2,USART2_RX_BUF,USART2_MAX_RECV_LEN,waittime);
		if(esp32_check_ack(ack))res = 1;
		return res;
} 	

/* ============================================================
函数名：esp32_send_data
作用：单独发送串口数据
形参：待发数据
返回值：返回0为发送数据出错，返回1为正常
=============================================================== */

int esp32_send_data(uint8_t *data)
{
		int len = strlen((const char*)data);
		HAL_UART_Transmit(&huart2, data ,len ,200);
		return 1;
}

/* ============================================================
函数名：esp32_debug_printf
作用：发送串口Debug数据
形参：待发数据
返回值：返回0为发送数据出错，返回1为正常
=============================================================== */

int esp32_debug_printf(char *data)
{
		int len = strlen((const char*)data);
		HAL_UART_Transmit(&huart1,(uint8_t*)data ,len ,200);
		return 1;
}

/* ============================================================
函数名：esp32_mod_init()
作用：检测模块是否初始化完毕
形参：无
返回值：返回1表示模块未初始化完成启动，返回0为已启动
=============================================================== */

int esp32_mod_init(){
	  char *AT = "AT\r\n";
    memset(esp32_write_buf, 0, 18);
	  sprintf((char*)esp32_write_buf, "%s", AT);
	  if(esp32_send_cmd(esp32_write_buf, "OK" ,1000)){
			  esp32_debug_printf("启动完成！\r\n");
				return 0;
		}else{
			  esp32_debug_printf("检测启动中...\r\n");
				return 1;
		}
}

/* ============================================================
函数名：esp_set_wifi_mode
作用：设置WIFI工作模式
形参：int型，为选择模式，0为无wifi模式，关闭wifi，1为Station模式
2为SoftAP模式，3为SoftAP+Station模式
返回值：返回1为发送数据出错，返回0为正常
=============================================================== */

int esp_set_wifi_mode(int mode)
{
    char *AT = "AT+CWMODE=";
    memset(esp32_write_buf, 0, 18);
    if (mode == 0 || mode == 1 || mode == 2 || mode == 3)
    {
        sprintf((char*)esp32_write_buf, "%s%d%s", AT, mode, "\r\n");
    }
    else
    { 
			  esp32_debug_printf("WIFI模式设置失败，参数有误！\r\n");
        return 1;
    }
    if(esp32_send_cmd(esp32_write_buf, "OK" ,1000))
		{      
			  esp32_debug_printf("WIFI模式设置成功！\r\n");
        return 0; 
    }else
    {
			  esp32_debug_printf("WIFI模式设置失败！\r\n");
        return 1;
    }
}

/* ============================================================
函数名：esp_set_wifi_connect
作用：设置WIFI连接参数
形参：ssid为目标AP的SSID，pwd为目标AP密码，mac为目标AP物理地址，macen为是否区分mac地址
返回值：返回1为设置出错，返回0为正常
=============================================================== */

int esp_set_wifi_connect(char *ssid,char *pwd, char *mac ,int macen)
{
    char *AT = "AT+CWJAP=";
    memset(esp32_write_buf, 0, 64);
	  if(macen){
				sprintf((char*)esp32_write_buf, "%s\"%s\",\"%s\",\"%s\"%s", AT, ssid, pwd, mac, "\r\n");
		}else
		{
				sprintf((char*)esp32_write_buf, "%s\"%s\",\"%s\"%s", AT, ssid, pwd , "\r\n");
		}
    if(esp32_send_cmd(esp32_write_buf, "WIFI CONNECTED" ,5000))
    {        
        esp32_debug_printf("WIFI连接成功！\r\n");
        return 0;
    }else                                              
    {
			  esp32_debug_printf("WIFI连接失败！\r\n");
        return 1;
    }
}

/* ============================================================
函数名：esp_set_wifi_disconnect
作用：断开与wifi的连接
形参：无
返回值：返回1为设置出错，返回0为正常
=============================================================== */

int esp_set_wifi_disconnect()
{
    char *AT = "AT+CWQAP\r\n";
    if(esp32_send_cmd((uint8_t *)AT, "OK" ,1000))
    {           
        esp32_debug_printf("WIFI已断开！\r\n");
        return 0;
    }else                                              
    {
			  esp32_debug_printf("WIFI断开失败！\r\n");
        return 1;
    }
}

/* ============================================================
函数名：esp_set_wifi_autocon
作用：设置wifi是否上电自动连接
形参：0，上电不自动连接。1，上电自动连接
返回值：返回1为设置出错，返回0为正常
=============================================================== */

int esp_set_wifi_autocon(int stat)
{
	  int res = 0;
    char *AT = "AT+CWAUTOCONN=";
    memset(esp32_write_buf, 0, 24);
    if (stat == 0 || stat == 1)
    {
        sprintf((char*)esp32_write_buf, "%s%d%s", AT, stat, "\r\n");
    }
    else
    { 
			  esp32_debug_printf("自动上电连接设置失败，参数有误！\r\n");
        res = 1;
    }
    if(esp32_send_cmd(esp32_write_buf, "OK" ,1000))
    {     
				esp32_debug_printf("自动上电连接设置成功！\r\n");
    }else                                              
    {
			  esp32_debug_printf("自动上电连接设置失败！\r\n");
        res = 1;
    }
		return res;
}

/* ============================================================
函数名：esp_set_wifi_cipmode
作用：设置wifi模组传输模式
形参：0为普通模式传输，1为透传模式传输。
返回值：返回1为设置出错，返回0为正常
=============================================================== */

int esp_set_wifi_cipmode(int mode)
{
    char *AT = "AT+CIPMODE=";
    memset(esp32_write_buf, 0, 24);
		if (mode == 0 || mode == 1)
    {
				sprintf((char*)esp32_write_buf, "%s%d%s", AT, mode, "\r\n");
		}else
    { 
			  esp32_debug_printf("设置失败，参数有误！\r\n");
        return 1;
    }
    if(esp32_send_cmd(esp32_write_buf, "OK" ,1000))
    {     
			  esp32_debug_printf("传输模式设置成功！\r\n");
        return 0;
    }else                                              
    {
			  esp32_debug_printf("传输模式设置失败！\r\n");
        return 1;
    }
}

/* ============================================================
函数名：esp_set_wifi_tcp
作用：建立与服务器的TCP连接
形参：1，服务器IP地址。2，端口号。
返回值：返回1为设置出错，返回0为正常
=============================================================== */

int esp_set_wifi_tcp(char* ip,int port)
{
    char *AT = "AT+CIPSTART=\"TCP\",\"";
    memset(esp32_write_buf, 0, 64);
    sprintf((char*)esp32_write_buf, "%s%s%s%d%s", AT, ip, "\",", port, "\r\n");
    if(esp32_send_cmd(esp32_write_buf, "CONNECT" ,1000))
    {     
			  esp32_debug_printf("TCP连接建立成功！\r\n");
        return 0;
    }else                                              
    {
			  esp32_debug_printf("TCP连接建立失败！\r\n");
        return 1;
    }
}

/* ============================================================
函数名：esp_set_wifi_tcp
作用：透传模式开关
形参：1为进入透传模式，0为退出透传模式
返回值：返回1为设置出错，返回0为正常
=============================================================== */

int esp_set_wifi_opmode(int mode)
{
    memset(esp32_write_buf, 0, 24);
		if (mode == 0 || mode == 1)
    {
				if(mode == 1)
				{
						sprintf((char*)esp32_write_buf, "%s", "AT+CIPSEND\r\n");
						if(esp32_send_cmd(esp32_write_buf, ">" ,1000))
						{     
								esp32_debug_printf("进入透传模式成功！\r\n");
								return 0;
						}else                                              
						{
								esp32_debug_printf("进入透传模式失败！\r\n");
								return 1;
						}
				}else{
						sprintf((char*)esp32_write_buf, "%s", "+++");
						esp32_send_data(esp32_write_buf);
						HAL_Delay(1000);
						esp32_debug_printf("已退出透传模式！\r\n");
					  return 0;
				}
		}else
    { 
			  esp32_debug_printf("设置失败，参数有误！\r\n");
        return 1;
    }
   
}

/* ============================================================
函数名：esp_set_wifi_tcp
作用：发送TCP数据
形参：1，TCP数据。2，长度
返回值：返回1为出错，返回0为正常
=============================================================== */

int esp_send_tcp_data(uint8_t *data,int len)
{
    char *AT = "AT+CIPSEND=";
    memset(esp32_write_buf, 0, 18);
    sprintf((char*)esp32_write_buf, "%s%d%s", AT, len, "\r\n");
    if(esp32_send_cmd(esp32_write_buf, ">" ,1000))
    {     
			  if(esp32_send_data(data))
				{
						esp32_debug_printf("TCP数据已发送！\r\n");
						return 0;
				}else{
						esp32_debug_printf("TCP数据发送失败！\r\n");
						return 1;
				}
    }else                                              
    {
				esp32_debug_printf("TCP数据发送失败，未建立发送命令！\r\n");
				return 1;
    }
}

/* ============================================================
函数名：esp32_init()
作用：检测，初始化模块，若出错则循环检测，输出debug
形参：无
返回值：无
=============================================================== */

void esp32_init()
{
	  MX_USART2_UART_Init(); //初始化模组串口
	  MX_USART1_UART_Init(); //初始化Debug串口
	  while(esp32_mod_init()); //循环检测模块启动，AT是否回复OK。
}

/* ============================================================
函数名：MX_USART2_UART_Init()
作用：HAL库串口初始化函数，静态函数，内部调用
形参：无
返回值：无
=============================================================== */

static void MX_USART2_UART_Init(void)
{

		huart2.Instance = USART2;
		huart2.Init.BaudRate = 115200;
		huart2.Init.WordLength = UART_WORDLENGTH_8B;
		huart2.Init.StopBits = UART_STOPBITS_1;
		huart2.Init.Parity = UART_PARITY_NONE;
		huart2.Init.Mode = UART_MODE_TX_RX;
		huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart2.Init.OverSampling = UART_OVERSAMPLING_16;
		if (HAL_UART_Init(&huart2) != HAL_OK)
		{
				_Error_Handler(__FILE__, __LINE__);
		}

}

/* ============================================================
函数名：MX_USART1_UART_Init()
作用：HAL库串口初始化函数，静态函数，内部调用
形参：无
返回值：无
=============================================================== */

static void MX_USART1_UART_Init(void)
{

		huart1.Instance = USART1;
		huart1.Init.BaudRate = 115200;
		huart1.Init.WordLength = UART_WORDLENGTH_8B;
		huart1.Init.StopBits = UART_STOPBITS_1;
		huart1.Init.Parity = UART_PARITY_NONE;
		huart1.Init.Mode = UART_MODE_TX_RX;
		huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart1.Init.OverSampling = UART_OVERSAMPLING_16;
		if (HAL_UART_Init(&huart1) != HAL_OK)
		{
				_Error_Handler(__FILE__, __LINE__);
		}

}

