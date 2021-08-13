#include <stdint.h>
#include "stm32g431xx.h"
#include "icm20600.h"
#include <math.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

#define FK 0.99f
#define TIME_GYRO 0.05f
#define TO_DEG 57.2957f

void SPI_Init();
void SPI1_Write(uint16_t data);
uint8_t SPI1_Read(void);
void delay(uint32_t time);
void ICM_Init();
void writeByte(uint8_t addr, uint8_t data);
uint8_t readByte(uint8_t addr);
void readData(int16_t* ACCEL_X_PTR,int16_t* ACCEL_Y_PTR, int16_t* ACCEL_Z_PTR,
		int16_t* GYRO_X_PTR, int16_t* GYRO_Y_PTR, int16_t* GYRO_Z_PTR);
void ICM_calibrate();
void receiveData();

int16_t ACCEL_X;
int16_t ACCEL_Y;
int16_t ACCEL_Z;

int16_t GYRO_X;
int16_t GYRO_Y;
int16_t GYRO_Z;

float gx_cal=0;
float gy_cal=0;
float gz_cal=0;

float angle_ax;
float angle_ay;
float angle_az;

float angle_gx = 0;
float angle_gy = 0;
float angle_gz = 0;

float angle_x;
float angle_y;
float angle_z;

float Ax;
float Ay;
float Az;

float Gx;
float Gy;
float Gz;

int main(void)
{

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

	GPIOA->MODER &= ~(GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk);
	GPIOA->MODER |= (2 << GPIO_MODER_MODE5_Pos) | (2 << GPIO_MODER_MODE7_Pos) | (2 << GPIO_MODER_MODE6_Pos)
			     | (1 << GPIO_MODER_MODE4_Pos);
	GPIOA->AFR[0] = (0b101 << GPIO_AFRL_AFSEL5_Pos) | (0b101 << GPIO_AFRL_AFSEL7_Pos) | (0b101 << GPIO_AFRL_AFSEL6_Pos);
	GPIOA->OTYPER |= (0 << GPIO_OTYPER_OT7_Pos) | (0 << GPIO_OTYPER_OT5_Pos);
	GPIOA->OSPEEDR |=  GPIO_OSPEEDER_OSPEEDR7 |  GPIO_OSPEEDER_OSPEEDR5 | GPIO_OSPEEDER_OSPEEDR6;
	GPIOA->PUPDR |= (0 << GPIO_PUPDR_PUPD6_Pos);

	TIM2->PSC = 1999;
	TIM2->ARR = 399;
	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM2_IRQn);

	CS_OFF();

	SPI_Init();

	delay(100000);
    ICM_Init();

    ICM_calibrate();

    TIM2->CR1 |= TIM_CR1_CEN;

	while(1)
	{

	}


}

void SPI_Init()
{
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	SPI1->CR1 |= (0 << SPI_CR1_CPHA_Pos) | (0 << SPI_CR1_CPOL_Pos) | SPI_CR1_MSTR | (0b111 << SPI_CR1_BR_Pos)
			  | (0 << SPI_CR1_BIDIMODE_Pos) | SPI_CR1_SSM;
	SPI1->CR1  &= ~SPI_CR1_LSBFIRST;
	SPI1->CR2 |= (0b111 << SPI_CR2_DS_Pos) | SPI_CR2_FRXTH;
	SPI1->CR1 |= SPI_CR1_SSI;
	SPI1->CR1 |= SPI_CR1_SPE;

}

void SPI1_Write(uint16_t data)
{
 	CS_ON();
 	while (SPI1->SR & SPI_SR_BSY);
  	//Ждем, пока не освободится буфер передатчика
    while(!(SPI1->SR & SPI_SR_TXE));
 	//заполняем буфер передатчика
  	SPI1->DR = data;
  	while (SPI1->SR & SPI_SR_BSY);
  	CS_OFF();
}

uint8_t SPI1_Read(void)
{
  	CS_ON();
  	while (SPI1->SR & SPI_SR_BSY);

  	//Ждем, пока не появится новое значение
  	//в буфере приемника
  	while ((SPI1->SR & SPI_SR_RXNE) == 0);

  	uint8_t temp;
  	temp = SPI1->DR;
  	while (SPI1->SR & SPI_SR_BSY);
  	CS_OFF();
  	//возвращаем значение буфера приемника
  	return temp;
}

void ICM_Init()
{
	writeByte(MPUREG_PWR_MGMT_1,(1<<3));
	writeByte(MPUREG_GYRO_CONFIG,0x00);
	writeByte(MPUREG_ACCEL_CONFIG,0x00);
	writeByte(MPUREG_SMPLRT_DIV,0x07);

	delay(100000);

}

void delay(uint32_t time)
{
	for(uint32_t i = 0;i < time; i++);
}

void writeByte(uint8_t addr, uint8_t data)
{
	SPI1_Write(addr | (data << 8));
	SPI1_Read();
	SPI1_Read();
}

uint8_t readByte(uint8_t addr)
{
	uint8_t temp;
	SPI1_Write(addr | READ_FLAG);
	SPI1_Read();
	temp = SPI1_Read();
	return temp;
}

void readData(int16_t* ACCEL_X_PTR, int16_t* ACCEL_Y_PTR, int16_t* ACCEL_Z_PTR,
		int16_t* GYRO_X_PTR, int16_t* GYRO_Y_PTR, int16_t* GYRO_Z_PTR)
{

	*ACCEL_X_PTR = (readByte(MPUREG_ACCEL_XOUT_H) << 8) | readByte(MPUREG_ACCEL_XOUT_L);
	*ACCEL_Y_PTR = (readByte(MPUREG_ACCEL_YOUT_H) << 8) | readByte(MPUREG_ACCEL_YOUT_L);
	*ACCEL_Z_PTR = (readByte(MPUREG_ACCEL_ZOUT_H) << 8) | readByte(MPUREG_ACCEL_ZOUT_L);

	*GYRO_X_PTR = (readByte(MPUREG_GYRO_XOUT_H) << 8) | readByte(MPUREG_GYRO_XOUT_L);
	*GYRO_Y_PTR = (readByte(MPUREG_GYRO_YOUT_H) << 8) | readByte(MPUREG_GYRO_YOUT_L);
	*GYRO_Z_PTR = (readByte(MPUREG_GYRO_ZOUT_H) << 8) | readByte(MPUREG_GYRO_XOUT_L);

	Ax = ACCEL_X / 16384.0f;
	Ay = ACCEL_Y / 16384.0f;
	Az = ACCEL_Z / 16384.0f;


	Gx = GYRO_X / (131.0f);
	Gy = GYRO_Y / (131.0f);
	Gz = GYRO_Z / (131.0f);

}

void ICM_calibrate()
{
	uint16_t num = 3000;
	gx_cal = 0;
	gy_cal = 0;
	gz_cal = 0;
	for(uint16_t i = 0;i < num;i++)
	{
		readData(&ACCEL_X,&ACCEL_Y,&ACCEL_Z,&GYRO_X,&GYRO_Y,&GYRO_Z);
		gx_cal += Gx;
		gy_cal += Gy;
		gz_cal += Gz;
		delay(1000);
	}
	gx_cal /= (float)num;
	gy_cal /= (float)num;
	gz_cal /= (float)num;
}

void receiveData()
{
	readData(&ACCEL_X,&ACCEL_Y,&ACCEL_Z,&GYRO_X,&GYRO_Y,&GYRO_Z);

	Gx -= gx_cal;
	Gy -= gy_cal;
	Gz -= gz_cal;

	angle_gx = angle_x + (Gx * TIME_GYRO);
	angle_gy = angle_y + (Gy * TIME_GYRO);
	angle_gz = angle_z + (Gz * TIME_GYRO);


	angle_ax = atan2f(Ay,Az) * TO_DEG;
	angle_ay = atan2f(Ax,Az) * TO_DEG;
	angle_az = atan2f(Ay,Ax) * TO_DEG;

	angle_x = angle_gx * FK + angle_ax*(1-FK);
	angle_y = angle_gy * FK + angle_ay*(1-FK);
	angle_z = angle_gz * FK + angle_az*(1-FK);

}

void TIM2_IRQHandler(void)
{
	receiveData();
	TIM2->SR &= ~TIM_SR_UIF;
}
