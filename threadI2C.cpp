#include "mbed.h"
#include "rtos.h"
#include "MMA8451Q.h"

#define MMA8451_I2C_ADDRESS (0x1C << 1) //0x1d<<1  0x00<<1

//INITIALIZE COMMON VARIABLES
I2C i2c(I2C_SDA, I2C_SCL); //pins for I2C communication (SDA, SCL)

//Serial pc(USBTX, USBRX, 9600); //9600 baudios - used to print some values

Thread threadI2C(osPriorityNormal, 512); // 1K stack size
void I2C_thread();

int contadorI2C = 1;
int contador_maxI2C = 31; //Every 30seg the thread returns to ready state, which means 1h = 120 times 30 seconds

extern int tiempo;

//INITIALIZE COLOR SENSOR TCS34725
DigitalOut ledColour(PH_1); // TCS34725 led. PA_10 is occuped
DigitalOut ledR(PA_8);		//RGB led - red light
DigitalOut ledG(PB_7);		//RGB led - green light
DigitalOut ledB(PB_15);		//RGB led - blue light

// We set the sensor address. For TCS34725 is 0x29 = 0010 1001 (bin) ->> 0101 0010 (bin) = 0x52
// We shift 1 bit to the left because in I2C protocol slave address is 7-bit. So we discard the 8th bit
int colorSensor_addr = 0x29 << 1;

//Registers to obtain and save the values sensed by the color sensor
char clear_reg[1] = {0xB4}; // 1011 0100 -> 0x34 and we set 1st bit to 1
char data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

int clear_value = 0;
int red_value = 0;
int green_value = 0;
int blue_value = 0;
char max_color_value;
int contRed = 0;
int contBlue = 0;
int contGreen = 0;
char dominantColour;
char dominantColour_hour;

void switchLED();

//bool normalModeI2C = false;
extern bool mode_change;

//INITIALIZE TEMPERATURE AND HUMIDITY SENSOR
int temperatureSensor_addr = 0x80; // Device ADDR: 0x80 = SI7021 7-bits address shifted one bit left.

char read_temp[1]; //F3
char read_hum[1];  //F5

char receive_temp[2];
char receive_hum[2];

float hum, temp;

float max_temp = -5.0;
float min_temp = 100.0;
float max_temp_hour;
float min_temp_hour;
float max_hum = -5.0;
float min_hum = 100.0;
float max_hum_hour;
float min_hum_hour;
float average_temp_hour = 0.0;
float average_temp = 0.0;
;
float average_hum_hour = 0.0;
float average_hum = 0.0;

//INITIALIZE ACCELEROMETER SENSOR
MMA8451Q acc(PB_9, PB_8, MMA8451_I2C_ADDRESS);

float x_accel = 0.0;
float y_accel = 0.0;
float z_accel = 0.0;

float max_x = -1.1;
float max_y = -1.1;
float max_z = -1.1;
float min_x = 1.1;
float min_y = 1.1;
float min_z = 1.1;

float max_x_hour = 0.0;
float max_y_hour = 0.0;
float max_z_hour = 0.0;
float min_x_hour = 0.0;
float min_y_hour = 0.0;
float min_z_hour = 0.0;

//EXTERN VALUES TO SWITCH ON THE RGB LED
extern float valueSM;
extern float valueLight;

//Method to initialize the color sensor
void initializeColorSensor()
{

	//We obtain device ID from ID register (0x12)
	char id_regval[1] = {0x92}; //?1001 0010? (bin)
	char data[1] = {0};			//?0000 0000?
	i2c.write(colorSensor_addr, id_regval, 1, true);
	i2c.read(colorSensor_addr, data, 1, false);

	//We check that the ID is the TCS34725 one. If it is, we switch off a LED on the board, wait for 2s, and switch on again
	if (data[0] == 0x44)
	{ // 0100 0100 -> Value for the part number (0x44 for TCS34725)
	}

	//Timing register address 0x01 (0000 0001). We set 1st bit to 1 -> 1000 0001
	char timing_register[2] = {0x81, 0x50}; //0x50 ~ 400ms
	i2c.write(colorSensor_addr, timing_register, 2, false);

	//Control register address 0x0F (0000 1111). We set 1st bit to 1 -> 1000 1111
	char control_register[2] = {0x8F, 0}; //{0x8F, 0x00}, {1000 1111, 0000 0000} -> 1x gain
	i2c.write(colorSensor_addr, control_register, 2, false);

	// Enable register address 0x00 (0000 0000). We set 1st bit to 1 -> 1000 0000
	char enable_register[2] = {0x80, 0x03}; //{0x80, 0x03}, {1000 0000, 0000 0011} -> AEN = PON = 1
	i2c.write(colorSensor_addr, enable_register, 2, false);
}

//Method to obtain the values given by the color sensor for each color: clear, red, green and blue
void readColorSensor()
{

	//Start color sensor
	i2c.write(colorSensor_addr, clear_reg, 1, true);

	//Read color sensor
	i2c.read(colorSensor_addr, data, 8, false);
}

//Get max value (r,g,b) function
char getMax(int r, int g, int b)
{
	char result;
	int max;
	if (r < g)
	{
		max = g;
		result = 'g';
	}
	else
	{
		max = r;
		result = 'r';
	}
	if (max < b)
	{
		result = 'b';
	}
	return result;
}

char obtainDominantColour(char value)
{

	char dc;

	if (value == 'r')
	{
		contRed++;
	}
	else if (value == 'b')
	{
		contBlue++;
	}
	else
	{
		contGreen++;
	}

	if ((contRed > contBlue) and (contRed > contGreen))
	{
		//Red dominant color
		dc = 'r';
	}
	else if ((contBlue > contRed) and (contBlue > contGreen))
	{
		//Blue dominant color
		dc = 'b';
	}
	else if ((contGreen > contRed) and (contGreen > contBlue))
	{
		//Green dominant color
		dc = 'g';
	}
	else
	{
		dc = 'y';
	}
	return dc;
}

//Method to switch ON the led with the max colour sensed by the color sensor
void switchLedTestMode(char max)
{

	//Switch OFF all colours
	ledR.write(1);
	ledG.write(1);
	ledB.write(1);

	//Switch ON the max colour
	if (max == 'r')
	{
		ledR.write(0);
		//pc.printf("Red\r\n");
	}
	else if (max == 'g')
	{
		//pc.printf("Green\r\n");
		ledG.write(0);
	}
	else
	{
		//pc.printf("Blue\r\n");
		ledB.write(0);
	}
}

void switchLedNormalMode()
{
	//Yellow    R255 G255 B0     -----------  HUMIDITY       25 A 75
	//Green     R0   G255 B0     -----------  TEMPERATURE   -10 A 50
	//Cyan      R0   G255 B255   -----------  AMBIENT LIGHT  10 A 90
	//Blue      R0   G0   B255   -----------  SOIL MOISTURE  10 A 90
	//Magenta   R255 G0   B255   -----------  COLOUR          GREEN
	//Red       R255 G0   B0     -----------  ACCELEROMETER   ENTRE 1 Y -1 (PARA CUALQUIER EJE)
	//White     R255 G255 B255   -----------  OK

	//Switch OFF all colours
	ledR.write(1);
	ledG.write(1);
	ledB.write(1);
	if (hum > 75.0 or hum < 25.0)
	{
		//Yellow
		ledR.write(0);
		ledG.write(0);
		ledB.write(1);
	}
	else if (temp < -10.0 or temp > 50.0)
	{
		//Green
		ledR.write(1);
		ledG.write(0);
		ledB.write(1);
	}
	else if (valueLight < 10.0 or valueLight > 90.0)
	{
		//Cyan
		ledR.write(1);
		ledG.write(0);
		ledB.write(0);
	}
	else if (valueSM < 10.0 or valueSM > 90.0)
	{
		//Blue
		ledR.write(1);
		ledG.write(1);
		ledB.write(0);
	}
	else if (max_color_value != 'g')
	{
		//Magenta
		ledR.write(0);
		ledG.write(1);
		ledB.write(0);
	}
	else if ((x_accel < -1.0 or x_accel > 1.0) or (y_accel < -1.0 or y_accel > 1.0) or (z_accel < -1.0 or z_accel > 1.0))
	{
		//Red
		ledR.write(0);
		ledG.write(1);
		ledB.write(1);
	}
	else
	{
		//White
		ledR.write(0);
		ledG.write(0);
		ledB.write(0);
	}
}

void obtainTemperature()
{
	read_temp[0] = 0xE3; //0xE3

	i2c.write(temperatureSensor_addr, read_temp, 1, true);
	//wait_ms(100);
	i2c.read(temperatureSensor_addr, receive_temp, 2); // Receive MSB = receive_temp[0], then LSB = receive_temp[1]
	temp = ((((receive_temp[0] << 8) + receive_temp[1]) * 175.72) / 65536.0) - 46.85;
}

void obtainHumidity()
{
	read_hum[0] = 0xE5; //0xE5

	i2c.write(temperatureSensor_addr, read_hum, 1, true);
	//wait_ms(100);
	i2c.read(temperatureSensor_addr, receive_hum, 2); // Receive MSB = receive_hum[0], then LSB = receive_hum[1]
	hum = ((((receive_hum[0] << 8) + receive_hum[1]) * 125.0) / 65536.0) - 6.0;
}

void obtainMaxOrMinTemp(float value)
{
	if (value > max_temp)
	{
		max_temp = value;
	}
	else if (value < min_temp)
	{
		min_temp = value;
	}
}

void obtainMaxOrMinHum(float value)
{
	if (value > max_hum)
	{
		max_hum = value;
	}
	else if (value < min_hum)
	{
		min_hum = value;
	}
}

void obtainMaxOrMinXAxis(float value)
{
	if (value > max_x)
	{
		max_x = value;
	}
	else if (value < min_x)
	{
		min_x = value;
	}
}
void obtainMaxOrMinYAxis(float value)
{
	if (value > max_y)
	{
		max_y = value;
	}
	else if (value < min_y)
	{
		min_y = value;
	}
}
void obtainMaxOrMinZAxis(float value)
{
	if (value > max_z)
	{
		max_z = value;
	}
	else if (value < min_z)
	{
		min_z = value;
	}
}

//I2C THREAD
void I2C_thread()
{

	//INITIALIZE COLOR SENSOR
	initializeColorSensor();
	ledColour = 1;

	while (true)
	{

		Thread::wait(tiempo); //In miliseconds
		//Thread::wait(30000);  //In miliseconds. Normal mode

		//OBTAIN COLOR SENSOR VALUES
		readColorSensor();

		clear_value = ((int)data[1] << 8) | data[0];
		red_value = ((int)data[3] << 8) | data[2];
		green_value = ((int)data[5] << 8) | data[4];
		blue_value = ((int)data[7] << 8) | data[6];

		//Obtain max value
		max_color_value = getMax(red_value, green_value, blue_value);
		if (!mode_change)
		{
			switchLedTestMode(max_color_value);
		}
		else
		{
			//Normal mode
			switchLedNormalMode();
		}

		//OBTAIN ACCELEROMETER VALUES
		//Read and print the values at the same time
		x_accel = acc.getAccX();
		y_accel = acc.getAccY();
		z_accel = acc.getAccZ();

		//OBTAIN HUMIDITY AND TEMPERATURE VALUES
		obtainTemperature();
		obtainHumidity();

		//temp = ((((receive_temp[0] <<8) + receive_temp[1]) * 175.72) / 65536.0) - 46.85;	// Conversion based on Datasheet
		//hum = ((((receive_hum[0] <<8) + receive_hum[1]) * 125.0) / 65536.0) - 6.0;				// Conversion based on Datasheet

		if (mode_change)
		{
			contadorI2C++;
			//Reset contador when one hour passes
			if (contadorI2C > contador_maxI2C)
			{
				contadorI2C = 1;
				//Reset contadores de color cuando pase una hora
				contRed = 0;
				contBlue = 0;
				contGreen = 0;
				dominantColour_hour = 'y';
				//Accelerometer
				max_x_hour = max_x;
				max_y_hour = max_y;
				max_z_hour = max_z;
				min_x_hour = min_x;
				min_y_hour = min_y;
				min_z_hour = min_z;
				//Reset values
				max_x = 0.0;
				max_y = 0.0;
				max_z = 0.0;
				min_x = 0.0;
				min_y = 0.0;
				min_z = 0.0;
				//Humidity and temperatura
				max_temp_hour = max_temp;
				min_temp_hour = min_temp;
				average_temp_hour = average_temp;
				max_hum_hour = max_hum;
				min_hum_hour = min_hum;
				average_hum_hour = average_hum;
				//Reset values
				max_temp = 0.0;
				min_temp = 0.0;
				average_temp = 0.0;
				max_hum = 0.0;
				min_hum = 0.0;
				average_hum = 0.0;
			}
			//COLOR SENSOR
			dominantColour = obtainDominantColour(max_color_value);

			//ACCELEROMETER
			//Normal mode
			obtainMaxOrMinXAxis(x_accel);
			obtainMaxOrMinYAxis(y_accel);
			obtainMaxOrMinZAxis(z_accel);

			//OBTAIN HUMIDITY AND TEMPERATURE VALUE
			obtainMaxOrMinTemp(temp);
			obtainMaxOrMinHum(hum);
			average_temp = (average_temp * (contadorI2C - 1) + temp) / contadorI2C;
			average_hum = (average_hum * (contadorI2C - 1) + hum) / contadorI2C;
		}
	}
}