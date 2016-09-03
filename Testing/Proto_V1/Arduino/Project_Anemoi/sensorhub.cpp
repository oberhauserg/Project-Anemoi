/**
*	sensorhub.c
*	Implementation of logic that takes in sensor readings and calculates orientations
*	Author: Grant Oberhauser
*/

#include "sensorhub.h"
#include <SparkFunLSM9DS1.h>

//#define FLIP_X_Y
//#define INVERSE_ACCEL_Y
//#define INVERSE_ACCEL_Z

//Always have the orientation of the device level
//#define ALL_LEVEL 

// begin() returns a 16-bit value which includes both the gyro 
// and accelerometers WHO_AM_I response. You can check this to
// make sure communication was successful.
//LSM9DS0 dof(MODE_I2C, LSM9DS0_G, LSM9DS0_XM);

LSM9DS1 imu;

#define LSM9DS1_M	0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG	0x6B // Would be 0x6A if SDO_AG is LOW

point SensorHub::accel;
point SensorHub::mag;
point SensorHub::gyro;

quaternion SensorHub::orient;



uint16_t SensorHub::lastUpdate = 0;    
uint16_t SensorHub::now = 0;           

float SensorHub::deltat = 0.0;

//Initialize sensors and sensor readings.
void SensorHub::init()
{

 // Set up interrupt pins as inputs:
 // pinMode(INT1XM, INPUT);
 // pinMode(INT2XM, INPUT);
 // pinMode(DRDYG,  INPUT);


  orient.a = 1.0f;
  orient.b = 0.0f;
  orient.c = 0.0f;
  orient.d = 0.0f;
  
  
  //uint16_t status = dof.begin();

  //Serial.print("LSM9DS0 WHO_AM_I's returned: 0x");
  //Serial.println(status, HEX);
  //Serial.println("Should be 0x49D4");
  //Serial.println();
  
  
  
    // Before initializing the IMU, there are a few settings
  // we may need to adjust. Use the settings struct to set
  // the device's communication mode and addresses:
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;
  
    // The above lines will only take effect AFTER calling
  // imu.begin(), which verifies communication with the IMU
  // and turns it on.
  if (!imu.begin())
  {
    Serial.println("Failed to start IMU!"); 
  }
  
  
  // Set data output ranges; choose lowest ranges for maximum resolution
  // Accelerometer scale can be: A_SCALE_2G, A_SCALE_4G, A_SCALE_6G, A_SCALE_8G, or A_SCALE_16G   
  //dof.setAccelScale(dof.A_SCALE_2G);
  imu.setAccelScale(2);

  // Gyro scale can be:  G_SCALE__245, G_SCALE__500, or G_SCALE__2000DPS
  //dof.setGyroScale(dof.G_SCALE_245DPS);
  imu.setGyroScale(245);
  
  // Magnetometer scale can be: M_SCALE_2GS, M_SCALE_4GS, M_SCALE_8GS, M_SCALE_12GS   
  //dof.setMagScale(dof.M_SCALE_2GS);
  imu.setMagScale(4);

  // Set output data rates  
  // Accelerometer output data rate (ODR) can be: A_ODR_3125 (3.225 Hz), A_ODR_625 (6.25 Hz), A_ODR_125 (12.5 Hz), A_ODR_25, A_ODR_50, 
  //                                              A_ODR_100,  A_ODR_200, A_ODR_400, A_ODR_800, A_ODR_1600 (1600 Hz)
  //dof.setAccelODR(dof.A_ODR_100); // Set accelerometer update rate at 100 Hz
  imu.setAccelODR(100);

  // Accelerometer anti-aliasing filter rate can be 50, 194, 362, or 763 Hz
  // Anti-aliasing acts like a low-pass filter allowing oversampling of accelerometer and rejection of high-frequency spurious noise.
  // Strategy here is to effectively oversample accelerometer at 100 Hz and use a 50 Hz anti-aliasing (low-pass) filter frequency
  // to get a smooth ~150 Hz filter update rate
  //  dof.setAccelABW(dof.A_ABW_50); // Choose lowest filter setting for low noise
  

  // Gyro output data rates can be: 95 Hz (bandwidth 12.5 or 25 Hz), 190 Hz (bandwidth 12.5, 25, 50, or 70 Hz)
  //                                 380 Hz (bandwidth 20, 25, 50, 100 Hz), or 760 Hz (bandwidth 30, 35, 50, 100 Hz)
//  dof.setGyroODR(dof.G_ODR_190_BW_125);  // Set gyro update rate to 190 Hz with the smallest bandwidth for low noise
  imu.setAccelODR(190);

  // Magnetometer output data rate can be: 3.125 (ODR_3125), 6.25 (ODR_625), 12.5 (ODR_125), 25, 50, or 100 Hz
//  dof.setMagODR(dof.M_ODR_125); // Set magnetometer to update every 80 ms
  imu.setMagODR(125);
  


}

//Update sensors and filterss
void SensorHub::update()
{
  
    now = millis();
    
    deltat = now - lastUpdate;
    
    lastUpdate = now;

    imu.readGyro();           // Read raw gyro data
    gyro.x = imu.calcGyro(imu.gx);   // Convert to degrees per seconds
    gyro.y = imu.calcGyro(imu.gy);
    gyro.z = imu.calcGyro(imu.gz);
    


 
    imu.readAccel();         // Read raw accelerometer data
    accel.x = imu.calcAccel(imu.ax);   // Convert to g's
    accel.y = imu.calcAccel(imu.ay);
    accel.z = imu.calcAccel(imu.az);


    
    imu.readMag();           // Read raw magnetometer data
    mag.x = -imu.calcMag(imu.mx);     // Convert to Gauss
    mag.y = imu.calcMag(imu.my);
    mag.z = imu.calcMag(imu.mz);
    

    
    
/*    myIMU.getCompassReg(*data, timestamp);

    //The MPU9250 needs the Z axis inverted and the x and y axis swapped for the magnetometer
    mag.y = ((float)data[0]/32767.0f) * magScale;
    mag.x = ((float)data[1]/32767.0f) * magScale; 
    mag.z = -((float)data[2]/32767.0f) * magScale;

*/    
    


#ifdef FLIP_X_Y

    float temp;

    temp = gyro.x;
    gyro.x = gyro.y;
    gyro.y = temp;

    temp = accel.x;
    accel.x = accel.y;
    accel.y = temp;
    
    temp = mag.x;
    mag.x = mag.y;
    mag.y = temp;

#endif

#ifdef INVERSE_Y

  gyro.y *= -1;
  accel.y *= -1;
  mag.y *= -1;

#endif


#ifdef INVERSE_X

  gyro.x *= -1;
  accel.x *= -1;
  mag.x *= -1;

#endif

#ifdef INVERSE_ACCEL_Y
  
  accel.y *= -1;

#endif

#ifdef INVERSE_ACCEL_Z

  accel.z *= -1;

#endif

#ifdef INVERSE_Z

  gyro.z *= -1;
  accel.z *= -1;
  mag.z *= -1;

#endif

#ifdef ALL_LEVEL

  orient.a = 0;
  orient.b = 0;
  orient.c = 0;
  orient.d = 1;

#else

    Serial.println("Gyro:");
    Serial.println(gyro.x);
    Serial.println(gyro.y);
    Serial.println(gyro.z);
    
    Serial.println("Accel:");
    Serial.println(accel.x);
    Serial.println(accel.y);
    Serial.println(accel.z);
    
    Serial.println("Mag:");
    Serial.println(mag.x);
    Serial.println(mag.y);
    Serial.println(mag.z);

    //The madgwick function works in radians. So the gyro readings need to be converted quick. 
    point radGyro;
    radGyro.x = gyro.x*PI/180.0f;
    radGyro.y = gyro.y*PI/180.0f;
    radGyro.z = gyro.z*PI/180.0f;

    orient = KalmanFilter::MadgwickQuaternionUpdate(accel, radGyro, mag, orient, deltat);

    Serial.println(orient.a);
    Serial.println(orient.b);
    Serial.println(orient.c);
    Serial.println(orient.d);

#endif


}

quaternion SensorHub::filteredOrientation()
{
	return orient;
}

point SensorHub::localToGlobal(point p)
{
	quaternion pointQ;

	pointQ.a = 0;
	pointQ.b = p.x;
	pointQ.c = p.y;
	pointQ.d = p.z;

	quaternion tempPoint = QuatOps::hProd(QuatOps::hProd(orient, pointQ),QuatOps::conj(orient));

	point finalPoint;

	finalPoint.x = tempPoint.b;
	finalPoint.y = tempPoint.c;
	finalPoint.z = tempPoint.d;

	return finalPoint;


}

point SensorHub::globalToLocal(point p)
{
	quaternion pointQ;

	pointQ.a = 0;
	pointQ.b = p.x;
	pointQ.c = p.y;
	pointQ.d = p.z;

	quaternion tempPoint = QuatOps::hProd(QuatOps::hProd(QuatOps::conj(orient), pointQ),orient);

	point finalPoint;

	finalPoint.x = tempPoint.b;
	finalPoint.y = tempPoint.c;
	finalPoint.z = tempPoint.d;

	return finalPoint;

}

point SensorHub::getAccel()
{
  return accel;
}

point SensorHub::getMag()
{
  return mag;
}

point SensorHub::getGyro()
{
  return gyro;
}

