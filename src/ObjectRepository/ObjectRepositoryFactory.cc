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
 *  \date   $Date: 2007/11/18 19:13:35 $
 *  \version $Id: ObjectRepositoryFactory.cc,v 1.16 2007/11/18 19:13:35 vpashka Exp $
*/
// -------------------------------------------------------------------------- 
#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#include <string.h>
#include <sstream>
#include "ObjectRepositoryFactory.h"
#include "ORepHelpers.h"
#include "Configuration.h"
#include "Debug.h"

// ---------------------------------------------------------------------------------------------------------------
using namespace UniSetTypes;
using namespace std;
using namespace omni;
// ---------------------------------------------------------------------------------------------------------------
ObjectRepositoryFactory::ObjectRepositoryFactory( Configuration* _conf ):
ObjectRepository(_conf)
{
}

ObjectRepositoryFactory::~ObjectRepositoryFactory()
{
}

// -------------------------------------------------------------------------------------------------------
/*!
 * \param name - ��� ����������� ������
 * \param in_section - ������ ��� ������ ������ ������� ��������� �����
 * \param section - ������ ��� ������ ������� � Root. 
 * \exception ORepFailed - ������������ ���� ��������� ��� ��������� ������� � ������
*/
bool ObjectRepositoryFactory::createSection( const char* name, const char* in_section)throw(ORepFailed,InvalidObjectName )
{

	const string str(name);
	char bad = ORepHelpers::checkBadSymbols(str);
	if (bad != 0)
	{
		ostringstream err;
		err << "ObjectRepository(registration): (InvalidObjectName) " << str;
		err << " �������� ������������ ������ " << bad;
		throw ( InvalidObjectName(err.str().c_str()) );
	}

	unideb[Debug::REPOSITORY] << "CreateSection: name = "<< name << "  in section = " << in_section << endl;
	if( sizeof(in_section)==0 )
	{
		unideb[Debug::REPOSITORY] << "CreateSection: in_section=0"<< endl;
		return createRootSection(name);
	}
	
	int argc(uconf->getArgc());
	char **argv(uconf->getArgv());
	CosNaming::NamingContext_var ctx = ORepHelpers::getContext(in_section, argc, argv, uconf->getNSName() );    
	return createContext( name, ctx.in() );
}

bool ObjectRepositoryFactory::createSection(const string& name, const string& in_section)throw(ORepFailed,InvalidObjectName)
{
	return createSection(name.c_str(), in_section.c_str());
}

// -------------------------------------------------------------------------------------------------------
/*!
 * \param fullName - ������ ��� ����������� ������
 * \exception ORepFailed - ������������ ���� ��������� ��� ��������� ������� � ������
*/
bool ObjectRepositoryFactory::createSectionF(const string& fullName)throw(ORepFailed,InvalidObjectName)
{
	string name(ObjectIndex::getBaseName(fullName));
    string sec(ORepHelpers::getSectionName(fullName));

	unideb[Debug::REPOSITORY] << name << endl;
	unideb[Debug::REPOSITORY] << sec << endl;

	if ( sec.empty() )
	{
		unideb[Debug::REPOSITORY] << "SectionName is empty!!!"<< endl;
		unideb[Debug::REPOSITORY] << "��������� � "<< uconf->getRootSection() << endl;
		return createSection(name, uconf->getRootSection());
	}
	else
		return createSection(name, sec);
}

// ---------------------------------------------------------------------------------------------------------------
bool ObjectRepositoryFactory::createRootSection(const char* name)
{
	CORBA::ORB_var orb = uconf->getORB();
	CosNaming::NamingContext_var ctx = ORepHelpers::getRootNamingContext(orb, uconf->getNSName());
	return createContext(name,ctx);
}

bool ObjectRepositoryFactory::createRootSection(const string& name)
{
	return createRootSection( name.c_str() );
}

// -----------------------------------------------------------------------------------------------------------
bool ObjectRepositoryFactory::createContext(const char *cname, CosNaming::NamingContext_ptr ctx)
{
	
	CosNaming::Name_var nc = omniURI::stringToName(cname);
	try
	{
		unideb[Debug::REPOSITORY] << "ORepFactory(createContext): ������ ����� �������� "<< cname << endl;
		ctx->bind_new_context(nc);
		unideb[Debug::REPOSITORY] << "ORepFactory(createContext): ������. " << endl;
		return true;
	}
	catch(const CosNaming::NamingContext::AlreadyBound &ab)
	{
//		ctx->resolve(nc);
		unideb[Debug::REPOSITORY] <<"ORepFactory(createContext): context "<< cname << " ��� ����"<< endl;		
		return true;
	}
	catch(CosNaming::NamingContext::NotFound)
    {
		unideb[Debug::REPOSITORY] <<"ORepFactory(createContext): NotFound "<< cname << endl;		
		throw NameNotFound();
    }
    catch(const CosNaming::NamingContext::InvalidName &nf)
    {
		//	unideb[Debug::WARN] << "invalid name"<< endl;
		unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "ORepFactory(createContext): (InvalidName)  " << cname;
    }
	catch(const CosNaming::NamingContext::CannotProceed &cp)
	{
		unideb[Debug::type(Debug::WARN|Debug::REPOSITORY)] << "ORepFactory(createContext): catch CannotProced " 
				<< cname << " bad part="
				<< omniURI::nameToString(cp.rest_of_name);
		throw NameNotFound();
	}
	catch( CORBA::SystemException& ex)
	{
		unideb[Debug::type(Debug::CRIT|Debug::REPOSITORY)] << "ORepFactory(createContext): CORBA::SystemException: "
			<< ex.NP_minorString() << endl;
	}
	catch(CORBA::Exception&)
    {
		unideb[Debug::type(Debug::CRIT|Debug::REPOSITORY)] << "������� CORBA::Exception." << endl;
    }
    catch(omniORB::fatalException& fe)
    {
		unideb[Debug::type(Debug::CRIT|Debug::REPOSITORY)] << "������� omniORB::fatalException:" << endl;
        unideb[Debug::type(Debug::CRIT|Debug::REPOSITORY)] << "  file: " << fe.file() << endl;
		unideb[Debug::type(Debug::CRIT|Debug::REPOSITORY)] << "  line: " << fe.line() << endl;
        unideb[Debug::type(Debug::CRIT|Debug::REPOSITORY)] << "  mesg: " << fe.errmsg() << endl;
    }

	return false;
}

// -----------------------------------------------------------------------------------------------------------
/*!
    \note ������� �� ������� ������, ���� �� ������ �������� ������ � ������
*/
void ObjectRepositoryFactory::printSection(const string& fullName)
{
	ListObjectName ls;
	ListObjectName::const_iterator li;

	try
	{
		list(fullName.c_str(),&ls);

		if( ls.empty() )
			cout << fullName << " ����!!!"<< endl;
	}
	catch( ORepFailed )
	{
	    cout << "printSection: cath exceptions ORepFailed..."<< endl;
	    return ;
	}
	
	cout << fullName << "(" << ls.size() <<"):" << endl;

	for( li=ls.begin();li!=ls.end();++li )
	{
		string ob(*li);
		cout << ob << endl;
	}
	
}

// -----------------------------------------------------------------------------------------------------------
/*!
 * \param fullName - ��� ��������� ������
 * \param recursive - ������� ���������� ��� ������ ��� ���������� �� ������� � ������ ( �������� )
 * \warning ������� �������� ������ ������ 1000 ��������, ��������� ������������...
*/
bool ObjectRepositoryFactory::removeSection(const string& fullName, bool recursive)
{
//	string name = getName(fullName.c_str(),'/');
//  string sec = getSectionName(fullName.c_str(),'/');
//	CosNaming::NamingContext_var ctx = getContext(sec, argc, argv);    
	unsigned int how_many = 1000;
	CosNaming::NamingContext_var ctx;
	try
	{	
		int argc(uconf->getArgc());
		char **argv(uconf->getArgv());
		ctx = ORepHelpers::getContext(fullName, argc, argv, nsName);    
	}
	catch( ORepFailed )
	{
		return false;
	}

	CosNaming::BindingList_var bl;
	CosNaming::BindingIterator_var bi;

	ctx->list(how_many,bl,bi);


	if(how_many>bl->length())
		how_many = bl->length();

	bool rem = true; // ������� ��� ��� 
	
	for(unsigned int i=0; i<how_many;i++)
	{
	
		if(	bl[i].binding_type == CosNaming::nobject)
	  	{
//			cout <<"������� "<< omniURI::nameToString(bl[i].binding_name) << endl;
			ctx->unbind(bl[i].binding_name);
		}
		else if( bl[i].binding_type == CosNaming::ncontext)
		{
			
			if( recursive )
			{
				unideb[Debug::REPOSITORY] << "ORepFactory: ������� ����������..." << endl;
				string rctx = fullName+"/"+omniURI::nameToString(bl[i].binding_name);
				unideb[Debug::REPOSITORY] << rctx << endl;
				if ( !removeSection(rctx))
				{
					unideb[Debug::REPOSITORY] << "���������� ������� �� �������" << endl;
					rem = false;
				}
			}
			else
			{
				unideb[Debug::REPOSITORY] << "ORepFactory: " << omniURI::nameToString(bl[i].binding_name) << " - ��������!!! ";
				unideb[Debug::REPOSITORY] << "ORepFactory: ���� �� �������" << endl;
				rem = false;
			}
		}
	}


	// ������� ��������, ���� �� ��� ������
	if (rem)
	{
		// �������� ��� ��������� ����������� ���������
		string in_sec(ORepHelpers::getSectionName(fullName));
		//�������� ��� ���������� ���������
		string name(ObjectIndex::getBaseName(fullName));

		try
		{
			int argc(uconf->getArgc());
			char **argv(uconf->getArgv());
			CosNaming::Name_var ctxName = omniURI::stringToName(name.c_str());
			CosNaming::NamingContext_var in_ctx = ORepHelpers::getContext(in_sec, argc, argv, nsName);    
			ctx->destroy();
			in_ctx->unbind(ctxName);
		}
		catch(const CosNaming::NamingContext::NotEmpty &ne)
		{
			unideb[Debug::REPOSITORY] << "ORepFactory: ��������" << fullName << " �� ������ " << endl;
			rem = false;				
		}
		catch( ORepFailed )
		{
			unideb[Debug::REPOSITORY] << "ORepFactory: �� ������ �������� ������ �� �������� " << in_sec << endl;
			rem=false;
		}
	}

	
	bi->destroy(); // ??
	return rem;
}

// -----------------------------------------------------------------------------------------------------------
/*!
 * \param newFName - ������ ��� ����� ������
 * \param oldFName - ������ ��� ��������� ������
*/
bool ObjectRepositoryFactory::renameSection( const string& newFName, const string& oldFName )
{
	string newName(ObjectIndex::getBaseName(newFName));
	string oldName(ObjectIndex::getBaseName(oldFName));
	CosNaming::Name_var ctxNewName = omniURI::stringToName(newName.c_str());
	CosNaming::Name_var ctxOldName = omniURI::stringToName(oldName.c_str());

	string in_sec(ORepHelpers::getSectionName(newFName));

	try
	{	
//		unideb[Debug::REPOSITORY] << "ORepFactory: � ��������� "<< in_sec << endl; 
//		unideb[Debug::REPOSITORY] << "ORepFactory: ��������������� " << oldFName << " � " << newFName << endl;				
		int argc(uconf->getArgc());
		char **argv(uconf->getArgv());
		CosNaming::NamingContext_var in_ctx = ORepHelpers::getContext(in_sec, argc, argv, nsName);    
		CosNaming::NamingContext_var ctx = ORepHelpers::getContext(oldFName, argc, argv, nsName);    

		// ������� �������� newFName ���� �� �����������
		in_ctx->rebind_context(ctxNewName, ctx);
		in_ctx->unbind(ctxOldName);
	}
	catch( ORepFailed )
	{
		return false;
	}

	return true;
}

// -----------------------------------------------------------------------------------------------------------
