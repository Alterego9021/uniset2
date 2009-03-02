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
 * \brief ���������� TimerService
 * \author Pavel Vainerman <pv>
 * \date $Date: 2007/06/17 21:30:56 $
 * \version $Id: TimerService.h,v 1.7 2007/06/17 21:30:56 vpashka Exp $
 */
//---------------------------------------------------------------------------
#ifndef TimerService_H_
#define TimerService_H_
//---------------------------------------------------------------------------
#include <list>
#include <functional>
#include <omnithread.h>
#include <string>
#include "UniSetTypes.h"
#include "UniSetObject.h"
#include "TimerService_i.hh"
#include "PassiveTimer.h"
//#include "OmniThreadCreator.h"
#include "ThreadCreator.h"

//-----------------------------------------------------------------------------------
/*!
 \page ServicesPage
 \section secTimerService ������ ��������
 \subsection subTS_common ����� ��������
  ������ ������ ������������� ����������� ������ ������������� ��������� 
  UniSetTypes::TimerMessage �.�. ��������.
  ������ �������� ����� ���������� ��������� ��������, ������������� ����������������. ��� ���� 
  �������������� ���������� ��� ��������. ��� ������ ���� ��������� ������ ��� ������ ���������.

  \note
  ������ �� ����������� �������� ����� � ������ ���������� ���������. �.�. � ���������� � ������ ������� 
 ����������� ����� �� ��������� � ������� ���������, �� ��� ����������� ��������� �������� �������� �������������.
	  
 \subsection subTS_idea �������� ������
 	�� ���� ����������� ���� ��������� �������. ������� ����� �������� ������, ����������� ���������:
	- ����� NameService
	- ��� ������ UniversalInterface::askTimer()

	\par 	
	������ �������� ���������, ������� ��� ������������� ����� �������� ��� ������ 
	UniSetTypes::Configuration::getTimerService() ������� UniSetTypes::conf.
	

	 \subsection subTS_interface ���������
	������ ������������� ���� ������� 
	\code
		void TimerService::askTimer( CORBA::Short timerid, CORBA::Long timeMS, const UniSetTypes::ConsumerInfo& ci,
								CORBA::Short ticks)
	\endcode
		��� ������ ������� �������������� ����� �������. 
	\param timerid - ���������� ������������� �������
	\par
		� ������ ������� �������� ������ � ��� ������������ (��� ������� ���������) ��������������� ��������������
	 ���������� TimerService_i::TimerAlreadyExist. 
	\param timeMS - ������ ����� �������������. ��� ������ �� ������� ���������� ������� timeMS=0.
	\param ticks ��������� ���������� ���������� �����������.
	\par
		���� ticks<0  ����������� ����� ���������, ���� �������� ��� �� ��������� �� ���. 
	 ����� ���������� �������� (\b �� \b ���� \b ����������) ���������� ( TimerService::MaxCountTimers ). 
	 � ������ ���������� ������� ������� �� ��� ������ ����� �������������� ���������� TimerService_i::LimitTimers.
	 ��� ����������� ����� �����������, � ��� �� ��� ����������� ����������� ������ �������, ����� ��������� �� �����
	 ���� ��������� TimerService-�� ��� ������������� ��������(�������) ����� ����. ��� ���� �� ������������� �������
	 ��������, �����������.

	\subsection subTS_lifetime ����� ����� ������
		��� ����, ����� �������������� ������ ������� � ��������� �� ���� �������� �������� �������
	"����� ����� ������". � ������, ���� � ������� TimerService::AskLifeTimeSEC �� ������� ������� ���������
	�����������, � ��������� ��� ������������� - ����� �����������.
	
	\note ��������� ����� �������� � ���������������� �����
	���������� ��. \ref TimerService
*/

 
/*! \class TimerService
 * �������� �� ������ PassiveTimer. 
*/
class TimerService: 
	public POA_TimerService_i,
	public UniSetObject
{
	public:
		
		TimerService( UniSetTypes::ObjectId id, const std::string confNodeName="LocalTimerService");
	    ~TimerService();

		//! ����� �������
		virtual void askTimer( const TimerService_i::Timer& ti, const UniSetTypes::ConsumerInfo& ci);
		void printList();

	protected:
		TimerService(const std::string confNodeName="LocalTimerService");

		/*! ���������� � ������� */
		struct TimerInfo
		{
			TimerInfo():id(0), curTimeMS(0){};
			TimerInfo(CORBA::Short id, CORBA::Long timeMS, const UniSetTypes::ConsumerInfo& cinf, 
						CORBA::Short cnt, unsigned int askLifeTime, UniSetTypes::Message::Priority p):
				cinf(cinf),
				ref(0),
				id(id), 
				curTimeMS(timeMS),
				priority(p),
				curTick(cnt-1),
				not_ping(false)
			{
				tmr.setTiming(timeMS);
				lifetmr.setTiming(askLifeTime*1000); // [���]
			};
			
			inline void reset()
			{
				curTimeMS = tmr.getInterval();
				tmr.reset();
			}
			
			UniSetTypes::ConsumerInfo cinf;		/*!<  ����� � ��������� */
			UniSetObject_i_var ref;				/*!<  ������ ��������� */
			UniSetTypes::TimerId id;			/*!<  ������������� ������� */
			int curTimeMS;						/*!<  ������� ������� */
			UniSetTypes::Message::Priority priority;	/*!<  ��������� ����������� ��������� */

			/*!
			 * ������� ����
			 * \note ���� ������ ���������� -1 �� ��������� ����� �������� ���������
			*/
			CORBA::Short curTick; 
			
			// �������� � ������� �������� �������� ����� ������� ���������
		   	bool operator < ( const TimerInfo& ti ) const
    		{ 
				return curTimeMS > ti.curTimeMS; 
			}

			PassiveTimer tmr;
			PassiveTimer lifetmr;	/*! ������ ����� ������ � ������ ���� ������ �� �������� */
			bool not_ping;			/* ������� ������������� ��������� */
		};

		typedef std::list<TimerInfo> TimersList; 			
		
		//! ������� ��������� � ����������� �������
		virtual bool send(TimerInfo& ti);

		//! �������������� ������� (���������������� ��� ����������� �������� ����� ������������)	 
		virtual bool disactivateObject();
		virtual bool activateObject();
		virtual void sigterm( int signo );


		unsigned int MaxCountTimers; 	/*!< ����������� ��������� ���������� �������� */	
		unsigned int AskLifeTimeSEC; 	/*!< [���] ����� ����� ������, ���� ������ ���������� */	

		void init(const std::string& confnode);
		void work();

	private:

		bool terminate;
		bool isSleep;
		
		UniSetTimer* sleepTimer;	/*!< ������ ��� ���������� ��������� � ���������� ������� */
		class Timer_eq: public std::unary_function<TimerInfo, bool>
		{
			public:			
				Timer_eq(const UniSetTypes::ConsumerInfo& coi, CORBA::Short t):tid(t),ci(coi){}

			inline bool operator()(const TimerInfo& ti) const
			{
				return ( ti.cinf.id == ci.id && ti.cinf.node == ci.node && ti.id == tid );
			}

			protected:
				UniSetTypes::TimerId tid;
				UniSetTypes::ConsumerInfo ci;
		};

		TimersList tlst; 				
		/*! ����� ��� ������������ ����������� ������� � c����� �������� */			
		UniSetTypes::uniset_mutex lstMutex; 
		int execute_pid; 
		ThreadCreator<TimerService>* exthread;		
};

#endif
