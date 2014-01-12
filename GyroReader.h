#include "WPILib.h"
#include <math.h>

#define kGyroDeadzone 0.05

/*
 * Reads gyro values, and corrects for accumulating error drift by deadzoning
 * 
 */

class GyroReader
{
	Gyro *inputGyro;
	float angle; // Cumulative sum...
	
public:
	GyroReader(Gyro *g)
	{
		inputGyro = g;
		angle = 0;
	}
	
	float Read()
	{
		float dA = inputGyro->GetAngle();
		
		if(fabs(dA) < kGyroDeadzone)
		{
			dA = 0;
		}
		
		angle += dA;
		inputGyro->Reset();
		
		return angle;	
	}
	
	void Reset()
	{
		angle = 0;
		inputGyro->Reset();
	}
	
};
