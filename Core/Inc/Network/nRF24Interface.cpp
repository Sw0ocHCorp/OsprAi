/*
 * nFR24Interface.cpp
 *
 *  Created on: Feb 17, 2025
 *      Author: nclsr
 */

#include <Network/nRF24Interface.h>

namespace Osprai {

nRF24Interface::nRF24Interface(int bufferSize): NetworkInterface(bufferSize) {
	// TODO Auto-generated constructor stub

}

nRF24Interface::~nRF24Interface() {
	// TODO Auto-generated destructor stub
}

HAL_StatusTypeDef nRF24Interface::PerformCustomConfiguration(uint16_t *sensorAddress, map<uint8_t, vector<uint8_t>> *configuration) {
	return HAL_ERROR;
}

HAL_StatusTypeDef nRF24Interface::PerformCustomConfiguration(uint16_t *sensorAddress, map<uint8_t, uint8_t> *configuration) {
	uint8_t mode;
	//Enable CRC & POWER DOWN => Enable sensor configuration
	mode=0x08;
	HAL_StatusTypeDef status= WriteSPIData(CONFIG_MODE_REG, mode, R_REGISTER_CMD);
	status= ApplyRegistersConfig(sensorAddress, configuration);
	//Enable CRC & POWER UP => StandBy MODE
	mode=0x0a;
	status= WriteSPIData(CONFIG_MODE_REG, mode, R_REGISTER_CMD);
	//Clear data in sensor
	CLearRxSensorBuffer();
	CLearTxSensorBuffer();
	uint8_t ret= 0x00;
	status= ReadSPIData(CONFIG_MODE_REG, &ret);
	//Using RX / TX Default ADDRESS => PIPE 0 (0xE7.E7.E7.E7.E7)
	return status;
}

bool nRF24Interface::IsDataReceived() {
	bool status= false;
	uint8_t value;
	ReadSPIData(FIFO_STATUS_REG, &value);
	//If RX_EMPTY bit == 0	| Data is Availables
	if (value & 0x01 == 0)
		status= true;
	return status;
}

HAL_StatusTypeDef nRF24Interface::ProcessFrame(vector<uint8_t> frame) {
	return HAL_ERROR;
}

HAL_StatusTypeDef nRF24Interface::EnterInRXMode() {
	//HAL_GPIO_WritePin(CE_nRF24_GPIO_Port, CE_nRF24_Pin, PinState);
	CLearTxSensorBuffer();
	HAL_StatusTypeDef status= WriteSPIData(EN_RXADDR_REG, 0x01);
	uint8_t mode=0b00001011;
	status= WriteSPIData(CONFIG_MODE_REG, mode);
	return status;
}

HAL_StatusTypeDef nRF24Interface::EnterInTXMode() {
	CLearRxSensorBuffer();
	HAL_GPIO_WritePin(CE_nRF24_GPIO_Port, CE_nRF24_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef status= WriteSPIData(EN_RXADDR_REG, 0x00);
	uint8_t mode=0b00001010;
	status= WriteSPIData(CONFIG_MODE_REG, mode);
	return status;
}

HAL_StatusTypeDef nRF24Interface::CLearTxSensorBuffer() {
	uint8_t data= 0x00;
	return WriteSPIData(FLUSH_TX_CMD, data);
}

HAL_StatusTypeDef nRF24Interface::CLearRxSensorBuffer() {
	uint8_t data=0x00;
	return WriteSPIData(FLUSH_RX_CMD, data);
}

} /* namespace Osprai */
