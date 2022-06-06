#include "nixietime.h"
RtcTime::RtcTime(const gpio_num_t sda,const gpio_num_t scl)
{
    sda_ = sda;
    scl_ = scl;
    ESP_ERROR_CHECK(i2cdev_init());
    memset(&i2c_dev_, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(ds1307_init_desc(&i2c_dev_,i2c_port_,sda_,scl_));
}

void RtcTime::Deinit()
{
    ESP_ERROR_CHECK(ds1307_free_desc(&i2c_dev_));
}

bool RtcTime::IsRunning()
{
    ESP_ERROR_CHECK(ds1307_is_running(&i2c_dev_,&is_running_));
    ESP_LOGI(TAG_, "%s", (is_running_?"RTC is running.":"RTC is not running."));
    return is_running_;
}

bool RtcTime::Set(tm time)
{
    ESP_LOGI(TAG_, "set: %s" ,asctime(&time));
    return (ESP_OK == ds1307_set_time(&i2c_dev_,&time));
}

tm * RtcTime::Get()
{
    ESP_ERROR_CHECK(ds1307_get_time(&i2c_dev_,&time_));
    ESP_LOGI(TAG_, "now: %s" ,asctime(&time_));
    return &time_;
}

//////////////////////////////////////////////////////////////////////
NtpTime::NtpTime(){
    
}


//init之后，将会周期性自动从ntp服务器校准时间，触发onObtain回调
void NtpTime::Init(){
    ESP_LOGI(TAG_, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "time1.aliyun.com");
    sntp_setservername(1, "time.nist.gov");
    sntp_set_time_sync_notification_cb([](struct timeval * t){
        NtpTime::instance().on_obtain_handler_(t);
    });
    sntp_init();
    ESP_LOGI(TAG_, "List of configured NTP servers:");
    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i){
        if (sntp_getservername(i))
            ESP_LOGI(TAG_, "server %d: %s", i, sntp_getservername(i));
    }
}

//立即校准时间，触发onObtain回调
void NtpTime::Obtain(){
    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG_, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}