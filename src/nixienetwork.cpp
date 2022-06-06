#include "nixienetwork.h"

void NixieNetwork::init()
{
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    //创建默认配置，并用它来初始化wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    //将所有WiFi_EVENT注册至networkHandler

    auto handler = [](void* arg,esp_event_base_t event_base,int32_t event_id,void* event_data){
            NixieNetwork::instance().networkHandler(arg,event_base,event_id,event_data);
    };
    
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, handler, NULL));
    //将IP_EVENT_STA_GOT_IP事件注册至networkHandler
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, handler, NULL) );
    //将所有SmartConfig事件(SC_EVENT)注册至networkHandler
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, handler, NULL) );

    //设置为STA模式
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    //打开wifi
    ESP_ERROR_CHECK( esp_wifi_start() );
}


void NixieNetwork::networkSmartConfigTask(void * pvParameters)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
    while (1) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

bool NixieNetwork::readStoredWifiConfig(uint8_t ssid[32],uint8_t password[64])
{
    bzero(ssid,32);
    bzero(password,64);
    esp_err_t err;
    err = NixieStorage::instance().open("wificonfig").nvsHandler->get_string("ssid",(char *)ssid,32);
    uint8_t flag = 0;
    switch (err) {
        case ESP_OK:
            flag++;
            ESP_LOGI(TAG,"Successfully Read Stored SSID : %s\n",ssid);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG,"SSID Never Stored!\n");
            break;
        default :
            ESP_LOGI(TAG,"Error (%s) reading!\n", esp_err_to_name(err));
    }
    err = NixieStorage::instance().open("wificonfig").nvsHandler->get_string("password",(char *)password,64);
    switch (err) {
        case ESP_OK:
            flag++;
            ESP_LOGI(TAG,"Successfully Read Stored Password : %s\n",password);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG,"Password Never Stored!\n");
            break;
        default :
            ESP_LOGI(TAG,"Error (%s) reading!\n", esp_err_to_name(err));
    }
    return flag == 2;
}

bool NixieNetwork::saveStoredWifiConfig(uint8_t ssid[32],uint8_t password[64])
{
    esp_err_t err;
    err = NixieStorage::instance().open("wificonfig").nvsHandler->set_string("ssid",(char *)ssid);
    if(err != ESP_OK) return false;
    err = NixieStorage::instance().open("wificonfig").nvsHandler->set_string("password",(char *)password);
    if(err != ESP_OK) return false;
    err = NixieStorage::instance().open("wificonfig").nvsHandler->commit();
    if(err == ESP_OK) ESP_LOGI(TAG, "Wifi Config Saved!\n");
    else return false;
    return true;
}

void NixieNetwork::networkHandler(void* arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void* event_data)
{
    //esp_wifi_start后
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {       
        bzero(&wifi_config_stored, sizeof(wifi_config_t));//清空wifi_config
        uint8_t ssid[32],password[64];
        hasStoredConfig = readStoredWifiConfig(ssid,password);
        if(hasStoredConfig){
            ESP_LOGI(TAG,"Detected Wi-Fi Config Stored. SmartConfig will Closed in 10s.\n");
            memcpy(wifi_config_stored.sta.ssid, ssid, sizeof(wifi_config_stored.sta.ssid));//复制ssid
            memcpy(wifi_config_stored.sta.password, password, sizeof(wifi_config_stored.sta.password));//复制密码
            //启动AutoClose定时器
            xTimerStart(auto_close_timer,0);
        }else{
            ESP_LOGI(TAG,"No Wi-Fi Config Stored. Try SmartConfig.\n");
        }
        //启动SmartConfig任务
        xTaskCreate([](void * p){
                                NixieNetwork::instance().networkSmartConfigTask(p);
                                },
             "networkSmartConfigTask", 4096, NULL, 3, NULL);
    }
    //wifi断开连接后
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    }
    //wifi成功连接，并且获取到了IP地址
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
        saveStoredWifiConfig(wifi_config_stored.sta.ssid,wifi_config_stored.sta.password);
        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(sta_netif, &ip_info);
        printf("My IP: " IPSTR "\n", IP2STR(&ip_info.ip));
        this->onIpGotHandler(ip_info.ip);
    } 
    //SmartConfig扫描完成
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "Scan done");
    }
    //SmartConfig发现Channel
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG, "Found channel, Delete Auto Close Timer.\n");
        if(xTimerIsTimerActive(auto_close_timer) != pdFALSE)
            xTimerStop(auto_close_timer,0);
    }
    //SmartConfig获取SSID和密码
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t & wifi_config = wifi_config_stored;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));//清空wifi_config
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));//复制ssid
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));//复制密码
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));//复制bssid（如果有）
        }
        //打印ssid和密码
        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);

        if (evt->type == SC_TYPE_ESPTOUCH_V2) { //如果手机上用的软件是EspTouch v2
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i=0; i<33; i++) {
                ESP_LOGI(TAG,"%02x ", rvd_data[i]);
            }
            ESP_LOGI(TAG,"\n");
        }

        ( esp_wifi_disconnect() );//断开连接
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );//重新配置
        esp_wifi_connect();//重新连接
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

void NixieNetwork::sc_auto_close_callback(TimerHandle_t xTimer)
{
    xTimerStop(auto_close_timer,0);
    ESP_LOGI(TAG, "Smart Config Timeout, use stored Wi-Fi config.");
    xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    ESP_LOGI(TAG, "Smart Config Terminated by Auto Close Timer");
    ( esp_wifi_disconnect() );//断开连接
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config_stored) );//重新配置
    esp_wifi_connect();//重新连接
}

NixieNetwork::NixieNetwork()
{
    this->auto_close_timer = xTimerCreate(
        "smartConfigAutoCloseTimer",
        pdMS_TO_TICKS(10000),
        pdFALSE,
        ( void * ) 0 ,
        [](TimerHandle_t xTimer){
            NixieNetwork::instance().sc_auto_close_callback(xTimer);
        }
    );
}
