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
 *  \author Pavel Vainerman
 *  \date   $Date: 2007/08/02 22:52:28 $
 *  \version $Id: ThrPassiveTimer.cc,v 1.7 2007/08/02 22:52:28 vpashka Exp $
*/
// -------------------------------------------------------------------------- 

#include <unistd.h>
#include <sstream>
#include <time.h>
#include <omnithread.h>

#include "PassiveTimer.h"

// ------------------------------------------------------------------------------------------
using namespace std;
// ------------------------------------------------------------------------------------------

ThrPassiveTimer::ThrPassiveTimer():
	terminated(1)
{
	// ���� ������� �����������
	// ����� ���� �� include � head-�����
	tmutex = new omni_mutex();
	tcondx = new omni_condition(tmutex);
}
// ------------------------------------------------------------------------------------------
ThrPassiveTimer::~ThrPassiveTimer()
{
	terminate();
//	while( !terminated ){};
	delete tcondx;
	delete tmutex;
}
// ------------------------------------------------------------------------------------------
void ThrPassiveTimer::terminate()
{
	if( !terminated )
	{
//		tmutex->lock();
		terminated = 1;
		tcondx->signal();
//		tmutex->unlock();
	}
}
// ------------------------------------------------------------------------------------------
int ThrPassiveTimer::wait(int timeMS)
{
	terminated = 0;
	{
		tmutex->lock();
		PassiveTimer::setTiming(timeMS); // �������� ��� ������������� � ������� PassiveTimer-��
		if( timeMS < 0 || timeMS == WaitUpTime )
		{
			while( !terminated )	// �� ������, ����� ��������� �� ������...
				tcondx->wait();
		}
		else
		{
			if( timeMS < MIN_QUANTITY_TIME_MS )
				timeMS = MIN_QUANTITY_TIME_MS;
				
			unsigned long sec, msec;
			omni_thread::get_time(&sec,&msec, timeMS/1000, (timeMS%1000)*1000000 );
//			cout <<"timer: ���� "<< timeMS/1000 << "[���] � " << (timeMS%1000)*1000000 <<"[����]" << endl;
			tcondx->timedwait(sec, msec);		
		}

		tmutex->unlock();
	}

	terminated = 1;
	return 1;	
}
// ------------------------------------------------------------------------------------------

