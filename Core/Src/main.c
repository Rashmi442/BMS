#include "main.h"
#include <stdio.h>
#include <string.h>

/* Handles */
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;

/* Constants */
#define ADC_REF 3.3f
#define VOLT_DIV_RATIO 2.0f
#define CAL_FACTOR 0.80f
#define ACS_SENSITIVITY 0.100f
#define LCD_ADDR (0x27 << 1)

/* Variables */
float vBat, tempC, currA;
float acs_offset = 2.5f;
int pct, alert_toggle = 0;

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);

/* LCD Functions */
void lcd_send_cmd(char cmd);
void lcd_send_data(char data);
void lcd_init(void);
void lcd_send_string(char *str);
void lcd_put_cur(int row, int col);

/* ADC */
uint32_t Read_ADC_Avg(uint32_t channel, int samples);

/* SoC */
int calc_soc(float v) {
    if (v >= 4.15f) return 100;
    if (v <= 3.10f) return 0;
    if (v > 3.85f) return (int)((v - 3.85f) * 83.3f + 75);
    if (v > 3.65f) return (int)((v - 3.65f) * 200.0f + 35);
    return (int)((v - 3.10f) * 63.0f);
}

/* MAIN */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();

    lcd_init();
    lcd_put_cur(0,0);
    lcd_send_string("BMS STARTING...");
    HAL_Delay(1000);

    /* ACS Zero Calibration */
    float sum = 0;
    for(int i=0;i<50;i++){
        sum += ((float)Read_ADC_Avg(ADC_CHANNEL_4,1)*ADC_REF)/4095.0f;
        HAL_Delay(5);
    }
    acs_offset = sum/50;

    while (1)
    {
        /* -------- Voltage -------- */
        uint32_t rawV = Read_ADC_Avg(ADC_CHANNEL_0, 30);
        float v_adc = (rawV * ADC_REF) / 4095.0f;
        vBat = v_adc * VOLT_DIV_RATIO * CAL_FACTOR;

        /* -------- Temperature -------- */
        uint32_t rawT = Read_ADC_Avg(ADC_CHANNEL_1, 30);
        tempC = (rawT * (ADC_REF * 62.0f)) / 4095.0f;

        /* -------- Current -------- */
        uint32_t rawI = Read_ADC_Avg(ADC_CHANNEL_4, 50);
        float vSensor = (rawI * ADC_REF) / 4095.0f;
        currA = (vSensor - acs_offset) / ACS_SENSITIVITY;

        if(currA < 0.12f && currA > -0.12f)
            currA = 0;

        pct = calc_soc(vBat);

        /* -------- LCD Display -------- */
        char buf1[17], buf2[17];

        if (pct < 30 || tempC > 45.0f)
        {
            if (alert_toggle == 0)
            {
                if (pct < 30)
                    sprintf(buf1, "!!! LOW BATT !!!");
                else
                    sprintf(buf1, "!!! OVERHEAT !!!");

                sprintf(buf2, "V:%.2f SoC:%d%%", vBat, pct);
                alert_toggle = 1;
            }
            else
            {
                sprintf(buf1, "V:%.2f T:%dC", vBat, (int)tempC);
                sprintf(buf2, "I:%.2f SoC:%d%%", currA, pct);
                alert_toggle = 0;
            }
        }
        else
        {
            sprintf(buf1, "V:%.2f T:%dC", vBat, (int)tempC);
            sprintf(buf2, "I:%.2f SoC:%d%%", currA, pct);
        }

        lcd_put_cur(0,0);
        lcd_send_string("                ");
        lcd_put_cur(0,0);
        lcd_send_string(buf1);

        lcd_put_cur(1,0);
        lcd_send_string("                ");
        lcd_put_cur(1,0);
        lcd_send_string(buf2);

        HAL_Delay(1000);
    }
}

/* ADC */
uint32_t Read_ADC_Avg(uint32_t channel, int samples){
    ADC_ChannelConfTypeDef sConfig={0};
    uint32_t sum=0;

    sConfig.Channel=channel;
    sConfig.Rank=1;
    sConfig.SamplingTime=ADC_SAMPLETIME_480CYCLES;

    HAL_ADC_ConfigChannel(&hadc1,&sConfig);

    for(int i=0;i<samples;i++){
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1,10);
        sum+=HAL_ADC_GetValue(&hadc1);
    }
    return sum/samples;
}

/* LCD */
void lcd_send_cmd(char cmd){
    uint8_t d[4];
    d[0]=(cmd&0xF0)|0x0C;
    d[1]=(cmd&0xF0)|0x08;
    d[2]=((cmd<<4)&0xF0)|0x0C;
    d[3]=((cmd<<4)&0xF0)|0x08;
    HAL_I2C_Master_Transmit(&hi2c1,LCD_ADDR,d,4,100);
    HAL_Delay(1);
}

void lcd_send_data(char data){
    uint8_t d[4];
    d[0]=(data&0xF0)|0x0D;
    d[1]=(data&0xF0)|0x09;
    d[2]=((data<<4)&0xF0)|0x0D;
    d[3]=((data<<4)&0xF0)|0x09;
    HAL_I2C_Master_Transmit(&hi2c1,LCD_ADDR,d,4,100);
    HAL_Delay(1);
}

void lcd_init(void){
    HAL_Delay(100);
    lcd_send_cmd(0x30); HAL_Delay(10);
    lcd_send_cmd(0x30); HAL_Delay(1);
    lcd_send_cmd(0x30); HAL_Delay(10);
    lcd_send_cmd(0x20); HAL_Delay(10);
    lcd_send_cmd(0x28);
    lcd_send_cmd(0x0C);
    lcd_send_cmd(0x01);
    HAL_Delay(5);
    lcd_send_cmd(0x06);
}

void lcd_send_string(char *str){
    while(*str) lcd_send_data(*str++);
}

void lcd_put_cur(int row,int col){
    lcd_send_cmd(row==0?(0x80|col):(0xC0|col));
}

/* INITS */
static void MX_ADC1_Init(void){
    hadc1.Instance=ADC1;
    hadc1.Init.Resolution=ADC_RESOLUTION_12B;
    hadc1.Init.ContinuousConvMode=DISABLE;
    hadc1.Init.DataAlign=ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion=1;
    HAL_ADC_Init(&hadc1);
}

static void MX_I2C1_Init(void){
    hi2c1.Instance=I2C1;
    hi2c1.Init.ClockSpeed=100000;
    hi2c1.Init.DutyCycle=I2C_DUTYCYCLE_2;
    hi2c1.Init.AddressingMode=I2C_ADDRESSINGMODE_7BIT;
    HAL_I2C_Init(&hi2c1);
}

static void MX_GPIO_Init(void){
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct={0};

    /* ADC pins */
    GPIO_InitStruct.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4;
    GPIO_InitStruct.Mode=GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

    /* I2C PB8 PB9 */
    GPIO_InitStruct.Pin=GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode=GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull=GPIO_PULLUP;
    GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate=GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);
}

void SystemClock_Config(void){
    RCC_OscInitTypeDef osc={0};
    RCC_ClkInitTypeDef clk={0};

    osc.OscillatorType=RCC_OSCILLATORTYPE_HSI;
    osc.HSIState=RCC_HSI_ON;
    osc.PLL.PLLState=RCC_PLL_ON;
    osc.PLL.PLLSource=RCC_PLLSOURCE_HSI;
    osc.PLL.PLLM=8;
    osc.PLL.PLLN=100;
    osc.PLL.PLLP=RCC_PLLP_DIV2;
    HAL_RCC_OscConfig(&osc);

    clk.ClockType=RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_HCLK;
    clk.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK;
    HAL_RCC_ClockConfig(&clk,FLASH_LATENCY_3);
}

















//
//#include "stm32f4xx.h"
//#include <stdio.h>
//
///* Constants */
//#define ADC_REF 3.3f
//#define VOLT_DIV_RATIO 2.0f
//#define CAL_FACTOR 0.90f
//#define ACS_SENSITIVITY 0.100f
//#define LCD_ADDR 0x4E   // (0x27 << 1)
//
///* Variables */
//float vBat, tempC, currA;
//float acs_offset = 2.5f;
//int pct, alert_toggle = 0;
//
///* Delay */
//void delay_ms(int ms){
//    for(int i=0;i<ms*4000;i++) __NOP();
//}
//
///* ---------- CLOCK ---------- */
//void SystemClock_Config(){
//    RCC->CR |= RCC_CR_HSION;
//    while(!(RCC->CR & RCC_CR_HSIRDY));
//
//    RCC->PLLCFGR = (8 << 0) | (100 << 6) | (0 << 16) | (1 << 22);
//    RCC->CR |= RCC_CR_PLLON;
//    while(!(RCC->CR & RCC_CR_PLLRDY));
//
//    RCC->CFGR |= RCC_CFGR_SW_PLL;
//}
//
///* ---------- GPIO ---------- */
//void GPIO_Init(){
//    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
//
//    /* PA0, PA1, PA4 -> Analog */
//    GPIOA->MODER |= (3<<0) | (3<<2) | (3<<8);
//
//    /* PB8, PB9 -> I2C */
//    GPIOB->MODER |= (2<<16) | (2<<18);
//    GPIOB->OTYPER |= (1<<8) | (1<<9);
//    GPIOB->OSPEEDR |= (3<<16) | (3<<18);
//    GPIOB->PUPDR |= (1<<16) | (1<<18);
//
//    GPIOB->AFR[1] |= (4<<0) | (4<<4);
//}
//
///* ---------- ADC ---------- */
//void ADC_Init(){
//    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
//
//    ADC1->CR2 = 0;
//    ADC1->SQR3 = 0;
//    ADC1->SMPR2 |= (7<<0); // long sample
//
//    ADC1->CR2 |= ADC_CR2_ADON;
//}
//
//uint16_t ADC_Read(int ch){
//    ADC1->SQR3 = ch;
//    ADC1->CR2 |= ADC_CR2_SWSTART;
//
//    while(!(ADC1->SR & ADC_SR_EOC));
//    return ADC1->DR;
//}
//
//uint32_t Read_ADC_Avg(int ch, int samples){
//    uint32_t sum = 0;
//    for(int i=0;i<samples;i++){
//        sum += ADC_Read(ch);
//    }
//    return sum / samples;
//}
//
///* ---------- I2C ---------- */
//void I2C_Init(){
//    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
//
//    I2C1->CR2 = 16; // 16 MHz
//    I2C1->CCR = 80; // 100kHz
//    I2C1->TRISE = 17;
//
//    I2C1->CR1 |= I2C_CR1_PE;
//}
//
//void I2C_Write(uint8_t addr, uint8_t *data, int len){
//    I2C1->CR1 |= I2C_CR1_START;
//    while(!(I2C1->SR1 & I2C_SR1_SB));
//
//    I2C1->DR = addr;
//    while(!(I2C1->SR1 & I2C_SR1_ADDR));
//    (void)I2C1->SR2;
//
//    for(int i=0;i<len;i++){
//        I2C1->DR = data[i];
//        while(!(I2C1->SR1 & I2C_SR1_TXE));
//    }
//
//    I2C1->CR1 |= I2C_CR1_STOP;
//}
//
///* ---------- LCD ---------- */
//void lcd_send_cmd(char cmd){
//    uint8_t d[4];
//    d[0]=(cmd&0xF0)|0x0C;
//    d[1]=(cmd&0xF0)|0x08;
//    d[2]=((cmd<<4)&0xF0)|0x0C;
//    d[3]=((cmd<<4)&0xF0)|0x08;
//    I2C_Write(LCD_ADDR,d,4);
//}
//
//void lcd_send_data(char data){
//    uint8_t d[4];
//    d[0]=(data&0xF0)|0x0D;
//    d[1]=(data&0xF0)|0x09;
//    d[2]=((data<<4)&0xF0)|0x0D;
//    d[3]=((data<<4)&0xF0)|0x09;
//    I2C_Write(LCD_ADDR,d,4);
//}
//
//void lcd_init(){
//    delay_ms(100);
//    lcd_send_cmd(0x30); delay_ms(10);
//    lcd_send_cmd(0x30); delay_ms(1);
//    lcd_send_cmd(0x30); delay_ms(10);
//    lcd_send_cmd(0x20); delay_ms(10);
//    lcd_send_cmd(0x28);
//    lcd_send_cmd(0x0C);
//    lcd_send_cmd(0x01);
//    delay_ms(5);
//    lcd_send_cmd(0x06);
//}
//
//void lcd_string(char *s){
//    while(*s) lcd_send_data(*s++);
//}
//
//void lcd_set(int r,int c){
//    lcd_send_cmd(r==0?(0x80|c):(0xC0|c));
//}
//
///* ---------- SoC ---------- */
//int calc_soc(float v){
//    if (v >= 4.15f) return 100;
//    if (v <= 3.10f) return 0;
//    if (v > 3.85f) return (int)((v - 3.85f) * 83.3f + 75);
//    if (v > 3.65f) return (int)((v - 3.65f) * 200.0f + 35);
//    return (int)((v - 3.10f) * 63.0f);
//}
//
///* ---------- MAIN ---------- */
//int main(){
//
//    SystemClock_Config();
//    GPIO_Init();
//    ADC_Init();
//    I2C_Init();
//    lcd_init();
//
//    char b1[16], b2[16];
//
//    while(1){
//
//        float v_adc = (Read_ADC_Avg(0,30)*ADC_REF)/4095.0f;
//        vBat = v_adc * VOLT_DIV_RATIO * CAL_FACTOR;
//
//        tempC = (Read_ADC_Avg(1,30)*(ADC_REF*50))/4095.0f;
//
//        float vSensor = (Read_ADC_Avg(4,50)*ADC_REF)/4095.0f;
//        currA = (vSensor - acs_offset)/ACS_SENSITIVITY;
//
//        pct = calc_soc(vBat);
//
//        sprintf(b1,"V:%.2f T:%dC",vBat,(int)tempC);
//        sprintf(b2,"I:%.2f SoC:%d%%",currA,pct);
//
//        lcd_set(0,0);
//        lcd_string(b1);
//
//        lcd_set(1,0);
//        lcd_string(b2);
//
//        delay_ms(1000);
//    }
//}
