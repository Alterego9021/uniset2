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
 * \brief ������������ ����� ��� ������ � ������������ ��������  
 * \author Pavel Vainerman <pv>
 * \date $Date: 2007/11/18 19:13:35 $
 * \version $Id: ObjectRepository.h,v 1.9 2007/11/18 19:13:35 vpashka Exp $	 *
 */
// -------------------------------------------------------------------------- 
#ifndef ObjectRepository_H_
#define ObjectRepository_H_
// -------------------------------------------------------------------------- 
#include <omniORB4/CORBA.h>
#include <omniORB4/Naming.hh>
#include <string>
#include "UniSetTypes.h"
#include "Exceptions.h"
#include "Configuration.h"
// -----------------------------------------------------------------------------------------
//namespase ORepository
//{

	/*! \class ObjectRepository  
	 * \par
	 * ... � ����� ���� ���������� ��������... (���������� ����� �� 40!...)
	 *
	 *	\note ����������� �������� ������, � ��������� ������������ 
	 * \todo ��������� ������ ������� � �������� ����� N.
	*/ 
    class ObjectRepository
    {
    	public:
						
			ObjectRepository(UniSetTypes::Configuration* conf); 
			~ObjectRepository();
			
		/**
   		 @defgroup ORepGroup ������ ������� ����������� � ����������� ��������
	     @{ 	*/
			//! ������� ����������� ������� �� ����� � ��������� ������ 
			void registration(const std::string& name, const UniSetTypes::ObjectPtr oRef, const std::string& section, bool force=false)
					throw(UniSetTypes::ORepFailed, UniSetTypes::ObjectNameAlready, UniSetTypes::InvalidObjectName, UniSetTypes::NameNotFound);
	
			//! ������� ����������� ������� �� ������� �����.
			void registration(const std::string& fullName, const UniSetTypes::ObjectPtr oRef, bool force=false)
					throw(UniSetTypes::ORepFailed, UniSetTypes::ObjectNameAlready,UniSetTypes::InvalidObjectName, UniSetTypes::NameNotFound);

			//! �������� ������ �� ������� name � ������ section
			void unregistration(const std::string& name, const std::string& section)throw(UniSetTypes::ORepFailed, UniSetTypes::NameNotFound);
			//! �������� ������ �� ������� �� ������� �����
			void unregistration(const std::string& fullName)throw(UniSetTypes::ORepFailed, UniSetTypes::NameNotFound);
		// @} 
		// end of ORepGroup

		/*! ��������� ������ �� ��������� ������� ����� (���������������) */
		UniSetTypes::ObjectPtr resolve(const std::string& name, const std::string NSName="NameService")throw(UniSetTypes::ORepFailed, UniSetTypes::NameNotFound);

		// ������� �� �� ���������� �������, � � ����������������
//		void setListId( ListObjectId *lst );

		/*!  �������� ������������� � ����������� ������� */
		bool isExist( UniSetTypes::ObjectPtr oref );
		/*!  �������� ������������� � ����������� ������� */
		bool isExist( const std::string& fullName );


		/**
		 @defgroup ORepServiceGroup ������ ��������� ������� ����������� ��������
		 @{
		*/
		
		 /*! ��� �������  */
		 enum ObjectType{
		 					ObjectRef,  /*!< ������ �� ������  */
							Section		/*!< ��������� 	*/
						};

		//! ��������� ������ how_many �������� �� ������ section.
		bool list(const std::string& section, UniSetTypes::ListObjectName *ls, unsigned int how_many=300)throw(UniSetTypes::ORepFailed);
		
		//! �������� ������ how_many ��������� �� ������ in_section.
		bool listSections(const std::string& in_section, UniSetTypes::ListObjectName *ls, unsigned int how_many=300)throw(UniSetTypes::ORepFailed);
//		bool list_at(unsigned int start_pos, const char* section, ListObjectName *ls, unsigned int how_many=300)throw(ORepFailed);		
		
	// @}
	// end of ORepServiceGroup 

	protected:

		ObjectRepository();
		const std::string nsName;
		UniSetTypes::Configuration* uconf;
		
		bool list(const std::string& section, UniSetTypes::ListObjectName *ls, unsigned int how_many, ObjectType type);

	private:
		bool init();
		CosNaming::NamingContext_var localctx;
    };

//};
// -----------------------------------------------------------------------------------------
#endif
// -----------------------------------------------------------------------------------------
