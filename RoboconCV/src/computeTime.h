#ifndef _COMPUTE_TIME_H
#define _COMPUTE_TIME_H

#include <iostream>
#include <Windows.h>

class ComputeTime    
{  
public:
    ComputeTime();
    virtual ~ComputeTime();
	double End();
	bool Begin();
	bool Avaliable();
	
private:  
	int Initialized;  
	__int64 Frequency;  
	__int64 BeginTime;  

};  

#endif