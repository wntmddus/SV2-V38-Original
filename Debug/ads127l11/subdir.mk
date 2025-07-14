################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ads127l11/ads_core.c 

OBJS += \
./ads127l11/ads_core.o 

C_DEPS += \
./ads127l11/ads_core.d 


# Each subdirectory must supply rules for building sources it contributes
ads127l11/%.o ads127l11/%.su ads127l11/%.cyclo: ../ads127l11/%.c ads127l11/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../ads127l11 -I../Drivers/CMSIS/DSP/Include -I../Algo/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-ads127l11

clean-ads127l11:
	-$(RM) ./ads127l11/ads_core.cyclo ./ads127l11/ads_core.d ./ads127l11/ads_core.o ./ads127l11/ads_core.su

.PHONY: clean-ads127l11

