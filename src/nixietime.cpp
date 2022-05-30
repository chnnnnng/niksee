#include "nixietime.h"
RtcTime::RtcTime(
        const gpio_num_t sda, 
        const gpio_num_t scl)
{
    this->_sda = sda;
    this->_scl = scl;
    ESP_ERROR_CHECK(i2cdev_init());
    memset(&_i2c_dev, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(ds1307_init_desc(&_i2c_dev,_i2c_port,_sda,_scl));
}

RtcTime::~RtcTime()
{
    ESP_ERROR_CHECK(ds1307_free_desc(&_i2c_dev));
}

bool RtcTime::isRunning()
{
    ESP_ERROR_CHECK(ds1307_is_running(&_i2c_dev,&_isRunning));
    return _isRunning;
}

bool RtcTime::set(tm time)
{
    return (ESP_OK == ds1307_set_time(&_i2c_dev,&time));
}

tm * RtcTime::get()
{
    ESP_ERROR_CHECK(ds1307_get_time(&_i2c_dev,&_time));
    return &_time;
}

//////////////////////////////////////////////////////////////////////


const char * NtpTime::TAG = "NtpTime";
std::function<void(struct timeval *)> NtpTime::onObtainedHandler;

//init之后，将会周期性自动从ntp服务器校准时间，触发onObtain回调
void NtpTime::init(){
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "time1.aliyun.com");
    sntp_setservername(1, "time.nist.gov");
    sntp_set_time_sync_notification_cb([](struct timeval * t){onObtainedHandler(t);});
    sntp_init();
    ESP_LOGI(TAG, "List of configured NTP servers:");
    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i){
        if (sntp_getservername(i))
            ESP_LOGI(TAG, "server %d: %s", i, sntp_getservername(i));
    }
}

//立即校准时间，触发onObtain回调
void NtpTime::obtain(){
    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}