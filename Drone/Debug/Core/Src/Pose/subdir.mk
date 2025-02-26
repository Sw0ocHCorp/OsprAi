################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/Pose/PoseManager.cpp 

OBJS += \
./Core/Src/Pose/PoseManager.o 

CPP_DEPS += \
./Core/Src/Pose/PoseManager.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Pose/%.o Core/Src/Pose/%.su Core/Src/Pose/%.cyclo: ../Core/Src/Pose/%.cpp Core/Src/Pose/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L476xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Pose

clean-Core-2f-Src-2f-Pose:
	-$(RM) ./Core/Src/Pose/PoseManager.cyclo ./Core/Src/Pose/PoseManager.d ./Core/Src/Pose/PoseManager.o ./Core/Src/Pose/PoseManager.su

.PHONY: clean-Core-2f-Src-2f-Pose

