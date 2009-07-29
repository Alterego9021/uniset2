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
 *  \date   $Date: 2007/11/29 22:19:54 $
 *  \version $Id: UniSetObject.cc,v 1.27 2007/11/29 22:19:54 vpashka Exp $
*/
// -------------------------------------------------------------------------- 
#include <unistd.h>
#include <signal.h>
#include <iomanip>
#include <pthread.h>
#include <sys/types.h>
#include <sstream>

#include "Exceptions.h"
#include "ORepHelpers.h"
#include "ObjectRepository.h"
#include "UniversalInterface.h"
#include "UniSetObject.h"
#include "ObjectsManager.h"
#include "Debug.h"

// ------------------------------------------------------------------------------------------
using namespace std;
using namespace UniSetTypes;

#define CREATE_TIMER	new ThrPassiveTimer(); 	
// new PassiveSysTimer();

// ------------------------------------------------------------------------------------------
UniSetObject::UniSetObject():
ui(UniSetTypes::DefaultObjectId),
mymngr(NULL),
msgpid(0),
reg(false),
active(false),
threadcreate(false),
tmr(NULL),
myid(UniSetTypes::DefaultObjectId),
oref(0),
thr(NULL),
SizeOfMessageQueue(1000),
MaxCountRemoveOfMessage(10),
stMaxQueueMessages(0),
stCountOfQueueFull(0)
{
	tmr = CREATE_TIMER;
	myname = "noname";
	section = "nonameSection";
	
	SizeOfMessageQueue = atoi(conf->getField("SizeOfMessageQueue").c_str());
	if( SizeOfMessageQueue <= 0 )
		SizeOfMessageQueue = 1000;
	
	MaxCountRemoveOfMessage = atoi(conf->getField("MaxCountRemoveOfMessage").c_str());
	if( MaxCountRemoveOfMessage <= 0 )
		MaxCountRemoveOfMessage = SizeOfMessageQueue / 4;
	if( MaxCountRemoveOfMessage <= 0 )
		MaxCountRemoveOfMessage = 10;
	recvMutexTimeout = atoi(conf->getField("RecvMutexTimeout").c_str());
	if( recvMutexTimeout <= 0 )
		recvMutexTimeout = 10000;

	pushMutexTimeout = atoi(conf->getField("PushMutexTimeout").c_str());
	if( pushMutexTimeout <= 0 )
		pushMutexTimeout = 9000;
}
// ------------------------------------------------------------------------------------------
UniSetObject::UniSetObject( ObjectId id ):
ui(id),
mymngr(NULL),
msgpid(0),
reg(false),
active(false),
threadcreate(true),
tmr(NULL),
myid(id),
oref(0),
thr(NULL),
SizeOfMessageQueue(1000),
MaxCountRemoveOfMessage(10),
stMaxQueueMessages(0),
stCountOfQueueFull(0)
{
	SizeOfMessageQueue = atoi(conf->getField("SizeOfMessageQueue").c_str());
	if( SizeOfMessageQueue <= 0 )
		SizeOfMessageQueue = 1000;
	
	MaxCountRemoveOfMessage = atoi(conf->getField("MaxCountRemoveOfMessage").c_str());
	if( MaxCountRemoveOfMessage <= 0 )
		MaxCountRemoveOfMessage = SizeOfMessageQueue / 4;
	if( MaxCountRemoveOfMessage <= 0 )
		MaxCountRemoveOfMessage = 10;

	tmr = CREATE_TIMER;
	if (myid >=0)
	{
		string myfullname = ui.getNameById(id);
		myname = ORepHelpers::getShortName(myfullname.c_str());
		section = ORepHelpers::getSectionName(myfullname.c_str());
	}
	else
	{
		threadcreate = false;
		myid=UniSetTypes::DefaultObjectId;
		myname = "noname";
		section = "nonameSection";
	}
	recvMutexTimeout = atoi(conf->getField("RecvMutexTimeout").c_str());
	if( recvMutexTimeout <= 0 )
		recvMutexTimeout = 10000;

	pushMutexTimeout = atoi(conf->getField("PushMutexTimeout").c_str());
	if( pushMutexTimeout <= 0 )
		pushMutexTimeout = 9000;

}


UniSetObject::UniSetObject(const string name, const string section):
ui(UniSetTypes::DefaultObjectId),
mymngr(NULL),
msgpid(0),
reg(false),
active(false),
threadcreate(true),
tmr(NULL),
myid(UniSetTypes::DefaultObjectId),
oref(0),
thr(NULL),
SizeOfMessageQueue(1000),
MaxCountRemoveOfMessage(10),
stMaxQueueMessages(0),
stCountOfQueueFull(0)
{
	/*! \warning UniverslalInterface �� ���������������� ��������������� ������� */
	tmr = CREATE_TIMER;
	myname = section+"/"+name;
	myid = ui.getIdByName(myname.c_str());
	if( myid == DefaultObjectId )
	{
		unideb[Debug::WARN] << "name: �� ������ ������������� � ObjectsMap!!!" << endl;
		throw Exception(name+": �� ������ ������������� � ObjectsMap!!!");
	}
	SizeOfMessageQueue = atoi(conf->getField("SizeOfMessageQueue").c_str());
	if( SizeOfMessageQueue <= 0 )
		SizeOfMessageQueue = 1000;
	
	MaxCountRemoveOfMessage = atoi(conf->getField("MaxCountRemoveOfMessage").c_str());
	if( MaxCountRemoveOfMessage <= 0 )
		MaxCountRemoveOfMessage = SizeOfMessageQueue / 4;
	if( MaxCountRemoveOfMessage <= 0 )
		MaxCountRemoveOfMessage = 10;

	recvMutexTimeout = atoi(conf->getField("RecvMutexTimeout").c_str());
	if( recvMutexTimeout <= 0 )
		recvMutexTimeout = 10000;

	pushMutexTimeout = atoi(conf->getField("PushMutexTimeout").c_str());
	if( pushMutexTimeout <= 0 )
		pushMutexTimeout = 9000;

	ui.initBackId(myid);
}

// ------------------------------------------------------------------------------------------
UniSetObject::~UniSetObject() 
{
	disactivate();
	delete tmr;
	if(thr)
		delete thr;
}
// ------------------------------------------------------------------------------------------
/*!
 *	\param om - ���������� �� ������� ����������� ��������
 *	\return ��������� \a true ���� ������������� ������ �������, � \a false ���� ���
*/
bool UniSetObject::init( ObjectsManager* om )
{
	if( unideb.debugging(Debug::INFO) )
		unideb[Debug::INFO] << myname << ": ������������� ..." << endl;
	this->mymngr = om;
	if( unideb.debugging(Debug::INFO) )
		unideb[Debug::INFO] << myname << ": ok..." << endl;
	return true;
}
// ------------------------------------------------------------------------------------------
void UniSetObject::setID( UniSetTypes::ObjectId id )
{
	if( myid!=UniSetTypes::DefaultObjectId )
		throw ObjectNameAlready("ObjectId ��� ����� (setID)");

	string myfullname = ui.getNameById(id);
	myname = ORepHelpers::getShortName(myfullname.c_str()); 
	section = ORepHelpers::getSectionName(myfullname.c_str());
	myid = id;
	ui.initBackId(myid);
}

// ------------------------------------------------------------------------------------------
/*!
 *	\param VoidMessage msg - ��������� �� ���������, ������� ����������� ���� ���� ���������
 *	\return ��������� \a true ���� ��������� ����, � \a false ���� ���
*/
bool UniSetObject::receiveMessage( VoidMessage& vm )
{
	{	// lock
		uniset_mutex_lock mlk(qmutex, recvMutexTimeout);
			
		if( !queueMsg.empty() )
		{
			// �������� ������������
			if( queueMsg.size() > SizeOfMessageQueue ) 
			{
				unideb[Debug::CRIT] << myname <<": ������������ ������� ���������!" << endl << flush;
				cleanMsgQueue(queueMsg);
				// ��������� ���������� �� �������������
				stCountOfQueueFull++;
				stMaxQueueMessages=0;	
			}

			if( !queueMsg.empty() )
			{
				vm = queueMsg.top(); // �������� ���������
//				�������� �� ���������������� ���������			
//				cout << myname << ": receive message....tm=" << vm.time << " msec=" << vm.time_msec << "\tprior="<< vm.priority << endl;
				queueMsg.pop(); // ������� ��������� �� �������
				return true;
			}
		}	
	} // unlock queue

	return false;	
}

// ------------------------------------------------------------------------------------------
// ��������� ������������ ����������� ���������� �����
// �� ������� ����� ������ � �������� ���������
// ������������ �������� � ������ � ������� ������� ������� ���������
struct MsgInfo
{
	MsgInfo():
	type(Message::Unused),
	id(DefaultObjectId),
	acode(DefaultMessageCode),
	ccode(DefaultMessageCode),
	ch(0),
	node(DefaultObjectId)
	{
//		struct timezone tz;
		tm.tv_sec = 0;
		tm.tv_usec = 0;
//		gettimeofday(&tm,&tz);
	}

	MsgInfo( AlarmMessage& am ):
	type(am.type),
	id(am.id),
	acode(am.alarmcode),
	ccode(am.causecode),
	ch(am.character),
	tm(am.tm),
	node(am.node)
	{}

	MsgInfo( InfoMessage& am ):
	type(am.type),
	id(am.id),
	acode(am.infocode),
	ccode(0),
	ch(am.character),
	tm(am.tm),
	node(am.node)
	{}

	MsgInfo( ConfirmMessage& am ):
	type(am.orig_type),
	id(am.orig_id),
	acode(am.code),
	ccode(am.orig_cause),
	ch(0),
	tm(am.orig_tm),
	node(am.orig_node)
	{}

	int type;
	ObjectId id;		// �� ����
	MessageCode acode;	// ��� ���������
	MessageCode ccode;	// ��� �������
	int ch;				// ��������
	struct timeval tm;	// �����
	ObjectId node;		// ������

   	inline bool operator < ( const MsgInfo& mi ) const
	{
		if( type != mi.type )
			return type < mi.type; 

		if( id != mi.id )
			return id < mi.id;

		if( node != mi.node )
			return node < mi.node; 

		if( acode != mi.acode )
			return acode < mi.acode;

		if( ch != mi.ch )
			return ch < mi.ch;

		if( tm.tv_sec != mi.tm.tv_sec )
			return tm.tv_sec < mi.tm.tv_sec;

		return tm.tv_usec < mi.tm.tv_usec;
	}	
	
};

// ------------------------------------------------------------------------------------------
bool UniSetObject::waitMessage(VoidMessage& vm, int timeMS)
{
	if( receiveMessage(vm) )
		return true;
	tmr->wait(timeMS);
	return receiveMessage(vm);
}
// ------------------------------------------------------------------------------------------
void UniSetObject::registered()
{
	if( unideb.debugging(Debug::INFO) )
		unideb[Debug::INFO] << myname << ": ����������� ..." << endl;

	if( myid == UniSetTypes::DefaultObjectId )
	{
		if( unideb.debugging(Debug::INFO) )
			unideb[Debug::INFO] << myname << "(registered): myid=DefaultObjectId \n";
		return;
	}

	if( !mymngr )
	{
		unideb[Debug::WARN] << myname << "(registered): �� ����� ��������" << endl;
		string err(myname+": �� ����� ��������");
		throw ORepFailed(err.c_str());
	}

	if( !oref )
	{
		unideb[Debug::CRIT] << myname << "(registered): ������� ������ oref!!!..." << endl;
		return;
	}

	try
	{
		for( int i=0; i<2; i++ )
		{		
			try
			{
				ui.registered(myid, getRef(),true);
				break;
			}
			catch( ObjectNameAlready& al )
			{
/*! 
	\warning �� ��������� ������� ������ ���� ���������! ������� ���� �ģ� ������� ��������� �����������. 
	�� ������ ������������ ������ � �������� ţ �� �����.	
	��� ������� ��� ����� �������� ������, ����� ����� ���������, ��� ���� ������ ����� �����������
	�� ������� �� ����� ������(�� ������������������), �� ������ �� ������� �� ������ ����� ������������������.
	�.�. \b ��ģ���� ������� �������� "���" �� ������ ���� ���...
	(��� �� ����� ���� ��������� � ���� "�� ���", �� ����� �������� ������ �� �����). �� ���������� �������� �������:
	���� ���������� ������ "���" � �������� ���� ������, �� �� ����� ��������� �� ����� ������ � ��� ����� ���(�����) 
	������ ������ ���������� ������, � ����� �� ���� �� �����!!!
	
*/
				unideb[Debug::CRIT] << myname << "(registered): ������� ������������ ������ (ObjectNameAlready)" << endl;
				reg = true;
				unregister();
//				unideb[Debug::CRIT] << myname << "(registered): �� ���� ������������������ � ����������� �������� (ObjectNameAlready)" << endl;
//				throw al;
			}
		}
	}
	catch( ORepFailed )
	{
		string err(myname+": �� ���� ������������������ � ����������� ��������");
		throw ORepFailed(err.c_str());
	}
	catch(Exception& ex)
	{
		unideb[Debug::WARN] << myname << "(registered):  " << ex << endl;
		string err(myname+": �� ���� ������������������ � ����������� ��������");
		throw ORepFailed(err.c_str());
	}
	reg = true;
}
// ------------------------------------------------------------------------------------------
void UniSetObject::unregister()
{
	if( myid<0 ) // || !reg )
		return;

	if( myid == UniSetTypes::DefaultObjectId )
	{
		if( unideb.debugging(Debug::INFO) )
			unideb[Debug::INFO] << myname << "(unregister): myid=DefaultObjectId \n";
		reg = false;
		return;
	}

	if( !oref )
	{
		unideb[Debug::WARN] << myname << "(unregister): ������� ������ oref!!!..." << endl;
		reg = false;
		return;
	}


	try
	{
		if( unideb.debugging(Debug::INFO) )
			unideb[Debug::INFO] << myname << ": unregister "<< endl;

		ui.unregister(myid);

		if( unideb.debugging(Debug::INFO) )
			unideb[Debug::INFO] << myname << ": unregister ok. "<< endl;
	}
	catch(...)
	{
		unideb[Debug::WARN] << myname << ": �� ���� ������������������� � ����������� ��������" << endl;
	}
	
	reg = false;
}
// ------------------------------------------------------------------------------------------
CORBA::Boolean UniSetObject::exist()
{
	return true;
}
// ------------------------------------------------------------------------------------------
void UniSetObject::termWaiting()
{
    if( tmr!=NULL )
		tmr->terminate();
}
// ------------------------------------------------------------------------------------------
void UniSetObject::setRecvMutexTimeout( unsigned long msec )
{
	recvMutexTimeout = msec;
}
// ------------------------------------------------------------------------------------------
void UniSetObject::setPushMutexTimeout( unsigned long msec )
{
	pushMutexTimeout = msec;
}
// ------------------------------------------------------------------------------------------
void UniSetObject::push(const TransportMessage& tm)
{
	{ // lock
		uniset_mutex_lock mlk(qmutex,pushMutexTimeout);
		// �������� ������������
		if( !queueMsg.empty() && queueMsg.size()>SizeOfMessageQueue )
		{
			
			unideb[Debug::CRIT] << myname <<": ������������ ������� ���������!\n";
			cleanMsgQueue(queueMsg);

			// ��������� ����������
			stCountOfQueueFull++;
			stMaxQueueMessages=0;	
		}

		queueMsg.push(VoidMessage(tm));
		
		// ������������ ����� ( ��� ���������� )
		if( queueMsg.size() > stMaxQueueMessages )
			stMaxQueueMessages = queueMsg.size();

	} // unlock

	termWaiting();
}
// ------------------------------------------------------------------------------------------
void UniSetObject::cleanMsgQueue( MessagesQueue& q )
{
	unideb[Debug::CRIT] << myname << "(cleanMsgQueue): ������� ������� ���������...\n";

	// �������� �� ���� ��������� ��� �����(�������)
	// ���� ��� ����������� ��������� � ��������� ������ ���������...
	unideb[Debug::CRIT] << myname << "(cleanMsgQueue): ������� ������ ������� ���������: " << q.size() << endl;

	VoidMessage vm;
	map<UniSetTypes::KeyType,VoidMessage> smap;
	map<int,VoidMessage> tmap;
	map<int,VoidMessage> sysmap;
	list<VoidMessage> lstOther;
	map<MsgInfo,VoidMessage> amap;
	map<MsgInfo,VoidMessage> imap;
	map<MsgInfo,VoidMessage> cmap;

//		while( receiveMessage(vm) );
//		while ������ ������������ ������-���, �� ������������� ������
//		����� ���������� � ������� �ݣ ���������.. � ��� ���� ������� �� ����ף���...

		int max = q.size();
		for( int i=0; i<=max; i++ )
		{
			vm = q.top();
			q.pop();
			
			switch( vm.type )
			{
				case Message::SensorInfo:
				{
					SensorMessage sm(&vm);
					UniSetTypes::KeyType k(key(sm.id,sm.node));
					// �.�. �� ������� ��������� ������ ���������� ����� ������, ����� ������ � �.�.
					// �� ���������� ������ ��������� ��������� ��������� ��� ���������� Key
					smap[k] = vm;
					break;
				}

				case Message::Timer:
				{
					TimerMessage tm(&vm);
					// �.�. �� ������� ��������� ������ ���������� ����� ������, ����� ������ � �.�.
					// �� ���������� ������ ��������� ��������� ��������� ��� ���������� TimerId
					tmap[tm.id] = vm;
					break;
				}		

				case Message::SysCommand:
				{
					SystemMessage sm(&vm);
					sysmap[sm.command] = vm;
				}
				break;

				case Message::Alarm:
				{
					AlarmMessage am(&vm);
					MsgInfo mi(am);
					// �.�. �� ������� ��������� ������ ���������� ����� ������, ����� ������ � �.�.
					// �� ���������� ������ ��������� ��������� ��������� ��� ���������� MsgInfo
					amap[mi] = vm;
				}
				break;

				case Message::Info:
				{
					InfoMessage im(&vm);
					MsgInfo mi(im);
					// �.�. �� ������� ��������� ������ ���������� ����� ������, ����� ������ � �.�.
					// �� ���������� ������ ��������� ��������� ��������� ��� ���������� MsgInfo
					imap[mi] = vm;
				}
				break;
				
				case Message::Confirm:
				{
					ConfirmMessage cm(&vm);
					MsgInfo mi(cm);
					// �.�. �� ������� ��������� ������ ���������� ����� ������, ����� ������ � �.�.
					// �� ���������� ������ ��������� ��������� ��������� ��� ���������� MsgInfo
					cmap[mi] = vm;
				}
				break;

				case Message::Unused:
					// ������ ���������� (����������)
				break;
				
				default:
					// ����� ������
					lstOther.push_front(vm);
				break;

			}
		}	

		unideb[Debug::CRIT] << myname << "(cleanMsgQueue): �������� �� SensorMessage: " << smap.size() << endl;
		unideb[Debug::CRIT] << myname << "(cleanMsgQueue): �������� �� TimerMessage: " << tmap.size() << endl;
		unideb[Debug::CRIT] << myname << "(cleanMsgQueue): �������� �� SystemMessage: " << sysmap.size() << endl;
		unideb[Debug::CRIT] << myname << "(cleanMsgQueue): �������� �� AlarmMessage: " << amap.size() << endl;
		unideb[Debug::CRIT] << myname << "(cleanMsgQueue): �������� �� InfoMessage: " << imap.size() << endl;
		unideb[Debug::CRIT] << myname << "(cleanMsgQueue): �������� �� ConfirmMessage: " << cmap.size() << endl;
		unideb[Debug::CRIT] << myname << "(cleanMsgQueue): �������� �� ���������: " << lstOther.size() << endl;

		// ������ ���������� ���������� ������� � �������...

		map<UniSetTypes::KeyType,VoidMessage>::iterator it=smap.begin();
		for( ; it!=smap.end(); ++it )
		{
			q.push(it->second);
		}

		map<int,VoidMessage>::iterator it1=tmap.begin();
		for( ; it1!=tmap.end(); ++it1 )
		{
			q.push(it1->second);
		}

		map<int,VoidMessage>::iterator it2=sysmap.begin();
		for( ; it2!=sysmap.end(); ++it2 )
		{
			q.push(it2->second);
		}

		map<MsgInfo,VoidMessage>::iterator it3=amap.begin();
		for( ; it3!=amap.end(); ++it3 )
		{
			q.push(it3->second);
		}

		map<MsgInfo,VoidMessage>::iterator it4=imap.begin();
		for( ; it4!=imap.end(); ++it4 )
		{
			q.push(it4->second);
		}

		map<MsgInfo,VoidMessage>::iterator it5=cmap.begin();
		for( ; it5!=cmap.end(); ++it5 )
		{
			q.push(it5->second);
		}

		list<VoidMessage>::iterator it6=lstOther.begin();
		for( ; it6!=lstOther.end(); ++it6 )
		{
			q.push(*it6);
		}
		
		unideb[Debug::CRIT] << myname 
					<< "(cleanMsgQueue): ��������� ������ ������� ���������: " 
					<< countMessages()
					<< " < " << getMaxSizeOfMessageQueue() << endl;
		
		if( q.size() >= getMaxSizeOfMessageQueue() )
		{
			unideb[Debug::CRIT] << myname << "(cleanMsgQueue): �� �������!!! ��������� �������� ������ > " << q.size() << endl;
			unideb[Debug::CRIT] << myname << "(cleanMsgQueue): ������ ������� " << getMaxCountRemoveOfMessage() << " ������ ���������\n";
			for( unsigned int i=0; i<getMaxCountRemoveOfMessage() && !q.empty(); i++ )
			{
				q.top(); 
				q.pop(); 
			}
		}
}
// ------------------------------------------------------------------------------------------
unsigned int UniSetObject::countMessages()
{
	{ // lock
		uniset_mutex_lock mlk(qmutex, 200);
		return queueMsg.size();
	}
}
// ------------------------------------------------------------------------------------------
bool UniSetObject::disactivate()
{
	if( !active )
	{
		try
		{
			disactivateObject();
		}
		catch(...){}
		return true;
	}

	active=false; // ��������� ����� ��������� ���������
	tmr->stop();

	// ������� �������
	{ // lock
		uniset_mutex_lock mlk(qmutex, 200);
		while( !queueMsg.empty() )
			queueMsg.pop(); 
	}

	try
	{
		if( unideb.debugging(Debug::INFO) )
			unideb[Debug::INFO] << "disactivateObject..." << endl;

		PortableServer::POA_var poamngr = mymngr->getPOA();
		if( !PortableServer::POA_Helper::is_nil(poamngr) )
		{
			try
			{
				disactivateObject();
			}
			catch(...){}
			unregister();
			PortableServer::ObjectId_var oid = poamngr->servant_to_id(static_cast<PortableServer::ServantBase*>(this));
			poamngr->deactivate_object(oid);
			if( unideb.debugging(Debug::INFO) )
				unideb[Debug::INFO] << "ok..." << endl;
			return true;
		}
		unideb[Debug::WARN] << "manager ��� ���������..." << endl;
	}
	catch(CORBA::TRANSIENT)
	{
		unideb[Debug::WARN] << "isExist: ��� �����..."<< endl;
	}
	catch( CORBA::SystemException& ex )
    {
		unideb[Debug::WARN] << "UniSetObject: "<<"������� CORBA::SystemException: " << ex.NP_minorString() << endl;
    }
    catch(CORBA::Exception& ex)
    {
		unideb[Debug::WARN] << "UniSetObject: "<<"������� CORBA::Exception." << endl;
    }
	catch(Exception& ex)
    {
		unideb[Debug::WARN] << "UniSetObject: "<< ex << endl;
    }
    catch(...)
    {
		unideb[Debug::WARN] << "UniSetObject: "<<" catch ..." << endl;
    }

	return false;
}

// ------------------------------------------------------------------------------------------
bool UniSetObject::activate()
{
	if( unideb.debugging(Debug::INFO) )
		unideb[Debug::INFO] << myname << ": activate..." << endl;

	if( mymngr == NULL )
	{
		unideb[Debug::CRIT] << myname << "(activate): mymngr=NULL!!! activate failure..." << endl;
		return false;
	}

	PortableServer::POA_var poa = mymngr->getPOA();
	if( poa == NULL || CORBA::is_nil(poa) )
	{
		string err(myname+": �� ����� ��������");
		throw ORepFailed(err.c_str());
	}

	if( conf->isTransientIOR() )
	{
	    // activate witch generate id
		poa->activate_object(static_cast<PortableServer::ServantBase*>(this));
	}
	else
	{
		// � ���� myid==UniSetTypes::DefaultObjectId 
		// �� myname = noname. ������! 
		if( myid == UniSetTypes::DefaultObjectId )
		{
			unideb[Debug::CRIT] << myname << "(activate): �� ����� ID!!! activate failure..." << endl;
			// �������� �� ������ ���� ��� �������������� � �������� �������
			// �������� � ObjectsManager, ���� ����� �� �������, �� �� ����� ���������������� �����Σ���� �������.
			// (��. ObjectsManager::activateObject)
			activateObject();
			return false;
		}

	    // Always use the same object id.
    	PortableServer::ObjectId_var oid =
		PortableServer::string_to_ObjectId(myname.c_str());

//		cerr << myname << "(activate): " << _refcount_value() << endl;

    	// Activate object...
	    poa->activate_object_with_id(oid, this);
	}
	

	
	oref = poa->servant_to_reference(static_cast<PortableServer::ServantBase*>(this) );

	registered();
	// ��������� ����� ��������� ���������
	active=true;

	if( myid!=UniSetTypes::DefaultObjectId && threadcreate )
	{
		thr = new ThreadCreator<UniSetObject>(this, &UniSetObject::work);
		thr->start();
	}
	else 
	{
		if( unideb.debugging(Debug::INFO) )
		{
			unideb[Debug::INFO] << myname << ": ?? �� ����� ObjectId...(" 
					<< "myid=" << myid << " threadcreate=" << threadcreate 
					<< ")" << endl;
		}
		thread(false);
	}

	activateObject();
	if( unideb.debugging(Debug::INFO) )
		unideb[Debug::INFO] << myname << ": activate ok." << endl;
	return true;
}
// ------------------------------------------------------------------------------------------
void UniSetObject::work()
{
	if( unideb.debugging(Debug::INFO) )
		unideb[Debug::INFO] << myname << ": thread processing messages run..." << endl;
	if( thr )
		msgpid = thr->getTID();
	while( active )
	{
		callback();
	}
	unideb[Debug::WARN] << myname << ": thread processing messages stop..." << endl;	
}
// ------------------------------------------------------------------------------------------
void UniSetObject::callback()
{
	try
	{
		if( waitMessage(msg) )
			processingMessage(&msg);
	}
	catch(...){}
}
// ------------------------------------------------------------------------------------------
void UniSetObject::processingMessage( UniSetTypes::VoidMessage *msg )
{
	if( unideb.debugging(Debug::INFO) )
		unideb[Debug::INFO] << myname << ": default processing messages..." << endl;	
}
// ------------------------------------------------------------------------------------------
UniSetTypes::SimpleInfo* UniSetObject::getInfo()
{
	ostringstream info;
	info.setf(ios::left, ios::adjustfield);
	info << "(" << myid << ")" << setw(40) << myname << "\n==================================================\n";
	info << "tid=" << setw(10);
	if( threadcreate )
	{
		if(thr)	
		{
			msgpid = thr->getTID();	// ������(�� ������) ������� � ���������� ����������
			info << msgpid;  
		}
		else
			info << "�� �������";
	}
	else
		info << "����.";  
	
	info << "\tcount=" << countMessages();
	info << "\tmaxMsg=" << stMaxQueueMessages;
	info << "\tqFull("<< SizeOfMessageQueue << ")=" << stCountOfQueueFull;
//	info << "\n";
	
	SimpleInfo* res = new SimpleInfo();
	res->info 	=  info.str().c_str(); // CORBA::string_dup(info.str().c_str());
	res->id 	=  myid;
	
	return res; // ._retn();
}
// ------------------------------------------------------------------------------------------
ostream& operator<<(ostream& os, UniSetObject& obj )
{
	SimpleInfo_var si = obj.getInfo();
	return os << si->info;
}
// ------------------------------------------------------------------------------------------

bool UniSetObject::PriorVMsgCompare::operator()(const UniSetTypes::VoidMessage& lhs, 
												const UniSetTypes::VoidMessage& rhs) const
{
	if( lhs.priority == rhs.priority )
	{
		if( lhs.tm.tv_sec == rhs.tm.tv_sec )
			return lhs.tm.tv_usec >= rhs.tm.tv_usec;
		return lhs.tm.tv_sec >= rhs.tm.tv_sec;
	}
	
	return lhs.priority < rhs.priority;
}
// ------------------------------------------------------------------------------------------

#undef CREATE_TIMER
