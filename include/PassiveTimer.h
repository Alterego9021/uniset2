/* This file is part of the UniSet project
 * Copyright (c) 2002 Free Software Foundation, Inc.
 * Copyright (c) 2002 Pavel Vainerman <pv>
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
 *  \author Vitaly Lipatov <lav>, Pavel Vainerman <pv>
 *  \date   $Date: 2007/08/02 22:52:27 $
 *  \version $Id: PassiveTimer.h,v 1.9 2007/08/02 22:52:27 vpashka Exp $
*/
//---------------------------------------------------------------------------- 
# ifndef PASSIVETIMER_H_
# define PASSIVETIMER_H_
//----------------------------------------------------------------------------
#include <signal.h>
#include <sys/time.h>
//#include "Exceptions.h"

//----------------------------------------------------------------------------------------
// CLK_TCK �������� �� ������ ���������
#ifndef CLK_TCK
#define CLK_TCK sysconf(_SC_CLK_TCK)
#endif
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
/*! \class UniSetTimer
 * \brief ������� ��������� �������� ��������
 * \author Pavel Vainerman <pv>
 * \date  $Date: 2007/08/02 22:52:27 $
 * \version $Id: PassiveTimer.h,v 1.9 2007/08/02 22:52:27 vpashka Exp $
*/ 
class UniSetTimer
{
	public:
		virtual ~UniSetTimer(){};
	
		virtual bool checkTime()=0;				/*!< �������� ����������� ��������� ������� */
		virtual void setTiming( int timeMS )=0; /*!< ���������� ������ � ��������� */
		virtual void reset()=0;					/*!< ������������� ������ */

		virtual int getCurrent()=0; 		/*!< �������� ������� �������� ������� */
		virtual int getInterval()=0;		/*!< �������� ��������, �� ������� ���������� ������, � �� */
		
		// ��������� �� ����� ������������ �.�.
		// ��������� ������ ����� �� ����� ��������
		// �������.
		virtual int wait(int timeMS){ return 0;} 	/*!< ������� ������ ����������� ������� */
		virtual void terminate(){}					/*!< �������� ������ ������� */
		virtual void stop(){ terminate(); };		/*!< ��������� ������ ������� */

		/*! ����� ���������, �� ������� ���� �� ����� ������� ������� ����������
		 *  terminate() ��� stop()
		 */
		static const int WaitUpTime = -1;
		
		/*! ����������� ����� ������������. �������� � ����. */
		static const int MIN_QUANTITY_TIME_MS = 30;
};
//----------------------------------------------------------------------------------------
/*! \class PassiveTimer
 * \brief ��������� ������
 * \author Vitaly Lipatov <lav>
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
	PassiveTimer( int timeMS ); 			/*!< ���������� ������ */
	

	virtual bool checkTime();				/*!< �������� ����������� ��������� ������� */
	virtual void setTiming( int timeMS ); 	/*!< ���������� ������ � ��������� */
	virtual void reset();					/*!< ������������� ������ */

	virtual int getCurrent(); 				/*!< �������� ������� �������� �������, � �� */
	virtual int getInterval()				/*!< �������� ��������, �� ������� ���������� ������, � �� */
	{
		return timeSS*10;
	}
	
	virtual void terminate();				/*!< �������� ������ ������� */

protected:
	int timeAct;	/*!< ����� ������������ �������, � ����� */
	int timeSS;		/*!< �������� �������, � ������������� */
	int timeStart;	/*!< ����� ��������� ������� (������) */
private:
	int clock_ticks; // CLK_TCK
};

//----------------------------------------------------------------------------------------
class omni_mutex;
class omni_condition;

/*! \class ThrPassiveTimer
 * \brief ��������� ������ � ������� ��������� (��������)
 * \author Pavel Vainerman <pv>
 * \date  $Date: 2007/08/02 22:52:27 $
 * \version $Id: PassiveTimer.h,v 1.9 2007/08/02 22:52:27 vpashka Exp $
 * \par
 * ��������� ������� �� �������� ����� wait(int timeMS).
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

		virtual int wait(int timeMS);	/*!< ����������� ���������� ����� �� �������� ����� */
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
 * \author Pavel Vainerman <pv>
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

		virtual int wait(int timeMS); //throw(UniSetTypes::NotSetSignal);
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
