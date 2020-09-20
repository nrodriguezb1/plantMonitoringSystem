//gps.cpp
//for use with Adafruit Ultimate GPS
//Reads in and parses GPS data

#include "mbed.h"
#include "MBed_Adafruit_GPS.h"


//GPS
Serial * gps_Serial = new Serial(PA_9,PA_10);
Adafruit_GPS myGPS(gps_Serial);
//Serial pc (USBTX, USBRX);
//char c; //when read via Adafruit_GPS::read(), the class returns single character stored here
Timer refresh_Timer; //sets up a timer for use in loop; how often do we print GPS info?
const int refresh_Time = 2000; //refresh time in ms

int hour;
int minute;
int seconds;
int fix;
float latitude;
float longitude;
char lat;
char lon;
float altitude;
int satellites;

//COMMON VARIABLES
Thread threadSERIAL(osPriorityNormal, 1024); // 1K stack size  512 //1G stack size 1024
void SERIAL_thread(); 


void initializeGPS(){
	  
	//sets baud rate for GPS communication; note this may be changed via Adafruit_GPS::sendCommand(char *)
  //a list of GPS commands is available at http://www.adafruit.com/datasheets/PMTK_A08.pdf
  myGPS.begin(9600);  
   
  //these commands are defined in MBed_Adafruit_GPS.h; a link is provided there for command creation
	myGPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); 
  myGPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  myGPS.sendCommand(PGCMD_ANTENNA);
}

void SERIAL_thread() {
   
	initializeGPS();
	
	Thread::wait(1000);
	
	refresh_Timer.start();  //starts the clock on the timer
	
	while(true){
		
		//Thread::wait(300);  //In miliseconds
		myGPS.read();   //queries the GPS
		
		//check if we recieved a new message from GPS, if so, attempt to parse it,
		if (myGPS.newNMEAreceived()) {
			if (!myGPS.parse(myGPS.lastNMEA())) {
				continue;   
			}
		}
		
		//check if enough time has passed to warrant printing GPS info to screen
    //note if refresh_Time is too low or pc.baud is too low, GPS data may be lost during printing
		if (refresh_Timer.read_ms() >= refresh_Time) {
			refresh_Timer.reset();
			//pc.printf("Time: %d:%d:%d.%u\n\r", myGPS.hour, myGPS.minute, myGPS.seconds, myGPS.milliseconds);
			hour = myGPS.hour;
			minute = myGPS.minute;
			seconds =  myGPS.seconds;
			//pc.printf("Date: %d/%d/20%d\n\r", myGPS.day, myGPS.month, myGPS.year);
			//pc.printf("Fix: %d\n\r", (int) myGPS.fix);
			fix = (int) myGPS.fix;
			//pc.printf("Quality: %d\n\r", (int) myGPS.fixquality);
			if (myGPS.fix) {
				latitude  = myGPS.latitude/100.0;
				longitude = myGPS.longitude/100.0;
				lat  = myGPS.lat;
				lon = myGPS.lon;
				altitude = myGPS.altitude;
				//pc.printf("Location: %5.2f%c, %5.2f%c\n\r", myGPS.latitude, myGPS.lat, myGPS.longitude, myGPS.lon);
				//pc.printf("Speed: %5.2f knots\n\r", myGPS.speed);
				//pc.printf("Angle: %5.2f\n\r", myGPS.angle);
				//pc.printf("Altitude: %5.2f\n\r", myGPS.altitude);
				altitude = myGPS.altitude;
				//pc.printf("Satellites: %d\n\r", myGPS.satellites);
				satellites = myGPS.satellites;
			}
		}
	}
}
