/*
 * UARTSensor.h
 *
 *  Created on: Aug 8, 2025
 *      Author: nclsr
 */

#ifndef INC_SENSORS_UARTSENSOR_H_
#define INC_SENSORS_UARTSENSOR_H_

#include "main.h"
#include "utils.h"

namespace OsprAi {
	class UARTSensor {
		protected:
			UART_HandleTypeDef *UartInterface;
		public:
			UARTSensor() {

			}

			~UARTSensor() {

			}

			void SetUARTInterface(UART_HandleTypeDef *uartInterface) {
				UartInterface= uartInterface;
			}

			virtual HAL_StatusTypeDef TakeMeasurement() = 0;
	};
}



#endif /* INC_SENSORS_UARTSENSOR_H_ */
