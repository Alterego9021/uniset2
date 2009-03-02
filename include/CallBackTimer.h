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
 *  \author Pavel Vainerman <pv>
 *  \date   $Date: 2005/01/28 20:52:20 $
 *  \version $Id: CallBackTimer.h,v 1.5 2005/01/28 20:52:20 vitlav Exp $
*/
//----------------------------------------------------------------------------
# ifndef CallBackTimer_H_
# define CallBackTimer_H_
//----------------------------------------------------------------------------
#include <list>
#include "Exceptions.h"
#include "ThreadCreator.h"
#include "PassiveTimer.h"
//-----------------------------------------------------------------------------
/**
  @defgroup UniSetExceptions ����������
  @{
*/

namespace UniSetTypes
{
	class LimitTimers: 
		public UniSetTypes::Exception
	{
		public:
			LimitTimers():Exception("LimitTimers"){ printException(); }
	
			/*! ����������� ����������� ������� � ��������� �� ������ �������������� ���������� err */
			LimitTimers(const string err):Exception(err){ printException(); }
	};
};
//@}
// end of UniSetException group
//----------------------------------------------------------------------------------------

/*! 
 * \brief ������ 
 * \author Pavel Vainerman <pv>
 * \par
 * ������� �����, � ������� ���������� ������ ������ (10ms). ��������� ���������� �� CallBackTimer::MAXCallBackTimer ��������.
 * ��� ������������ ����� ������� ��������� ������� � ��������� \b Id �������, ������� ��������. 
 * ������� ��������� ������ ������ ������������� ������� CallBackTimer::Action.
 * ������ �������� �������:
 *	
	\code
		class MyClass
		{
			public:
				void Time(int id){ cout << "Timer id: "<< id << endl;}
		};
		
		MyClass* rec = new MyClass();
	 	...
 		CallBackTimer<MyClass> *timer1 = new CallBackTimer<MyClass>(rec);
		timer1->add(1, &MyClass::Time, 1000);
		timer1->add(5, &MyClass::Time, 1200);
		timer1->run();	
	\endcode
 *
 * \note ������ ��������� ������ CallBackTimer ������� �����, ������� \b ���������� �� ��������� ������ ������ ����������,
 * ��� ������ �������� (����� �� ��������� ����� �������).
*/ 
template <class Caller>
class CallBackTimer
//	public PassiveTimer
{ 
	public:

		/*! ������������ ���������� �������� */
		static const int MAXCallBackTimer = 20;

		/*! �������� ������� ������ */
		typedef void(Caller::* Action)( int id );	

		CallBackTimer(Caller* r, Action a);
		~CallBackTimer();

		// ���������� ��������
		void run();			/*!< ������ ������� */
		void terminate();	/*!< ���������	*/
		
		// ������ � ��������� (�� ������ ���������� PassiveTimer)
		void reset(int id);					/*!< ������������� ������ */
		void setTiming(int id, int timrMS);	/*!< ���������� ������ � ��������� */
		int getInterval(int id);			/*!< �������� ��������, �� ������� ���������� ������, � �� */
		int getCurrent(int id);				/*!< �������� ������� �������� ������� */


		void add( int id, int timeMS )throw(UniSetTypes::LimitTimers); /*!< ���������� ������ ������� */
		void remove( int id ); /*!< �������� ������� */
				
	protected:

		CallBackTimer();
		void work();
		
		void startTimers();
		void clearTimers();

	private:

		typedef CallBackTimer<Caller> CBT;
		friend class ThreadCreator<CBT>;
		Caller* cal;
		Action act;
		ThreadCreator<CBT> *thr;
		
		bool terminated;

		struct TimerInfo
		{
			TimerInfo(int id, PassiveTimer& pt):
				id(id), pt(pt){};

			int id;
			PassiveTimer pt;
		};
		
		typedef list<TimerInfo> TimersList;
		TimersList lst;

		// �������-������ ��� ������ �� id
		struct FindId_eq: public unary_function<TimerInfo, bool>
		{
			FindId_eq(const int id):id(id){}
			inline bool operator()(const TimerInfo& ti) const{return ti.id==id;}
			int id;
		};
};

#include "CallBackTimer_template.h"
# endif //CallBackTimer_H_
