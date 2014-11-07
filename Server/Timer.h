#ifndef _TIMER_H__
#define _TIMER_H__


#include "ICC_Timer_Handler.h"
#include "Options.h"


template<class T>
class CTimer: public CCTimer
{
public:
	virtual int ProcessOnTimerOut()
	{
		return m_pT->ProcessOnTimerOut(m_nId);
	}

	void SetTimeEventObj(T* obj, int id)
	{
		m_pT = obj;
		m_nId = id;
	}

	void StartTimer(long interval, bool isloop=false)		
	{			
		CCTimer::StartTimer(interval*1000,isloop);							
	}  

	virtual void StopTimer(void)
	{
		CCTimer::StopTimer();
	}	


private:
	int m_nId;
	T*	m_pT;

};




#endif

