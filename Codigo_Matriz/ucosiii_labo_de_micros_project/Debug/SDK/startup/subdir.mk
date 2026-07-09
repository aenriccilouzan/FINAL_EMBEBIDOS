################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SDK/startup/hardware.c \
../SDK/startup/semihost_hardfault.c \
../SDK/startup/startup_mk64f12.c 

C_DEPS += \
./SDK/startup/hardware.d \
./SDK/startup/semihost_hardfault.d \
./SDK/startup/startup_mk64f12.d 

OBJS += \
./SDK/startup/hardware.o \
./SDK/startup/semihost_hardfault.o \
./SDK/startup/startup_mk64f12.o 


# Each subdirectory must supply rules for building sources it contributes
SDK/startup/%.o: ../SDK/startup/%.c SDK/startup/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -DSERIAL_PORT_TYPE_UART=1 -DNDEBUG -D__USE_CMSIS -DDEBUG -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\ucosiii_config" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\board" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\component" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\component\lists" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\component\serial_manager" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\component\uart" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\device" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\drivers" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\SDK" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\SDK\CMSIS" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\SDK\startup" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\utilities" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU\ARM-Cortex-M4\GNU" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-LIB" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Ports\ARM-Cortex-M4\Generic\GNU" -I"C:\Alvaro\Sistemas Embebidos\TP FINAL FEBRERO\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Source" -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-SDK-2f-startup

clean-SDK-2f-startup:
	-$(RM) ./SDK/startup/hardware.d ./SDK/startup/hardware.o ./SDK/startup/semihost_hardfault.d ./SDK/startup/semihost_hardfault.o ./SDK/startup/startup_mk64f12.d ./SDK/startup/startup_mk64f12.o

.PHONY: clean-SDK-2f-startup

