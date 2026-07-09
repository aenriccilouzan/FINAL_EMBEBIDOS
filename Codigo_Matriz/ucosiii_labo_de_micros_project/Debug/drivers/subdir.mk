################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/fsl_adc16.c \
../drivers/fsl_clock.c \
../drivers/fsl_common.c \
../drivers/fsl_common_arm.c \
../drivers/fsl_dmamux.c \
../drivers/fsl_dspi.c \
../drivers/fsl_edma.c \
../drivers/fsl_ftfx_cache.c \
../drivers/fsl_ftfx_controller.c \
../drivers/fsl_ftfx_flash.c \
../drivers/fsl_ftfx_flexnvm.c \
../drivers/fsl_ftm.c \
../drivers/fsl_gpio.c \
../drivers/fsl_i2c.c \
../drivers/fsl_rtc.c \
../drivers/fsl_smc.c \
../drivers/fsl_uart.c 

C_DEPS += \
./drivers/fsl_adc16.d \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_common_arm.d \
./drivers/fsl_dmamux.d \
./drivers/fsl_dspi.d \
./drivers/fsl_edma.d \
./drivers/fsl_ftfx_cache.d \
./drivers/fsl_ftfx_controller.d \
./drivers/fsl_ftfx_flash.d \
./drivers/fsl_ftfx_flexnvm.d \
./drivers/fsl_ftm.d \
./drivers/fsl_gpio.d \
./drivers/fsl_i2c.d \
./drivers/fsl_rtc.d \
./drivers/fsl_smc.d \
./drivers/fsl_uart.d 

OBJS += \
./drivers/fsl_adc16.o \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_common_arm.o \
./drivers/fsl_dmamux.o \
./drivers/fsl_dspi.o \
./drivers/fsl_edma.o \
./drivers/fsl_ftfx_cache.o \
./drivers/fsl_ftfx_controller.o \
./drivers/fsl_ftfx_flash.o \
./drivers/fsl_ftfx_flexnvm.o \
./drivers/fsl_ftm.o \
./drivers/fsl_gpio.o \
./drivers/fsl_i2c.o \
./drivers/fsl_rtc.o \
./drivers/fsl_smc.o \
./drivers/fsl_uart.o 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c drivers/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -DSERIAL_PORT_TYPE_UART=1 -DNDEBUG -D__USE_CMSIS -DDEBUG -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\ucosiii_config" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\board" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\component" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\component\lists" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\component\serial_manager" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\component\uart" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\device" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\drivers" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\SDK" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\SDK\CMSIS" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\SDK\startup" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\utilities" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU\ARM-Cortex-M4\GNU" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-LIB" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Ports\ARM-Cortex-M4\Generic\GNU" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Source" -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-drivers

clean-drivers:
	-$(RM) ./drivers/fsl_adc16.d ./drivers/fsl_adc16.o ./drivers/fsl_clock.d ./drivers/fsl_clock.o ./drivers/fsl_common.d ./drivers/fsl_common.o ./drivers/fsl_common_arm.d ./drivers/fsl_common_arm.o ./drivers/fsl_dmamux.d ./drivers/fsl_dmamux.o ./drivers/fsl_dspi.d ./drivers/fsl_dspi.o ./drivers/fsl_edma.d ./drivers/fsl_edma.o ./drivers/fsl_ftfx_cache.d ./drivers/fsl_ftfx_cache.o ./drivers/fsl_ftfx_controller.d ./drivers/fsl_ftfx_controller.o ./drivers/fsl_ftfx_flash.d ./drivers/fsl_ftfx_flash.o ./drivers/fsl_ftfx_flexnvm.d ./drivers/fsl_ftfx_flexnvm.o ./drivers/fsl_ftm.d ./drivers/fsl_ftm.o ./drivers/fsl_gpio.d ./drivers/fsl_gpio.o ./drivers/fsl_i2c.d ./drivers/fsl_i2c.o ./drivers/fsl_rtc.d ./drivers/fsl_rtc.o ./drivers/fsl_smc.d ./drivers/fsl_smc.o ./drivers/fsl_uart.d ./drivers/fsl_uart.o

.PHONY: clean-drivers

