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
 *  \date   $Date: 2007/11/27 21:54:19 $
 *  \version $Id: ObjectRepository.cc,v 1.13 2007/11/27 21:54:19 vpashka Exp $
*/
// -------------------------------------------------------------------------- 
#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#include <string.h>
#include <sstream>
#include "ObjectRepository.h"
#include "ORepHelpers.h"
#include "UniSetObject_i.hh"
#include "Debug.h"
//#include "Configuration.h"
// --------------------------------------------------------------------------
using namespace omni;
using namespace UniSetTypes;
using namespace std;
// --------------------------------------------------------------------------
/*
ObjectRepository::ObjectRepository(int* argc, char* **argv, const char* NSName):
	argc(*argc), 
	argv(*argv),
	nsName(NSName)
{
}
*/

ObjectRepository::ObjectRepository( Configuration* _conf ):
	nsName(_conf->getNSName()),
	uconf(_conf)
{
	init();
}


ObjectRepository::~ObjectRepository()
{
}

ObjectRepository::ObjectRepository():
nsName("NameService")
{
	init();
}

bool ObjectRepository::init()
{
	try
	{
		CORBA::ORB_var orb = uconf->getORB();
		localctx = ORepHelpers::getRootNamingContext(orb, nsName );
		if( CORBA::is_nil(localctx) )
			localctx=0;
	}
	catch(...)
	{
		localctx=0;
		return false;
	}
	
	return true;
}
// --------------------------------------------------------------------------
/*!
 *  ������:  registration("sens1", oRef, "Root/SensorSection");
 * \param name - ��� ��������������� �������
 * \param oRef - ������ �� ������ 
 * \param section - ��� ������ � ������� ��������� ��������������� ������
 * \exception ORepFailed - ������������ ���� ��������� ������ ��� �����������
 * \sa registration(const string fullName, const CORBA::Object_ptr oRef) 
*/
void ObjectRepository::registration(const string& name, const ObjectPtr oRef, const string& section, bool force)
        throw(ORepFailed, ObjectNameAlready, InvalidObjectName, NameNotFound)
{
	ostringstream err;

	if( unideb.debugging(Debug::type(Debug::INFO|Debug::REPOSITORY)) )
		unideb[Debug::type(Debug::INFO|Debug::REPOSITORY)] << "ObjectRepository(registration): ������������ " << name << endl;

	// �������� ������������ �����
	char bad = ORepHelpers::checkBadSymbols(name);
	if( bad != 0 )
	{
		cerr << "orep reg: BAD Symbols" << endl;
		err << "ObjectRepository(registration): (InvalidObjectName) " << name;
		err << " �������� ������������ ������ " << bad;
		throw ( InvalidObjectName(err.str().c_str()) );
	}
	
	CosNaming::Name_var oName = omniURI::stringToName(name.c_str());
	CosNaming::NamingContext_var ctx;
	for( int i=0; i<2; i++ )
	{
	    try
    	{
	        // ��������� � ����������� ����� ������ (������� ���� ���� ������)
	
			CORBA::ORB_var orb = uconf->getORB();
			ctx = ORepHelpers::getContext(orb, section, nsName);    
		
			ctx->bind(oName, oRef);
			return;
		}
		catch(const CosNaming::NamingContext::AlreadyBound &nf)
		{
			if( unideb.debugging(Debug::type(Debug::WARN|Debug::REPOSITORY)) )
				unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "(registration): "<< name <<" ��� ��������������� � "<< section << "!!!" << endl;

			if( !force )
				throw ObjectNameAlready();
				
			// ��������������, ����� ��������� ��������
			ctx->unbind(oName);
			continue;
/*
			// ������ �������� ��� �� ������
			bool life(false);
			try
			{
				unideb[Debug::type(Debug::INFO|Debug::REPOSITORY)] << "(registration): " << name << " ��� ����... ��������� ����� ��? "<< endl;
				CORBA::Object_var ref = ctx->resolve(oName);	
				UniSetObject_i_var uobj = UniSetObject_i::_narrow(ref);
				uobj->getId(); // �������� ������� ������� (�����), ���� �� ������ ��������� ����������
				life = true;
				unideb[Debug::type(Debug::INFO|Debug::REPOSITORY)] << "(registration): " << name << " �����! "<< endl;
			}
			catch(...)
			{ 
				life=false;
			}
	
			if( !life )
			{
				unideb(Debug::type(Debug::WARN|Debug::REPOSITORY)) << "(registration): " << name << " ���� � �����������, �� ����������. �������� �� ����� ������ "<< endl;
				ctx->rebind(oName, oRef);
			}
			else
			{
				unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "(registration): "<< name <<" ��� ��������������� � "<< section<< "!!!!!!!!!!!" << endl;
				throw ObjectNameAlready();
			}
			return;
*/
		}
	    catch(ORepFailed)
	    {
	  		string er("ObjectRepository(registrartion): (getContext) �� ���� ���������������� "+name);
				throw ORepFailed(er.c_str());
	    }
		catch(CosNaming::NamingContext::NotFound)
	    {
			throw NameNotFound();
	    }
	    catch(const CosNaming::NamingContext::InvalidName &nf)
	    {
			err << "ObjectRepository(registration): (InvalidName) �� ���� ���������������� ������  " << name;;
	    }
		catch(const CosNaming::NamingContext::CannotProceed &cp)
		{
				err << "ObjectRepository(registrartion): catch CannotProced " << name << " bad part=";
			err << omniURI::nameToString(cp.rest_of_name);
		}
	    catch(CORBA::SystemException& ex)
	    {
			unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "ObjectRepository(registrartion): ������� CORBA::SystemException: "
					<< ex.NP_minorString() << endl;
	
			err << "ObjectRepository(registrartion): ������� CORBA::SystemException: " << ex.NP_minorString();
	    }
	//	catch(...)
	//	{
	//		unideb[Debug::WARN] << "������� ���-�� �����������..."<< endl;
	//	}
	}
	
	throw ORepFailed(err.str().c_str());
}
// --------------------------------------------------------------------------

/*!
 *  ������� ������������ ������ � ������ "fullName" � ����������� �������� � ��������� ��� ��� �� ������ "oRef".
 *  \note ��� ���� ���� ����� �����, ��� �������� ������ ��� �������.
 *	������: registration("Root/SensorSection/sens1", oRef);
 *	\param fullName - ������ ��� ��������������� ������� (�.�. ���������� � ���� ��� ������)
 *	\param oRef - ������ �� ������ 
 *  \exception ORepFailed - ������������ ���� ��������� ������ ��� �����������
 *  \sa registration(const string name, const ObjectPtr oRef, const string section)
*/
void ObjectRepository::registration( const std::string& fullName, const UniSetTypes::ObjectPtr oRef, bool force )
	throw(ORepFailed,ObjectNameAlready,InvalidObjectName, NameNotFound)
{
//	string n(ORepHelpers::getShortName(fullName));
	string n( uconf->oind->getBaseName(fullName) );
    string s(ORepHelpers::getSectionName(fullName.c_str()));
    registration(n, oRef, s,force);
}
// --------------------------------------------------------------------------

/*!
 *	\param name - ��� ��������������� ������� (�.�. ���������� � ���� ��� ������)
 *	\param section - ��� ������ � ������� ��������������� ������
 *  \exception ORepFailed - ������������ ���� ��������� ������ ��� ��������
 * 	\warning ��� �������� ������������ ���������� �����. �.�.
 *	�������� ��, �� �� �������� �� ��� ������� �� ������ ��� ��������
 *	�.�. ��� �������� ������ �� �������� ����� �������� ���������...
*/
void ObjectRepository::unregistration(const string& name, const string& section)
	throw(ORepFailed, NameNotFound)
{
//	unideb[Debug::INFO] << "OREP: unregistration "<< name << " �� "<< section << endl;
	ostringstream err;
    CosNaming::Name_var oName = omniURI::stringToName(name.c_str());
	
//	unideb[Debug::INFO] << "OREP: string to name ok"<< endl;
	CosNaming::NamingContext_var ctx;
	CORBA::ORB_var orb = uconf->getORB();	
	ctx = ORepHelpers::getContext(orb, section, nsName);    

//	unideb[Debug::INFO] << "OREP: get context " << section <<" ok"<< endl;

    try
    {
	    // ������� ������ �� �������
		ctx->unbind(oName);
		
//		unideb[Debug::INFO] << "OREP: ok" << endl;
		return;
    }	
	catch(const CosNaming::NamingContext::NotFound &nf)
    {
		err << "ObjectRepository(unregistrartion): �� ������ ������ ->" << name;
    }
	catch(const CosNaming::NamingContext::InvalidName &in)
	{
		err << "ObjectRepository(unregistrartion): �� ���������� ��� ������� -> " << name;
	}
	catch(const CosNaming::NamingContext::CannotProceed &cp)
	{
		err << "ObjectRepository(unregistrartion): catch CannotProced " << name << " bad part=";
		err << omniURI::nameToString(cp.rest_of_name);
	}

	if (err.str().empty())
		err << "ObjectRepository(unregistrartion): �� ���� ������� " << name;
   	throw ORepFailed(err.str().c_str());
}
// --------------------------------------------------------------------------
/*!
 *	\param fullName - ������ ��� ��������������� ������� (�.�. ���������� � ���� ��� ������)
 *  \exception ORepFailed - ������������ ���� ��������� ������ ��� ��������
 * 	\sa unregistration(const string name, const string section)
*/
void ObjectRepository::unregistration(const string& fullName)
	throw(ORepFailed, NameNotFound)
{
//    string n(ORepHelpers::getShortName(fullName));
    string n(uconf->oind->getBaseName(fullName));
    string s(ORepHelpers::getSectionName(fullName));
    unregistration(n,s);
}
// --------------------------------------------------------------------------

ObjectPtr ObjectRepository::resolve( const string& name, const string NSName )
	throw(ORepFailed,  NameNotFound)
{
	ostringstream err;
	try
	{
		if( !localctx && !init() )
			throw ORepFailed("ObjectRepository(resolve): �� ���� �������� ������ �� NameServices");

		CORBA::Object_var oRef;
		CosNaming::Name_var nc = omniURI::stringToName(name.c_str());
		oRef=localctx->resolve(nc);
		if ( !CORBA::is_nil(oRef) )
			return oRef._retn();					

		err << "ObjectRepository(resolve): �� ���� �������� ������ �� ������ " << name.c_str();
	}
	catch(const CosNaming::NamingContext::NotFound &nf)
	{
		err << "ObjectRepository(resolve): NameNotFound name= " << name;
	}
	catch(const CosNaming::NamingContext::InvalidName &nf)
	{
		err << "ObjectRepository(resolve): �� ���� �������� ������ �� ��������(InvalidName) ";
	}
	catch(const CosNaming::NamingContext::CannotProceed& cp)
	{
		err << "ObjectRepository(resolve): catch CannotProced " << name << " bad part=";
		err << omniURI::nameToString(cp.rest_of_name);
	}
    catch(CORBA::SystemException& ex)
    {
		err << "ObjectRepository(resolve): catch SystemException: " << ex.NP_minorString()
			<< " ��� " << name;
	}
	catch(...)
	{
		err << "ObjectRepository(resolve): catch ... ��� " << name;
	}

	if(err.str().empty())
		err << "ObjectRepository(resolve): ??? ��� " << name;			
	
	throw ORepFailed(err.str().c_str());
}

// --------------------------------------------------------------------------

/*!
 * \param ls - ��������� �� ������ ������� ���� ���������
 * \param how_many - ������������ ���������� ��������� ���������
 * \param section - ������ ��� ������ ������� � Root. 
 * \return ������� ���������� true, ���� � ������ ���� ������� �� ��� ��������. �.�. ��������������
 * ���������� �������� � ���� ������ ��������� �������� how_many.
 * \exception ORepFailed - ������������ ���� ��������� ��� ��������� ������� � ������
*/ 
bool ObjectRepository::list(const string& section, ListObjectName *ls, unsigned int how_many)throw(ORepFailed)
{
	return list(section, ls, how_many, ObjectRef);
}

// --------------------------------------------------------------------------
/*!
 * \param ls - ��������� �� ������ ������� ���� ���������
 * \param how_many - ������������ ���������� ��������� ���������
 * \param in_section - ������ ��� ������ ������� � Root. 
 * \return ������� ���������� true, ���� � ������ ���� ������� �� ��� ��������. �.�. ��������������
 * ���������� �������� � ���� ������ ��������� �������� how_many.
 * \exception ORepFailed - ������������ ���� ��������� ��� ��������� ������� � ������
*/ 
bool ObjectRepository::listSections(const string& in_section, ListObjectName *ls, unsigned int how_many)throw(ORepFailed)
{
	return list(in_section, ls, how_many, Section);
}

// --------------------------------------------------------------------------
/*!
 * \param ls - ��������� �� ������ ������� ���� ���������
 * \param how_many - ������������ ���������� ��������� ���������
 * \param in_section - ������ ��� ������ ������� � Root. 
 * \param type - ��� ����������(��������� � ������) ��������.
 * \return ������� ���������� true, ���� � ������ ���� ������� �� ��� ��������. �.�. ��������������
 * ���������� �������� � ���� ������ ��������� �������� how_many.
 * \exception ORepFailed - ������������ ���� ��������� ��� ��������� ������� � ������
*/ 
bool ObjectRepository::list(const string& section, ListObjectName *ls, unsigned int how_many, ObjectType type)
{
  // ���������� false ���� ����� �� ���� ������...
//  	unideb[Debug::INFO] << "�������� ������ �� "<< section << endl;
	CosNaming::NamingContext_var ctx;
	try
	{
		CORBA::ORB_var orb = uconf->getORB();
		ctx = ORepHelpers::getContext(orb, section, nsName);    
	}
	catch(ORepFailed)
	{
		unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "ORepository(list): �� ���� �������� ������ �� "<< section << endl;
		throw;
//		return false;
	}
	
	if( CORBA::is_nil(ctx) )
	{
		unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "ORepository(list): �� ���� �������� ������ �� "<< section << endl;
		throw ORepFailed();
	}
	
	CosNaming::BindingList_var bl;
	CosNaming::BindingIterator_var bi;
	ctx->list(how_many,bl,bi);

	bool res = true;

	if(how_many>=bl->length())
		how_many = bl->length();
	else
	{
		if ( bi!=NULL )
			res = false;
	}

//	cout << "�������� ������ "<< section << " �������� " << bl->length()<< endl;

	for( unsigned int i=0; i<how_many;i++)
	{
		switch( type )
		{ 
			case ObjectRef:
			{
				if(bl[i].binding_type == CosNaming::nobject)
			  	{
					string objn= omniURI::nameToString(bl[i].binding_name);
					ls->push_front(objn);
				}
				break;
			}
			case Section:
			{
				if( bl[i].binding_type == CosNaming::ncontext)
			  	{
					string objn= omniURI::nameToString(bl[i].binding_name);
					ls->push_front(objn);
				}
				break;
			}
		}
	}

	return res;
}

// --------------------------------------------------------------------------
bool ObjectRepository::isExist( const string& fullName )
{
	try
	{
		CORBA::Object_var oRef = resolve(fullName, nsName);	
		return isExist(oRef);
	}
	catch(...){}

	return false;
}

// --------------------------------------------------------------------------

bool ObjectRepository::isExist( ObjectPtr oref )
{
	try
	{
		UniSetObject_i_var o = UniSetObject_i::_narrow(oref);
		return o->exist();
	}
	catch(CORBA::TRANSIENT){}
	catch(CORBA::SystemException&){}
    catch(CORBA::Exception&){}
    catch(omniORB::fatalException& fe)
    {
		unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "ObjectRepository(isExist): "<< "������� omniORB::fatalException:" << endl;
        unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "  file: " << fe.file() << endl;
		unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "  line: " << fe.line() << endl;
        unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "  mesg: " << fe.errmsg() << endl;
    }
	catch(...){}

	return false;
}

// --------------------------------------------------------------------------
