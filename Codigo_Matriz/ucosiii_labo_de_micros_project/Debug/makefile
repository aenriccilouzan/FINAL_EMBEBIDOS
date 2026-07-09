################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utilities/subdir.mk
-include source/rtos/uCOSIII/src/uCOS-III/Source/subdir.mk
-include source/rtos/uCOSIII/src/uCOS-III/Ports/ARM-Cortex-M4/Generic/GNU/subdir.mk
-include source/rtos/uCOSIII/src/uC-CPU/ARM-Cortex-M4/GNU/subdir.mk
-include source/rtos/uCOSIII/src/uC-CPU/subdir.mk
-include source/rtos/uCOSIII/BSP/subdir.mk
-include source/subdir.mk
-include drivers/subdir.mk
-include device/subdir.mk
-include component/uart/subdir.mk
-include component/serial_manager/subdir.mk
-include component/lists/subdir.mk
-include board/subdir.mk
-include SDK/startup/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := ucosiii_test_project
BUILD_ARTIFACT_EXTENSION := axf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: ucosiii_test_project.axf

# Tool invocations
ucosiii_test_project.axf: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -Xlinker -Map="ucosiii_test_project.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -Xlinker --sort-section=alignment -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -T ucosiii_test_project_Debug.ld -o "ucosiii_test_project.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ucosiii_test_project.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "ucosiii_test_project.axf"; # arm-none-eabi-objcopy -v -O binary "ucosiii_test_project.axf" "ucosiii_test_project.bin" ; # checksum -p MK64FN1M0xxx12 -d "ucosiii_test_project.bin";
	-@echo ' '

.PHONY: all clean dependents main-build post-build

-include ../makefile.targets
