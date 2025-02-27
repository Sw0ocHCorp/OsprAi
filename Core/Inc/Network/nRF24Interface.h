/*
 * nFR24Interface.h
 *
 *  Created on: Feb 17, 2025
 *      Author: nclsr
 */

#ifndef INC_NETWORK_NRF24INTERFACE_H_
#define INC_NETWORK_NRF24INTERFACE_H_

// Command
//Write on register => 00.1A.AA.AA	| With A.A.AA.AA, 5 bits register address
#define R_REGISTER_CMD          0b00000000
#define W_REGISTER_CMD          0b00100000
#define R_RX_PAYLOAD_CMD        0b01100001
#define W_TX_PAYLOAD_CMD        0b10100000
#define FLUSH_TX_CMD           	0b11100001
#define FLUSH_RX_CMD            0b11100010
#define REUSE_TX_PL_CMD         0b11100011
#define ACTIVATE_CMD            0b01010000      //nRF24L01 Product Specification
#define R_RX_PL_WID_CMD         0b01100000
#define W_ACK_PAYLOAD_CMD       0b10101000      // 0b10100PPP
#define W_TX_PAYLOAD_NOACK_CMD  0b10110000
#define NOP_CMD                 0b11111111

//Registers
#define CONFIG_MODE_REG				0x00
#define EN_AA_MODE_REG				0x01
#define EN_RXADDR_REG           	0x02
#define SETUP_AW_REG            	0x03
#define SETUP_RETR_REG          	0x04
#define RF_CH_REG               	0x05
#define RF_SETUP_REG            	0x06
#define STATUS_REG              	0x07
#define OBSERVE_TX_REG          	0x08
#define RPD_REG                 	0x09
#define RX_ADDR_P0_REG          	0x0A
#define RX_ADDR_P1_REG          	0x0B
#define RX_ADDR_P2_REG          	0x0C
#define RX_ADDR_P3_REG          	0x0D
#define RX_ADDR_P4_REG          	0x0E
#define RX_ADDR_P5_REG          	0x0F
#define TX_ADDR_REG             	0x10
#define RX_PW_P0_REG            	0x11
#define RX_PW_P1_REG            	0x12
#define RX_PW_P2_REG            	0x13
#define RX_PW_P3_REG            	0x14
#define RX_PW_P4_REG            	0x15
#define RX_PW_P5_REG            	0x16
#define FIFO_STATUS_REG         	0x17
#define DYNPD_REG               	0x1C
#define FEATURE_REG             	0x1D


#define TRANSMITTER         0
#define RECEIVER            1

#include "NetworkInterface.h"

namespace Osprai {

class nRF24Interface : NetworkInterface {
private:

protected:
	HAL_StatusTypeDef CLearTxSensorBuffer();
	HAL_StatusTypeDef CLearRxSensorBuffer();
	HAL_StatusTypeDef ProcessFrame(vector<uint8_t> frame);
	HAL_StatusTypeDef EnterInRXMode();
	HAL_StatusTypeDef EnterInTXMode();
	bool IsDataReceived();
public:
	nRF24Interface(int bufferSize);
	virtual ~nRF24Interface();
	HAL_StatusTypeDef PerformCustomConfiguration(uint16_t *sensorAddress, map<uint8_t, vector<uint8_t>> *configuration= nullptr);
	HAL_StatusTypeDef PerformCustomConfiguration(uint16_t *sensorAddress, map<uint8_t, uint8_t> *configuration= nullptr);
};

} /* namespace Osprai */

#endif /* INC_NETWORK_NRF24INTERFACE_H_ */
