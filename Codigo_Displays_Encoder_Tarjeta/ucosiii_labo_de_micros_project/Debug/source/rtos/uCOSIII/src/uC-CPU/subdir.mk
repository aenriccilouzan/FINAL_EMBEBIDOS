################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/rtos/uCOSIII/src/uC-CPU/cpu_core.c 

C_DEPS += \
./source/rtos/uCOSIII/src/uC-CPU/cpu_core.d 

OBJS += \
./source/rtos/uCOSIII/src/uC-CPU/cpu_core.o 


# Each subdirectory must supply rules for building sources it contributes
source/rtos/uCOSIII/src/uC-CPU/%.o: ../source/rtos/uCOSIII/src/uC-CPU/%.c source/rtos/uCOSIII/src/uC-CPU/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -D__USE_CMSIS -DDEBUG -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\ucosiii_config" -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU\ARM-Cortex-M4\GNU" -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU" -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-LIB" -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Ports\ARM-Cortex-M4\Generic\GNU" -I"G:\Mi unidad\Sync 1Q2026\TP_Final_Embebidos\Repositorio\FINAL_EMBEBIDOS\Codigo_Displays_Encoder_Tarjeta\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Source" -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source-2f-rtos-2f-uCOSIII-2f-src-2f-uC-2d-CPU

clean-source-2f-rtos-2f-uCOSIII-2f-src-2f-uC-2d-CPU:
	-$(RM) ./source/rtos/uCOSIII/src/uC-CPU/cpu_core.d ./source/rtos/uCOSIII/src/uC-CPU/cpu_core.o

.PHONY: clean-source-2f-rtos-2f-uCOSIII-2f-src-2f-uC-2d-CPU

