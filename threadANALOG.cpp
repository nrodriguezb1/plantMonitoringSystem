#include "mbed.h"
#include "rtos.h"

//SOIL MOISTURE
AnalogIn soilmois(PA_0);
float valueSM = 0.0;

float max_soil = 0.0;
float min_soil = 200.0;
float soil_average = 0.0;

float max_soil_hour = 0.0;
float min_soil_hour = 0.0;
float average_soil_hour = 0.0;

//PHOTORESISTOR
AnalogIn light(PA_4);
float valueLight = 0.0;

float max_light = 0.0;
float min_light = 200.0;
float light_average = 0.0;

float max_light_hour = 0.0;
float min_light_hour = 0.0;
float average_light_hour = 0.0;

//COMMON VARIABLES
Thread threadANALOG(osPriorityNormal, 512); // 1K stack size
void ANALOG_thread();

int contadorA = 1;
int contador_maxA = 31; //Every 30seg the thread returns to ready state, which means 1h = 120 times 30 seconds

extern bool mode_change;
extern int tiempo;

void obtainMaxOrMinSoil(float value)
{

	if (value > max_soil)
	{
		max_soil = value;
	}
	else if (value < min_soil)
	{
		min_soil = value;
	}
}

void obtainMaxOrMinLight(float value)
{

	if (value > max_light)
	{
		max_light = value;
	}
	else if (value < min_light)
	{
		min_light = value;
	}
}

void ANALOG_thread()
{

	while (true)
	{
		Thread::wait(tiempo); //ms
		//Thread::wait(30000);  //ms

		//To obtain the percent is necessary to multiply by 100
		//SOIL MOISTURE
		valueSM = soilmois * 100;

		//LIGHT SENSOR
		valueLight = light * 100;

		if (mode_change)
		{

			contadorA++;
			//Reset contador when one hour passes
			if (contadorA > contador_maxA)
			{
				contadorA = 1;

				//Soil moisture
				max_soil_hour = max_soil;
				min_soil_hour = min_soil;
				average_soil_hour = soil_average;
				//put to zero max_soil, min_soil, soil_average
				max_soil = 0.0;
				min_soil = 0.0;
				soil_average = 0.0;

				//Light sensor
				max_light_hour = max_light;
				min_light_hour = min_light;
				average_light_hour = light_average;
				//put to zero max_light, min light, light average
				max_light = 0.0;
				min_light = 0.0;
				light_average = 0.0;
			}

			//SOIL MOISTURE
			obtainMaxOrMinSoil(valueSM);
			soil_average = (soil_average * (contadorA - 1) + valueSM) / contadorA;

			//LIGHT SENSOR
			obtainMaxOrMinLight(valueLight);
			light_average = (light_average * (contadorA - 1) + valueLight) / contadorA;
		}
	}
}
