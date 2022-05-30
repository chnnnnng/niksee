#include "nixietube.h"

//辉光管字形码
const uint16_t NIXIE_DIGITs_CODE_IN12[10] = {
    0b0000000000000001,
    0b0000000000000010,
    0b0000000000000100,
    0b0000000000001000,
    0b0000000000010000,
    0b0000000000100000,
    0b0000000001000000,
    0b0000000010000000,
    0b0000000100000000,
    0b0000001000000000
};

#define DIGITs_CODE NIXIE_DIGITs_CODE_IN12

NixieTube::NixieTube(const gpio_num_t hv_en_pin,
        const gpio_num_t data_pin,
        const gpio_num_t shift_pin,
        const gpio_num_t latch_pin,
        const gpio_num_t rst_n_pin)
{
    this->hv_en_pin = hv_en_pin;
    this->sr_data_pin = data_pin;
    this->sr_shift_pin = shift_pin;
    this->sr_latch_pin = latch_pin;
    this->sr_rst_n_pin = rst_n_pin;

    gpio_set_direction(sr_data_pin,GPIO_MODE_OUTPUT);  
    gpio_set_direction(sr_latch_pin,GPIO_MODE_OUTPUT);  
    gpio_set_direction(sr_shift_pin,GPIO_MODE_OUTPUT);  
    gpio_set_direction(sr_rst_n_pin,GPIO_MODE_OUTPUT); 
    gpio_set_direction(hv_en_pin,GPIO_MODE_OUTPUT);

    this->pwmInit();

    this->disable();
    this->reset();
}

void NixieTube::reset(){
    this->gpio_pull_down_once(this->sr_rst_n_pin);
}

void NixieTube::enable(){
    ledc_set_fade_with_time(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,8191,20);
    ledc_fade_start(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,LEDC_FADE_NO_WAIT);
}

void NixieTube::disable(){
    ledc_set_fade_with_time(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,0,20);
    ledc_fade_start(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,LEDC_FADE_NO_WAIT);
}

void NixieTube::write_16b(uint16_t data16){
  for(int i=0;i<16;i++){
    bool b = data16 & 0b1000000000000000;
    data16 <<= 1;
    gpio_set_level(this->sr_data_pin,(uint32_t)b);
    this->gpio_pull_up_once(this->sr_shift_pin);
  }  
  this->gpio_pull_up_once(this->sr_latch_pin);
}

void NixieTube::gpio_pull_down_once(gpio_num_t pin)
{
    gpio_set_level(pin,0);
    gpio_set_level(pin,1);
}

void NixieTube::gpio_pull_up_once(gpio_num_t pin)
{
    gpio_set_level(pin,1);
    gpio_set_level(pin,0);
}

void NixieTube::gpio_pull_high(gpio_num_t pin)
{
    gpio_set_level(pin,1);
}

void NixieTube::gpio_pull_low(gpio_num_t pin)
{
    gpio_set_level(pin,0);
}

void NixieTube::pwmInit()
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .timer_num = LEDC_LS_TIMER,            // timer index
        .freq_hz = 5000,                      // frequency of PWM signal 
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
        {
            .gpio_num   = LEDC_LS_CH0_GPIO,
            .speed_mode = LEDC_LS_MODE,
            .channel    = LEDC_LS_CH0_CHANNEL,
            .intr_type  = LEDC_INTR_DISABLE,
            .timer_sel  = LEDC_LS_TIMER,
            .duty       = 0,
            .hpoint     = 0,
        }
    };
    for (int ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }
    ledc_fade_func_install(ledc_intr_flag);
}

void NixieTube::show(){
    reset();
    enable();
    write_16b(DIGITs_CODE[this->digit]);
}

void NixieTube::show(const uint8_t & digit)
{
    this->setDigit(digit);
    reset();
    enable();
    write_16b(DIGITs_CODE[this->digit]);
}

void NixieTube::hide(){
    reset();
    write_16b(0);
    disable();
}

void NixieTube::fadeIn()
{
    reset();
    write_16b(DIGITs_CODE[this->digit]);
    ledc_set_fade_with_time(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,8191,500);
    ledc_fade_start(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,LEDC_FADE_WAIT_DONE);
}

void NixieTube::fadeIn(const uint8_t & digit)
{
    this->setDigit(digit);
    reset();
    write_16b(DIGITs_CODE[this->digit]);
    ledc_set_fade_with_time(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,8191,500);
    ledc_fade_start(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,LEDC_FADE_WAIT_DONE);
}

void NixieTube::fadeOut()
{
    ledc_set_fade_with_time(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,0,300);
    ledc_fade_start(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,LEDC_FADE_WAIT_DONE);
    reset();
    write_16b(0);
}


void NixieTube::detoxificate(int n){
    uint8_t last = 0;
    uint8_t x = 0;
    for(int i = 0;i<n;i++){
        while(x == last){
            x = rand()%10;
        }
        last = x;
        show(x);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    fadeOut();
}

void NixieTube::showTime(const time_t *time){
    this->showTime(gmtime(time));
}

void NixieTube::showTime(const tm *time)
{
    if(showTimeLock) return;
    showTimeLock = true;
    fadeIn(time->tm_hour/10);
    //vTaskDelay(pdMS_TO_TICKS(500));
    fadeOut();
    //vTaskDelay(pdMS_TO_TICKS(100));
    fadeIn(time->tm_hour%10);
    //vTaskDelay(pdMS_TO_TICKS(500));
    fadeOut();
    vTaskDelay(pdMS_TO_TICKS(300));
    fadeIn(time->tm_min/10);
    //vTaskDelay(pdMS_TO_TICKS(500));
    fadeOut();
    //vTaskDelay(pdMS_TO_TICKS(100));
    fadeIn(time->tm_min%10);
    //vTaskDelay(pdMS_TO_TICKS(500));
    fadeOut();
    showTimeLock = false;
}

void NixieTube::showIP(const uint8_t ip[4])
{
    for(int i=0;i<4;i++){
        const uint8_t & num = ip[i];
        uint8_t h,m,l; 
        h = num/100;
        if(h != 0){
            fadeIn(h);
            //vTaskDelay(pdMS_TO_TICKS(500));
            fadeOut();
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        m = (num%100)/10;
        if(m != 0 || h != 0){
            fadeIn(m);
            //vTaskDelay(pdMS_TO_TICKS(500));
            fadeOut();
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        l = num%10;
        fadeIn(l);
        vTaskDelay(pdMS_TO_TICKS(500));
        fadeOut();
    }
}