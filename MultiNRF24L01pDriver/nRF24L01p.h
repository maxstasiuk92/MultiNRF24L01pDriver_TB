/*
 * nRF24L01.h
 *
 *  Created on: 28 ����. 2020 �.
 *      Author: MaxCm
 */
#ifndef _nRF24L01p_h_
#define _nRF24L01p_h_

/* Memory Map */
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define OBSERVE_TX  0x08
#define RPD         0x09
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
#define RX_ADDR_P2  0x0C
#define RX_ADDR_P3  0x0D
#define RX_ADDR_P4  0x0E
#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_Px	0x11
#define RX_PW_P0    0x11
#define RX_PW_P1    0x12
#define RX_PW_P2    0x13
#define RX_PW_P3    0x14
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17
#define DYNPD       0x1C
#define FEATURE     0x1D

/* Bit Mnemonics */

/* configuration register */
#define MASK_RX_DR  (1<<6)
#define MASK_TX_DS  (1<<5)
#define MASK_MAX_RT (1<<4)
#define EN_CRC      (1<<3)
#define CRCO        (1<<2)
#define PWR_UP      (1<<1)
#define PRIM_RX     (1<<0)

/* enable auto acknowledgment */
#define ENAA_P5     (1<<5)
#define ENAA_P4     (1<<4)
#define ENAA_P3     (1<<3)
#define ENAA_P2     (1<<2)
#define ENAA_P1     (1<<1)
#define ENAA_P0     (1<<0)

/* enable rx addresses */
#define ERX_P5      (1<<5)
#define ERX_P4      (1<<4)
#define ERX_P3      (1<<3)
#define ERX_P2      (1<<2)
#define ERX_P1      (1<<1)
#define ERX_P0      (1<<0)

/* setup of address width */
#define AW_BASE		0 /* 2 bits */
#define AW_MASK		(0x03<<AW_BASE)


/* setup of auto re-transmission */
#define ARD_BASE	4 /* 4 bits */
#define ARD_MASK	(0x0F<<ARD_BASE)
#define ARC_BASE	0 /* 4 bits */
#define ARC_MASK	(0x0F<<ARC_BASE)

/* RF setup register */
#define RF_DR_LOW   (1<<5)
#define RF_DR_HIGH  (1<<3)
#define RF_PWR_BASE	1
#define RF_PWR_MASK	(0x03<<RF_PWR_BASE)
#define PLL_LOCK    (1<<4)

/* general status register */
#define RX_DR       (1<<6)
#define TX_DS       (1<<5)
#define MAX_RT      (1<<4)
#define RX_P_NO_BASE     1 /* 3 bits */
#define RX_P_NO_MASK	(0x07<<RX_P_NO_BASE)
#define TX_FULL     (1<<0)

/* transmit observe register */
#define PLOS_CNT_BASE	4 /* 4 bits */
#define PLOS_CNT_MASK	(0x0F<<PLOS_CNT_BASE)
#define ARC_CNT_BASE	0 /* 4 bits */
#define ARC_CNT_MASK	(0x0F<<ARC_CNT_BASE)

/* fifo status */
#define TX_REUSE    (1<<6)
#define FIFO_FULL   (1<<5)
#define TX_EMPTY    (1<<4)
#define RX_FULL     (1<<1)
#define RX_EMPTY    (1<<0)

/* dynamic length */
#define DPL_P0      (1<<0)
#define DPL_P1      (1<<1)
#define DPL_P2      (1<<2)
#define DPL_P3      (1<<3)
#define DPL_P4      (1<<4)
#define DPL_P5      (1<<5)

/* features */
#define EN_DPL		(1<<2)
#define EN_ACK_PAY	(1<<1)
#define EN_DYN_ACK	(1<<0)

/* Instruction Mnemonics */
#define R_REGISTER    0x00 /* last 4 bits will indicate reg. address */
#define W_REGISTER    0x20 /* last 4 bits will indicate reg. address */
#define REGISTER_MASK 0x1F
#define R_RX_PAYLOAD  0x61
#define W_TX_PAYLOAD  0xA0
#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2
#define REUSE_TX_PL   0xE3
#define ACTIVATE      0x50
#define R_RX_PL_WID   0x60
#define W_ACK_PAYLOAD		0xA8
#define W_TX_PAYLOAD_NOACK	0xB0
#define NOP           0xFF

#endif /* _nRF24L01p_h_ */
