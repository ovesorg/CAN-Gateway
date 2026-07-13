#include "sif.h"

#define SIF_SYNC 35

static uint8_t sif_sync_tosc = 0;
static uint8_t sif_send_tosc = 0;
static uint8_t length = 11;
static int8_t bit_cnt = 7;
static uint8_t byte_cnt = 0;
uint8_t result[12] = {0x08, 0x61, 0x00, 0x10, 0x01, 0x00, 0x80, 0x00, 0x10, 0x20, 0x30, 0};
uint8_t state_mode = 0;

Da da = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void displayParking() // 第一字节内容
{
    if (da.reverse == 0) // P挡不显示
    {
        if (da.parking == 0) // R挡不显示
        {
            if (da.soc <= 20) // 20%低电量显示
            {
                result[2] = 0x04;
            }
            else
            {
                result[2] = 0x00;
            }
        }
        else // R挡显示
        {
            if (da.soc <= 20)
            {
                result[2] = 0xC;
            }
            else
            {
                result[2] = 0x08;
            }
        }
    }

    else // P挡显示
    {

        if (da.soc <= 20) // 20%低电量显示M N
        {
            result[2] = 0x05;
        }
        else
        {
            result[2] = 0x01;
        }
    }
}

// void displayReverse() // 第一字节内容
//{
//     if (da.reverse == 0) // R挡不显示
//     {
//         if (da.soc <= 20) // 20%低电量显示
//         {
//             result[2] = 0x04;
//         }
//         else
//         {
//             result[2] = 0x00;
//         }
//     }
//     else // R挡显示
//     {
//         if (da.soc <= 20)
//         {
//             result[2] = 0xC;
//         }
//         else
//         {
//             result[2] = 0x08;
//         }
//     }
// }

void EcuandElectricalandHandle() // 第二字节内容
{
    if (da.Ecu == 0)
    {
        if (da.handle == 0)
        {
            if (da.electrical == 0)
            {
                result[3] = 0x00;
            }
            else
            {
                result[3] = 0x40;
            }
        }
        else
        {
            if (da.electrical == 0)
            {
                result[3] = 0x20;
            }
            else
            {
                result[3] = 0x60;
            }
        }
    }
    else
    {
        if (da.handle == 0)
        {
            if (da.electrical == 0)
            {
                result[3] = 0x10;
            }
            else
            {
                result[3] = 0x50;
            }
        }
        else
        {
            if (da.electrical == 0)
            {
                result[3] = 0x30;
            }
            else
            {
                result[3] = 0x70;
            }
        }
    }
}

void checkGearlevel() // 第三字节内容
{
    if (da.check == 0)
    {
        switch (da.gearlevel)
        {
        case 1:
            result[4] = 0x01;
            break;
        case 2:
            result[4] = 0x02;
            break;
        case 3:
            result[4] = 0x03;
            break;
        }
    }
    else
    {
        switch (da.gearlevel)
        {
        case 1:
            result[4] = 0x21;
            break;
        case 2:
            result[4] = 0x22;
            break;
        case 3:
            result[4] = 0x23;
            break;
        }
    }
}

void displayReady() // 第四字节内容
{
    if (da.handle || da.Ecu || da.electrical || da.speed || da.parking || da.check)
    {
        result[5] = 0x00;
    }
    else
    {
        result[5] = 0x01;
    }
}

void displaySpeed() // 第七字节内容
{
    result[8] = da.speed;
}

void displaySoc() // 第八字节内容
{
    result[9] = da.soc;
}

void displayVolts() // 第九字节内容
{
    result[10] = da.volts;
}

void displayInit()
{
    EcuandElectricalandHandle();
    checkGearlevel();
    displaySpeed();
    displaySoc();
    displayVolts();
    displayParking();
    // displayReverse();
    displayReady(); 
    Sum_Handle();
	
//    printf("da.check = %d\n", da.check);
//    printf("result[4] =  %d\n", result[4]);
//    printf("da.paking = %d\n",da.parking);
//	  printf("da.check = %d",da.check);
//	  printf("da.handle = %d",da.handle);
//	  printf("da.Ecu = %d",da.Ecu);
//	  printf("da.electrical = %d",da.electrical);
//	  printf("da.speed = %d",da.speed);
//	  printf("da.parking = %d",da.parking);
//	  printf("da.reverse = %d",da.reverse);
//	  printf("da.gearlevel = %d",da.gearlevel);
//	  printf("da.socsoc = %d",da.soc);
//	  printf("da.voltsvolts = %d",da.volts);
}

void SetDashBoardData(uint8_t state, uint8_t date)
{
    switch (state)
    {
    case LIST_check:
        da.check = date;
        break;
    case LIST_handle:
        da.handle = date;
        break;
    case LIST_Ecu:
        da.Ecu = date;
        break;
    case LIST_electrical:
        da.electrical = date;
        break;
    case LIST_speed:
        da.speed = date;
        break;
    case LIST_parking:
        da.parking = date;
        break;
    case LIST_reverse:
        da.reverse = date;
        break;
    case LIST_gearlevel:
        da.gearlevel = date;
        break;
    case LIST_soc:
        da.soc = date;
        break;
    case LIST_volts:
        da.volts = date;
        break;
    default:
        break;
    }
}

void Sif_off()
{
    gpio_bit_write(GPIOC, GPIO_PIN_0, SET);
}

void Sif_on()
{
    gpio_bit_write(GPIOC, GPIO_PIN_0, RESET);
}

void Sum_Handle(void)
{
    unsigned char i = 0;
    unsigned char checkXor = 0;
    for (i = 0; i < (sizeof(result) / sizeof(result[0])) - 1; i++)
    {
        checkXor = checkXor ^ result[i];
    }
    result[11] = checkXor;
}

void Sif_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_0);
    Sum_Handle();
    Sif_off();
    state_mode = SYNC_SIGNAL;
    timer1_config();
}

void sif_send_data_handle(uint8_t state)
{

    uint8_t data;
    // uint8_t *p = (uint8_t *)result;
    data = (result[byte_cnt] >> bit_cnt) & 0x1;

    switch (state)
    {

    case SYNC_SIGNAL: // ?????
        if (sif_sync_tosc < SIF_SYNC - 1)
        {

            sif_sync_tosc++;
        }
        else if (sif_sync_tosc == SIF_SYNC - 1)
        {
            Sif_on();
            sif_sync_tosc++;
        }
        if (sif_sync_tosc >= SIF_SYNC)
        {
            state_mode = SEND_DATA;
            sif_sync_tosc = 0;
            bit_cnt = 7;
            byte_cnt = 0;
            sif_send_tosc = 0;
        }

        break;

    case SEND_DATA: // ????????
        if (data)
        {
            if (sif_send_tosc == 0)
            {
                Sif_off();
                sif_send_tosc++;
            }
            else if (sif_send_tosc == 1)
            {
                Sif_on();
                sif_send_tosc++;
            }
            else if (sif_send_tosc == 2)
            {
                Sif_on();
                sif_send_tosc = 0;
                bit_cnt--;
                if (bit_cnt < 0)
                {
                    // printf("bit_cnt = %d",bit_cnt);
                    byte_cnt++;
                    bit_cnt = 7;
                }
                if (byte_cnt > length)
                {

                    // printf("byte_cnt = %d",byte_cnt);
                    state_mode = SEND_DATA_COMPLET;
                    break;
                }
            }
        }
        else
        {
            if (sif_send_tosc == 0)
            {
                Sif_off();
                sif_send_tosc++;
            }
            else if (sif_send_tosc == 1)
            {
                Sif_off();
                sif_send_tosc++;
            }
            else if (sif_send_tosc == 2)
            {
                Sif_on();
                sif_send_tosc = 0;
                bit_cnt--;
                if (bit_cnt < 0)
                { // printf("bit_cnt = %d",bit_cnt);
                    byte_cnt++;
                    bit_cnt = 7;
                }
                if (byte_cnt > length)
                {
                    // printf("byte_cnt = %d",byte_cnt);
                    state_mode = SEND_DATA_COMPLET;
                    break;
                }
            }
        }
        break;
    case SEND_DATA_COMPLET: // ????????????????????0
    {
        state_mode = 0;
        Sif_off();
        byte_cnt = 0;
        bit_cnt = 7;
			  // displayInit();
    }
    break;

    default:
        break;
    }
}

void timer1_config(void)
{
    timer_parameter_struct timer_initpara;
    rcu_periph_clock_enable(RCU_TIMER1);

    timer_deinit(TIMER1);
    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler = 270 - 1;
    timer_initpara.period = 200 - 1;
    timer_initpara.counterdirection = TIMER_COUNTER_UP;
    timer_init(TIMER1, &timer_initpara);
    timer_interrupt_enable(TIMER1, TIMER_INT_UP);
    timer_enable(TIMER1);
    nvic_irq_enable(TIMER1_IRQn, 0, 0);
}
