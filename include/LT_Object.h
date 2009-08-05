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
 * \author Pavel Vainerman
 * \date $Date: 2006/12/20 10:39:01 $
 * \version $Id: LT_Object.h,v 1.5 2006/12/20 10:39:01 vpashka Exp $
 */
//---------------------------------------------------------------------------
#ifndef Object_LT_H_
#define Object_LT_H_
//--------------------------------------------------------------------------
#include <list>
#include "UniSetTypes.h"
#include "MessageType.h"
#include "PassiveTimer.h"
#include "Exceptions.h"

//---------------------------------------------------------------------------
class UniSetObject;
//---------------------------------------------------------------------------
/*! \class LT_Object

	\note '_LT' - ��� "local timers".
 	����� ����������� ������� ��������� ��������. ������������ ����� ��ģ���� ������
	�.�. ��������� ��������� ��� ���̣����� ������ �������� � Tim�Servic-�. 
	�� ������� �������, ��� ��� ���� ������ ������������ ����� ������� ���������� ����� ������ϣ����, 
	�.�. �� ����� ������ ����� ��������� ��������� �� "����", ��� � �������� UniSetObject-�, � ������
	����� �� �������� �������� (������ ��� �������, ��� � ������ ���� ������ ���� �����)
	
	\par �������� �������
		��������� ������ �������� � ��� ������������ ��������� ����������� ����������� UniSetTypes::TimerMessage,
	������� ���������� � ������� ���������� �������. ������ ��� ����������������� ������ � ���������� ����� ����������
	�� ���������� ������������. ���� � ������ �� �������� �� ������ ������� - ���������� UniSetTimers::WaitUpTime.

	��������� ��� ������������� �������� ���:

	\code
		class MyClass:
			public UniSetObject
		{
			...
			int sleepTime;
			UniSetObject_LT lt;
			void callback();
		
		}

		void callback()
		{
			// ��� ���������� � �������������� waitMessage() ������ ��� ��� ������ askTimer() ���������� 
			// ��������� ������������ �������� �� UniSetTimers::WaitUpTime � �������� termWaiting(), 
			// ����� �������� ��������, ����� ������� �� ������ ������� '����'(� ������� waitMessage()) � ����� 
			// ������ ��������� �����(�.�. ���������� ������ �� �����)...

			try
			{	
				if( waitMessage(msg, sleepTime) )
					processingMessage(&msg);
	
				sleepTime=lt.checkTimers(this);
			}
			catch(Exception& ex)
			{
				cout << myname << "(callback): " << ex << endl;
			}
		}
		
		void askTimers()
		{
			// ���������� ������������ ��������
			if( lt.askTimer(Timer1, 1000) != UniSetTimer::WaitUpTime )
				termWaiting();
		}
		
	\endcode	
		
	
	\warning �������� ������ ������������ �������������� ������ ������������.
	\sa TimerService
*/ 
class LT_Object
{
	public:
		LT_Object();
		virtual ~LT_Object();


		/*! ����� �������
			\param timerid - ������������� �������
			\param timeMS - ������. 0 - �������� ����� �� �������
			\param ticks - ���������� �����������. "-1"- ���������
			\param p - ��������� ������������ ���������
			\return ���������� ����� [����] ���������� �� ������������
			���������� �������
		*/
		int askTimer( UniSetTypes::TimerId timerid, long timeMS, short ticks=-1, 
						UniSetTypes::Message::Priority p=UniSetTypes::Message::High );


		/*! 
			�������� ������� ���������.
			\param obj - ��������� �� ������, �������� ���������� �����������
			\return ���������� ����� [����] ���������� �� ������������
				���������� �������
		*/
		int checkTimers( UniSetObject* obj );

		/*! �������� ������� ����� �������� */
		inline int getSleepTimeMS(){ return sleepTime; }

	protected:

		/*! ���������� � ������� */
		struct TimerInfo
		{
			TimerInfo():id(0), curTimeMS(0), priority(UniSetTypes::Message::High){};
			TimerInfo(UniSetTypes::TimerId id, long timeMS, short cnt, UniSetTypes::Message::Priority p):
				id(id),
				curTimeMS(timeMS),
				priority(p),
				curTick(cnt-1)
			{
				tmr.setTiming(timeMS);
			};
			
			inline void reset()
			{
				curTimeMS = tmr.getInterval();
				tmr.reset();
			}
			
			UniSetTypes::TimerId id;	/*!<  ������������� ������� */
			int curTimeMS;				/*!<  ������� ������� */
			UniSetTypes::Message::Priority priority; /*!<  ��������� ����������� ��������� */

			/*!
			 * ������� ����
			 * \note ���� ������ ���������� -1 �� ��������� ����� �������� ���������
			*/
			short curTick; 
			
			// ������ � ������� �������� �������� ����� ������� ���������
			bool operator < ( const TimerInfo& ti ) const
			{ 
				return curTimeMS > ti.curTimeMS; 
			}

			PassiveTimer tmr;
		};

		class Timer_eq: public std::unary_function<TimerInfo, bool>
		{
			public:			
				Timer_eq(UniSetTypes::TimerId t):tid(t){}

			inline bool operator()(const TimerInfo& ti) const
			{
				return ( ti.id == tid );
			}

			protected:
				UniSetTypes::TimerId tid;
		};
		
		typedef std::list<TimerInfo> TimersList;

	private:
		TimersList tlst; 
		/*! ����� ��� ������������ ����������� ������� � c����� �������� */
		UniSetTypes::uniset_mutex lstMutex; 
		timeout_t sleepTime; /*!< ������� ����� �������� */
		PassiveTimer tmLast;
};
//--------------------------------------------------------------------------
#endif
