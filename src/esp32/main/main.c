#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "include/display.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#define USE_DISPLAY	1
#define FLOAT_PRECISION	1

// WPA ----------------------------------------------------------------------------
#define EXAMPLE_WIFI_SSID "<your-ssid>"
#define EXAMPLE_WIFI_PASS "<your-wifi-password>"
// --------------------------------------------------------------------------------

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
int lampPort = 22;
int blinkLedPort = 21;
int wifiErrorLedPort = 25;
int socketErrorLedPort = 26;

#define CHECK_INTERVAL 10
#define WEB_SERVER "www.example.com"
#define WEB_PORT "80"
#define WEB_URL "/rest/<your-endpoint>"

static const char *TAG = "watcher";
static const char *checkString = "e!";
int isWifiConnected;
int isDnsFailure;
int isSocketFailure;
int isDead;
char responsePayload[4096];
int socketFailureCount;

static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
    "Host: "WEB_SERVER"\r\n"
    "User-Agent: ESP-32-Commuting\r\n"
    "\r\n";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
	ESP_LOGI(TAG, "Starting up, connecting to WiFi");
        esp_wifi_connect();
	isWifiConnected = 0;
	
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
	ESP_LOGI(TAG, "Got IP Address, setting connected bit to TRUE");
	isWifiConnected = 1;
	
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
	ESP_LOGI(TAG, "WiFi disconnected, trying to reconnect after 5 seconds, and clearing connected bit");
	isWifiConnected = 0;
	
	vTaskDelay(5000 / portTICK_PERIOD_MS);
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];
    char payload[32];
    int nc = 0, payloadStarted = 0, payloadLength = 0;
    int firstCycle = 1;
    int led_state = 0;

    socketFailureCount = 0;
    while(1) {
	if (socketFailureCount == 10) {
	    esp_restart();
	}

	// --------------------------------------------------------------------
	// wait
	// --------------------------------------------------------------------
	if (firstCycle != 1) {
	    for(int countdown = CHECK_INTERVAL * 2; countdown >= 0; countdown--) {
        	ESP_LOGI(TAG, "%d... ", countdown);

		fc_set_led(led_state);
		if (led_state == 0) led_state = 1;
		else led_state = 0;

        	vTaskDelay(500 / portTICK_PERIOD_MS);
    	    }
	}

	firstCycle = 0;

        ESP_LOGI(TAG, "Starting again!");

        // Wait for the callback to set the CONNECTED_BIT in the event group.
        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                            false, true, portMAX_DELAY);
        ESP_LOGI(TAG, "Connected to AP");

        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            isDnsFailure = 1;
            
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        // Code to print the resolved IP. Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));
        isDnsFailure = 0;
        

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
	    socketFailureCount++;
            isSocketFailure = 1;
            
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        isSocketFailure = 0;
        
        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            isSocketFailure = 1;
            
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");
        isSocketFailure = 0;
        

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            isSocketFailure = 1;
            
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        // Read HTTP response
	responsePayload[0] = '\0';
	payloadStarted = 0;
	payloadLength = 0;
	int charCount = 0;
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            for(int i = 0; i < r; i++) {
		if (charCount < sizeof(responsePayload)) {
		    responsePayload[charCount++] = recv_buf[i];

		    if (payloadStarted == 1 && recv_buf[i] != '\n' && recv_buf[i] != '\0') {
			payload[payloadLength] = recv_buf[i];
			payloadLength++;
		    }


		    if (recv_buf[i] == '\n') {
			nc++;
			if (nc == 2) payloadStarted = 1;
		    }
		    else {
			if (nc != 2 && recv_buf[i] != '\r') {
			    nc = 0;
			}
		    }
		}
                //putchar(recv_buf[i]);
            }
        } while(r > 0);

	responsePayload[charCount] = '\0';
	payload[payloadLength] = '\0';

	ESP_LOGI(TAG, "Payload: length: %d, content: '%s'", payloadLength, payload);

	char *ret = strstr(responsePayload, checkString);
	isDead = (ret == NULL) ? 0 : 1;
	ESP_LOGI(TAG, "Is application down: %d", isDead);

	if (isDead) {
	    ESP_LOGI(TAG, "Service is down, invalid payload received");
	    printf("%s", responsePayload);
	    fc_print_digit(200); // display --
	}
	else {
	    int secondsLeft = atoi(payload);
	    double minutesLeft = secondsLeft / 60;
	    printf("Seconds left: %f", minutesLeft);
#if USE_DISPLAY
	fc_print_digit(minutesLeft);
#endif
	}

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        isSocketFailure = 0;
        
        close(s);
    }
}

void app_main()
{
    isWifiConnected = 0;
    isDnsFailure = 0;
    isSocketFailure = 0;
    isDead = 0;
    
#if USE_DISPLAY
    fc_setup_display();
    fc_blank_digits();
#endif

    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_LOGI(TAG, "Using WPA WiFi");
    initialise_wifi();

    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
}

