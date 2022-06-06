#include "nixieclock.h"

constexpr gpio_num_t    SDA         =   GPIO_NUM_21;
constexpr gpio_num_t    SCL         =   GPIO_NUM_22;
constexpr gpio_num_t    RGB_DATA    =   GPIO_NUM_4;
constexpr gpio_num_t    HV_EN       =   GPIO_NUM_16;
constexpr gpio_num_t    DISP_DATA   =   GPIO_NUM_17;
constexpr gpio_num_t    DISP_LATCH  =   GPIO_NUM_18;
constexpr gpio_num_t    DISP_SHIFT  =   GPIO_NUM_19;
constexpr gpio_num_t    DISP_RST_N  =   GPIO_NUM_23;
constexpr gpio_num_t    LED_ONBOARD =   GPIO_NUM_2;
constexpr gpio_num_t    MIC_ADC     =   GPIO_NUM_34;

NixieTube       tube            = NixieTube(HV_EN,DISP_DATA,DISP_SHIFT,DISP_LATCH,DISP_RST_N);
SimpleLED       onboard_led     = SimpleLED(LED_ONBOARD);
RtcTime&        rtc             = RtcTime::instance(SDA,SCL);
NtpTime&        ntp             = NtpTime::instance();
NixieStorage&   storage         = NixieStorage::instance();
NixieNetwork&   network         = NixieNetwork::instance();
NixieMQTT&      mqtt            = NixieMQTT::instance();

TaskHandle_t nixie_show_time_task_handle;
TaskHandle_t nixie_show_config_indication_task_handle;


void NixieInit(void){
    uint8_t IP_Got[4];
    network.setOnIpGotHandler([&IP_Got](esp_ip4_addr_t ip){
        vTaskSuspend(nixie_show_config_indication_task_handle);
        vTaskDelete(nixie_show_config_indication_task_handle);
        IP_Got[0] = esp_ip4_addr1(&ip);
        IP_Got[1] = esp_ip4_addr2(&ip);
        IP_Got[2] = esp_ip4_addr3(&ip);
        IP_Got[3] = esp_ip4_addr4(&ip);
        tube.Hide();
        vTaskDelay(pdMS_TO_TICKS(1000));
        tube.ShowIP(IP_Got);
        tube.Hide();
        vTaskDelay(pdMS_TO_TICKS(1000));        
        
        vTaskResume(nixie_show_time_task_handle);
        ntp.Init();
        //ntp.Obtain();
    });

    ntp.SetOnObtainedHandler([](struct timeval *tv)
    {
        setenv("TZ", "CST-8", 1);
        tzset();
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        std::cout << "NTP time is " << asctime(&timeinfo);
        std::cout << "Sync NTP time to RTC\n";
        rtc.Set(timeinfo);

        mqtt.init("mqtt://rpi");
    });
    NixieMQTT::instance().setOnCommandHandler([=](const char *cmd,int len)
    {
        printf("CMD:%.*s\r\n", len, cmd);
        if(strncmp(cmd,"detoxificate",len)==0){
            vTaskSuspend(nixie_show_time_task_handle);
            vTaskDelay(pdMS_TO_TICKS(1000));
            tube.Detoxificate(50);
            vTaskDelay(pdMS_TO_TICKS(1000));
            vTaskResume(nixie_show_time_task_handle);
        }
        else if(strncmp(cmd,"ip",len)==0){
            vTaskSuspend(nixie_show_time_task_handle);
            vTaskDelay(pdMS_TO_TICKS(1000));
            tube.ShowIP(IP_Got);
            vTaskDelay(pdMS_TO_TICKS(1000));
            vTaskResume(nixie_show_time_task_handle);
        }
        else if(strncmp(cmd,"suspend",len)==0){
            vTaskSuspend(nixie_show_time_task_handle);
        }
        else if(strncmp(cmd,"resume",len)==0){
            vTaskResume(nixie_show_time_task_handle);
        }
    });
    network.init();
}


void NixieRun()
{
    xTaskCreate(
        NixieShowConfigIndicationTask,
        "nixieConfigIndication",
        configMINIMAL_STACK_SIZE * 8, 
        NULL,
        5, 
        &nixie_show_config_indication_task_handle
    );
    xTaskCreate(
        NixieShowTimeTask,
        "nixieShowCurrentTime",
        configMINIMAL_STACK_SIZE * 8, 
        NULL,
        5, 
        &nixie_show_time_task_handle
    );
    vTaskSuspend(nixie_show_time_task_handle);
}


void NixieShowConfigIndicationTask(void *p){
    while(1){
        tube.Detoxificate(50);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void NixieShowTimeTask(void *p)
{
    int detoxificationCounter = 0;
    while(1){
        if(detoxificationCounter++ == 12){
            detoxificationCounter = 0;
            tube.Detoxificate(20);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        if(rtc.IsRunning()){
            tm * time = rtc.Get();
            onboard_led.Toggle();
            tube.ShowTime(time);
        }else{
            onboard_led.Toggle();
            tube.Detoxificate(50);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}