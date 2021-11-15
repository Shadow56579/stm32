################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Inc/Init.c \
../Inc/implementation.c \
../Inc/nrf24l01p_impi.c 

OBJS += \
./Inc/Init.o \
./Inc/implementation.o \
./Inc/nrf24l01p_impi.o 

C_DEPS += \
./Inc/Init.d \
./Inc/implementation.d \
./Inc/nrf24l01p_impi.d 


# Each subdirectory must supply rules for building sources it contributes
Inc/Init.o: ../Inc/Init.c Inc/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32G0 -DSTM32G071CBUx -c -I../Inc -I"D:/stm/g071cb/Inc/CMSIS" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Inc/Init.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Inc/implementation.o: ../Inc/implementation.c Inc/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32G0 -DSTM32G071CBUx -c -I../Inc -I"D:/stm/g071cb/Inc/CMSIS" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Inc/implementation.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Inc/nrf24l01p_impi.o: ../Inc/nrf24l01p_impi.c Inc/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32G0 -DSTM32G071CBUx -c -I../Inc -I"D:/stm/g071cb/Inc/CMSIS" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Inc/nrf24l01p_impi.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

