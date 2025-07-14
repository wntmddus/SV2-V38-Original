################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/BQ25883.c \
../Core/Src/ILI9488.c \
../Core/Src/Widget_trasducer.c \
../Core/Src/ad7124.c \
../Core/Src/ad7124_regs.c \
../Core/Src/fonts.c \
../Core/Src/freertos.c \
../Core/Src/main.c \
../Core/Src/os_spi.c \
../Core/Src/parameters.c \
../Core/Src/printf.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_hal_timebase_tim.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32h7xx.c \
../Core/Src/task_dsp.c \
../Core/Src/task_key.c \
../Core/Src/task_lcd.c \
../Core/Src/wavefile.c \
../Core/Src/widget_blast.c \
../Core/Src/widget_common.c \
../Core/Src/widget_evs.c \
../Core/Src/widget_ftp.c \
../Core/Src/widget_inputbox.c \
../Core/Src/widget_menu_main.c \
../Core/Src/widget_opt_analyz_blast.c \
../Core/Src/widget_opt_analyz_evs.c \
../Core/Src/widget_opt_rec.c \
../Core/Src/widget_selectbox.c \
../Core/Src/widget_trigger_blast.c \
../Core/Src/widget_trigger_evs.c 

OBJS += \
./Core/Src/BQ25883.o \
./Core/Src/ILI9488.o \
./Core/Src/Widget_trasducer.o \
./Core/Src/ad7124.o \
./Core/Src/ad7124_regs.o \
./Core/Src/fonts.o \
./Core/Src/freertos.o \
./Core/Src/main.o \
./Core/Src/os_spi.o \
./Core/Src/parameters.o \
./Core/Src/printf.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_hal_timebase_tim.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32h7xx.o \
./Core/Src/task_dsp.o \
./Core/Src/task_key.o \
./Core/Src/task_lcd.o \
./Core/Src/wavefile.o \
./Core/Src/widget_blast.o \
./Core/Src/widget_common.o \
./Core/Src/widget_evs.o \
./Core/Src/widget_ftp.o \
./Core/Src/widget_inputbox.o \
./Core/Src/widget_menu_main.o \
./Core/Src/widget_opt_analyz_blast.o \
./Core/Src/widget_opt_analyz_evs.o \
./Core/Src/widget_opt_rec.o \
./Core/Src/widget_selectbox.o \
./Core/Src/widget_trigger_blast.o \
./Core/Src/widget_trigger_evs.o 

C_DEPS += \
./Core/Src/BQ25883.d \
./Core/Src/ILI9488.d \
./Core/Src/Widget_trasducer.d \
./Core/Src/ad7124.d \
./Core/Src/ad7124_regs.d \
./Core/Src/fonts.d \
./Core/Src/freertos.d \
./Core/Src/main.d \
./Core/Src/os_spi.d \
./Core/Src/parameters.d \
./Core/Src/printf.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_hal_timebase_tim.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32h7xx.d \
./Core/Src/task_dsp.d \
./Core/Src/task_key.d \
./Core/Src/task_lcd.d \
./Core/Src/wavefile.d \
./Core/Src/widget_blast.d \
./Core/Src/widget_common.d \
./Core/Src/widget_evs.d \
./Core/Src/widget_ftp.d \
./Core/Src/widget_inputbox.d \
./Core/Src/widget_menu_main.d \
./Core/Src/widget_opt_analyz_blast.d \
./Core/Src/widget_opt_analyz_evs.d \
./Core/Src/widget_opt_rec.d \
./Core/Src/widget_selectbox.d \
./Core/Src/widget_trigger_blast.d \
./Core/Src/widget_trigger_evs.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../ads127l11 -I../Drivers/CMSIS/DSP/Include -I../Algo/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/BQ25883.cyclo ./Core/Src/BQ25883.d ./Core/Src/BQ25883.o ./Core/Src/BQ25883.su ./Core/Src/ILI9488.cyclo ./Core/Src/ILI9488.d ./Core/Src/ILI9488.o ./Core/Src/ILI9488.su ./Core/Src/Widget_trasducer.cyclo ./Core/Src/Widget_trasducer.d ./Core/Src/Widget_trasducer.o ./Core/Src/Widget_trasducer.su ./Core/Src/ad7124.cyclo ./Core/Src/ad7124.d ./Core/Src/ad7124.o ./Core/Src/ad7124.su ./Core/Src/ad7124_regs.cyclo ./Core/Src/ad7124_regs.d ./Core/Src/ad7124_regs.o ./Core/Src/ad7124_regs.su ./Core/Src/fonts.cyclo ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/fonts.su ./Core/Src/freertos.cyclo ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/os_spi.cyclo ./Core/Src/os_spi.d ./Core/Src/os_spi.o ./Core/Src/os_spi.su ./Core/Src/parameters.cyclo ./Core/Src/parameters.d ./Core/Src/parameters.o ./Core/Src/parameters.su ./Core/Src/printf.cyclo ./Core/Src/printf.d ./Core/Src/printf.o ./Core/Src/printf.su ./Core/Src/stm32h7xx_hal_msp.cyclo ./Core/Src/stm32h7xx_hal_msp.d ./Core/Src/stm32h7xx_hal_msp.o ./Core/Src/stm32h7xx_hal_msp.su ./Core/Src/stm32h7xx_hal_timebase_tim.cyclo ./Core/Src/stm32h7xx_hal_timebase_tim.d ./Core/Src/stm32h7xx_hal_timebase_tim.o ./Core/Src/stm32h7xx_hal_timebase_tim.su ./Core/Src/stm32h7xx_it.cyclo ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32h7xx.cyclo ./Core/Src/system_stm32h7xx.d ./Core/Src/system_stm32h7xx.o ./Core/Src/system_stm32h7xx.su ./Core/Src/task_dsp.cyclo ./Core/Src/task_dsp.d ./Core/Src/task_dsp.o ./Core/Src/task_dsp.su ./Core/Src/task_key.cyclo ./Core/Src/task_key.d ./Core/Src/task_key.o ./Core/Src/task_key.su ./Core/Src/task_lcd.cyclo ./Core/Src/task_lcd.d ./Core/Src/task_lcd.o ./Core/Src/task_lcd.su ./Core/Src/wavefile.cyclo ./Core/Src/wavefile.d ./Core/Src/wavefile.o ./Core/Src/wavefile.su ./Core/Src/widget_blast.cyclo ./Core/Src/widget_blast.d ./Core/Src/widget_blast.o ./Core/Src/widget_blast.su ./Core/Src/widget_common.cyclo ./Core/Src/widget_common.d ./Core/Src/widget_common.o ./Core/Src/widget_common.su ./Core/Src/widget_evs.cyclo ./Core/Src/widget_evs.d ./Core/Src/widget_evs.o ./Core/Src/widget_evs.su ./Core/Src/widget_ftp.cyclo ./Core/Src/widget_ftp.d ./Core/Src/widget_ftp.o ./Core/Src/widget_ftp.su ./Core/Src/widget_inputbox.cyclo ./Core/Src/widget_inputbox.d ./Core/Src/widget_inputbox.o ./Core/Src/widget_inputbox.su ./Core/Src/widget_menu_main.cyclo ./Core/Src/widget_menu_main.d ./Core/Src/widget_menu_main.o ./Core/Src/widget_menu_main.su ./Core/Src/widget_opt_analyz_blast.cyclo ./Core/Src/widget_opt_analyz_blast.d ./Core/Src/widget_opt_analyz_blast.o ./Core/Src/widget_opt_analyz_blast.su ./Core/Src/widget_opt_analyz_evs.cyclo ./Core/Src/widget_opt_analyz_evs.d ./Core/Src/widget_opt_analyz_evs.o ./Core/Src/widget_opt_analyz_evs.su ./Core/Src/widget_opt_rec.cyclo ./Core/Src/widget_opt_rec.d ./Core/Src/widget_opt_rec.o ./Core/Src/widget_opt_rec.su ./Core/Src/widget_selectbox.cyclo ./Core/Src/widget_selectbox.d ./Core/Src/widget_selectbox.o ./Core/Src/widget_selectbox.su ./Core/Src/widget_trigger_blast.cyclo ./Core/Src/widget_trigger_blast.d ./Core/Src/widget_trigger_blast.o ./Core/Src/widget_trigger_blast.su ./Core/Src/widget_trigger_evs.cyclo ./Core/Src/widget_trigger_evs.d ./Core/Src/widget_trigger_evs.o ./Core/Src/widget_trigger_evs.su

.PHONY: clean-Core-2f-Src

