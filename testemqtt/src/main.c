#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"


#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define SSID "Julio.net"
#define PASSWORD "Saturnina5"

static EventGroupHandle_t s_wifi_event_group;
static const char *TAG = "[ ESP32:INFO ]";

static int maximum_retry = 50;
static int s_retry_num = 0;

const char *topico = "sala1/lampada1/";

void callback(int len, int topic_len, char *payload, char *topic);
bool init_wifi();
void init_mqtt(void);
void setup(void);
void loop(void);

void app_main() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    setup();

}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < maximum_retry) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Tentando conectar ao ponto de acesso");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"Erro ao conectar ao ponto de acesso");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Endereço ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

bool init_wifi(){
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));


    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            }
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

    bool sucess = 0;

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Conectado ao Wi-Fi %s com sucesso ", SSID);
        sucess = 1;
    } else if(bits & WIFI_FAIL_BIT){
        ESP_LOGI(TAG, "Erro ao se conectar ao Wi-Fi %s ", SSID);
    } else {
        ESP_LOGI(TAG, "Erro desconhecido ao tentar realizar conexão Wi-Fi \n");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
    return sucess;
}

void callback(int len, int topic_len, char *payload, char *topic){
    ESP_LOGI(TAG, "Menssagem recebida:  ");
    printf("[ %s ] %s", topic, payload);
}

static void log_error_if_nonzero(const char * message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event){
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Conectado");
            msg_id = esp_mqtt_client_publish(client, topico, "Esp_conectado", 13, 1, 0);
            ESP_LOGI(TAG, "Enviado publicação com sucesso, msg_id=%d ", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, topico, 0);
            ESP_LOGI(TAG, "Enviado inscrição com sucesso, msg_id=%d ", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Desconectado do MQTT ");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "Inscrito em um topico, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, topico, "conectado", 9, 0, 0);
            ESP_LOGI(TAG, "Enviado publicação com sucesso, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "Desconectado do topico, msg_id=%d ", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Menssagem enviada, msg_id=%d ", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            callback(event->data_len, event->topic_len, event->data, event->topic);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s) ", strerror(event->error_handle->esp_transport_sock_errno));

            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d ", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}


void init_mqtt(void){
    esp_mqtt_client_config_t mqtt_config = {
        .uri = "mqtt://mqtt.eclipseprojects.io",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
}

void setup(void) {
    ESP_LOGI(TAG, "Iniciando Wi-Fi");
    while (!init_wifi()) {
        ESP_LOGI("", ".");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "Iniciando MQTT ");
    init_mqtt();
}

void loop(void){

}
