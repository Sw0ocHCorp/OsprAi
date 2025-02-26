################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/SH2/euler.c \
../Drivers/SH2/sh2.c \
../Drivers/SH2/sh2_SensorValue.c \
../Drivers/SH2/sh2_util.c \
../Drivers/SH2/shtp.c 

C_DEPS += \
./Drivers/SH2/euler.d \
./Drivers/SH2/sh2.d \
./Drivers/SH2/sh2_SensorValue.d \
./Drivers/SH2/sh2_util.d \
./Drivers/SH2/shtp.d 

OBJS += \
./Drivers/SH2/euler.o \
./Drivers/SH2/sh2.o \
./Drivers/SH2/sh2_SensorValue.o \
./Drivers/SH2/sh2_util.o \
./Drivers/SH2/shtp.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/SH2/%.o Drivers/SH2/%.su Drivers/SH2/%.cyclo: ../Drivers/SH2/%.c Drivers/SH2/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L476xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-SH2

clean-Drivers-2f-SH2:
	-$(RM) ./Drivers/SH2/euler.cyclo ./Drivers/SH2/euler.d ./Drivers/SH2/euler.o ./Drivers/SH2/euler.su ./Drivers/SH2/sh2.cyclo ./Drivers/SH2/sh2.d ./Drivers/SH2/sh2.o ./Drivers/SH2/sh2.su ./Drivers/SH2/sh2_SensorValue.cyclo ./Drivers/SH2/sh2_SensorValue.d ./Drivers/SH2/sh2_SensorValue.o ./Drivers/SH2/sh2_SensorValue.su ./Drivers/SH2/sh2_util.cyclo ./Drivers/SH2/sh2_util.d ./Drivers/SH2/sh2_util.o ./Drivers/SH2/sh2_util.su ./Drivers/SH2/shtp.cyclo ./Drivers/SH2/shtp.d ./Drivers/SH2/shtp.o ./Drivers/SH2/shtp.su

.PHONY: clean-Drivers-2f-SH2

