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
 * \brief ������������ ����� ��� �������� ��������� ����������� �������� 
 * \author Pavel Vainerman
 * \version $Id: ObjectRepositoryFactory.h,v 1.8 2007/07/07 18:58:42 vpashka Exp $
 * \date  $Date: 2007/07/07 18:58:42 $
 */
// -------------------------------------------------------------------------- 
#ifndef ObjectRepositoryFactory_H_
#define ObjectRepositoryFactory_H_
// -------------------------------------------------------------------------- 
#include <omniORB4/CORBA.h>
#include <omniORB4/Naming.hh>
#include "Exceptions.h"
#include "ObjectRepository.h"

// -----------------------------------------------------------------------------------------
//namespase ORepositoryFacotry
//{
	
	/*!\class ObjectRepositoryFactory */ 
    class ObjectRepositoryFactory: private ObjectRepository
    {
    	public:
//		  	ObjectRepositoryFactory();
//			ObjectRepositoryFactory(int* argc=argc_ptr, char* **argv=argv_ptr); // ��������� ������������� ORB
			ObjectRepositoryFactory( UniSetTypes::Configuration* conf );
			~ObjectRepositoryFactory();
			
			//! �������� ������		
			bool createSection(const char* name, const char* in_section )throw(UniSetTypes::ORepFailed,UniSetTypes::InvalidObjectName);
			/*! \overload */
			bool createSection(const std::string& name, const std::string& in_section)throw(UniSetTypes::ORepFailed,UniSetTypes::InvalidObjectName);
			/*! �������� ������ �� ������� ����� */
			bool createSectionF(const std::string& fullName)throw(UniSetTypes::ORepFailed,UniSetTypes::InvalidObjectName);

			//! ������� �������� ������ � �������� '��������'
			bool createRootSection(const char* name);
			/*! \overload */
			bool createRootSection(const std::string& name);


			//! ������� �������� ������ 
			bool removeSection(const std::string& fullName, bool recursive=false);

			//! ������� �������������� ������ 
			bool renameSection(const std::string& newName, const std::string& fullName);
		/**
		    @addtogroup ORepServiceGroup 
	    	    @{
		*/
			
			/*! ������� ��������� �� ����� ������ ���� �������� ������������� � ������ ������ */
			void printSection(const std::string& fullName);
//			void printSection(CosNaming::NamingContext_ptr ctx);
		// @}
		// end of add to ORepServiceGroup 

		protected:

		private:
			/*! �������� ������ ���������(������) */
			bool createContext(const char *cname, CosNaming::NamingContext_ptr ctx);
    };
//};

#endif
