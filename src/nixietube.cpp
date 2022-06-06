#include "nixietube.h"

//辉光管字形码
constexpr uint16_t NIXIE_DIGITs_CODE_IN12[10] = {
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

NixieTube::NixieTube(
        const gpio_num_t hv_en_pin,
        const gpio_num_t data_pin,
        const gpio_num_t shift_pin,
        const gpio_num_t latch_pin,
        const gpio_num_t rst_n_pin)
        :
        hv_en_pin_(hv_en_pin),
        sr_data_pin_(data_pin),
        sr_shift_pin_(shift_pin),
        sr_latch_pin_(latch_pin),
        sr_rst_n_pin_(rst_n_pin)
{
    gpio_set_direction(sr_data_pin_,GPIO_MODE_OUTPUT);  
    gpio_set_direction(sr_latch_pin_,GPIO_MODE_OUTPUT);  
    gpio_set_direction(sr_shift_pin_,GPIO_MODE_OUTPUT);  
    gpio_set_direction(sr_rst_n_pin_,GPIO_MODE_OUTPUT); 
    gpio_set_direction(hv_en_pin,GPIO_MODE_OUTPUT);
    #if TUBE_ENABLE_FADE
    InitPWM();
    #else
    gpio_set_direction(hv_en_pin_,GPIO_MODE_OUTPUT);
    #endif
    Disable();
    Reset();
}

void NixieTube::Reset(){
    GpioPullDownOnce(sr_rst_n_pin_);
}

void NixieTube::Enable(){
    #if TUBE_ENABLE_FADE
    ledc_set_fade_with_time(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,8191,20);
    ledc_fade_start(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,LEDC_FADE_NO_WAIT);
    #else
    GpioPullHigh(hv_en_pin_);
    #endif
}

void NixieTube::Disable(){
    #if TUBE_ENABLE_FADE
    ledc_set_fade_with_time(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,0,20);
    ledc_fade_start(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,LEDC_FADE_NO_WAIT);
    #else
    GpioPullLow(hv_en_pin_);
    #endif
}

void NixieTube::Write(uint16_t data16){
  for(int i=0;i<16;i++){
    bool b = data16 & 0b1000000000000000;
    data16 <<= 1;
    gpio_set_level(this->sr_data_pin_, (uint32_t)b);
    GpioPullUpOnce(sr_shift_pin_);
  }
  GpioPullUpOnce(sr_latch_pin_);  
}

void NixieTube::GpioPullDownOnce(gpio_num_t pin)
{
    gpio_set_level(pin,0);
    gpio_set_level(pin,1);
}

void NixieTube::GpioPullUpOnce(gpio_num_t pin)
{
    gpio_set_level(pin,1);
    gpio_set_level(pin,0);
}

void NixieTube::GpioPullHigh(gpio_num_t pin)
{
    gpio_set_level(pin,1);
}

void NixieTube::GpioPullLow(gpio_num_t pin)
{
    gpio_set_level(pin,0);
}

#if TUBE_ENABLE_FADE
void NixieTube::InitPWM()
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
    ledc_fade_func_install(ledc_intr_flag_);
}
#endif

void NixieTube::Show(){
    Reset();
    Enable();
    Write(DIGITs_CODE[digit_]);
}

void NixieTube::Show(const uint8_t & digit)
{
    SetDigit(digit);
    Reset();
    Enable();
    Write(DIGITs_CODE[digit_]);
}

void NixieTube::Hide(){
    Reset();
    Write(0);
    Disable();
}

#if TUBE_ENABLE_FADE
void NixieTube::FadeIn()
{
    Reset();
    Write(DIGITs_CODE[digit_]);
    ledc_set_fade_with_time(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,8191,500);
    ledc_fade_start(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,LEDC_FADE_WAIT_DONE);
}


void NixieTube::FadeIn(const uint8_t & digit)
{
    SetDigit(digit);
    Reset();
    Write(DIGITs_CODE[digit_]);
    ledc_set_fade_with_time(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,8191,500);
    ledc_fade_start(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,LEDC_FADE_WAIT_DONE);
}

void NixieTube::FadeOut()
{
    ledc_set_fade_with_time(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,0,300);
    ledc_fade_start(LEDC_LS_MODE,LEDC_LS_CH0_CHANNEL,LEDC_FADE_WAIT_DONE);
    Reset();
    Write(0);
}
#endif

void NixieTube::Detoxificate(int n){
    uint8_t last = 0;
    uint8_t x = 0;
    for(int i = 0;i<n;i++){
        while(x == last){
            x = rand()%10;
        }
        last = x;
        Show(x);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    #if TUBE_ENABLE_FADE
    FadeOut();
    #else
    Hide();
    #endif
}

void NixieTube::ShowTime(const time_t *time){
    ShowTime(gmtime(time));
}

#if TUBE_ENABLE_FADE

void NixieTube::ShowTime(const tm *time)
{
    FadeIn(time->tm_hour/10);
    FadeOut();
    FadeIn(time->tm_hour%10);
    FadeOut();
    vTaskDelay(pdMS_TO_TICKS(300));
    FadeIn(time->tm_min/10);
    FadeOut();
    FadeIn(time->tm_min%10);
    FadeOut();
}

void NixieTube::ShowIP(const uint8_t ip[4])
{
    for(int i=0;i<4;i++){
        const uint8_t & num = ip[i];
        uint8_t h,m,l; 
        h = num/100;
        if(h != 0){
            FadeIn(h);
            FadeOut();
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        m = (num%100)/10;
        if(m != 0 || h != 0){
            FadeIn(m);
            FadeOut();
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        l = num%10;
        FadeIn(l);
        vTaskDelay(pdMS_TO_TICKS(500));
        FadeOut();
    }
}

#else

void NixieTube::ShowTime(const tm *time)
{
    Show(time->tm_hour/10);
    vTaskDelay(pdMS_TO_TICKS(500));
    Hide();
    vTaskDelay(pdMS_TO_TICKS(100));
    Show(time->tm_hour%10);
    vTaskDelay(pdMS_TO_TICKS(500));
    Hide();
    vTaskDelay(pdMS_TO_TICKS(300));
    Show(time->tm_min/10);
    vTaskDelay(pdMS_TO_TICKS(500));
    Hide();
    vTaskDelay(pdMS_TO_TICKS(100));
    Show(time->tm_min%10);
    vTaskDelay(pdMS_TO_TICKS(500));
    Hide();
}

void NixieTube::ShowIP(const uint8_t ip[4])
{
    for(int i=0;i<4;i++){
        const uint8_t & num = ip[i];
        uint8_t h,m,l; 
        h = num/100;
        if(h != 0){
            Show(h);
            vTaskDelay(pdMS_TO_TICKS(300));
            Hide();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        m = (num%100)/10;
        if(m != 0 || h != 0){
            Show(m);
            vTaskDelay(pdMS_TO_TICKS(300));
            Hide();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        l = num%10;
        Show(l);
        vTaskDelay(pdMS_TO_TICKS(300));
        Hide();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

#endif