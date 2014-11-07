#ifndef  _ICC_TIMER_HANDLE_H_
#define  _ICC_TIMER_HANDLE_H_

#include "timerlist.h"
#include "CCReactor.h"

class ICC_Timer_Handler: public CTimerObject
{
	public:
		virtual void StartTimer(long interval, bool isloop=false)=0;
		virtual void StopTimer(void)=0;
	
	protected:
		virtual int ProcessOnTimerOut()=0;

};

class CCTimer: public ICC_Timer_Handler
{
	public:
		virtual void StartTimer(long interval, bool isloop=false)
		{
			this->_interval = interval;
			this->_isloop = isloop;
			CCReactor::Instance()->RegistTimer(this,interval);

		}
		virtual void StopTimer(void)
		{
			CTimerObject::DisableTimer();
			this->_isloop=false;
		}

		virtual void TimerNotify(void)
		{
			ProcessOnTimerOut();
			if(_isloop)
				StartTimer( _interval, _isloop);
		}	
	protected:
		virtual int ProcessOnTimerOut()=0;

	private:
		bool _isloop;
		long _interval;

};

#endif

