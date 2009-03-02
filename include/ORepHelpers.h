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
 * \author Pavel Vainerman <pv>
 * \version $Id: ORepHelpers.h,v 1.9 2007/11/18 19:13:35 vpashka Exp $
 * \date  $Date: 2007/11/18 19:13:35 $
 */
// -------------------------------------------------------------------------- 
#ifndef ORepHelpers_H_
#define ORepHelpers_H_
// -------------------------------------------------------------------------- 
#include <omniORB4/CORBA.h>
#include <omniORB4/Naming.hh>
#include <string>
#include "Exceptions.h"

// -----------------------------------------------------------------------------------------
/*!
 * \namespace ORepHelpers
 * � ���� ������������ ���� ��������� ��������������� ������� ������������ ��������� ObjectRepository
*/
namespace ORepHelpers
{
    //! ��������� ������ �� ������ ����������� 
    CosNaming::NamingContext_ptr getRootNamingContext(CORBA::ORB_ptr orb, 
														const std::string& nsName, int timeOutSec=2);

    //! ��������� ��������� �� ��������� ����� 
    CosNaming::NamingContext_ptr getContext(const std::string& cname, int argc, 
											char* *argv, const std::string& nsName)
															throw(UniSetTypes::ORepFailed);

	CosNaming::NamingContext_ptr getContext(CORBA::ORB_ptr orb, const std::string& cname,  
											const std::string& nsName)
															throw(UniSetTypes::ORepFailed);
    
    //! ������� ���������� ��� ������ �� ������� ����� 
    const std::string getSectionName(const std::string& fullName, const std::string brk="/");
    
    //! ������� ��������� ����� �� ������� ����� 
    const std::string getShortName(const std::string& fullName, const std::string brk="/");
	
	
	//! �������� �� ������� ������������ ��������
	char checkBadSymbols(const std::string& str);
	
	/*! ��������� ������ ����������� �������� � ���� '.', '/', � �.�. */
	std::string BadSymbolsToStr();

}
#endif
