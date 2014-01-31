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
	// пытаемся получить ссылку на NameSerivice
	// в любом случае. даже если включён режим
	// localIOR
	localctx=CosNaming::NamingContext::_nil();
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
	catch( Exception& ex )
	{
//		if( !uconf->isLocalIOR() )
//			throw ex;

		localctx=CosNaming::NamingContext::_nil();
	}
	catch( ... )
	{
//		if( !uconf->isLocalIOR() )
//			throw;

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
 * \param name - полное имя дискретного датчика
 * \sa UniversalInterface::getState( ObjectId id ) 
*/
/*
bool UniversalInterface::getState ( const string name, const string node="NameService" )throw(TimeOut,IOBadParam)
{
	return getState(oind->getIdByName(name.c_str()));
}
*/

/*
 * \param id - идентификатор датчика
 *═\return текущее состояние датчика
 * \exception IOBadParam - генерируется если указано неправильное имя датчика или секции
 * \exception IOTimeOut - генерируется если в течение времени timeout небыл получен ответ
*/
bool UniversalInterface::getState(ObjectId name, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(getState): error: getState for id=UniSetTypes::DefaultObjectId ?!");

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
		throw UniSetTypes::NameNotFound("UI(getState): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam("UI(getState): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam(set_err("UI(getState): ORepFailed ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam(set_err("UI(getState): method no implemented",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam(set_err("UI(getState): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getState): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(getState): TimeOut ",name,node));
}

bool UniversalInterface::getState( ObjectId name )
{
	return getState( name, uconf->getLocalNode() );
}

// ---------------------------------------------------------------------
/*!
 * \param name - полное имя аналогового датчика
 * \sa UniversalInterface::getValue( ObjectId id ) 
*/
/*
long UniversalInterface::getValue(const string name, const string node="NameService")throw(TimeOut,IOBadParam)
{
	return getValue(oind->getIdByName(name.c_str()));
}
*/

/*!
 * \param id - идентификатор датчика
 *═\return текущее значение датчика
 * \exception IOBadParam - генерируется если указано неправильное имя датчика или секции
 * \exception IOTimeOut - генерируется если в течение времени timeout небыл получен ответ
*/
long UniversalInterface::getValue(ObjectId name, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(getValue): error id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(getValue): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam("UI(getValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(getValue): ORepFailed",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(getValue): method no implement",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(getValue): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(name, node);		
	throw UniSetTypes::TimeOut(set_err("UI(getValue): TimeOut",name,node));
}

long UniversalInterface::getValue( ObjectId name ) 
{
	return getValue(name, uconf->getLocalNode());
}


// ------------------------------------------------------------------------------------------------------------
/*!
 * \param id - идентификатор датчика
 * \param state - состояние в которое его необходимо перевести
 *═\return текущее значение датчика
 * \exception IOBadParam - генерируется если указано неправильное имя вывода или секции
*/
void UniversalInterface::setState(ObjectId name, bool state, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(setState): error id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound( set_err("UI(setState):"+string(ex.err),name,node) );
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam("UI(setState): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(setState): ORepFailed",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(setState): method no implement",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(setState): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(setState): CORBA::COMM_FAILURE " << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(setState): CORBA::SystemException" << endl;
	}	
			
	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(setState): TimeOut",name,node));
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
// функция не вырабатывает исключений!
void UniversalInterface::fastSetState( IOController_i::SensorInfo& si, bool state, UniSetTypes::ObjectId sup_id ) 
{
	if( si.id == DefaultObjectId )
	{
		unideb[Debug::WARN] << "UI(fastSetState): ID=UniSetTypes::DefaultObjectId" << endl;
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
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(si.id, si.node);
		throw UniSetTypes::IOBadParam("UI(fastSetState): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);	
		// не смогли получить ссылку на объект
		unideb[Debug::WARN] << set_err("UI(fastSetState): resolve failed",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);	
		unideb[Debug::WARN] << set_err("UI(fastSetState): method no implement",si.id,si.node) << endl;	
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);			
		unideb[Debug::WARN] << set_err("UI(fastSetState): object not exist",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE){}
	catch(CORBA::SystemException& ex){}
	catch(...){}		
	
	rcache.erase(si.id, si.node);	
	unideb[Debug::WARN] << set_err("UI(fastSetState): Timeout",si.id,si.node) << endl;
}

// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::setUndefinedState( IOController_i::SensorInfo& si, bool undefined, 
											UniSetTypes::ObjectId sup_id )
{
	if( si.id == DefaultObjectId )
	{
		unideb[Debug::WARN] << "UI(setUndefinedState): ID=UniSetTypes::DefaultObjectId" << endl;
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
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(si.id, si.node);
		throw UniSetTypes::IOBadParam("UI(setUndefinedState): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);
		// не смогли получить ссылку на объект
		unideb[Debug::WARN] << set_err("UI(setUndefinedState): resolve failed",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << set_err("UI(setUndefinedState): method no implement",si.id,si.node) << endl;	
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << set_err("UI(setUndefinedState): object not exist",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE){}
	catch(CORBA::SystemException& ex){}
	catch(...){}
	
	rcache.erase(si.id, si.node);
	unideb[Debug::WARN] << set_err("UI(setUndefinedState): Timeout",si.id,si.node) << endl;
}
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param id - идентификатор датчика
 * \param value - значение которое необходимо установить
 * \return текущее значение датчика
 * \exception IOBadParam - генерируется если указано неправильное имя вывода или секции
*/
void UniversalInterface::setValue(ObjectId name, long value, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(setValue): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound(set_err("UI(setValue): NameNotFound для объекта",name,node));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam("UI(setValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(setValue): resolve failed ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam(set_err("UI(setValue): method no implement",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam(set_err("UI(setValue): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(setValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(setValue): Timeout",name,node));
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
// функция не вырабатывает исключий!
void UniversalInterface::fastSetValue( IOController_i::SensorInfo& si, long value, UniSetTypes::ObjectId sup_id )
{
	if ( si.id == DefaultObjectId )
	{
		unideb[Debug::WARN] << "UI(fastSetValue): ID=UniSetTypes::DefaultObjectId" << endl;
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
		unideb[Debug::WARN] << set_err("UI(fastSetValue): NameNotFound для объекта",si.id,si.node) << endl;
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(si.id, si.node);
		throw UniSetTypes::IOBadParam("UI(fastSetValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id,si.node);	
		// не смогли получить ссылку на объект
		unideb[Debug::WARN] << set_err("UI(fastSetValue): resolve failed ",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSetValue): method no implement",si.id,si.node) << endl;
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSetValue): object not exist",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(setValue): CORBA::SystemException" << endl;
	}	
	catch(...){}

	rcache.erase(si.id,si.node);
	unideb[Debug::WARN] << set_err("UI(fastSetValue): Timeout",si.id,si.node) << endl;
}


// ------------------------------------------------------------------------------------------------------------
/*!
 * \param name - имя дискретного датчика
 * \param fromName - имя объекта(заказчика) кому присылать сообщение об изменении
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
 * \param sensor 	- идентификатор датчика
 * \param node		- идентификатор узла на котором заказывается датчик
 * \param cmd - команда см. \ref UniversalIO::UIOCommand
 * \param backid - обратный адрес (идентификатор заказчика)
*/
void UniversalInterface::askRemoteState( ObjectId name, UniversalIO::UIOCommand cmd, ObjectId node,
									UniSetTypes::ObjectId backid ) throw(IO_THROW_EXCEPTIONS)
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw UniSetTypes::IOBadParam("UI(askRemoteState): Unknown back ID");

	if ( name == DefaultObjectId )
		throw ORepFailed("UI(askRemoteState): id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(askState): "+string(ex.err) );
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam("UI(askState): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(askState): resolve failed ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askState): method no implement",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askState): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askState): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askState): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(askState): Timeout",name,node));
}

void UniversalInterface::askState( ObjectId name, UniversalIO::UIOCommand cmd, UniSetTypes::ObjectId backid )
{
	askRemoteState(name, cmd, uconf->getLocalNode(), backid);
}

// ------------------------------------------------------------------------------------------------------------
/*!
 * \param sensor 	- идентификатор датчика
 * \param node		- идентификатор узла на котором заказывается датчик
 * \param cmd - команда см. \ref UniversalIO::UIOCommand
 * \param backid - обратный адрес (идентификатор заказчика)
*/
void UniversalInterface::askRemoteSensor( ObjectId name, UniversalIO::UIOCommand cmd, ObjectId node,
									UniSetTypes::ObjectId backid ) throw(IO_THROW_EXCEPTIONS)
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw UniSetTypes::IOBadParam("UI(askRemoteSensor): unknown back ID");

	if ( name == DefaultObjectId )
		throw ORepFailed("UI(askRemoteSensor): id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(askSensor): "+string(ex.err) );
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam("UI(askSensor): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(askSensor): resolve failed ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askSensor): method no implement",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askSensor): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askSensor): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askSensor): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(askSensor): Timeout",name,node));
}

void UniversalInterface::askSensor( ObjectId name, UniversalIO::UIOCommand cmd, UniSetTypes::ObjectId backid )
{
	askRemoteSensor(name, cmd, uconf->getLocalNode(), backid);
}

// ------------------------------------------------------------------------------------------------------------

/*!
 * \param name - имя аналогового датчика
 * \param fromName - имя объекта(заказчика) кому присылать сообщение об изменении
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
 * \param id - идентификатор датчика
 * \param backid - идентификатор заказчика, кому присылать сообщение об изменении
 * \param cmd - команда см. \ref UniversalIO::UIOCommand
 * \param backid - обратный адрес (идентификатор заказчика)
 * \exception IOBadParam - генерируется если указано неправильное имя вывода или секции
 * \exception TimeOut - генерируется если нет связи объектом отвечающим за инф-ию об этом датчике
*/
void UniversalInterface::askRemoteValue( ObjectId sensid, UniversalIO::UIOCommand cmd, ObjectId node, 
									UniSetTypes::ObjectId backid) throw(IO_THROW_EXCEPTIONS)	
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw UniSetTypes::IOBadParam("UI(askRemoteValue): unknown back ID");

	if ( sensid == DefaultObjectId )
		throw ORepFailed("UI(askRemoteValue): id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(askValue): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(sensid, node);
		throw UniSetTypes::IOBadParam("UI(askValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(sensid, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askValue): resolve failed ",sensid,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(sensid, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askValue): method no implement",sensid,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(sensid, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askValue): object not exist",sensid,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askValue): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(sensid, node);	
	throw UniSetTypes::TimeOut(set_err("UI(askValue): Timeout",sensid,node));
}

void UniversalInterface::askValue( ObjectId name, UniversalIO::UIOCommand cmd, UniSetTypes::ObjectId backid )
{
	askRemoteValue(name, cmd, uconf->getLocalNode(), backid);
}

// ------------------------------------------------------------------------------------------------------------
/*!
 * \param id - идентификатор датчика
 * \param backid - идентификатор заказчика, кому присылать сообщение об изменении
 * \param cmd - команда см. \ref UniversalIO::UIOCommand
 * \param backid - обратный адрес (идентификатор заказчика)
 * \exception IOBadParam - генерируется если указано неправильное имя вывода или секции
 * \exception TimeOut - генерируется если нет связи объектом отвечающим за инф-ию об этом датчике
*/
void UniversalInterface::askRemoteOutput( ObjectId sensid, UniversalIO::UIOCommand cmd, ObjectId node, 
									UniSetTypes::ObjectId backid) throw(IO_THROW_EXCEPTIONS)	
{
	if( backid == UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw UniSetTypes::IOBadParam("UI(askRemoteOutput): unknown back ID");

	if ( sensid == DefaultObjectId )
		throw ORepFailed("UI(askRemoteOutput): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(askOutput): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(sensid, node);
		throw UniSetTypes::IOBadParam("UI(askOutput): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(sensid, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askOutput): resolve failed ",sensid,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(sensid, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askOutput): method no implement",sensid,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(sensid, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askOutput): object not exist",sensid,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askOutput): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askOutput): CORBA::SystemException" << endl;
	}	
	rcache.erase(sensid, node);	
	throw UniSetTypes::TimeOut(set_err("UI(askOutput): Timeout",sensid,node));
}

// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::askOutput( ObjectId name, UniversalIO::UIOCommand cmd, UniSetTypes::ObjectId backid )
{
	askRemoteOutput(name, cmd, uconf->getLocalNode(), backid);
}

// ------------------------------------------------------------------------------------------------------------

/*!
 * \param timerid - идентификатор таймера
 * \param timeMS - интервал (0 -  означает отказ)
 * \param ticks - количество уведомлений (0 - постоянно)
 * \param backid - обратный адрес (идентификатор заказчика)
*/
void UniversalInterface::askTimer( UniSetTypes::TimerId timerid, CORBA::Long timeMS, CORBA::Short ticks, 
									UniSetTypes::Message::Priority priority, UniSetTypes::ObjectId backid) 
								throw(IO_THROW_EXCEPTIONS)
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw UniSetTypes::IOBadParam("UI(askTimer): unknown back ID");

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
		// unideb[Debug::WARN] << "UI(askTimer): таймер с таким id уже заказан " << endl;
		return;
	}
	catch(TimerService_i::LimitTimers& ex )
	{
		ostringstream err;
		err << "UI(askTimer): Превышено максимальное количество заказчиков " << ex.maxTimers;
//		unideb[Debug::WARN] <<  err.str() << endl;
		throw Exception(err.str());
	}
	catch(TimerService_i::TimeMSLowLimit& ex )
	{
		ostringstream err;
		err << "UI(askTimer): Временной интервал меньше разрешённого " << ex.lowLimitMS  << " [мс]";
//		unideb[Debug::WARN] <<  err.str() << endl;
		throw OutOfRange(err.str());
	}
	catch(ORepFailed)
	{
		rcache.erase(uconf->getTimerService(), uconf->getLocalNode());
		throw UniSetTypes::IOBadParam(set_err("UI(askTimer): resolve failed ",uconf->getTimerService(), uconf->getLocalNode()));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(uconf->getTimerService(), uconf->getLocalNode());
		throw UniSetTypes::IOBadParam(set_err("UI(askTimer): method no implement",uconf->getTimerService(), uconf->getLocalNode()));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(uconf->getTimerService(), uconf->getLocalNode());
		throw UniSetTypes::IOBadParam(set_err("UI(askTimer): object not exist",uconf->getTimerService(), uconf->getLocalNode()));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askTimer): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askTimer): CORBA::SystemException" << endl;
	}	
	// unideb[Debug::WARN] << "UI(askTimer): catch...." << endl;
	rcache.erase( uconf->getTimerService(), uconf->getLocalNode());	
	throw UniSetTypes::TimeOut(set_err("UI(askTimer): Timeout",uconf->getTimerService(),uconf->getLocalNode()));
}
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param mid - код заказываемого сообщения
 * \param cmd - команда см. \ref UniversalIO::UIOCommand
 * \param ask - присылать уведомление о подтверждении
 * \param backid - обратный адрес (идентификатор заказчика)
*/
void UniversalInterface::askMessage( UniSetTypes::MessageCode mid, UniversalIO::UIOCommand cmd, bool ack, 
										UniSetTypes::ObjectId backid ) throw(IO_THROW_EXCEPTIONS)
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw UniSetTypes::IOBadParam("UI(askMessage): unknown back ID");

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
		err << "UI(askMessage): неизвестный код сообщения " << ex.bad_code;
//		unideb[Debug::WARN] <<  err.str() << endl;
		throw UniSetTypes::NameNotFound(err.str());
	}
	catch(ORepFailed)
	{
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(askMessage): resolve failed ",uconf->getInfoServer(), uconf->getLocalNode()));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
		throw UniSetTypes::IOBadParam(set_err("UI(askMessage): method no implement",uconf->getInfoServer(), uconf->getLocalNode()));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
		throw UniSetTypes::IOBadParam(set_err("UI(askMessage): object not exist",uconf->getInfoServer(), uconf->getLocalNode()));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askTimer): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askTimer): CORBA::SystemException" << endl;
	}	

	rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
	throw UniSetTypes::TimeOut(set_err("UI(askMessage): Timeout",uconf->getInfoServer(), uconf->getLocalNode()));
}
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param from - код начального сообщения
 * \param to - код конечного сообщения
 * \param cmd - команда см. \ref UniversalIO::UIOCommand
 * \param ask - присылать уведомление о подтверждении
 * \param backid - обратный адрес (идентификатор заказчика)
*/
void UniversalInterface::askMessageRange( UniSetTypes::MessageCode from, UniSetTypes::MessageCode to,
			UniversalIO::UIOCommand cmd, bool ack, UniSetTypes::ObjectId backid ) throw(IO_THROW_EXCEPTIONS)
{
	if( backid==UniSetTypes::DefaultObjectId )
		backid = myid;
		
	if( backid==UniSetTypes::DefaultObjectId )
		throw UniSetTypes::IOBadParam("Ul(askMessageRange): unknown back ID");

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
		err << "UI(askMessage): неизвестный код сообщения " << ex.bad_code;
//		unideb[Debug::WARN] <<  err.str() << endl;
		throw UniSetTypes::NameNotFound(err.str());
	}
	catch(InfoServer_i::MsgBadRange)
	{
//		unideb[Debug::WARN] << "UI(askMessageRange): неверно задан диапазон " << endl;
		throw OutOfRange("UI(askMessageRange): неверно задан диапазон");
	}
	catch(ORepFailed)
	{
		rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(askMessageRange): resolve failed ",uconf->getInfoServer(), uconf->getLocalNode()));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
		throw UniSetTypes::IOBadParam(set_err("UI(askMessageRange): method no implement",uconf->getInfoServer(), uconf->getLocalNode()));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
		throw UniSetTypes::IOBadParam(set_err("UI(askMessageRange): object not exist",uconf->getInfoServer(), uconf->getLocalNode()));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askTimer): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askTimer): CORBA::SystemException" << endl;
	}	
	// unideb[Debug::WARN] << "UI(askTimer): catch...." << endl;
	rcache.erase( uconf->getInfoServer(), uconf->getLocalNode());	
	throw UniSetTypes::TimeOut(set_err("UI(getMessageRange): Timeout",uconf->getInfoServer(), uconf->getLocalNode()));
}								
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param name - идентификатор объекта
 * \param node - идентификатор узла
*/
IOTypes UniversalInterface::getIOType(ObjectId name, ObjectId node)
	throw(IO_THROW_EXCEPTIONS)	
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(getIOType): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(getIOType): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam("UI(getIOType): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(getIOType): resolve failed ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(getIOType): method no implement",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(getIOType): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getIOType): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getIOType): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);	
	throw UniSetTypes::TimeOut(set_err("UI(getIOType): Timeout",name, node));
}

IOTypes UniversalInterface::getIOType(ObjectId name)
{
	return getIOType(name, uconf->getLocalNode() );
}
// ------------------------------------------------------------------------------------------------------------
/*!
 * \param name - идентификатор объекта
 * \param node - идентификатор узла
*/
ObjectType UniversalInterface::getType(ObjectId name, ObjectId node)
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(getType): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(getType): resolve failed ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(getType): method no implement",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(getType): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getType): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getType): CORBA::SystemException" << endl;
	}	
	catch(UniSetTypes::TimeOut){}
	
	rcache.erase(name, node);	
	throw UniSetTypes::TimeOut(set_err("UI(getType): Timeout",name, node));
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
	// если влючён режим использования локальных файлов
	// то пишем IOR в файл
	if( uconf->isLocalIOR() )
	{
		if( CORBA::is_nil(orb) )
			orb = uconf->getORB();

		uconf->iorfile.setIOR(id,node,orb->object_to_string(oRef));
		return;
	}

	try
	{
		rep.registration(oind->getNameById(id, node),oRef,(bool)force);
	}
	catch(Exception& ex )
	{
		throw;
	}
}

// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::unregister(UniSetTypes::ObjectId id, UniSetTypes::ObjectId node)throw(ORepFailed)
{
	if( uconf->isLocalIOR() )
	{
		uconf->iorfile.unlinkIOR(id,node);
		return;
	}

	try
	{
		rep.unregistration(oind->getNameById(id,node));
	}
	catch(Exception& ex )
	{
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
		throw ResolveNameError("UI(resolve): ID=UniSetTypes::DefaultObjectId");

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
				rcache.cache(rid, node, nso); // заносим в кэш 
				return nso._retn();
			}
			else
			{
				// если NameService недоступен то,
				// сразу выдаём ошибку
//				if( CORBA::is_nil(localctx) )
//				{
					if( unideb.debugging(Debug::WARN) )
					{
						unideb[Debug::WARN] << "не найден IOR-файл для " << uconf->oind->getNameById(rid,node) << endl;
					}
					throw UniSetTypes::ResolveNameError();
//				}
				// иначе пытаемся получить ссылку через NameService (omniNames)
//				unideb[Debug::WARN] << "не найден IOR-файл для " << uconf->oind->getNameById(rid,node) 
//									<< " пытаемся получить доступ через NameService \n";

			}
		}
		
	
		if( node!=uconf->getLocalNode() )
		{
			// Получаем доступ к NameService на данном узле
			string nodeName( oind->getRealNodeName(node) );
			string bname(nodeName); // сохраняем базовое название
			for(unsigned int curNet=1; curNet<=uconf->getCountOfNet(); curNet++)
			{
				try
				{
//					// unideb[Debug::INFO] << "пытаемся связаться с "<< node << endl;
					if( CORBA::is_nil(orb) )
						orb = uconf->getORB();

					ctx = ORepHelpers::getRootNamingContext( orb, nodeName.c_str() );
//					// unideb[Debug::INFO] << "ok. "<< endl;
					break;
				}
//				catch(CORBA::COMM_FAILURE& ex )
				catch(ORepFailed& ex)
				{
					// нет связи с этим узлом
					// пробуем связатся по другой сети
					// ПО ПРАВИЛАМ узел в другой должен иметь имя NodeName1...NodeNameX
					ostringstream s;
					s << bname << curNet;
					nodeName=s.str();
				}	
			}
			
			if( CORBA::is_nil(ctx) )
			{
				// unideb[Debug::WARN] << "NameService недоступен на узле "<< node << endl;
				throw NSResolveError();
			}
		}
		else
		{
			if( CORBA::is_nil(localctx) )
			{
				if( CORBA::is_nil(orb) )
				{
					CORBA::ORB_var _orb = uconf->getORB();
					localctx = ORepHelpers::getRootNamingContext( _orb, oind->getRealNodeName(uconf->getLocalNode()) );
				}
				else
					localctx = ORepHelpers::getRootNamingContext( orb, oind->getRealNodeName(uconf->getLocalNode()) );
			}
			else
				ctx = localctx;
		}

		CosNaming::Name_var oname = omniURI::stringToName( oind->getNameById(rid,node).c_str() );
		for (unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try		
			{
				CORBA::Object_var nso = ctx->resolve(oname);
				if( CORBA::is_nil(nso) )
					throw UniSetTypes::ResolveNameError();

				// Для var
				rcache.cache(rid, node, nso); // заносим в кэш 
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
		// ошибка системы коммуникации
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
		throw ORepFailed("UI(send): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::IOBadParam(set_err("UI(send): resolve failed ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam(set_err("UI(send): method no implement",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam(set_err("UI(send): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(send): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(send): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(send): Timeout",name, node));
}

void UniversalInterface::send( ObjectId name, TransportMessage& msg )
{
	send(name, msg, uconf->getLocalNode());
}

// ------------------------------------------------------------------------------------------------------------
/*!
 * \param id - идентификатор датчика
 * \param node - идентификатор узла
 * \param type - тип датчика 
 * \param value - значение которое необходимо установить
*/
bool UniversalInterface::saveValue(ObjectId name, long value, IOTypes type, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(saveValue): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(saveValue): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam("UI(saveValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(saveValue): resolve failed ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(saveValue): method no implement",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(saveValue): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(saveValue): CORBA::SystemException" << endl;
	}	
	
	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(saveValue): Timeout",name, node));
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
		unideb[Debug::WARN] << "UI(fastSaveValue): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId" << endl;
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
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(si.id, si.node);
		throw UniSetTypes::IOBadParam("UI(saveValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSaveValue): resolve failed ",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSaveValue): method no implement",si.id,si.node) << endl;
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id,si.node);
		unideb[Debug::WARN] << set_err("UI(fastSaveValue): object not exist",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(saveValue): CORBA::SystemException" << endl;
	}	
	catch(...){}
	
	rcache.erase(si.id,si.node);
	unideb[Debug::WARN] << set_err("UI(saveValue): Timeout",si.id, si.node) << endl;
}
// ------------------------------------------------------------------------------------------------------------

/*!
 * \param id - идентификатор датчика
 * \param state - состояние в которое его необходимо перевести
 * \param type - тип датчика
 * \param node - идентификатор узла
*/
bool UniversalInterface::saveState(ObjectId name, bool state, IOTypes type, ObjectId node) 
	throw(IO_THROW_EXCEPTIONS)
{
	if ( name == DefaultObjectId )
		throw ORepFailed("UI(saveState): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(saveState): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex )
	{
		rcache.erase(name, node);
		throw UniSetTypes::IOBadParam("UI(saveState): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(saveState): resolve failed ",name,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(saveState): method no implement",name,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(name, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(saveState): object not exist",name,node));
	}	
	catch(CORBA::COMM_FAILURE)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(saveState): CORBA::COMM_FAILURE " << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(saveState): CORBA::SystemException" << endl;
	}	

	rcache.erase(name, node);
	throw UniSetTypes::TimeOut(set_err("UI(saveState): Timeout",name, node));
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
		unideb[Debug::WARN] << "UI(fastSaveState): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId" << endl;
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
	catch(IOController_i::IOBadParam& ex )
	{
		rcache.erase(si.id, si.node);
		throw UniSetTypes::IOBadParam("UI(fastSaveState): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << set_err("UI(fastSaveState): resolve failed ",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);		
		unideb[Debug::WARN] << set_err("UI(fastSaveState): method no implement",si.id,si.node) << endl;
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << set_err("UI(fastSaveState): object not exist",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(saveState): CORBA::COMM_FAILURE " << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(saveState): CORBA::SystemException" << endl;
	}	
	catch(...){}
	
	rcache.erase(si.id, si.node);
	unideb[Debug::WARN] << set_err("UI(fastSaveState): Timeout",si.id, si.node) << endl;
}

// ------------------------------------------------------------------------------------------------------------
IOController_i::ShortIOInfo UniversalInterface::getChangedTime( UniSetTypes::ObjectId id, UniSetTypes::ObjectId node )
{
	if( id == DefaultObjectId )
		throw ORepFailed("UI(getChangedTime): Unknown id=UniSetTypes::DefaultObjectId");

	IOController_i::SensorInfo si;
	si.id = id;
	si.node = node;

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
				return iom->getChangedTime(si);
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
		unideb[Debug::WARN] << "UI(getChangedTime): " << ex.err << endl;
	}
	catch(IOController_i::IOBadParam& ex )
	{
		rcache.erase(si.id, si.node);
		throw UniSetTypes::IOBadParam("UI(getChangedTime): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << set_err("UI(getChangedTime): resolve failed ",si.id,si.node) << endl;
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);		
		unideb[Debug::WARN] << set_err("UI(getChangedTime): method no implement",si.id,si.node) << endl;
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);
		unideb[Debug::WARN] << set_err("UI(getChangedTime): object not exist",si.id,si.node) << endl;
	}	
	catch(CORBA::COMM_FAILURE)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(saveState): CORBA::COMM_FAILURE " << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(saveState): CORBA::SystemException" << endl;
	}	
	catch(...){}
	
	rcache.erase(si.id, si.node);
	throw UniSetTypes::TimeOut(set_err("UI(getChangedTime): Timeout",si.id, si.node));
}
// ------------------------------------------------------------------------------------------------------------

ObjectPtr UniversalInterface::CacheOfResolve::resolve( ObjectId id, ObjectId node )
	throw(NameNotFound)
{
	UniSetTypes::uniset_mutex_lock l(cmutex,200);

//#warning Временно отключён кэш
//	throw UniSetTypes::NameNotFound();

	CacheMap::iterator it = mcache.find( key(id,node) );
	if( it == mcache.end() )
		throw UniSetTypes::NameNotFound();

	it->second.timestamp = time(NULL); // фиксируем время последнего обращения
	
	// т.к. функция возвращает указатель
	// и тот кто вызывает отвечает за освобождение памяти
	// то мы делаем _duplicate....
	
	if( !CORBA::is_nil(it->second.ptr) )
	    return CORBA::Object::_duplicate(it->second.ptr);

	throw UniSetTypes::NameNotFound();
}
// ------------------------------------------------------------------------------------------------------------
void UniversalInterface::CacheOfResolve::cache( ObjectId id, ObjectId node, ObjectVar ptr )
{
	UniSetTypes::uniset_mutex_lock l(cmutex,220);
//#warning Временно отключён кэш
//	return;

//	if( mcache.size() > MaxSize )
//	{
//		if( !clean() )
//			 unideb[Debug::CRIT] << "UI(resolve cache): не удалось уменьшить размер кэш-а!!!!"<< endl;
//	}

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
	UniSetTypes::uniset_mutex_lock l(cmutex,180);
//    return true;
    
    if( unideb.debugging(Debug::INFO) )
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
	UniSetTypes::uniset_mutex_lock l(cmutex,220);
//#warning Временно отключён кэш
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
		unideb[Debug::WARN] << "UI(info): не указан идентификатор атора" << endl;
		
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
		unideb[Debug::WARN] << "UI(alarm): не указан идентификатор атора" << endl;
		
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
		if( uconf->isLocalIOR() )
		{
			if( CORBA::is_nil(orb) )
				orb = uconf->getORB();

			string sior(uconf->iorfile.getIOR(id,uconf->getLocalNode()));
			if( !sior.empty() )
			{
				CORBA::Object_var oref = orb->string_to_object(sior.c_str());
				return rep.isExist( oref );
			}
			
			return false;
		}
	
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
		throw UniSetTypes::IOBadParam("UI(askRemoteThreshold): unknown back ID");

	if ( sid == DefaultObjectId )
		throw ORepFailed("UI(askRemoteThreshold): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(askThreshold): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(sid, node);
		throw UniSetTypes::IOBadParam("UI(askThreshold): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(sid, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askThreshold): resolve failed ",sid,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(sid, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askThreshold): method no implement",sid,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(sid, node);		
		throw UniSetTypes::IOBadParam(set_err("UI(askThreshold): object not exist",sid,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askThreshold): ошибка системы коммуникации" << endl;
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(askThreshold): CORBA::SystemException" << endl;
	}	
	rcache.erase(sid, node);	
	throw UniSetTypes::TimeOut(set_err("UI(askThreshold): Timeout",sid,node));

}
// --------------------------------------------------------------------------------------------
CORBA::Long UniversalInterface::getRawValue( const IOController_i::SensorInfo& si )
{
	if ( si.id == DefaultObjectId )
		throw ORepFailed("UI(getRawValue): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(getRawValue): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(si.id, si.node);
		throw UniSetTypes::IOBadParam("UI(getRawValue): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);		
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(getRawValue): resolve failed ",si.id,si.node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);		
		throw UniSetTypes::IOBadParam(set_err("UI(getRawValue): method no implement",si.id,si.node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);		
		throw UniSetTypes::IOBadParam(set_err("UI(getRawValue): object not exist",si.id,si.node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(si.id, si.node);		
	throw UniSetTypes::TimeOut(set_err("UI(getRawValue): Timeout",si.id,si.node));
}
// --------------------------------------------------------------------------------------------
void UniversalInterface::calibrate(const IOController_i::SensorInfo& si, 
								   const IOController_i::CalibrateInfo& ci,
								   UniSetTypes::ObjectId admId )
{
	if( admId==UniSetTypes::DefaultObjectId )
		admId = myid;
		
//	if( admId==UniSetTypes::DefaultObjectId )
//		throw UniSetTypes::IOBadParam("UI(askTreshold): неизвестен ID администратора");

	if ( si.id == DefaultObjectId )
		throw ORepFailed("UI(calibrate): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(calibrate): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(si.id, si.node);
		throw UniSetTypes::IOBadParam("UI(calibrate): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);		
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(calibrate): resolve failed ",si.id,si.node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);		
		throw UniSetTypes::IOBadParam(set_err("UI(calibrate): method no implement",si.id,si.node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);		
		throw UniSetTypes::IOBadParam(set_err("UI(calibrate): object not exist",si.id,si.node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(si.id, si.node);		
	throw UniSetTypes::TimeOut(set_err("UI(calibrate): Timeout",si.id,si.node));
}			   
// --------------------------------------------------------------------------------------------
IOController_i::CalibrateInfo UniversalInterface::getCalibrateInfo( const IOController_i::SensorInfo& si )
{
	if ( si.id == DefaultObjectId )
		throw ORepFailed("UI(getCalibrateInfo): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(getCalibrateInfo): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(si.id, si.node);
		throw UniSetTypes::IOBadParam("UI(getCalibrateInfo): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(si.id, si.node);		
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(getCalibrateInfo): resolve failed ",si.id,si.node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(si.id, si.node);		
		throw UniSetTypes::IOBadParam(set_err("UI(getCalibrateInfo): method no implement",si.id,si.node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(si.id, si.node);		
		throw UniSetTypes::IOBadParam(set_err("UI(getCalibrateInfo): object not exist",si.id,si.node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(si.id, si.node);		
	throw UniSetTypes::TimeOut(set_err("UI(getCalibrateInfo): Timeout",si.id,si.node));
}
// --------------------------------------------------------------------------------------------
IOController_i::ASensorInfoSeq_var UniversalInterface::getSensorSeq( UniSetTypes::IDList& lst )
{
	if( lst.size() == 0 )
		return IOController_i::ASensorInfoSeq_var();

	ObjectId sid = lst.getFirst();

	if ( sid == DefaultObjectId )
		throw ORepFailed("UI(getSensorSeq): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(getSensorSeq): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw UniSetTypes::IOBadParam("UI(getSensorSeq): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(sid,conf->getLocalNode());
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(getSensorSeq): resolve failed ",sid,conf->getLocalNode()));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw UniSetTypes::IOBadParam(set_err("UI(getSensorSeq): method no implement",sid,conf->getLocalNode()));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw UniSetTypes::IOBadParam(set_err("UI(getSensorSeq): object not exist",sid,conf->getLocalNode()));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(sid,conf->getLocalNode());
	throw UniSetTypes::TimeOut(set_err("UI(getSensorSeq): Timeout",sid,conf->getLocalNode()));

}
// --------------------------------------------------------------------------------------------
IDSeq_var UniversalInterface::setOutputSeq( const IOController_i::OutSeq& lst, UniSetTypes::ObjectId sup_id )
{
	if( lst.length() == 0 )
		return UniSetTypes::IDSeq_var();


	if ( lst[0].si.id == DefaultObjectId )
		throw ORepFailed("UI(setOutputSeq): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(setOutputSeq): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(lst[0].si.id,lst[0].si.node);
		throw UniSetTypes::IOBadParam("UI(setOutputSeq): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(lst[0].si.id,lst[0].si.node);
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(setOutputSeq): resolve failed ",lst[0].si.id,lst[0].si.node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(lst[0].si.id,lst[0].si.node);
		throw UniSetTypes::IOBadParam(set_err("UI(setOutputSeq): method no implement",lst[0].si.id,lst[0].si.node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(lst[0].si.id,lst[0].si.node);
		throw UniSetTypes::IOBadParam(set_err("UI(setOutputSeq): object not exist",lst[0].si.id,lst[0].si.node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(lst[0].si.id,lst[0].si.node);
	throw UniSetTypes::TimeOut(set_err("UI(setOutputSeq): Timeout",lst[0].si.id,lst[0].si.node));
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
		throw UniSetTypes::IOBadParam("UI(askSensorSeq): unknown back ID");

	ObjectId sid = lst.getFirst();

	if ( sid == DefaultObjectId )
		throw ORepFailed("UI(askSensorSeq): попытка обратиться к объекту с id=UniSetTypes::DefaultObjectId");

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
		throw UniSetTypes::NameNotFound("UI(getSensorSeq): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw UniSetTypes::IOBadParam("UI(getSensorSeq): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(sid,conf->getLocalNode());
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(askSensorSeq): resolve failed ",sid,conf->getLocalNode()));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw UniSetTypes::IOBadParam(set_err("UI(askSensorSeq): method no implement",sid,conf->getLocalNode()));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(sid,conf->getLocalNode());
		throw UniSetTypes::IOBadParam(set_err("UI(askSensorSeq): object not exist",sid,conf->getLocalNode()));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(sid,conf->getLocalNode());
	throw UniSetTypes::TimeOut(set_err("UI(askSensorSeq): Timeout",sid,conf->getLocalNode()));
}
// -----------------------------------------------------------------------------
IOController_i::ShortMapSeq* UniversalInterface::getSensors( UniSetTypes::ObjectId id, UniSetTypes::ObjectId node )
{
	try
	{
		CORBA::Object_var oref;
		try
		{
			oref = rcache.resolve(id,node);
		}
		catch( NameNotFound ){}

		for( unsigned int i=0; i<uconf->getRepeatCount(); i++)
		{
			try
			{
				if( CORBA::is_nil(oref) )
					oref = resolve(id,node);

				IOController_i_var iom = IOController_i::_narrow(oref);
				return iom->getSensors();
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
		rcache.erase(id,node);
		throw UniSetTypes::NameNotFound("UI(getSensors): "+string(ex.err));
	}
	catch(IOController_i::IOBadParam& ex)
	{
		rcache.erase(id, node);
		throw UniSetTypes::IOBadParam("UI(getSensors): "+string(ex.err));
	}
	catch(ORepFailed)
	{
		rcache.erase(id,node);
		// не смогли получить ссылку на объект
		throw UniSetTypes::IOBadParam(set_err("UI(getSensors): resolve failed ",id,node));
	}	
	catch(CORBA::NO_IMPLEMENT)
	{
		rcache.erase(id,node);
		throw UniSetTypes::IOBadParam(set_err("UI(getSensors): method no implement",id,node));
	}	
	catch(CORBA::OBJECT_NOT_EXIST)
	{
		rcache.erase(id,node);
		throw UniSetTypes::IOBadParam(set_err("UI(getSensors): object not exist",id,node));
	}	
	catch(CORBA::COMM_FAILURE& ex)
	{
		// ошибка системы коммуникации
	}	
	catch(CORBA::SystemException& ex)
	{
		// ошибка системы коммуникации
		// unideb[Debug::WARN] << "UI(getValue): CORBA::SystemException" << endl;
	}	
	rcache.erase(id,node);
	throw UniSetTypes::TimeOut(set_err("UI(getSensors): Timeout",id,node));
}
// -----------------------------------------------------------------------------
bool UniversalInterface::waitReady( UniSetTypes::ObjectId id, int msec, int pmsec, ObjectId node )
{
	PassiveTimer ptReady(msec);
	bool ready = false;
	while( !ptReady.checkTime() && !ready )
	{
		try
		{
			ready = isExist(id,node);
			if( ready )
				break;
		}
		catch(...){}
	
		msleep(pmsec);
	}
	
	return ready;
}
// -----------------------------------------------------------------------------
bool UniversalInterface::waitWorking( UniSetTypes::ObjectId id, int msec, int pmsec, ObjectId node )
{
	PassiveTimer ptReady(msec);
	bool ready = false;

	while( !ptReady.checkTime() && !ready )
	{
		try
		{
			getState(id,node);
			ready = true;
			break;
		}
		catch(...){}
		msleep(pmsec);
	}

	return ready;

}
// -----------------------------------------------------------------------------
UniversalIO::IOTypes UniversalInterface::getConfIOType( UniSetTypes::ObjectId id )
{
	if( !conf )
		return UniversalIO::UnknownIOType;
		
	xmlNode* x = conf->getXMLObjectNode(id);
	if( !x )
		return UniversalIO::UnknownIOType;
	
	UniXML_iterator it(x);
	return UniSetTypes::getIOType( it.getProp("iotype") );
}
// -----------------------------------------------------------------------------
