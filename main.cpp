
#include "mbed.h"
#include "rtos.h"
#include "MBed_Adafruit_GPS.h"

////////////////////////////////////////////////////PROGRAM////////////////////////////////////////////////////

//COMMON VARIABLES
Serial pc(USBTX, USBRX, 9600);

extern Thread threadI2C;
extern void I2C_thread();

extern Thread threadANALOG;
extern void ANALOG_thread();

extern Thread threadSERIAL;
extern void SERIAL_thread();

DigitalOut led_test(PB_5);	 //LED1
DigitalOut led_normal(PA_5); //LED2

InterruptIn change_mode(USER_BUTTON);

//Ticker tTest;
//Ticker tNormal;
//bool obtain_values_test = false;
//bool obtain_values_normal = false;

bool mode_change = false; //Test mode = false. Normal mode = true;
//bool changeTicker;
int tiempo = 0;

//COLOR SENSOR VARIABLES
extern int clear_value;
extern int red_value;
extern int blue_value;
extern int green_value;
extern char max_color_value;
extern void switchLED();
//Normal mode
extern char dominantColour_hour;

//ACCELEROMETER VARIABLES
extern float x_accel;
extern float y_accel;
extern float z_accel;
//Normal mode
extern float max_x_hour;
extern float min_x_hour;
extern float max_y_hour;
extern float min_y_hour;
extern float max_z_hour;
extern float min_z_hour;

//TEMPERATURE AND HUMIDITY VARIABLES
extern float temp, hum;
extern float max_temp_hour;
extern float min_temp_hour;
extern float average_temp_hour;
extern float max_hum_hour;
extern float min_hum_hour;
extern float average_hum_hour;

//SOIL MOISTURE VARIABLES
extern float valueSM;
//Normal mode
//extern float max_soil;
//extern float min_soil;
extern float max_soil_hour;
extern float min_soil_hour;
extern float average_soil_hour;

//PHOTORESISTOR VALUES
extern float valueLight;
//Normal mode
//extern float max_light;
//extern float min_light;
extern float max_light_hour;
extern float min_light_hour;
extern float average_light_hour;

//GPS VALUES
extern int hour;
extern int minute;
extern int seconds;
//extern int day;
extern float latitude;
extern char lat;
extern float longitude;
extern char lon;
extern float altitude;
extern int satellites;

////ISR code
//void print_values_test_mode (void) {
//	obtain_values_test =  true;
//}

//void print_values_normal_mode(void){
//	obtain_values_normal = true;
//}

void change_mode_handler(void)
{
	mode_change = !mode_change;
	//changeTicker = true;
}

int main()
{

	//ISR
	//tTest.attach(print_values_test_mode, 2.0); // Every two seconds the ticker triggers an interruption
	//tNormal.attach(print_values_normal_mode, 30.0);

	//From IRQ
	change_mode.mode(PullUp);
	change_mode.fall(change_mode_handler);

	//START I2C THREAD
	threadI2C.start(I2C_thread);
	pc.printf("Hemos entrado al main\n\r");

	//START ANALOG THREAD
	threadANALOG.start(ANALOG_thread);

	//START SERIAL THREAD
	threadSERIAL.start(SERIAL_thread);

	led_test.write(0);
	led_normal.write(0);

	while (true)
	{

		if (mode_change == false)
		{ //Test mode

			//if(changeTicker){
			//tTest.attach(print_values_test_mode, 2.0);
			//tNormal.detach();
			//changeTicker = false;
			//}
			//tTest.attach(print_values_test_mode, 2.0);
			//tNormal.detach();
			tiempo = 2000;
			led_test.write(1);
			led_normal.write(0);

			pc.printf("////////////////////////////////////////// TEST MODE ///////////////////////////////////////////// \n\r");
			//PRINT SOIL MOISTURE READING
			pc.printf("SOIL MOISTURE: %5.2f%% \n\r", valueSM);

			//PRINT PHOTORESISTOR READING
			pc.printf("LIGHT SENSOR: %5.2f%% \n\r", valueLight);

			//PRINT GPS VALUES
			pc.printf("GPS: Sats: %d, Lat(UIC): %5.2f%c, Long(UIC): %5.2f%c, Altitude: %5.2f, GPS_time: %d:%d:%d\n\r", satellites, latitude, lat, longitude, lon, altitude, hour, minute, seconds);
			//pc.printf("Location: %5.2f%c, %5.2f%c\n\r",latitude,lat,longitude,lon);

			//PRINT COLOR SENSOR READINGS
			pc.printf("COLOR SENSOR: Clear %d, Red %d, Green %d, Blue %d --Dominant color: %c\n\r", clear_value, red_value, green_value, blue_value, max_color_value);
			//pc.printf("Max color: (%c)\n\r", max_color_value);

			//PRINT ACCELEROMETER READINGS
			pc.printf("ACCELEROMETER: X_axis = %f \t Y_axis = %f \t Z_axis = %f \t \n\r", x_accel, y_accel, z_accel);

			//PRINT HUMIDITY AND TEMPERATURE SENSOR READINGS
			pc.printf("TEMPERATURE AND HUMIDITY: Temperature: %5.2f \t Relative humidity: %5.2f\n\r", temp, hum);

			pc.printf("\n\r");
			pc.printf("\n\r");

			wait(2);
		}
		if (mode_change)
		{ //Normal mode

			//if(changeTicker){
			//tTest.detach();
			//tNormal.attach(print_values_normal_mode, 30.0);
			//changeTicker = false;
			//}
			tiempo = 30000;
			led_test.write(0);
			led_normal.write(1);

			pc.printf("////////////////////////////////////////// NORMAL MODE ///////////////////////////////////////////// \n\r");

			//PRINT SOIL MOISTURE READING
			pc.printf("SOIL MOISTURE: %5.2f%% ,Max value: %5.2f%%, Min value:  %5.2f%%, Main value: %5.2f%%\n\r", valueSM, max_soil_hour, min_soil_hour, average_soil_hour);

			//PRINT PHOTORESISTOR READING
			pc.printf("LIGHT SENSOR: %5.2f%%, Max value: %5.2f%%, Min value:  %5.2f%%, Main value: %5.2f%%\n\r", valueLight, max_light_hour, min_light_hour, average_light_hour);

			//PRINT GPS VALUES
			pc.printf("GPS: Sats: %d, Lat(UIC): %5.2f%c, Long(UIC): %5.2f%c, Altitude: %5.2f, GPS_time: %d:%d:%d\n\r", satellites, latitude, lat, longitude, lon, altitude, hour, minute, seconds);
			//pc.printf("Location: %5.2f%c, %5.2f%c\n\r",latitude,lat,longitude,lon);

			//PRINT COLOR SENSOR READINGS
			pc.printf("COLOR SENSOR: Clear %d, Red %d, Green %d, Blue %d --Dominant color: %c\n\r", clear_value, red_value, green_value, blue_value, dominantColour_hour);
			//pc.printf("Max color: (%c)\n\r", max_color_value);

			//PRINT ACCELEROMETER READINGS
			pc.printf("ACCELEROMETER: X_axis = %f Max: %f, Min: %f\t Y_axis = %f Max: %f, Min: %f\t Z_axis = %f Max: %f, Min: %f\n\r", x_accel, max_x_hour, min_x_hour, y_accel, max_y_hour, min_y_hour, z_accel, max_z_hour, min_z_hour);

			//PRINT HUMIDITY AND TEMPERATURE SENSOR READINGS
			pc.printf("TEMPERATURE AND HUMIDITY: Temperature: %5.2f Max: %5.2f, Min: %5.2f, Main %5.2f\t Relative humidity: %5.2f Max: %5.2f, Min: %5.2f, Main: %5.2f\n\r", temp, max_temp_hour, min_temp_hour, average_temp_hour, hum, max_hum_hour, min_hum_hour, average_hum_hour);

			pc.printf("\n\r");
			pc.printf("\n\r");

			wait(30);
		}
	}
}