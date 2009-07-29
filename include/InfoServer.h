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
 * \date  $Date: 2007/01/06 00:03:34 $
 * \version $Id: InfoServer.h,v 1.11 2007/01/06 00:03:34 vpashka Exp $
*/
// -------------------------------------------------------------------------- 
#ifndef InfoServer_H_
#define InfoServer_H_
//---------------------------------------------------------------------------
#include <map>
#include <list>
#include "MessageType.h"
#include "UniSetObject.h"
#include "InfoServer_i.hh"
//---------------------------------------------------------------------------
class ISRestorer;
//---------------------------------------------------------------------------
/*!
	\page ServicesPage
	\section secInfoServer ������ ���������
	 \subsection subIS_common ����� ��������
		� ��� ������ ������ ��������� ���� ��������� ��� ���������.
	��� ������� ��������� �� ���������� ��������� ��������:
	- ������������ ��������� � �� (\ref secDBServer)
	- ��������� ��������� �� ��� ����, ���� ��� �������� ��� broadcast.
	- ��������� ��������� ���� ����ӣ���� � RouteList(��. ����.����).
	- ������������ ��������� ����������� ��� ������� ������� ������� (�.�. �������� ����������� 
	������� InfoServer::processing() )

	\subsection subIS_idea �������� ������
	 	�� ����, ��� ��ģ��� �� ����������� ���� ��������� �������. ������� ����� �������� ������, ����������� ���������:
		- ����� NameService
		- ��� ������ UniversalInterface::send()
		
	\par 		
	������ �������� ���������, ������� ��� ������������� ����� �������� ��� ������ 
	UniSetTypes::Configuration::getInfoServer() ������� UniSetTypes::conf.

	\subsection subIS_interface ���������	
	InfoServer ��������� ���������� ����������� � ������� ��� ��� ���� ���������, 
	� ����� �������������(������������). ����� ����������� ����� ��������� �� ����
	\code
		InfoServer::ackMessage(UniSetTypes::MessageCode msgid, const UniSetTypes::ConsumerInfo& ci, 
									UniversalIO::UIOCommand cmd, CORBA::Boolean acknotify);
	\endcode
	��� ����� �� ��������� �����
	\code
		InfoServer::ackMessageRange(UniSetTypes::MessageCode from, UniSetTypes::MessageCode to,
									const UniSetTypes::ConsumerInfo& ci, 
									UniversalIO::UIOCommand cmd, CORBA::Boolean acknotify);
	\endcode


	���������� ��. \ref InfoServer				
*/

/*!
 * \class InfoServer
 * \brief ��������� ��� ������ ����������
*/ 
class InfoServer: 
	public UniSetObject,
	public POA_InfoServer_i
{
	public:
		InfoServer( UniSetTypes::ObjectId id=UniSetTypes::DefaultObjectId, ISRestorer* d=0 );
		virtual ~InfoServer();

		virtual UniSetTypes::ObjectType getType(){ return UniSetTypes::getObjectType("InfoServer"); }

		
		// ���������� IDL ����������
		/*! ����� ����������� � ������� ��������� */
		virtual void ackMessage(UniSetTypes::MessageCode msgid, const UniSetTypes::ConsumerInfo& ci, 
							UniversalIO::UIOCommand cmd, CORBA::Boolean acknotify);
		/*! ����� ����������� � ������� ��������� �� ��������� */
		virtual void ackMessageRange(UniSetTypes::MessageCode from, UniSetTypes::MessageCode to,
									const UniSetTypes::ConsumerInfo& ci, 
									UniversalIO::UIOCommand cmd, CORBA::Boolean acknotify);


		/*! ���������� � ����������� (���������) */
		struct ConsumerInfoExt:
			public	UniSetTypes::ConsumerInfo
		{
			ConsumerInfoExt( UniSetTypes::ConsumerInfo ci,
							UniSetObject_i_ptr ref=0):
			ConsumerInfo(ci),ref(ref){}

			UniSetObject_i_ptr ref;
			CORBA::Boolean ask;
		};

		/*! ������ ������������ */
		typedef std::list<ConsumerInfoExt> ConsumerList; 				

		/*! ������ ��� �������������->������ ������������ */
		typedef std::map<UniSetTypes::MessageCode,ConsumerList> AskMap;		

	
	protected:
		// ������� ��������� ��������� ��������� 
		virtual void processingMessage( UniSetTypes::VoidMessage *msg );

		/*! ������� ��������� UniSetTypes::AlarmMessage.
		 ���������������� � ��������� �������, ���� ��������� ����������� ���������. */
		virtual void processing(UniSetTypes::AlarmMessage &amsg){};
		/*! ������� ��������� UniSetTypes::InfoMessage.
		 ���������������� � ��������� �������, ���� ��������� ����������� ���������. */
		virtual void processing(UniSetTypes::InfoMessage &imsg){};
		/*! ������� ��������� UniSetTypes::AckMessage.
		 ���������������� � ��������� �������, ���� ��������� ����������� ���������. */
		virtual void processing(UniSetTypes::ConfirmMessage &cmsg){};		

		
		/*! ��������������� ��������� ���������. ��������� �� ������ ���� �
			���������� � ����.
		*/
		void preprocessing( UniSetTypes::TransportMessage& tmsg, bool broadcast );
		void preprocessingConfirm( UniSetTypes::ConfirmMessage& am, bool broadcast );

		virtual bool activateObject();


		/*! ���������� ������ ���������� 
			�� ��������� ������ dump, ���� �������� dumper.
		*/
		virtual void dumpOrdersList(UniSetTypes::MessageCode mid, const ConsumerList& lst);

		/*! ������ dump-����� */
		virtual void readDump();

		//! ������� ���������� �� ������� ���������
		template<class TMessage>
		void event(UniSetTypes::MessageCode key, TMessage& msg, CORBA::Boolean askn);

		//----------------------
		//! ������� ���������� �� ��������� ��������� �������
		template <class TMessage>
		void send(ConsumerList& lst, TMessage& msg, CORBA::Boolean acknotify);
		bool addConsumer(ConsumerList& lst, const UniSetTypes::ConsumerInfo& cons, CORBA::Boolean acknotify); 	//!< �������� ����������� ���������
		bool removeConsumer(ConsumerList& lst, const UniSetTypes::ConsumerInfo& cons, CORBA::Boolean acknotify);	//!< ������� ����������� ���������

		
		//! ��������� ������ 
		void ask(AskMap& askLst, UniSetTypes::MessageCode key, 
					const UniSetTypes::ConsumerInfo& cons, UniversalIO::UIOCommand cmd,
					CORBA::Boolean acknotify);

		/*! ��������� �� ������ ����������� ���� ������ ���������� */
		ISRestorer* restorer;		

	private:
		friend class ISRestorer;
		bool dbrepeat; /*!< ���� ��������� ��������� DBServer-� */


		AskMap askList;	/*!< ������ ������������ */
		/*! ����� ��� ������������ ����������� ������� � c����� ������������ */			
		UniSetTypes::uniset_mutex askMutex; 
		
		std::list<UniSetTypes::ConsumerInfo> routeList;
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
