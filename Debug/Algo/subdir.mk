################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Algo/BLSProcess_1634HzFs.c \
../Algo/EVSProcess_1634HzFs.c \
../Algo/SLMCali_24kHzFs.c \
../Algo/SLMProcess_24kHzFs.c \
../Algo/VLMCali.c \
../Algo/stdafx.c 

OBJS += \
./Algo/BLSProcess_1634HzFs.o \
./Algo/EVSProcess_1634HzFs.o \
./Algo/SLMCali_24kHzFs.o \
./Algo/SLMProcess_24kHzFs.o \
./Algo/VLMCali.o \
./Algo/stdafx.o 

C_DEPS += \
./Algo/BLSProcess_1634HzFs.d \
./Algo/EVSProcess_1634HzFs.d \
./Algo/SLMCali_24kHzFs.d \
./Algo/SLMProcess_24kHzFs.d \
./Algo/VLMCali.d \
./Algo/stdafx.d 


# Each subdirectory must supply rules for building sources it contributes
Algo/%.o Algo/%.su Algo/%.cyclo: ../Algo/%.c Algo/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../ads127l11 -I../Drivers/CMSIS/DSP/Include -I../Algo/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Algo

clean-Algo:
	-$(RM) ./Algo/BLSProcess_1634HzFs.cyclo ./Algo/BLSProcess_1634HzFs.d ./Algo/BLSProcess_1634HzFs.o ./Algo/BLSProcess_1634HzFs.su ./Algo/EVSProcess_1634HzFs.cyclo ./Algo/EVSProcess_1634HzFs.d ./Algo/EVSProcess_1634HzFs.o ./Algo/EVSProcess_1634HzFs.su ./Algo/SLMCali_24kHzFs.cyclo ./Algo/SLMCali_24kHzFs.d ./Algo/SLMCali_24kHzFs.o ./Algo/SLMCali_24kHzFs.su ./Algo/SLMProcess_24kHzFs.cyclo ./Algo/SLMProcess_24kHzFs.d ./Algo/SLMProcess_24kHzFs.o ./Algo/SLMProcess_24kHzFs.su ./Algo/VLMCali.cyclo ./Algo/VLMCali.d ./Algo/VLMCali.o ./Algo/VLMCali.su ./Algo/stdafx.cyclo ./Algo/stdafx.d ./Algo/stdafx.o ./Algo/stdafx.su

.PHONY: clean-Algo

