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
 *  \date   $Date: 2008/12/14 21:57:51 $
 *  \version $Id: ObjectsActivator.cc,v 1.15 2008/12/14 21:57:51 vpashka Exp $
*/
// -------------------------------------------------------------------------- 

#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sstream>

#include "Exceptions.h"
#include "ORepHelpers.h"
#include "UniversalInterface.h"
#include "ObjectsActivator.h"
#include "Debug.h"
#include "Configuration.h"

// ------------------------------------------------------------------------------------------
using namespace UniSetTypes;
using namespace std;
// ------------------------------------------------------------------------------------------
/*
 	���������� ������ ������������ ��������� �������.
	������� ���������� ��������� gActivator (�.�. ��������� � ������� ������ ���� ������ ����).
	�� ���������� �� ���� ��� ������� ��������� � ����������� ������.
	
	� �������� ����������� �������� �������������� ObjectsActivator::terminated( int signo ).
	� ���� ����������� ���������� ����� ObjectsActivator::oaDestroy(int signo) ��� ������������
	���������� ������ � ������������ ������ SIG_ALRM �� ����� TERMINATE_TIMEOUT,
	c ������������ ObjectsActivator::finishterm � ������� ����������
	"��������" ���������� �������� �������� (raise(SIGKILL)). ��� ������� �� ��� ������, ����
	� oaDestroy ���������� ���������.
*/
// ------------------------------------------------------------------------------------------
/*! ����� ��� ������������ ����������� ������� � ������� �������� �������� */
static UniSetTypes::uniset_mutex signalMutex("Activator::signalMutex");
// static UniSetTypes::uniset_mutex waittermMutex("Activator::waittermMutex");

/*! ����� ��� ������������ ����������� � ������ ����������� �������� */
//UniSetTypes::uniset_mutex sigListMutex("Activator::sigListMutex");

//static omni_mutex pmutex;
//static omni_condition pcondx(&pmutex);

// ------------------------------------------------------------------------------------------
static ObjectsActivator* gActivator=0;
//static omni_mutex termutex;
//static omni_condition termcond(&termutex);
//static ThreadCreator<ObjectsActivator>* termthread=0;
static int SIGNO;
static int MYPID;
static const int TERMINATE_TIMEOUT = 2; //  ����� ���������� �� ���������� �������� [���]
volatile sig_atomic_t procterm = 0;
volatile sig_atomic_t doneterm = 0;

// PassiveTimer termtmr;
// ------------------------------------------------------------------------------------------
ObjectsActivator::ObjectsActivator( ObjectId id ):
ObjectsManager(id),
orbthr(0),
omDestroy(false),
sig(false)
{
	ObjectsActivator::init();
}
// ------------------------------------------------------------------------------------------
ObjectsActivator::ObjectsActivator():
ObjectsManager(UniSetTypes::DefaultObjectId),
orbthr(0),
omDestroy(false),
sig(false)
{
//	thread(false);	//	��������� ����� (��� �� ����� id)
	ObjectsActivator::init();
}

// ------------------------------------------------------------------------------------------
void ObjectsActivator::init()
{
	orb = conf->getORB();
	CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);
	pman = root_poa->the_POAManager();
	CORBA::PolicyList pl = conf->getPolicy();
	poa = root_poa->create_POA("my poa", pman, pl);

	if( CORBA::is_nil(poa) )
		unideb[Debug::CRIT] << myname << "(init): init poa failed!!!" << endl;

	gActivator=this;
	atexit( ObjectsActivator::normalexit );
	set_terminate( ObjectsActivator::normalterminate ); // ������� ��� ����������� ����������
}

// ------------------------------------------------------------------------------------------

ObjectsActivator::~ObjectsActivator()
{
	if(!procterm )
	{
		unideb[Debug::SYSTEM] << myname << "(destructor): ..."<< endl << flush;
		if( !omDestroy )
			oaDestroy();

		procterm = 1;
		doneterm = 1;
		set_signals(false);	
		gActivator=0;	
	}

	if( orbthr )
		delete orbthr;
}
// ------------------------------------------------------------------------------------------

void ObjectsActivator::oaDestroy(int signo)
{
//		waittermMutex.lock();
		if( !omDestroy )
		{
			omDestroy = true;
			unideb[Debug::SYSTEM] << myname << "(oaDestroy): begin..."<< endl;			

			unideb[Debug::SYSTEM] << myname << "(oaDestroy): terminate... " << endl;		
			term(signo);
			unideb[Debug::SYSTEM] << myname << "(oaDestroy): terminate ok. " << endl;		

			try
			{	
				stop();
			}
			catch(...){}

			unideb[Debug::SYSTEM] << myname << "(oaDestroy): pman deactivate... " << endl;		
			pman->deactivate(false,true);
			unideb[Debug::SYSTEM] << myname << "(oaDestroy): pman deactivate ok. " << endl;		

			unideb[Debug::SYSTEM] << myname << "(oaDestroy): orb destroy... " << endl;		
			try
			{
				orb->destroy();
			}
			catch(...){}
			unideb[Debug::SYSTEM] << myname << "(oaDestroy): orb destroy ok."<< endl;

			if( orbthr )
			{
				delete orbthr;
				orbthr = 0;
			}

		}
//		waittermMutex.unlock();
}

// ------------------------------------------------------------------------------------------
/*! 
 *	���� thread=true �� ������� ������� ��������� ����� ��� ��������� ���������� ���������.
 * 	� �������� ��� ������� ����� ������ orb. � ����� ������������ ������� � �����������.
 *	\note ������ ����� ����� ������ ���������� �������� ������ ���������
 *	� ����� �������...
 *	����� ��� ������� ��������� ������ ���������� ��� ��������� ���������� ��������� (� ��� �� �������)
 *
*/
void ObjectsActivator::run(bool thread)
{
	unideb[Debug::SYSTEM] << myname << "(run): ������ �������� "<< endl;

	ObjectsManager::initPOA(this);

	if( getId() == UniSetTypes::DefaultObjectId )
		offThread(); // ���������� ������ ��������� ���������, ��� �� ����� ObjectId
	
	ObjectsManager::activate(); // � ��� ���������� ��������� ���� ����������� �������� � ����������

	getinfo();		// ���������� ���������� �� ��������
	active=true;

	unideb[Debug::SYSTEM] << myname << "(run): ������������ ��������"<<endl;	
	pman->activate();
	msleep(50);

	set_signals(true);	
	if( thread )
	{
		if( unideb.debugging(Debug::INFO) )
			unideb[Debug::INFO] << myname << "(run): ����������� � ��������� ���������� ������...  "<< endl;
		orbthr = new ThreadCreator<ObjectsActivator>(this, &ObjectsActivator::work);
		if( !orbthr->start() )
		{
			unideb[Debug::CRIT] << myname << "(run):  �� ������ ������� ORB-�����"<<endl;	
			throw SystemError("(ObjectsActivator::run): CREATE ORB THREAD FAILED");
		}	
	}
	else
	{
		if( unideb.debugging(Debug::INFO) )
			unideb[Debug::INFO] << myname << "(run): ����������� ��� �������� ���������� ������...  "<< endl;
		work();
	}
}
// ------------------------------------------------------------------------------------------
/*! 
 *	������� ������������� ������ orb � ��������� �����. � ��� �� ������� ������ �� �����������.
 *	\note ������ ���������� ���������� ������ ���������
*/
void ObjectsActivator::stop()
{
//	uniset_mutex_lock l(disactivateMutex, 500);
	if( active )
	{
		active=false;				
		
		if( unideb.debugging(Debug::SYSTEM) )	
			unideb[Debug::SYSTEM] << myname << "(stop): disactivate...  "<< endl;
		disactivate();
		if( unideb.debugging(Debug::SYSTEM) )
		{
			unideb[Debug::SYSTEM] << myname << "(stop): disactivate ok.  "<<endl;
			unideb[Debug::SYSTEM] << myname << "(stop): discard request..."<< endl;
		}
		pman->discard_requests(true);
		if( unideb.debugging(Debug::SYSTEM) )	
			unideb[Debug::SYSTEM] << myname << "(stop): discard request ok."<< endl;		
/*
		try
		{
			unideb[Debug::SYSTEM] << myname << "(stop):: shutdown orb...  "<<endl;
			orb->shutdown(false);
		}
		catch(...){}
		unideb[Debug::SYSTEM] << myname << "(stop): shutdown ok."<< endl;
*/
	}
}

// ------------------------------------------------------------------------------------------

void ObjectsActivator::work()
{
	if( unideb.debugging(Debug::SYSTEM) )	
		unideb[Debug::SYSTEM] << myname << "(work): ��������� orb �� ��������� ��������..."<< endl;
	try
	{
		if(orbthr)
			thpid = orbthr->getTID();
		else
			thpid = getpid();

		orb->run();
	}
	catch(CORBA::SystemException& ex)
    {
		unideb[Debug::CRIT] << myname << "(work): ������� CORBA::SystemException: " << ex.NP_minorString() << endl;
    }
    catch(CORBA::Exception& ex)
    {
		unideb[Debug::CRIT] << myname << "(work): ������� CORBA::Exception." << endl;
    }
    catch(omniORB::fatalException& fe)
    {
		unideb[Debug::CRIT] << myname << "(work): : ������� omniORB::fatalException:" << endl;
        unideb[Debug::CRIT] << myname << "(work):   file: " << fe.file() << endl;
		unideb[Debug::CRIT] << myname << "(work):   line: " << fe.line() << endl;
        unideb[Debug::CRIT] << myname << "(work):   mesg: " << fe.errmsg() << endl;
    }
	catch(...)
	{
		unideb[Debug::CRIT] << myname << "(work): catch ..." << endl;
	}
	
	if( unideb.debugging(Debug::SYSTEM) )	
		unideb[Debug::SYSTEM] << myname << "(work): orb ����!!!"<< endl;

/*
	unideb[Debug::SYSTEM] << myname << "(oaDestroy): orb destroy... " << endl;		
	try
	{
		orb->destroy();
	}
	catch(...){}
	unideb[Debug::SYSTEM] << myname << "(oaDestroy): orb destroy ok."<< endl;
*/	
}
// ------------------------------------------------------------------------------------------
void ObjectsActivator::getinfo()
{
	for( ObjectsManagerList::const_iterator it= beginMList();
			it!= endMList(); ++it )
	{
		MInfo mi;
		mi.mnr = (*it);
		mi.msgpid = (*it)->getMsgPID();
		lstMInfo.push_back(mi);
	}

	for( ObjectsList::const_iterator it= beginOList();
			it!= endOList(); ++it )
	{
		OInfo oi;
		oi.obj = (*it);
		oi.msgpid = (*it)->getMsgPID();
		lstOInfo.push_back(oi);
	}
}
// ------------------------------------------------------------------------------------------
void ObjectsActivator::processingMessage( UniSetTypes::VoidMessage *msg )
{
	try
	{
		switch( msg->type )
		{
			case Message::SysCommand:
			{
				SystemMessage sm( msg );
				sysCommand(&sm);
			}
			break;

			default:
				break;
		}	
	}
	catch(Exception& ex)
	{
		unideb[Debug::CRIT]  << myname << "(processingMessage): " << ex << endl;
	}

}
// -------------------------------------------------------------------------
void ObjectsActivator::sysCommand( UniSetTypes::SystemMessage *sm )
{
	switch(sm->command)
	{
		case SystemMessage::LogRotate:
		{
			unideb[Debug::SYSTEM] << myname << "(sysCommand): logRotate" << endl;
			// ������������� ����
			string fname = unideb.getLogFile();
			if( !fname.empty() )
			{
				unideb.logFile(fname.c_str());
				unideb << myname << "(sysCommand): ***************** UNIDEB LOG ROTATE *****************" << endl;
			}
		}
		break;
	}
}
// -------------------------------------------------------------------------

/*
void ObjectsActivator::sig_child(int signo)
{
	unideb[Debug::SYSTEM] << gActivator->getName() << "(sig_child): �������� ������� �������� ������...(sig=" << signo << ")" << endl;
	while( waitpid(-1, 0, WNOHANG) > 0);
}
*/
// ------------------------------------------------------------------------------------------
void ObjectsActivator::set_signals(bool ask)
{
	
	struct sigaction act, oact;
	sigemptyset(&act.sa_mask);

	// ��������� �������, ������� ����� ��������������
	// ��� ��������� ������� 
	sigaddset(&act.sa_mask, SIGINT);
	sigaddset(&act.sa_mask, SIGTERM);
	sigaddset(&act.sa_mask, SIGABRT );
	sigaddset(&act.sa_mask, SIGQUIT);
//	sigaddset(&act.sa_mask, SIGSEGV);


//	sigaddset(&act.sa_mask, SIGALRM);
//	act.sa_flags = 0;
//	act.sa_flags |= SA_RESTART;
//	act.sa_flags |= SA_RESETHAND;
	if(ask)
		act.sa_handler = terminated;
	else
		act.sa_handler = SIG_DFL;
		
	sigaction(SIGINT, &act, &oact);
	sigaction(SIGTERM, &act, &oact);
	sigaction(SIGABRT, &act, &oact);
	sigaction(SIGQUIT, &act, &oact);

//	sigaction(SIGSEGV, &act, &oact);
}

// ------------------------------------------------------------------------------------------
void ObjectsActivator::finishterm( int signo )
{
	if( !doneterm )
	{
		if( unideb.debugging(Debug::SYSTEM) && gActivator )
			unideb[Debug::SYSTEM] << gActivator->getName() 
				<< "(finishterm): ��������� ������� ����������...!" << endl<< flush;

		ObjectsActivator::set_signals(false);
		sigset(SIGALRM, SIG_DFL);
		doneterm = 1;
		raise(SIGKILL);
	}
}
// ------------------------------------------------------------------------------------------
void ObjectsActivator::terminated( int signo )
{
	if( !signo || doneterm || !gActivator || procterm )
		return;

	{	// lock

		// �� ������ ������� ���������� ��������
		uniset_mutex_lock l(signalMutex, TERMINATE_TIMEOUT*1000);
		if( !procterm )
		{
			procterm = 1;			
			SIGNO = signo;
			MYPID = getpid();
			if( unideb.debugging(Debug::SYSTEM) && gActivator )
			{
				unideb[Debug::SYSTEM] << gActivator->getName() << "(terminated): catch SIGNO="<< signo << "("<< strsignal(signo) <<")"<< endl << flush;
					unideb[Debug::SYSTEM] << gActivator->getName() << "(terminated): ������������� timer ���������� �� "
						<< TERMINATE_TIMEOUT << " ��� " << endl << flush;
			}
			sighold(SIGALRM);
			sigset(SIGALRM, ObjectsActivator::finishterm);
			alarm(TERMINATE_TIMEOUT);
			sigrelse(SIGALRM);				
			if( gActivator )
				gActivator->oaDestroy(SIGNO); // gActivator->term(SIGNO);

			doneterm = 1;
			if( unideb.debugging(Debug::SYSTEM) )				
				unideb[Debug::SYSTEM] << gActivator->getName() << "(terminated): �����������..."<< endl<< flush;
			if( gActivator )
				ObjectsActivator::set_signals(false);

			sigset(SIGALRM, SIG_DFL);
			raise(SIGNO);
		}
	}
}
// ------------------------------------------------------------------------------------------

void ObjectsActivator::normalexit()
{
	if( gActivator && unideb.debugging(Debug::SYSTEM) )
		unideb[Debug::SYSTEM] << gActivator->getName() << "(default exit): good bye."<< endl << flush;
}

void ObjectsActivator::normalterminate()
{
	if( gActivator )
		unideb[Debug::CRIT] << gActivator->getName() << "(default exception terminate): ����� �� ������� ����������!!! Good bye."<< endl<< flush;
//	abort();
}
// ------------------------------------------------------------------------------------------
void ObjectsActivator::term( int signo )
{
	if( unideb.debugging(Debug::SYSTEM) )	
		unideb[Debug::SYSTEM] << myname << "(term): TERM" << endl;

	if( doneterm )
		return;
			
	if( signo )
		sig = true;

	try
	{
		if( unideb.debugging(Debug::SYSTEM) )
			unideb[Debug::SYSTEM] << myname << "(term): �������� sigterm()" << endl;
		sigterm(signo);
	
		if( unideb.debugging(Debug::SYSTEM) )
			unideb[Debug::SYSTEM] << myname << "(term): sigterm() ok." << endl;
	}
	catch(Exception& ex)
	{
		unideb[Debug::CRIT] << myname << "(term): " << ex << endl;
	}
	catch(...){}

	if( unideb.debugging(Debug::SYSTEM) )
		unideb[Debug::SYSTEM] << myname << "(term): END TERM" << endl;
}
// ------------------------------------------------------------------------------------------
void ObjectsActivator::waitDestroy()
{
	for(;;)
	{
		if( doneterm || !gActivator )
			break;
			
		msleep(50);
	}
	
	gActivator = 0;
}
// ------------------------------------------------------------------------------------------
