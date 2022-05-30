#include "nixieclock.h"

#define SDA         GPIO_NUM_21
#define SCL         GPIO_NUM_22
#define RGB_DATA    GPIO_NUM_4
#define HV_EN       GPIO_NUM_16
#define DISP_DATA   GPIO_NUM_17
#define DISP_LATCH  GPIO_NUM_18
#define DISP_SHIFT  GPIO_NUM_19
#define DISP_RST_N  GPIO_NUM_23
#define LED_ONBOARD GPIO_NUM_2
#define MIC_ADC     GPIO_NUM_34

NixieTube NixieClock::tube = NixieTube(HV_EN,DISP_DATA,DISP_SHIFT,DISP_LATCH,DISP_RST_N);
SimpleLED  NixieClock::onboard_led = SimpleLED(LED_ONBOARD);
RtcTime NixieClock::rtc = RtcTime(SDA,SCL);
NtpTime NixieClock::ntp = NtpTime();
TaskHandle_t NixieClock::nixieShowCurrentTimeTaskHandler;
TaskHandle_t NixieClock::nixieConfigIndicationTaskHandler;

void NixieClock::init(){
    NixieStorage::instance();
    NixieNetwork::instance().init();
    NixieNetwork::instance().setOnIpGotHandler([](esp_ip4_addr_t ip){
        vTaskSuspend(nixieConfigIndicationTaskHandler);
        vTaskDelete(nixieConfigIndicationTaskHandler);
        uint8_t IP_Got[4] = {esp_ip4_addr1(&ip),esp_ip4_addr2(&ip),esp_ip4_addr3(&ip),esp_ip4_addr4(&ip)};
        tube.hide();
        vTaskDelay(pdMS_TO_TICKS(1000));
        tube.showIP(IP_Got);
        tube.hide();
        vTaskDelay(pdMS_TO_TICKS(1000));
        vTaskResume(nixieShowCurrentTimeTaskHandler);
        ntp.init();
        ntp.setOnObtainedHandler([](struct timeval *tv)
        {
            setenv("TZ", "CST-8", 1);
            tzset();
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            std::cout << "NTP time is " << asctime(&timeinfo);
            std::cout << "Sync NTP time to RTC\n";
            rtc.set(timeinfo);
        });
        ntp.obtain();

        NixieMQTT::instance().init("mqtt://rpi");
        NixieMQTT::instance().setOnCommandHandler([=](const char *cmd,int len){
            printf("CMD:%.*s\r\n", len, cmd);
            if(strncmp(cmd,"detoxificate",len)==0){
                vTaskSuspend(nixieShowCurrentTimeTaskHandler);
                vTaskDelay(pdMS_TO_TICKS(1000));
                tube.detoxificate(50);
                vTaskDelay(pdMS_TO_TICKS(1000));
                vTaskResume(nixieShowCurrentTimeTaskHandler);
            }
            else if(strncmp(cmd,"ip",len)==0){
                vTaskSuspend(nixieShowCurrentTimeTaskHandler);
                vTaskDelay(pdMS_TO_TICKS(1000));
                tube.showIP(IP_Got);
                vTaskDelay(pdMS_TO_TICKS(1000));
                vTaskResume(nixieShowCurrentTimeTaskHandler);
            }
            else if(strncmp(cmd,"suspend",len)==0){
                vTaskSuspend(nixieShowCurrentTimeTaskHandler);
            }
            else if(strncmp(cmd,"resume",len)==0){
                vTaskResume(nixieShowCurrentTimeTaskHandler);
            }
        });
    });
}


void NixieClock::run(){
    xTaskCreate(
        nixieShowCurrentTimeTask,
        "nixieShowCurrentTime",
        configMINIMAL_STACK_SIZE * 8, 
        NULL,
        5, 
        &nixieShowCurrentTimeTaskHandler
    );
    vTaskSuspend(nixieShowCurrentTimeTaskHandler);
    xTaskCreate(
        nixieConfigIndicationTask,
        "nixieConfigIndication",
        configMINIMAL_STACK_SIZE * 8, 
        NULL,
        5, 
        &nixieConfigIndicationTaskHandler
    );
}

void NixieClock::nixieConfigIndicationTask(void * p){
    while(1){
        tube.detoxificate(50);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void NixieClock::nixieShowCurrentTimeTask(void * pvParameters)
{
    int detoxificationCounter = 0;
    while(true){
        if(detoxificationCounter++ == 12){
            detoxificationCounter = 0;
            tube.detoxificate(20);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        if(rtc.isRunning()){
            tm * time = rtc.get();
            onboard_led.toggle();
            tube.showTime(time);
            std::cout << "RTC time is " << asctime(time) << "\n";
        }else{
            onboard_led.toggle();
            tube.detoxificate(50);
            std::cout << "RTC not running\n";
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}