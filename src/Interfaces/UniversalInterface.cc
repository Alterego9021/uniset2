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
 *  \version $Id: UniversalInterface.cc,v 1.26 2008/12/14 21:57:51 vpashka Exp $
*/
// -------------------------------------------------------------------------- 
#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "ORepHelpers.h"
#include "InfoServer_i.hh"
#include "TimerService_i.hh"
#include "UniversalInterface.h"
//#include "Debug.h"
#include "Configuration.h"
#include "PassiveTimer.h"

// -----------------------------------------------------------------------------
using namespace omni;
using namespace UniversalIO;
using namespace UniSetTypes;
using namespace std;
// -----------------------------------------------------------------------------
UniversalInterface::UniversalInterface( UniSetTypes::Configuration* _uconf ):
	rep(_uconf),
	myid(UniSetTypes::DefaultObjectId),
	orb(CORBA::ORB::_nil()),
	rcache(100,5),
	oind(_uconf->oind),
	uconf(_uconf)
{
	init();
}
// -----------------------------------------------------------------------------
UniversalInterface::UniversalInterface( ObjectId backid, CORBA::ORB_var orb, ObjectIndex* _oind ):
	rep(UniSetTypes::conf),
	myid(backid),
	orb(orb),
	rcache(200,120),
	oind(_oind),
	uconf(UniSetTypes::conf)
{
	if( oind == NULL )
		oind = uconf->oind;

	init();
}	

UniversalInterface::~UniversalInterface()
{
}

void UniversalInterface::init()
{
	// �������� �������� ������ �� NameSerivice
	// � ����� ������. ���� ���� ����ޣ� �����
	// localIOR
	try
	{
		if( CORBA::is_nil(orb) )
		{
			CORBA::ORB_var _orb = uconf->getORB();
			localctx = ORepHelpers::getRootNamingContext( _orb, oind->getRealNodeName(uconf->getLocalNode()) );
		}
		else
			localctx = ORepHelpers::getRootNamingContext( orb, oind->getRealNodeName(uconf->getLocalNode()) );
	}
	catch(Exception& ex )
	{
		if( !uconf->isLocalIOR() )
			throw ex;

		localctx=CosNaming::NamingContext::_nil();
	}
}
// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::initBackId( UniSetTypes::ObjectId backid )
{
	myid = backid;
}
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param name - ������ ��� ����������� �������
 * \sa UniversalInterface::getState( ObjectId id ) 
*/
/*
bool UniversalInterface::getState ( const string name, const string node="NameService" )throw(TimeOut,IOBadParam)
{
	return getState(oind->getIdByName(name.c_str()));
}
*/

/*
 * \param id - ������������� �������
 *�\return ������� ��������� �������
 * \exception IOBadParam - ������������ ���� ������� ������������ ��� ������� ��� ������
 * \exception IOTimeOut - ������������ ���� � ������� ������� timeout ����� ������� �����
*/
bool UniversalInterface::getState(ObjectId name, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(getState): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;

		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}
		for( unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve( name, node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = name;
				si->node = node;
				return iom->getState(si);
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();
		}
	}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(name, node);
		throw NameNotFound("UI(getState): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(getState): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(getState): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(getState): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getState): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(getState): TimeOut ��� �������",name,node));
}

bool UniversalInterface::getState( ObjectId name )
{
	return getState( name, uconf->getLocalNode() );
}

// ---------------------------------------------------------------------
/*!
 * \param name - ������ ��� ����������� �������
 * \sa UniversalInterface::getValue( ObjectId id ) 
*/
/*
long UniversalInterface::getValue(const string name, const string node="NameService")throw(TimeOut,IOBadParam)
{
	return getValue(oind->getIdByName(name.c_str()));
}
*/

/*!
 * \param id - ������������� �������
 *�\return ������� �������� �������
 * \exception IOBadParam - ������������ ���� ������� ������������ ��� ������� ��� ������
 * \exception IOTimeOut - ������������ ���� � ������� ������� timeout ����� ������� �����
*/
long UniversalInterface::getValue(ObjectId name, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(getValue): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}

		for( unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve( name, node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = name;
				si->node = node;
				return iom->getValue(si.in());
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());
			oref = CORBA::Object::_nil();
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(name, node);
		throw NameNotFound("UI(getValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(getValue): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(getValue): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(getValue): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(name, node);		
	throw UniSetTypes::TimeOut(set_err("UI(getValue): TimeOut ��� �������",name,node));
}

long UniversalInterface::getValue( ObjectId name ) 
{
	return getValue(name, uconf->getLocalNode());
}


// ------------------------------------------------------------------------------------------------------------
/*!
 * \param id - ������������� �������
 * \param state - ��������� � ������� ��� ���������� ���������
 *�\return ������� �������� �������
 * \exception IOBadParam - ������������ ���� ������� ������������ ��� ������ ��� ������
*/
void UniversalInterface::setState(ObjectId name, bool state, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(setState): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve( name, node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = name;
				si->node = node;
				iom->setState(si, state, myid);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(name, node);
		throw NameNotFound( set_err("UI(setState):"+string(ex.err),name,node) );
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(setState): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(setState): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(setState): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(setState): CORBA::COMM_FAILURE " << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(setState): CORBA::SystemException" << endl;
	}	
			
	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(setState): TimeOut ��� �������",name,node));
}

void UniversalInterface::setState(ObjectId name, bool state) 
{
	setState(name, state, uconf->getLocalNode());
}

void UniversalInterface::setState( IOController_i::SensorInfo& si, bool state, UniSetTypes::ObjectId supplier )
{
	ObjectId old = myid;
	try
	{
		myid = supplier;
		setState(si.id,state,si.node);
	}
	catch(...)
	{
		myid = old;
		throw;
	}
	
	myid = old;
}

// ------------------------------------------------------------------------------------------------------------
// ������� �� ������������ ����������!
void UniversalInterface::fastSetState( IOController_i::SensorInfo& si, bool state, UniSetTypes::ObjectId sup_id ) 
{
	if( si.id == DefaultObjectId )
	{
		unideb[Debug::WARN] << "UI(fastSetState): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId" << endl;
		return;
	}

	if( sup_id == DefaultObjectId )
		sup_id = myid;

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(si.id, si.node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve( si.id, si.node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				iom->fastSetState(si, state, sup_id );
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << set_err("UI(fastSetState):"+string(ex.err),si.id,si.node) << endl;
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);	
		// �� ������ �������� ������ �� ������
		unideb[Debug::WARN] << set_err("UI(fastSetState): �� ���� �������� ������ �� ������",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);	
		unideb[Debug::WARN] << set_err("UI(fastSetState): ���������� ���������� ������",si.id,si.node) << endl;	
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);			
		unideb[Debug::WARN] << set_err("UI(fastSetState): ������ �� �� ������������ ������",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE){}
	catch(CORBA::SystemException& ex){}
	catch(...){}		
	
	rcache.erase(si.id, si.node);	
	unideb[Debug::WARN] << set_err("UI(fastSetState): TimeOut ��� �������",si.id,si.node) << endl;
}

// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::setUndefinedState( IOController_i::SensorInfo& si, bool undefined, 
											UniSetTypes::ObjectId sup_id )
{
	if( si.id == DefaultObjectId )
	{
		unideb[Debug::WARN] << "UI(setUndefinedState): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId" << endl;
		return;
	}

	if( sup_id == DefaultObjectId )
		sup_id = myid;

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(si.id, si.node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve( si.id, si.node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				iom->setUndefinedState(si, undefined, sup_id );
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << set_err("UI(setUndefinedState):"+string(ex.err),si.id,si.node) << endl;
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);	
		// �� ������ �������� ������ �� ������
		unideb[Debug::WARN] << set_err("UI(setUndefinedState): �� ���� �������� ������ �� ������",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);	
		unideb[Debug::WARN] << set_err("UI(setUndefinedState): ���������� ���������� ������",si.id,si.node) << endl;	
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);			
		unideb[Debug::WARN] << set_err("UI(setUndefinedState): ������ �� �� ������������ ������",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE){}
	catch(CORBA::SystemException& ex){}
	catch(...){}		
	
	rcache.erase(si.id, si.node);	
	unideb[Debug::WARN] << set_err("UI(setUndefinedState): TimeOut ��� �������",si.id,si.node) << endl;
}
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param id - ������������� �������
 * \param value - �������� ������� ���������� ����������
 * \return ������� �������� �������
 * \exception IOBadParam - ������������ ���� ������� ������������ ��� ������ ��� ������
*/
void UniversalInterface::setValue(ObjectId name, long value, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(setValue): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve( name, node );
			
				IOController_i_var iom = IOController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = name;
				si->node = node;
				iom->setValue(si, value, myid);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(name, node);	
		throw NameNotFound(set_err("UI(setValue): NameNotFound ��� �������",name,node));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(setValue): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(setValue): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(setValue): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(setValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(name, node);		
	throw UniSetTypes::TimeOut(set_err("UI(setValue): TimeOut ��� �������",name,node));
}

void UniversalInterface::setValue(ObjectId name, long value) 
{
	setValue(name, value, uconf->getLocalNode());
}


void UniversalInterface::setValue( IOController_i::SensorInfo& si, long value, UniSetTypes::ObjectId supplier )
{
	ObjectId old = myid;
	try
	{
		myid = supplier;
		setValue(si.id,value,si.node);
	}
	catch(...)
	{
		myid = old;
		throw;
	}
	
	myid = old;
}

// ------------------------------------------------------------------------------------------------------------
// ������� �� ������������ ��������!
void UniversalInterface::fastSetValue( IOController_i::SensorInfo& si, long value, UniSetTypes::ObjectId sup_id )
{
	if ( si.id == DefaultObjectId )
	{
		unideb[Debug::WARN] << "UI(fastSetValue): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId" << endl;
		return;
	}

	if( sup_id == DefaultObjectId )
		sup_id = myid;

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(si.id, si.node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve( si.id,si.node );
			
				IOController_i_var iom = IOController_i::_narrow(oref);
				iom->fastSetValue(si, value,sup_id);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSetValue): NameNotFound ��� �������",si.id,si.node) << endl;
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id,si.node);	
		// �� ������ �������� ������ �� ������
		unideb[Debug::WARN] << set_err("UI(fastSetValue): �� ���� �������� ������ �� ������ ",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSetValue): ���������� ���������� ������",si.id,si.node) << endl;
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSetValue): ������ �� �� ������������ ������",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(setValue): CORBA::SystemException" << endl;
	}	
	catch(...){}

	rcache.erase(si.id,si.node);
	unideb[Debug::WARN] << set_err("UI(fastSetValue): TimeOut ��� �������",si.id,si.node) << endl;
}


// ------------------------------------------------------------------------------------------------------------
/*!
 * \param name - ��� ����������� �������
 * \param fromName - ��� �������(���������) ���� ��������� ��������� �� ���������
 * \sa UniversalInterface::askState( ObjectId id, ObjectId backid, UniversalIO::UIOCommand cmd)
*/
/*
void UniversalInterface::askState(const string name, const string fromName, UniversalIO::UIOCommand cmd)
		throw(TimeOut,IOBadParam)
{
    askState( oind->getIdByName(name.c_str()), oind->getIdByName(fromName.c_str()), cmd);
}
*/

/*!
 * \param sensor 	- ������������� �������
 * \param node		- ������������� ���� �� ������� ������������ ������
 * \param cmd - ������� ��. \ref UniversalIO::UIOCommand
 * \param backid - �������� ����� (������������� ���������)
*/
void UniversalInterface::askRemoteState( ObjectId name, UniversalIO::UIOCommand cmd, ObjectId node,
									UniSetTypes::ObjectId backid ) throw(IO_THROW_EXCEPTIONS)
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw IOBadParam("UI(askRemoteState): ���������� �������� �����");

	if ( name == DefaultObjectId )
		throw ORepFailed("UI(askRemoteState): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( name, node );
	
				IONotifyController_i_var inc = IONotifyController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = name;
				si->node = node;

				ConsumerInfo_var ci;
				ci->id = backid;
				ci->node = uconf->getLocalNode();
				inc->askState(si, ci, cmd );
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(name, node);
		throw NameNotFound("UI(askState): "+string(ex.err) );
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(askState): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(askState): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(askState): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askState): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askState): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(askState): TimeOut ��� �������",name,node));
}

void UniversalInterface::askState( ObjectId name, UniversalIO::UIOCommand cmd, UniSetTypes::ObjectId backid )
{
	askRemoteState(name, cmd, uconf->getLocalNode(), backid);
}

// ------------------------------------------------------------------------------------------------------------
/*!
 * \param sensor 	- ������������� �������
 * \param node		- ������������� ���� �� ������� ������������ ������
 * \param cmd - ������� ��. \ref UniversalIO::UIOCommand
 * \param backid - �������� ����� (������������� ���������)
*/
void UniversalInterface::askRemoteSensor( ObjectId name, UniversalIO::UIOCommand cmd, ObjectId node,
									UniSetTypes::ObjectId backid ) throw(IO_THROW_EXCEPTIONS)
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw IOBadParam("UI(askRemoteSensor): ���������� �������� �����");

	if ( name == DefaultObjectId )
		throw ORepFailed("UI(askRemoteSensor): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( name, node );
	
				IONotifyController_i_var inc = IONotifyController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = name;
				si->node = node;

				ConsumerInfo_var ci;
				ci->id = backid;
				ci->node = uconf->getLocalNode();
				inc->askSensor(si, ci, cmd );
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(name, node);
		throw NameNotFound("UI(askSensor): "+string(ex.err) );
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(askSensor): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(askSensor): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(askSensor): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askSensor): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askSensor): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(askSensor): TimeOut ��� �������",name,node));
}

void UniversalInterface::askSensor( ObjectId name, UniversalIO::UIOCommand cmd, UniSetTypes::ObjectId backid )
{
	askRemoteSensor(name, cmd, uconf->getLocalNode(), backid);
}

// ------------------------------------------------------------------------------------------------------------

/*!
 * \param name - ��� ����������� �������
 * \param fromName - ��� �������(���������) ���� ��������� ��������� �� ���������
 * \sa UniversalInterface::askValue( ObjectId id, ObjectId backid, UniversalIO::UIOCommand cmd) 
*/
/*
void UniversalInterface::askValue(const string name, const string fromName, UniversalIO::UIOCommand cmd)
					throw(TimeOut,IOBadParam)
{
	askValue(oind->getIdByName(name.c_str()),oind->getIdByName(fromName.c_str()), cmd);
}
*/

/*!
 * \param id - ������������� �������
 * \param backid - ������������� ���������, ���� ��������� ��������� �� ���������
 * \param cmd - ������� ��. \ref UniversalIO::UIOCommand
 * \param backid - �������� ����� (������������� ���������)
 * \exception IOBadParam - ������������ ���� ������� ������������ ��� ������ ��� ������
 * \exception TimeOut - ������������ ���� ��� ����� �������� ���������� �� ���-�� �� ���� �������
*/
void UniversalInterface::askRemoteValue( ObjectId sensid, UniversalIO::UIOCommand cmd, ObjectId node, 
									UniSetTypes::ObjectId backid) throw(IO_THROW_EXCEPTIONS)	
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw IOBadParam("UI(askRemoteValue): ���������� �������� �����");

	if ( sensid == DefaultObjectId )
		throw ORepFailed("UI(askRemoteValue): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(sensid, node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( sensid, node );
			
				IONotifyController_i_var inc = IONotifyController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = sensid;
				si->node = node;

				ConsumerInfo_var ci;
				ci->id = backid;
				ci->node = uconf->getLocalNode();

				inc->askValue(si,ci, cmd);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound& ex)
	{
		rcache.erase(sensid, node);
		throw NameNotFound("UI(askValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(sensid, node);		
		throw IOBadParam(set_err("UI(askValue): �� ���� �������� ������ �� ������ ",sensid,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(sensid, node);		
		throw IOBadParam(set_err("UI(askValue): ���������� ���������� ������",sensid,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(sensid, node);		
		throw IOBadParam(set_err("UI(askValue): ������ �� �� ������������ ������",sensid,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askValue): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(sensid, node);	
	throw UniSetTypes::TimeOut(set_err("UI(askValue): TimeOut ��� �������",sensid,node));
}

void UniversalInterface::askValue( ObjectId name, UniversalIO::UIOCommand cmd, UniSetTypes::ObjectId backid )
{
	askRemoteValue(name, cmd, uconf->getLocalNode(), backid);
}

// ------------------------------------------------------------------------------------------------------------
/*!
 * \param id - ������������� �������
 * \param backid - ������������� ���������, ���� ��������� ��������� �� ���������
 * \param cmd - ������� ��. \ref UniversalIO::UIOCommand
 * \param backid - �������� ����� (������������� ���������)
 * \exception IOBadParam - ������������ ���� ������� ������������ ��� ������ ��� ������
 * \exception TimeOut - ������������ ���� ��� ����� �������� ���������� �� ���-�� �� ���� �������
*/
void UniversalInterface::askRemoteOutput( ObjectId sensid, UniversalIO::UIOCommand cmd, ObjectId node, 
									UniSetTypes::ObjectId backid) throw(IO_THROW_EXCEPTIONS)	
{
	if( backid == UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw IOBadParam("UI(askRemoteOutput): ���������� �������� �����");

	if ( sensid == DefaultObjectId )
		throw ORepFailed("UI(askRemoteOutput): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(sensid, node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( sensid, node );
			
				IONotifyController_i_var inc = IONotifyController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = sensid;
				si->node = node;

				ConsumerInfo_var ci;
				ci->id = backid;
				ci->node = uconf->getLocalNode();

				inc->askOutput(si,ci, cmd);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound& ex)
	{
		rcache.erase(sensid, node);
		throw NameNotFound("UI(askOutput): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(sensid, node);		
		throw IOBadParam(set_err("UI(askOutput): �� ���� �������� ������ �� ������ ",sensid,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(sensid, node);		
		throw IOBadParam(set_err("UI(askOutput): ���������� ���������� ������",sensid,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(sensid, node);		
		throw IOBadParam(set_err("UI(askOutput): ������ �� �� ������������ ������",sensid,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askOutput): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askOutput): CORBA::SystemException" << endl;
	}	
	rcache.erase(sensid, node);	
	throw UniSetTypes::TimeOut(set_err("UI(askOutput): TimeOut ��� �������",sensid,node));
}

// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::askOutput( ObjectId name, UniversalIO::UIOCommand cmd, UniSetTypes::ObjectId backid )
{
	askRemoteOutput(name, cmd, uconf->getLocalNode(), backid);
}

// ------------------------------------------------------------------------------------------------------------

/*!
 * \param timerid - ������������� �������
 * \param timeMS - �������� (0 -  �������� �����)
 * \param ticks - ���������� ����������� (0 - ���������)
 * \param backid - �������� ����� (������������� ���������)
*/
void UniversalInterface::askTimer( UniSetTypes::TimerId timerid, CORBA::Long timeMS, CORBA::Short ticks, 
									UniSetTypes::Message::Priority priority, UniSetTypes::ObjectId backid) 
								throw(IO_THROW_EXCEPTIONS)
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw IOBadParam("UI(askTimer): ���������� �������� ������");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve( uconf->getTimerService(), uconf->getLocalNode() );
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( uconf->getTimerService(), uconf->getLocalNode() );

				TimerService_i_var ts = TimerService_i::_narrow(oref);

				ConsumerInfo_var ci;
				ci->id = backid;
				ci->node = uconf->getLocalNode();

				TimerService_i::Timer ti;
				ti.timerid = timerid;
				ti.timeMS = timeMS;
				ti.ticks = ticks;
				ti.msgPriority = priority;
				ts->askTimer(ti, ci);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}

	}
	catch(TimerService_i::TimerAlreadyExist)
	{
		// unideb[Debug::WARN] << "UI(askTimer): ������ � ����� id ��� ������� " << endl;
		return;
	}
	catch(TimerService_i::LimitTimers& ex )
	{
		ostringstream err;
		err << "UI(askTimer): ��������� ������������ ���������� ���������� " << ex.maxTimers;
//		unideb[Debug::WARN] <<  err.str() << endl;
		throw Exception(err.str());
	}
	catch(TimerService_i::TimeMSLowLimit& ex )
	{
		ostringstream err;
		err << "UI(askTimer): ��������� �������� ������ �����ۣ����� " << ex.lowLimitMS  << " [��]";
//		unideb[Debug::WARN] <<  err.str() << endl;
		throw OutOfRange(err.str());
	}
	catch(ORepFailed)
	{
		rcache.erase(uconf->getTimerService(), uconf->getLocalNode());
		throw IOBadParam(set_err("UI(askTimer): �� ���� �������� ������ �� ������ ",uconf->getTimerService(), uconf->getLocalNode()));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(uconf->getTimerService(), uconf->getLocalNode());
		throw IOBadParam(set_err("UI(askTimer): ���������� ���������� ������",uconf->getTimerService(), uconf->getLocalNode()));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(uconf->getTimerService(), uconf->getLocalNode());
		throw IOBadParam(set_err("UI(askTimer): ������ �� �� ������������ ������",uconf->getTimerService(), uconf->getLocalNode()));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askTimer): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askTimer): CORBA::SystemException" << endl;
	}	
	// unideb[Debug::WARN] << "UI(askTimer): catch...." << endl;
	rcache.erase( uconf->getTimerService(), uconf->getLocalNode());	
	throw UniSetTypes::TimeOut(set_err("UI(askTimer): TimeOut ��� �������",uconf->getTimerService(),uconf->getLocalNode()));
}
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param mid - ��� ������������� ���������
 * \param cmd - ������� ��. \ref UniversalIO::UIOCommand
 * \param ask - ��������� ����������� � �������������
 * \param backid - �������� ����� (������������� ���������)
*/
void UniversalInterface::askMessage( UniSetTypes::MessageCode mid, UniversalIO::UIOCommand cmd, bool ack, 
										UniSetTypes::ObjectId backid ) throw(IO_THROW_EXCEPTIONS)
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw IOBadParam("UI(askMessage): ���������� �������� ������");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve( uconf->getInfoServer(), uconf->getLocalNode() );
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( uconf->getInfoServer(), uconf->getLocalNode() );

				InfoServer_i_var is = InfoServer_i::_narrow(oref);
				ConsumerInfo_var ci;
				ci->id = backid;
				ci->node = uconf->getLocalNode();
				is->ackMessage(mid, ci, cmd, ack);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}

			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}

	}
	catch(InfoServer_i::MsgNotFound& ex)
	{
		ostringstream err;
		err << "UI(askMessage): ����������� ��� ��������� " << ex.bad_code;
//		unideb[Debug::WARN] <<  err.str() << endl;
		throw NameNotFound(err.str());
	}
	catch(ORepFailed)
	{
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(askMessage): �� ���� �������� ������ �� ������ ",uconf->getInfoServer(), uconf->getLocalNode()));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
		throw IOBadParam(set_err("UI(askMessage): ���������� ���������� ������",uconf->getInfoServer(), uconf->getLocalNode()));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
		throw IOBadParam(set_err("UI(askMessage): ������ �� �� ������������ ������",uconf->getInfoServer(), uconf->getLocalNode()));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askTimer): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askTimer): CORBA::SystemException" << endl;
	}	

	rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
	throw UniSetTypes::TimeOut(set_err("UI(askMessage): TimeOut ��� �������",uconf->getInfoServer(), uconf->getLocalNode()));
}
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param from - ��� ���������� ���������
 * \param to - ��� ��������� ���������
 * \param cmd - ������� ��. \ref UniversalIO::UIOCommand
 * \param ask - ��������� ����������� � �������������
 * \param backid - �������� ����� (������������� ���������)
*/
void UniversalInterface::askMessageRange( UniSetTypes::MessageCode from, UniSetTypes::MessageCode to,
			UniversalIO::UIOCommand cmd, bool ack, UniSetTypes::ObjectId backid ) throw(IO_THROW_EXCEPTIONS)
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw IOBadParam("Ul(askMessageRange): ���������� �������� ������");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve( uconf->getInfoServer(), uconf->getLocalNode() );
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( uconf->getInfoServer(), uconf->getLocalNode() );

				InfoServer_i_var is = InfoServer_i::_narrow(oref);

				ConsumerInfo_var ci;
				ci->id = backid;
				ci->node = uconf->getLocalNode();
				is->ackMessageRange(from, to, ci, cmd, ack);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}

	}
	catch(InfoServer_i::MsgNotFound& ex)
	{
		ostringstream err;
		err << "UI(askMessage): ����������� ��� ��������� " << ex.bad_code;
//		unideb[Debug::WARN] <<  err.str() << endl;
		throw NameNotFound(err.str());
	}
	catch(InfoServer_i::MsgBadRange)
	{
//		unideb[Debug::WARN] << "UI(askMessageRange): ������� ����� �������� " << endl;
		throw OutOfRange("UI(askMessageRange): ������� ����� ��������");
	}
	catch(ORepFailed)
	{
		rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(askMessageRange): �� ���� �������� ������ �� ������ ",uconf->getInfoServer(), uconf->getLocalNode()));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
		throw IOBadParam(set_err("UI(askMessageRange): ���������� ���������� ������",uconf->getInfoServer(), uconf->getLocalNode()));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
		throw IOBadParam(set_err("UI(askMessageRange): ������ �� �� ������������ ������",uconf->getInfoServer(), uconf->getLocalNode()));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askTimer): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askTimer): CORBA::SystemException" << endl;
	}	
	// unideb[Debug::WARN] << "UI(askTimer): catch...." << endl;
	rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
	throw UniSetTypes::TimeOut(set_err("UI(getMessageRange): TimeOut ��� �������",uconf->getInfoServer(), uconf->getLocalNode()));
}								
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param name - ������������� �������
 * \param node - ������������� ����
*/
IOTypes UniversalInterface::getIOType(ObjectId name, ObjectId node)
	throw(IO_THROW_EXCEPTIONS)	
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(getIOType): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}

		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve(name, node);

				IOController_i_var inc = IOController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = name;
				si->node = node;
				return inc->getIOType(si);
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(IOController_i::NameNotFound& ex)
	{
		rcache.erase(name, node);	
		throw NameNotFound("UI(getIOType): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(getIOType): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(getIOType): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(getIOType): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getIOType): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getIOType): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);	
	throw UniSetTypes::TimeOut(set_err("UI(getIOType): TimeOut ��� �������",name, node));
}

IOTypes UniversalInterface::getIOType(ObjectId name)
{
	return getIOType(name, uconf->getLocalNode() );
}
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param name - ������������� �������
 * \param node - ������������� ����
*/
ObjectType UniversalInterface::getType(ObjectId name, ObjectId node)
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(getType): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( name, node );

				UniSetObject_i_var uo = UniSetObject_i::_narrow(oref);
				return uo->getType();
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(getType): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(getType): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(getType): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getType): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getType): CORBA::SystemException" << endl;
	}	
	catch(UniSetTypes::TimeOut){}
	
	rcache.erase(name, node);	
	throw UniSetTypes::TimeOut(set_err("UI(getType): TimeOut ��� �������",name, node));
}

ObjectType UniversalInterface::getType(ObjectId name)
{
	return getType(name, uconf->getLocalNode());
}

// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::registered(UniSetTypes::ObjectId id, const UniSetTypes::ObjectPtr oRef, bool force)
																					throw(UniSetTypes::ORepFailed)
{
	registered(id,uconf->getLocalNode(), oRef,force);
}

// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::registered( UniSetTypes::ObjectId id, UniSetTypes::ObjectId node, 
			const UniSetTypes::ObjectPtr oRef, bool force ) throw(ORepFailed)
{
	// ���� ���ޣ� ����� ������������� ��������� ������
	// �� ����� IOR � ����
	// �� ��� ���� �� ������ �ݣ � �������� �����������������
	// ����� NameService (omniNames)
	if( uconf->isLocalIOR() )
	{
		if( CORBA::is_nil(orb) )
			orb = uconf->getORB();

		string sior(orb->object_to_string(oRef));
		uconf->iorfile.setIOR(id,node,sior);
		return;
	}

	try
	{
		string nm=oind->getNameById(id, node);
		rep.registration(nm,oRef,(bool)force);
	}
	catch(Exception& ex )
	{
		if( !uconf->isLocalIOR() )
			throw;
	}
}

// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::unregister(UniSetTypes::ObjectId id, UniSetTypes::ObjectId node)throw(ORepFailed)
{
	if( uconf->isLocalIOR() )
		uconf->iorfile.unlinkIOR(id,node);

	try
	{
		string nm = oind->getNameById(id,node);
		rep.unregistration(nm);
	}
	catch(Exception& ex )
	{
		if( !uconf->isLocalIOR() )
			throw;
	}
}

// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::unregister(UniSetTypes::ObjectId id)throw(UniSetTypes::ORepFailed)
{
	unregister(id,uconf->getLocalNode());
}

// ------------------------------------------------------------------------------------------------------------
ObjectPtr UniversalInterface::resolve( ObjectId rid , ObjectId node, int timeoutSec )
	throw(ResolveNameError, UniSetTypes::TimeOut )
{
	if ( rid == DefaultObjectId )
		throw ResolveNameError("UI(resolve): ������� resolve id=UniSetTypes::DefaultObjectId");

	CosNaming::NamingContext_var ctx;
	rcache.erase(rid, node);	
	try
	{
		if( uconf->isLocalIOR() )
		{
			if( CORBA::is_nil(orb) )
				orb = uconf->getORB();

			string sior(uconf->iorfile.getIOR(rid,node));
			if( !sior.empty() )
			{
				CORBA::Object_var nso = orb->string_to_object(sior.c_str());
				rcache.cache(rid, node, nso); // ������� � ��� 
				return nso._retn();
			}
			else
			{
				// ���� NameService ���������� ��,
				// ����� ������ ������
				if( CORBA::is_nil(localctx) )
				{
					unideb[Debug::WARN] << "�� ������ IOR-���� ��� " << uconf->oind->getNameById(rid,node) << endl;
					throw UniSetTypes::ResolveNameError();
				}
				// ����� �������� �������� ������ ����� NameService (omniNames)
				unideb[Debug::WARN] << "�� ������ IOR-���� ��� " << uconf->oind->getNameById(rid,node) 
									<< " �������� �������� ������ ����� NameService \n";

			}
		}
		
	
		if( node!=uconf->getLocalNode() )
		{
			// �������� ������ � NameService �� ������ ����
			string nodeName( oind->getRealNodeName(node) );
			string bname(nodeName); // ��������� ������� ��������
			for(unsigned int curNet=1; curNet<=uconf->getCountOfNet(); curNet++)
			{
				try
				{
//					// unideb[Debug::INFO] << "�������� ��������� � "<< node << endl;
					if( CORBA::is_nil(orb) )
						orb = uconf->getORB();

					ctx = ORepHelpers::getRootNamingContext( orb, nodeName.c_str() );
//					// unideb[Debug::INFO] << "ok. "<< endl;
					break;
				}
//				catch(CORBA::COMM_FAILURE& ex )
				catch(ORepFailed& ex)
				{
					// ��� ����� � ���� �����
					// ������� �������� �� ������ ����
					// �� �������� ���� � ������ ������ ����� ��� NodeName1...NodeNameX
					ostringstream s;
					s << bname << curNet;
					nodeName=s.str();
				}	
			}
			
			if( CORBA::is_nil(ctx) )
			{
				// unideb[Debug::WARN] << "NameService ���������� �� ���� "<< node << endl;
				throw NSResolveError();
			}
		}
		else
			ctx = localctx;

		CosNaming::Name_var oname = omniURI::stringToName( oind->getNameById(rid,node).c_str() );
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try		
			{
				CORBA::Object_var nso = ctx->resolve(oname);
				if( CORBA::is_nil(nso) )
					throw UniSetTypes::ResolveNameError();

				// ��� var
				rcache.cache(rid, node, nso); // ������� � ��� 
				return nso._retn();
			}
			catch(CORBA::TRANSIENT){}

			msleep(uconf->getRepeatTimeout());
		}
		
		throw UniSetTypes::TimeOut();			
	}
	catch(const CosNaming::NamingContext::NotFound &nf){}
	catch(const CosNaming::NamingContext::InvalidName &nf){}
	catch(const CosNaming::NamingContext::CannotProceed &cp){}
	catch(Exception){}
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		throw UniSetTypes::ResolveNameError("ObjectNOTExist");
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		throw UniSetTypes::ResolveNameError("CORBA::CommFailure");
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(resolve): CORBA::SystemException" << endl;
		throw UniSetTypes::TimeOut();
	}	

	throw UniSetTypes::ResolveNameError();
}

// -------------------------------------------------------------------------------------------

string UniversalInterface::timeToString(time_t tm, const std::string brk )
{
    struct tm *tms = localtime(&tm);
	ostringstream time;
	time << std::setw(2) << std::setfill('0') << tms->tm_hour << brk;
	time << std::setw(2) << std::setfill('0') << tms->tm_min << brk;
	time << std::setw(2) << std::setfill('0') << tms->tm_sec;	
	return time.str();
}

string UniversalInterface::dateToString(time_t tm, const std::string brk )
{
    struct tm *tms = localtime(&tm);
	ostringstream date;
	date << std::setw(4) << std::setfill('0') << tms->tm_year+1900 << brk;
	date << std::setw(2) << std::setfill('0') << tms->tm_mon+1 << brk;
	date << std::setw(2) << std::setfill('0') << tms->tm_mday;	
	return date.str();
}

//--------------------------------------------------------------------------------------------

void UniversalInterface::send( ObjectId name, TransportMessage& msg, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(send): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}

		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( name, node );

				UniSetObject_i_var obj = UniSetObject_i::_narrow(oref);
				obj->push(msg);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();
		}
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(send): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(send): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(send): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(send): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(send): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);			
	throw UniSetTypes::TimeOut(set_err("UI(send): TimeOut ��� �������",name, node));
}

void UniversalInterface::send( ObjectId name, TransportMessage& msg )
{
	send(name, msg, uconf->getLocalNode());
}

// ------------------------------------------------------------------------------------------------------------
/*!
 * \param id - ������������� �������
 * \param node - ������������� ����
 * \param type - ��� ������� 
 * \param value - �������� ������� ���������� ����������
*/
bool UniversalInterface::saveValue(ObjectId name, long value, IOTypes type, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(saveValue): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}

		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( name, node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = name;
				si->node = node;
				iom->saveValue(si, value, type, myid);
				return true;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(name, node);
		throw NameNotFound("UI(saveValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(saveValue): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(saveValue): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(saveValue): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(saveValue): CORBA::SystemException" << endl;
	}	
	
	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(saveValue): TimeOut ��� �������",name, node));
}

bool UniversalInterface::saveValue(ObjectId name, long value, IOTypes type)
{
	return saveValue(name, value, type, uconf->getLocalNode() );
}

// ------------------------------------------------------------------------------------------------------------

bool UniversalInterface::saveValue( IOController_i::SensorInfo& si, long value, UniversalIO::IOTypes type, 
									UniSetTypes::ObjectId supplier )
{
	ObjectId old = myid;
	bool res(false);
	try
	{
		myid = supplier;
		res = saveValue(si.id,value,type,si.node);
	}
	catch(...)
	{
		myid = old;
		throw;
	}
	
	myid = old;
	return res;
}

// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::fastSaveValue( IOController_i::SensorInfo& si, long value, UniversalIO::IOTypes type, 
										UniSetTypes::ObjectId supplier ) 
{
	if ( si.id == DefaultObjectId )
	{
		unideb[Debug::WARN] << "UI(fastSaveValue): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId" << endl;
		return;
	}

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(si.id, si.node);
		}
		catch( NameNotFound ){}

		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( si.id, si.node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				iom->fastSaveValue(si, value, type,supplier);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << "UI(saveValue): " << ex.err << endl;
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSaveValue): �� ���� �������� ������ �� ������ ",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSaveValue): ���������� ���������� ������",si.id,si.node) << endl;
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSaveValue): ������ �� �� ������������ ������",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(saveValue): CORBA::SystemException" << endl;
	}	
	catch(...){}
	
	rcache.erase(si.id,si.node);
	unideb[Debug::WARN] << set_err("UI(saveValue): TimeOut ��� �������",si.id, si.node) << endl;
}
// ------------------------------------------------------------------------------------------------------------

/*!
 * \param id - ������������� �������
 * \param state - ��������� � ������� ��� ���������� ���������
 * \param type - ��� �������
 * \param node - ������������� ����
*/
bool UniversalInterface::saveState(ObjectId name, bool state, IOTypes type, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(saveState): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(name, node);
		}
		catch( NameNotFound ){}

		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( name, node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = name;
				si->node = node;
				iom->saveState(si, state, type, myid);
				return true;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(name, node);
		throw NameNotFound("UI(saveState): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(saveState): �� ���� �������� ������ �� ������ ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(saveState): ���������� ���������� ������",name,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw IOBadParam(set_err("UI(saveState): ������ �� �� ������������ ������",name,node));
	}	
	catch(CORBA::COMM_FAILURE)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(saveState): CORBA::COMM_FAILURE " << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(saveState): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(saveState): TimeOut ��� �������",name, node));
}

bool UniversalInterface::saveState(ObjectId name, bool state, IOTypes type) 
{
	return saveState(name, state, type, uconf->getLocalNode() );
}

// ------------------------------------------------------------------------------------------------------------

bool UniversalInterface::saveState( IOController_i::SensorInfo& si, bool state, UniversalIO::IOTypes type, 
									UniSetTypes::ObjectId supplier )
{
	ObjectId old = myid;
	bool res(false);
	try
	{
		myid = supplier;
		res = saveState(si.id,state,type,si.node);
	}
	catch(...)
	{
		myid = old;
		throw;
	}

	myid = old;
	return res;
}
// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::fastSaveState( IOController_i::SensorInfo& si, bool state, UniversalIO::IOTypes type, 
									UniSetTypes::ObjectId supplier) 
{
	if( si.id == DefaultObjectId )
	{
		unideb[Debug::WARN] << "UI(fastSaveState): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId" << endl;
		return;
	}

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(si.id, si.node);
		}
		catch( NameNotFound ){}

		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( si.id, si.node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				iom->fastSaveState(si, state, type, supplier);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << "UI(fastSaveState): " << ex.err << endl;
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << set_err("UI(fastSaveState): �� ���� �������� ������ �� ������ ",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);		
		unideb[Debug::WARN] << set_err("UI(fastSaveState): ���������� ���������� ������",si.id,si.node) << endl;
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << set_err("UI(fastSaveState): ������ �� �� ������������ ������",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(saveState): CORBA::COMM_FAILURE " << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(saveState): CORBA::SystemException" << endl;
	}	
	catch(...){}
	
	rcache.erase(si.id, si.node);
	unideb[Debug::WARN] << set_err("UI(fastSaveState): TimeOut ��� �������",si.id, si.node) << endl;
}

// ------------------------------------------------------------------------------------------------------------

ObjectPtr UniversalInterface::CacheOfResolve::resolve( ObjectId id, ObjectId node )
	throw(NameNotFound)
{

//#warning �������� �����ޣ� ���
//	throw NameNotFound();

	CacheMap::iterator it = mcache.find( key(id,node) );
	if( it == mcache.end() )
		throw NameNotFound();

	it->second.timestamp = time(NULL); // ��������� ����� ���������� ���������
	
	// �.�. ������� ���������� ���������
	// � ��� ��� �������� �������� �� ������������ ������
	// �� �� ������ _duplicate....
	
	if( !CORBA::is_nil(it->second.ptr) )
		return it->second.ptr.out(); // CORBA::Object::_duplicate(it->second.ptr);

	throw NameNotFound();		
}
// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::CacheOfResolve::cache( ObjectId id, ObjectId node, ObjectVar ptr )
{
//#warning �������� �����ޣ� ���
//	return;

	if( mcache.size() > MaxSize )
	{
		if( !clean() )
			 unideb[Debug::CRIT] << "UI(resolve cache): �� ������� ��������� ������ ���-�!!!!"<< endl;
	}

	UniSetTypes::KeyType k(key(id,node));

	CacheMap::iterator it = mcache.find(k);
	if( it==mcache.end() )
		mcache.insert(CacheMap::value_type(k,Info(ptr)));
	else
	{
		it->second.ptr = ptr; // CORBA::Object::_duplicate(ptr);
		it->second.timestamp = time(NULL);
	}
}
// ------------------------------------------------------------------------------------------------------------
bool UniversalInterface::CacheOfResolve::clean()
{
	unideb[Debug::INFO] << "UI: clean cache...."<< endl;
	time_t tm = time(NULL)-CleanTime*60;
//	remove_if(mcache.begin(), mcache.end(),OldRef_eq(tm));
	for( CacheMap::iterator it=mcache.begin(); it!=mcache.end();)
	{
		if( it->second.timestamp < tm )
			mcache.erase(it++);
		else
			++it;
	}

	if( mcache.size() < MaxSize )
		return true;
		
	return false;
}
// ------------------------------------------------------------------------------------------------------------

void UniversalInterface::CacheOfResolve::erase( UniSetTypes::ObjectId id, UniSetTypes::ObjectId node )
{
//#warning �������� �����ޣ� ���
//	return;

	CacheMap::iterator it = mcache.find( key(id,node) );
	if( it != mcache.end() )
		mcache.erase(it);
}

// ------------------------------------------------------------------------------------------------------------
bool UniversalInterface::info( string msg, ObjectId messenger, ObjectId node,
								InfoMessage::Character ch, ObjectId from )
{
	if( from==UniSetTypes::DefaultObjectId )
		from = myid;

	if( from==UniSetTypes::DefaultObjectId )
		unideb[Debug::WARN] << "UI(info): �� ������ ������������� �����" << endl;
		
	InfoMessage im(from, msg, node, ch);
	return info(im, messenger);
}
// ------------------------------------------------------------------------------------------------------------

bool UniversalInterface::alarm( string msg, ObjectId messenger, ObjectId node,
								AlarmMessage::Character ch, ObjectId from )
{
	if( from==UniSetTypes::DefaultObjectId )
		from = myid;

	if( from==UniSetTypes::DefaultObjectId )
		unideb[Debug::WARN] << "UI(alarm): �� ������ ������������� �����" << endl;
		
	AlarmMessage am(from, msg, node, ch);
	return alarm(am, messenger);
}
// ------------------------------------------------------------------------------------------------------------
bool UniversalInterface::alarm( UniSetTypes::AlarmMessage& msg,  UniSetTypes::ObjectId messenger)
{
	try
	{
		TransportMessage tm(msg.transport_msg());
		send(messenger, tm);
		return true;
	}
	catch(...){}
	return false;
}
// ------------------------------------------------------------------------------------------------------------
bool UniversalInterface::info( UniSetTypes::InfoMessage& msg,  UniSetTypes::ObjectId messenger)
{
	try
	{
		TransportMessage tm(msg.transport_msg());
		send(messenger, tm);
		return true;
	}
	catch(...){}
	return false;
}
// ------------------------------------------------------------------------------------------------------------
bool UniversalInterface::isExist( UniSetTypes::ObjectId id )
{
	try
	{
/*	
		try
		{
			oref = rcache.resolve(id, uconf->getLocalNode());
		}
		catch(NameNotFound){}

		if(!oref)
			oref = resolve(id, uconf->getLocalNode());
		return rep.isExist( oref );
*/		
		string nm = oind->getNameById(id);
		return rep.isExist(nm);
	}
	catch(UniSetTypes::Exception& ex)
	{
//		unideb[Debug::WARN] << "UI(isExist): " << ex << endl;
	}
	catch(...){}	
	return false;
}
// ------------------------------------------------------------------------------------------------------------
bool UniversalInterface::isExist( UniSetTypes::ObjectId id, UniSetTypes::ObjectId node )
{
	if( node==uconf->getLocalNode() )
		return isExist(id);

	CORBA::Object_var oref;
	try
	{
		try
		{
			oref = rcache.resolve(id, node);
		}
		catch(NameNotFound){}

		if( CORBA::is_nil(oref) )		
			oref = resolve(id, node);

		return rep.isExist( oref );
	}
	catch(...){}

	return false;
}
// --------------------------------------------------------------------------------------------
string UniversalInterface::set_err(const string& pre, UniSetTypes::ObjectId id, UniSetTypes::ObjectId node)
{
	if( id==UniSetTypes::DefaultObjectId )
		return string(pre+" DefaultObjectId");

	string nm(oind->getNameById(id,node));
	
	if( nm.empty() )
		nm = "UnknownName";

	ostringstream s;
	s << pre << " (" << id << ")" << nm; 
	return s.str();	
}
// --------------------------------------------------------------------------------------------
void UniversalInterface::askThreshold( UniSetTypes::ObjectId sid, UniSetTypes::ThresholdId tid,
										UniversalIO::UIOCommand cmd,
										CORBA::Long low, CORBA::Long hi, CORBA::Long sb, 
										UniSetTypes::ObjectId backid)
{
	askRemoteThreshold(sid, uconf->getLocalNode(), tid, cmd, low,hi,sb, backid);
}							
// --------------------------------------------------------------------------------------------
void UniversalInterface::askRemoteThreshold( UniSetTypes::ObjectId sid, UniSetTypes::ObjectId node,
										 UniSetTypes::ThresholdId tid, UniversalIO::UIOCommand cmd,
										 CORBA::Long lowLimit, CORBA::Long hiLimit, CORBA::Long sensibility, 
										 UniSetTypes::ObjectId backid )
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw IOBadParam("UI(askRemoteThreshold): ���������� �������� �����");

	if ( sid == DefaultObjectId )
		throw ORepFailed("UI(askRemoteThreshold): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(sid, node);
		}
		catch( NameNotFound ){}
		
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )				
					oref = resolve( sid, node );
			
				IONotifyController_i_var inc = IONotifyController_i::_narrow(oref);
				IOController_i::SensorInfo_var si;
				si->id = sid;
				si->node = node;

				ConsumerInfo_var ci;
				ci->id = backid;
				ci->node = uconf->getLocalNode();

				inc->askThreshold(si,ci,tid,lowLimit,hiLimit,sensibility,cmd);
				return;
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());			
			oref = CORBA::Object::_nil();			
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound& ex)
	{
		rcache.erase(sid, node);
		throw NameNotFound("UI(askThreshold): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(sid, node);		
		throw IOBadParam(set_err("UI(askThreshold): �� ���� �������� ������ �� ������ ",sid,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(sid, node);		
		throw IOBadParam(set_err("UI(askThreshold): ���������� ���������� ������",sid,node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(sid, node);		
		throw IOBadParam(set_err("UI(askThreshold): ������ �� �� ������������ ������",sid,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askThreshold): ������ ������� ������������" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(askThreshold): CORBA::SystemException" << endl;
	}	
	rcache.erase(sid, node);	
	throw UniSetTypes::TimeOut(set_err("UI(askThreshold): TimeOut ��� �������",sid,node));

}
// --------------------------------------------------------------------------------------------
CORBA::Long UniversalInterface::getRawValue( const IOController_i::SensorInfo& si )
{
	if ( si.id == DefaultObjectId )
		throw ORepFailed("UI(getRawValue): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(si.id, si.node);
		}
		catch( NameNotFound ){}

		for( unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve( si.id, si.node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				return iom->getRawValue(si);
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());
			oref = CORBA::Object::_nil();
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(si.id, si.node);
		throw NameNotFound("UI(getRawValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);		
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(getRawValue): �� ���� �������� ������ �� ������ ",si.id,si.node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);		
		throw IOBadParam(set_err("UI(getRawValue): ���������� ���������� ������",si.id,si.node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);		
		throw IOBadParam(set_err("UI(getRawValue): ������ �� �� ������������ ������",si.id,si.node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(si.id, si.node);		
	throw UniSetTypes::TimeOut(set_err("UI(getRawValue): TimeOut ��� �������",si.id,si.node));
}
// --------------------------------------------------------------------------------------------
void UniversalInterface::calibrate(const IOController_i::SensorInfo& si, 
								   const IOController_i::CalibrateInfo& ci,
								   UniSetTypes::ObjectId admId )
{
	if( admId==UniSetTypes::DefaultObjectId )
		admId = myid;
		
//	if( admId==UniSetTypes::DefaultObjectId )
//		throw IOBadParam("UI(askTreshold): ���������� ID ��������������");

	if ( si.id == DefaultObjectId )
		throw ORepFailed("UI(calibrate): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(si.id, si.node);
		}
		catch( NameNotFound ){}

		for( unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve( si.id, si.node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				iom->calibrate(si,ci,admId);
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());
			oref = CORBA::Object::_nil();
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(si.id, si.node);
		throw NameNotFound("UI(calibrate): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);		
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(calibrate): �� ���� �������� ������ �� ������ ",si.id,si.node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);		
		throw IOBadParam(set_err("UI(calibrate): ���������� ���������� ������",si.id,si.node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);		
		throw IOBadParam(set_err("UI(calibrate): ������ �� �� ������������ ������",si.id,si.node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(si.id, si.node);		
	throw UniSetTypes::TimeOut(set_err("UI(calibrate): TimeOut ��� �������",si.id,si.node));
}			   
// --------------------------------------------------------------------------------------------
IOController_i::CalibrateInfo UniversalInterface::getCalibrateInfo( const IOController_i::SensorInfo& si )
{
	if ( si.id == DefaultObjectId )
		throw ORepFailed("UI(getCalibrateInfo): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(si.id, si.node);
		}
		catch( NameNotFound ){}

		for( unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve( si.id, si.node );

				IOController_i_var iom = IOController_i::_narrow(oref);
				return iom->getCalibrateInfo(si);
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());
			oref = CORBA::Object::_nil();
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(si.id, si.node);
		throw NameNotFound("UI(getCalibrateInfo): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);		
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(getCalibrateInfo): �� ���� �������� ������ �� ������ ",si.id,si.node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);		
		throw IOBadParam(set_err("UI(getCalibrateInfo): ���������� ���������� ������",si.id,si.node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);		
		throw IOBadParam(set_err("UI(getCalibrateInfo): ������ �� �� ������������ ������",si.id,si.node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(si.id, si.node);		
	throw UniSetTypes::TimeOut(set_err("UI(getCalibrateInfo): TimeOut ��� �������",si.id,si.node));
}
// --------------------------------------------------------------------------------------------
IOController_i::ASensorInfoSeq_var UniversalInterface::getSensorSeq( UniSetTypes::IDList& lst )
{
	if( lst.size() == 0 )
		return IOController_i::ASensorInfoSeq_var();

	ObjectId sid = lst.getFirst();

	if ( sid == DefaultObjectId )
		throw ORepFailed("UI(getSensorSeq): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(sid,conf->getLocalNode());
		}
		catch( NameNotFound ){}

		for( unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve(sid,conf->getLocalNode());

				IOController_i_var iom = IOController_i::_narrow(oref);
				
				UniSetTypes::IDSeq_var seq = lst.getIDSeq();
				return iom->getSensorSeq(seq);
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());
			oref = CORBA::Object::_nil();
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw NameNotFound("UI(getSensorSeq): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(sid,conf->getLocalNode());
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(getSensorSeq): �� ���� �������� ������ �� ������ ",sid,conf->getLocalNode()));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw IOBadParam(set_err("UI(getSensorSeq): ���������� ���������� ������",sid,conf->getLocalNode()));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw IOBadParam(set_err("UI(getSensorSeq): ������ �� �� ������������ ������",sid,conf->getLocalNode()));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(sid,conf->getLocalNode());
	throw UniSetTypes::TimeOut(set_err("UI(getSensorSeq): TimeOut ��� �������",sid,conf->getLocalNode()));

}
// --------------------------------------------------------------------------------------------
IDSeq_var UniversalInterface::setOutputSeq( const IOController_i::OutSeq& lst, UniSetTypes::ObjectId sup_id )
{
	if( lst.length() == 0 )
		return UniSetTypes::IDSeq_var();


	if ( lst[0].si.id == DefaultObjectId )
		throw ORepFailed("UI(setOutputSeq): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(lst[0].si.id,lst[0].si.node);
		}
		catch( NameNotFound ){}

		for( unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve(lst[0].si.id,lst[0].si.node);

				IONotifyController_i_var iom = IONotifyController_i::_narrow(oref);
				return iom->setOutputSeq(lst,sup_id);
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());
			oref = CORBA::Object::_nil();
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(lst[0].si.id,lst[0].si.node);
		throw NameNotFound("UI(setOutputSeq): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(lst[0].si.id,lst[0].si.node);
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(setOutputSeq): �� ���� �������� ������ �� ������ ",lst[0].si.id,lst[0].si.node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(lst[0].si.id,lst[0].si.node);
		throw IOBadParam(set_err("UI(setOutputSeq): ���������� ���������� ������",lst[0].si.id,lst[0].si.node));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(lst[0].si.id,lst[0].si.node);
		throw IOBadParam(set_err("UI(setOutputSeq): ������ �� �� ������������ ������",lst[0].si.id,lst[0].si.node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(lst[0].si.id,lst[0].si.node);
	throw UniSetTypes::TimeOut(set_err("UI(setOutputSeq): TimeOut ��� �������",lst[0].si.id,lst[0].si.node));
}
// --------------------------------------------------------------------------------------------
UniSetTypes::IDSeq_var UniversalInterface::askSensorsSeq( UniSetTypes::IDList& lst, 
														UniversalIO::UIOCommand cmd, UniSetTypes::ObjectId backid )
{
	if( lst.size() == 0 )
		return UniSetTypes::IDSeq_var();

	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw IOBadParam("UI(askSensorSeq): ���������� �������� �����");

	ObjectId sid = lst.getFirst();

	if ( sid == DefaultObjectId )
		throw ORepFailed("UI(askSensorSeq): ������� ���������� � ������� � id=UniSetTypes::DefaultObjectId");

	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(sid,conf->getLocalNode());
		}
		catch( NameNotFound ){}

		for( unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve(sid,conf->getLocalNode());

				IONotifyController_i_var iom = IONotifyController_i::_narrow(oref);

				ConsumerInfo_var ci;
				ci->id = backid;
				ci->node = uconf->getLocalNode();
				UniSetTypes::IDSeq_var seq = lst.getIDSeq();

				return iom->askSensorsSeq(seq,ci,cmd);
			}
			catch(CORBA::TRANSIENT){}
			catch(CORBA::OBJECT_NOT_EXIST){}
			catch(CORBA::SystemException& ex){}
			msleep(uconf->getRepeatTimeout());
			oref = CORBA::Object::_nil();
		}
	}
	catch(UniSetTypes::TimeOut){}
	catch(IOController_i::NameNotFound &ex)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw NameNotFound("UI(getSensorSeq): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(sid,conf->getLocalNode());
		// �� ������ �������� ������ �� ������
		throw IOBadParam(set_err("UI(askSensorSeq): �� ���� �������� ������ �� ������ ",sid,conf->getLocalNode()));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw IOBadParam(set_err("UI(askSensorSeq): ���������� ���������� ������",sid,conf->getLocalNode()));		
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw IOBadParam(set_err("UI(askSensorSeq): ������ �� �� ������������ ������",sid,conf->getLocalNode()));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ������ ������� ������������
	}	
	catch(CORBA::SystemException& ex)
	{
		// ������ ������� ������������
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(sid,conf->getLocalNode());
	throw UniSetTypes::TimeOut(set_err("UI(askSensorSeq): TimeOut ��� �������",sid,conf->getLocalNode()));
}
// -----------------------------------------------------------------------------
bool UniversalInterface::waitReady( UniSetTypes::ObjectId id, int msec, int pmsec )
{
	PassiveTimer ptReady(msec);
	bool ready = false;
	while( !ptReady.checkTime() && !ready )
	{
		try
		{
			ready = isExist(id);
			if( ready )
				break;
		}
		catch(...){}
	
		msleep(pmsec);
	}
	
	return ready;
}
// -----------------------------------------------------------------------------
bool UniversalInterface::waitWorking( UniSetTypes::ObjectId id, int msec, int pmsec )
{
	PassiveTimer ptReady(msec);
	bool ready = false;

	while( !ptReady.checkTime() && !ready )
	{
		try
		{
			getState(id);
			ready = true;
			break;
		}
		catch(...){}
		msleep(pmsec);
	}

	return ready;

}
// -----------------------------------------------------------------------------
