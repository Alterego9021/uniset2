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
 * \brief ��������� � ������� ������������ ������ ���������� � ��������� ��� InfoServer-�
 * \author Pavel Vainerman <pv>
 * \version $Id: Restorer.h,v 1.4 2007/11/18 19:13:35 vpashka Exp $
 * \date $Date: 2007/11/18 19:13:35 $
 */
// --------------------------------------------------------------------------
#ifndef Restorer_H_
#define Restorer_H_
// --------------------------------------------------------------------------
#include <sigc++/sigc++.h>
#include <string>
#include "UniXML.h"
#include "UniSetTypes.h"
// --------------------------------------------------------------------------
/*!
	��� ����������� ���������. 
	�������� ����� ��� ���� xxx_XML ����������� �������.
	�������� �� ������ � ������ ������� ����� �������.
	��� ������� ��� ��������� ������� ������� ����� ������� old_xxx
*/ 
class Restorer_XML
{
	public:

		Restorer_XML();
	    virtual ~Restorer_XML();

		/*! ���� ��� ����������� ������� ������ ������� �� xml-�����. 
			\param uxml	- ��������� ��� ������ � xml-������
			\param it 	- ���������(���������) �� ������� ����������� xml-���� (<item>)
			\param sec	- ���������(���������) �� �������� ���� ������ (<SubscriberList>)
			\return TRUE - ���� ������ ���������� ������ �������, FALSE - ���� ���
		*/
		typedef sigc::slot<bool,UniXML&,UniXML_iterator&,xmlNode*> ReaderSlot;

		/*! ���������� ������� ��� callback-������ ��� ������ ������ ���������
			For example: 
				setReadItem( sigc::mem_fun(this,&MyClass::myReadItem) );

			bool myReadItem::myfunc(UniXML& xml, 
										UniXML_iterator& it, xmlNode* sec)

			uxml	- ��������� ��� ������ � xml-������
			it 	- ���������(���������) �� ������� ����������� xml-���� (<item>)
			sec	- ��������� �� �������� ���� ������ (<SubscriberList>)
		*/
		void setReadItem( ReaderSlot sl );


		/*! ���������� ������� ��� callback-������ ��� ������ ������ ����������
			For example: 
				setReadItem( sigc::mem_fun(this,&MyClass::myReadItem) );

			bool myReadItem::myfunc(UniXML& xml, 
										UniXML_iterator& it, xmlNode* sec)

			uxml	- ��������� ��� ������ � xml-������
			it 	- ���������(���������) �� ������� ����������� xml-���� (<consumer>)
			sec	- ��������� �� ������� ���� ��������� (<item>)
		*/

		void setReadConsumerItem( ReaderSlot sl );


		/*! ���������� ������ �� ������ ������ ��������
			\note ������� ���������� �������� �� ������ read(...)
		 */
		void setItemFilter( const std::string filterField, const std::string filterValue="" );

		/*! ���������� ������ �� ������ ������ ���������� (�� ������� �������)
			\note ������� ���������� �������� �� ������ read(...)
		 */
		void setConsumerFilter( const std::string filterField, const std::string filterValue="" );


		/*! ������������� ������� ��������� ���������� � ��������� (id � node) 
			�� ������ ������� ����� (<consumer name="xxxx" type="objects" />)
			\return true - ���� �������������� ����������
		*/
		bool getConsumerInfo( UniXML_iterator& it, 
								UniSetTypes::ObjectId& cid, UniSetTypes::ObjectId& cnode );

		/*! ������������� ������� ��������� ���������� � ��������� (id � node)
			�� ������� ������� ����� (<consumer name="/Root/Section/Name" node="xxxx" />)
			\return true - ���� �������������� ����������
		*/
		bool old_getConsumerInfo( UniXML_iterator& it, 
								UniSetTypes::ObjectId& cid, UniSetTypes::ObjectId& cnode );



		/*! ������� ������ �� �������� ������ (��� �������� ��� �������� �����) */
		static xmlNode* find_node( UniXML& xml, xmlNode* root, const std::string& nodename, const std::string nm="" );

	protected:

		virtual bool check_list_item( UniXML_iterator& it );
		virtual bool check_consumer_item( UniXML_iterator& it );

		ReaderSlot rslot;
		ReaderSlot cslot;

		std::string i_filterField;
		std::string i_filterValue;
		std::string c_filterField;
		std::string c_filterValue;
};
// --------------------------------------------------------------------------
#endif
// --------------------------------------------------------------------------
