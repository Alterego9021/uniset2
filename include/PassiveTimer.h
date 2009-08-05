/* This file is part of the UniSet project
 * Copyright (c) 2002 Free Software Foundation, Inc.
 * Copyright (c) 2002 Pavel Vainerman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
// --------------------------------------------------------------------------
/*! \file
 *  \author Vitaly Lipatov, Pavel Vainerman
 *  \date   $Date: 2007/08/02 22:52:27 $
 *  \version $Id: PassiveTimer.h,v 1.9 2007/08/02 22:52:27 vpashka Exp $
*/
//---------------------------------------------------------------------------- 
# ifndef PASSIVETIMER_H_
# define PASSIVETIMER_H_
//----------------------------------------------------------------------------
#include <signal.h>
#include <sys/time.h>
#include <cc++/socket.h>
//#include "Exceptions.h"


//----------------------------------------------------------------------------------------
/*! \class UniSetTimer
 * \brief ������� ��������� �������� ��������
 * \author Pavel Vainerman
 * \date  $Date: 2007/08/02 22:52:27 $
 * \version $Id: PassiveTimer.h,v 1.9 2007/08/02 22:52:27 vpashka Exp $
*/ 
class UniSetTimer
{
	public:
		virtual ~UniSetTimer(){};
	
		virtual bool checkTime()=0;				/*!< �������� ����������� ��������� ������� */
		virtual timeout_t setTiming( timeout_t timeMS )=0; /*!< ���������� ������ � ��������� */
		virtual void reset()=0;					/*!< ������������� ������ */

		virtual timeout_t getCurrent()=0; 		/*!< �������� ������� �������� ������� */
		virtual timeout_t getInterval()=0;		/*!< �������� ��������, �� ������� ���������� ������, � �� */
		timeout_t getLeft(timeout_t timeout)		/*< �������� �����, ������� �������� �� timeout ����� ���������� ������� getCurrent() */
		{
			timeout_t ct = getCurrent();
			if( timeout <= ct )
				return 0;
			return timeout - ct;
		}
		
		// ��������� �� ����� ������������ �.�.
		// ��������� ������ ����� �� ����� ��������
		// �������.
		virtual bool wait(timeout_t timeMS){ return 0;} 	/*!< ������� ������ ����������� ������� */
		virtual void terminate(){}					/*!< �������� ������ ������� */
		virtual void stop(){ terminate(); };		/*!< ��������� ������ ������� */

		/*! ����� ���������, �� ������� ���� �� ����� ������� ������� ����������
		 *  terminate() ��� stop()
		 */
		static const timeout_t WaitUpTime = TIMEOUT_INF;
		
		/*! ����������� ����� ������������. �������� � ����. */
		static const timeout_t MinQuantityTime = 30;
		static const timeout_t MIN_QUANTITY_TIME_MS = 30; /*< ��������, �� ������������! */
};
//----------------------------------------------------------------------------------------
/*! \class PassiveTimer
 * \brief ��������� ������
 * \author Vitaly Lipatov
 * \date  $Date: 2007/08/02 22:52:27 $
 * \version $Id: PassiveTimer.h,v 1.9 2007/08/02 22:52:27 vpashka Exp $
 * \par
 * ��������� ������ � ������������ ��� � ������� setTiming,
 * ����� � ������� checkTime ���������, �� ��������� �� ������ �����
 * \note ���� timeMS<0, ������ ������� �� ���������
 * \note timeMS=0 - ������ ��������� �����
*/ 
class PassiveTimer: 
		public UniSetTimer
{
public:
	PassiveTimer();
	PassiveTimer( timeout_t timeMS ); 			/*!< ���������� ������ */
	

	virtual bool checkTime();				/*!< �������� ����������� ��������� ������� */
	virtual timeout_t setTiming( timeout_t timeMS ); 	/*!< ���������� ������ � ��������� */
	virtual void reset();					/*!< ������������� ������ */

	virtual timeout_t getCurrent(); 				/*!< �������� ������� �������� �������, � �� */
	virtual timeout_t getInterval()				/*!< �������� ��������, �� ������� ���������� ������, � �� */
	{
		return timeSS*10;
	}
	
	virtual void terminate();				/*!< �������� ������ ������� */

protected:
	clock_t timeAct;	/*!< ����� ������������ �������, � ����� */
	timeout_t timeSS;		/*!< �������� �������, � ������������� */
	clock_t timeStart;	/*!< ����� ��������� ������� (������) */
private:
	clock_t clock_ticks; // ���������� ����� � �������
	clock_t times(); // ���������� ������� ����� � �����
};

//----------------------------------------------------------------------------------------
class omni_mutex;
class omni_condition;

/*! \class ThrPassiveTimer
 * \brief ��������� ������ � ������� ��������� (��������)
 * \author Pavel Vainerman
 * \date  $Date: 2007/08/02 22:52:27 $
 * \version $Id: PassiveTimer.h,v 1.9 2007/08/02 22:52:27 vpashka Exp $
 * \par
 * ��������� ������� �� �������� ����� wait(timeout_t timeMS).
 * �������� �������� �� ������ �������� ���������� ������� (mutex � condition). 
 * \note ���� ������ ������� � ������ �������� (WaitUpTime), �� �� ����� ���� ������� �� ����
 * ��� ������ terminate().
*/ 
class ThrPassiveTimer:
		public PassiveTimer
{ 
	public:
	
		ThrPassiveTimer();
		~ThrPassiveTimer();

		virtual bool wait(timeout_t timeMS);	/*!< ����������� ���������� ����� �� �������� ����� */
		virtual void terminate();		/*!< �������� ������ ������� */
	protected:
	private:
		volatile sig_atomic_t terminated;
		omni_mutex* tmutex;
		omni_condition* tcondx;
};
//----------------------------------------------------------------------------------------

/*! \class PassiveSysTimer
 * \brief ��������� ������ � ������� ��������� (��������)
 * \author Pavel Vainerman
 * \date  $Date: 2007/08/02 22:52:27 $
 * \version $Id: PassiveTimer.h,v 1.9 2007/08/02 22:52:27 vpashka Exp $
 * \par
 * ������ �� ������ ������� (SIGALRM).
*/ 
class PassiveSysTimer:
		public PassiveTimer
{ 
	public:
	
		PassiveSysTimer();
		~PassiveSysTimer();

		virtual bool wait(timeout_t timeMS); //throw(UniSetTypes::NotSetSignal);
		virtual void terminate();
		
	protected:

	private:
		struct itimerval mtimer;
		pid_t pid;	

//		bool terminated;
		volatile sig_atomic_t terminated;
		
		void init();

		static void callalrm(int signo );
		static void call(int signo, siginfo_t *evp, void *ucontext);
		
};

//----------------------------------------------------------------------------------------

# endif //PASSIVETIMER_H_
