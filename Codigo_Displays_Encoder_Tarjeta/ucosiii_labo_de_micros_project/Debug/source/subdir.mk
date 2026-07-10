################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/App.c \
../source/SysTick.c \
../source/display.c \
../source/encoder.c \
../source/fsm.c \
../source/gpio.c \
../source/main.c \
../source/tarjeta_magnetica.c \
../source/timer.c 

C_DEPS += \
./source/App.d \
./source/SysTick.d \
./source/display.d \
./source/encoder.d \
./source/fsm.d \
./source/gpio.d \
./source/main.d \
./source/tarjeta_magnetica.d \
./source/timer.d 

OBJS += \
./source/App.o \
./source/SysTick.o \
./source/display.o \
./source/encoder.o \
./source/fsm.o \
./source/gpio.o \
./source/main.o \
./source/tarjeta_magnetica.o \
./source/timer.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -D__USE_CMSIS -DDEBUG -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\ucosiii_config" -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU\ARM-Cortex-M4\GNU" -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU" -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-LIB" -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Ports\ARM-Cortex-M4\Generic\GNU" -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Source" -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/App.d ./source/App.o ./source/SysTick.d ./source/SysTick.o ./source/display.d ./source/display.o ./source/encoder.d ./source/encoder.o ./source/fsm.d ./source/fsm.o ./source/gpio.d ./source/gpio.o ./source/main.d ./source/main.o ./source/tarjeta_magnetica.d ./source/tarjeta_magnetica.o ./source/timer.d ./source/timer.o

.PHONY: clean-source

