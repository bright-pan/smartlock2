/*
 * File      : usart.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#ifndef __USART_H__
#define __USART_H__

#include <rthw.h>
#include <rtthread.h>
#include "stm32f10x.h"

#define UART_ENABLE_IRQ(n)            NVIC_EnableIRQ((n))
#define UART_DISABLE_IRQ(n)           NVIC_DisableIRQ((n))

#define RT_DEVICE_CTRL_CLR_TX_GPIO 0x15
#define RT_DEVICE_CTRL_SET_TX_GPIO 0x16
#define RT_DEVICE_SET_USART_RX_PIN 0x17
#define RT_DEVICE_INTERRUPT_RX_PIN 0x18

/* STM32 uart driver */
struct stm32_uart
{
    rt_mutex_t lock;
    
    USART_TypeDef* uart_device;
    IRQn_Type irq;

    uint16_t uart_tx_pin;
    GPIO_TypeDef *uart_tx_gpio;

    uint16_t uart_rx_pin;
    GPIO_TypeDef *uart_rx_gpio;

    uint16_t uart_cts_pin;
    GPIO_TypeDef *uart_cts_gpio;

    uint16_t uart_rts_pin;
    GPIO_TypeDef *uart_rts_gpio;

    uint32_t uart_remap;

};

void rt_hw_usart_init(void);

#endif
