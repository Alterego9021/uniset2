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
 * \brief ���������� ��������(����������������) ������ ��� �������� ������� 
 * (��������� ����������, ��������� ������������ ���������� � �.�.)
 * \author Pavel Vainerman <pv>
 * \date $Date: 2009/01/16 23:16:42 $
 * \version $Id: UniSetObject.h,v 1.19 2009/01/16 23:16:42 vpashka Exp $
 */
//---------------------------------------------------------------------------
#ifndef UniSetObject_H_
#define UniSetObject_H_
//--------------------------------------------------------------------------
#include <unistd.h>
#include <sys/time.h>
#include <queue>
#include <ostream>
#include <string>
#include <list>

#include "UniSetTypes.h"
#include "MessageType.h"
#include "PassiveTimer.h"
#include "Exceptions.h"
#include "UniversalInterface.h"
#include "UniSetObject_i.hh"
#include "ThreadCreator.h"

//---------------------------------------------------------------------------
//#include <omnithread.h>
//---------------------------------------------------------------------------
class ObjectsActivator;
class ObjectsManager;

//---------------------------------------------------------------------------
class UniSetObject;
typedef std::list<UniSetObject *> ObjectsList; 	/*!< ������ ����������� �������� */
//---------------------------------------------------------------------------
/*! \class UniSetObject
 *	����� ������ ����� �������� ������� ���: ��������� ���������, ��������� ��������� � ������� � �.�.
 *	��� �������� ��������� ������������ ������� waitMessage(), ���������� �� �������.
 *	�������� ����������� ���� �� ��������� ���������� �������, ���� �� ������� ���������, ��� ������ ��������
 *	termWaiting() ���������� �� push(). 
 * 	\note ���� �� ����� ����� ObjectId(-1), �� ����� ��������� ������� �� �����.
 *	����� �������� ������ ����� ������������� ��������� ��� ������ ������� void thread(). �� ���������� ������� �� ��������� �������
 *	(�������� � ������������). ��� ���� ��������������� �� ����� receiveMessage() � processingMessage() ����������� 
 *	�� ������������.
*/ 
class UniSetObject:
	public POA_UniSetObject_i
{
	public:
		UniSetObject(const std::string name, const std::string section); 
		UniSetObject(UniSetTypes::ObjectId id);
		UniSetObject();
		virtual ~UniSetObject();

		// ������� ����������� � IDL
		virtual CORBA::Boolean exist();
		virtual char* getName(){return (char*)myname.c_str();}
		virtual UniSetTypes::ObjectId getId(){ return myid; }
		virtual UniSetTypes::ObjectType getType() { static UniSetTypes::ObjectType ot("UniSetObject"); return ot; }
		virtual UniSetTypes::SimpleInfo* getInfo();
		friend std::ostream& operator<<(std::ostream& os, UniSetObject& obj );

		//! ��������� ��������� � �������
		virtual void push(const UniSetTypes::TransportMessage& msg);

		/*! �������� ������ (�� ����) */
		inline UniSetTypes::ObjectPtr getRef()
		{
			UniSetTypes::uniset_mutex_lock lock(refmutex, 300);
			return (UniSetTypes::ObjectPtr)CORBA::Object::_duplicate(oref);
		}

	protected:
			/*! ��������� ���������� ��������� */
			virtual void processingMessage(UniSetTypes::VoidMessage *msg);

			/*! �������� ��������� */
			bool receiveMessage(UniSetTypes::VoidMessage& msg);		
	
			/*! ������� ��������� ��������� � ������� */
			unsigned int countMessages();
			 	
			/*! �������� �������� ��������� */
			void termWaiting();

			UniversalInterface ui; /*!< ������������� ��������� ��� ������ � ������� ���������� */			
			std::string myname;
			std::string section;

			//! �������������� ������� (���������������� ��� ����������� �������� ����� ������������)	 
			virtual bool disactivateObject(){return true;}
			//! ����������� ������� (���������������� ��� ����������� �������� ����� �����������)	 
			virtual bool activateObject(){return true;}

			/*! ������(����������) �������� ������ ��� ��������� ��������� */
			inline void thread(bool create){ threadcreate = create; }
			/*! ���������� ������ ��������� ��������� */
			inline void offThread(){ threadcreate = false; }
			/*! ��������� ������ ��������� ��������� */
			inline void onThread(){ threadcreate = true; }

			/*! ������� ���������� �� ������ */
			virtual void callback();

			/*! ������� ���������� ��� ������� ������� ���������� ��� ���������� ��������. ������������� �� �����
			 *	��������� ����������� ��� �������� �������� �� ��������� �������.
			 *	�������� ������� � ���������� ���������.
			 *  \warning � ����������� �������� \b ��������� �������� ������� �������� exit(..), abort()!!!! 
			*/
			virtual void sigterm( int signo ){};

			inline void terminate(){ disactivate(); }

			/*! ������� ��������� timeMS */
			virtual bool waitMessage(UniSetTypes::VoidMessage& msg, int timeMS=UniSetTimer::WaitUpTime);		

			void setID(UniSetTypes::ObjectId id);


			void setMaxSizeOfMessageQueue( unsigned int s )
			{
				if( s>=0 )
					SizeOfMessageQueue = s;
			}

			inline unsigned int getMaxSizeOfMessageQueue()
			{ return SizeOfMessageQueue; }
			
			void setMaxCountRemoveOfMessage( unsigned int m )
			{
				if( m >=0 )
					MaxCountRemoveOfMessage = m;
			}

			inline unsigned int getMaxCountRemoveOfMessage()
			{ return MaxCountRemoveOfMessage; }


			// ������� ����������� ������������� ��������� ��� ���������
			struct PriorVMsgCompare: 
				public std::binary_function<UniSetTypes::VoidMessage, UniSetTypes::VoidMessage, bool>
			{
				bool operator()(const UniSetTypes::VoidMessage& lhs, 
								const UniSetTypes::VoidMessage& rhs) const;
			};
			typedef std::priority_queue<UniSetTypes::VoidMessage,std::vector<UniSetTypes::VoidMessage>,PriorVMsgCompare> MessagesQueue;


			/*! ���������� ��� ����������� ������� ��������� (� ���� ������ push � receive)
				��� ������� �������.
				\warning �� ��������� ������� �� ������� ��� ������������� 
				 - SensorMessage
				 - TimerMessage
				 - SystemMessage
			 ���� �� ������� ������� �� ������� UniSetObject::MaxCountRemoveOfMessage
			\note ��� ����������� ��������� ����� ���� ��������������
			\warning �.�. ��� ������������ SensorMessage �� ��������� ��������, ��
			��� �������� ��������� �� ��������� ���������� �������� ������� ����� ��������
			� ������������ ������ ���������� ���������� ���������� � "��������" ��������� N ��������.
			(������-��� ��������� ���� ���������)
			*/
			virtual void cleanMsgQueue( MessagesQueue& q );


			void setRecvMutexTimeout( unsigned long msec );
			inline unsigned long getRecvMutexTimeout(){ return recvMutexTimeout; }
			
			void setPushMutexTimeout( unsigned long msec );
			unsigned long getPushMutexTimeout(){ return pushMutexTimeout; }


			UniSetTypes::VoidMessage msg;	
			ObjectsManager* mymngr; 

	private:

			friend class ObjectsManager;
			friend class ObjectsActivator;
			friend class ThreadCreator<UniSetObject>;
			inline pid_t getMsgPID()
			{
				return msgpid;
			}

			/*! ������� ������ */
			void work();	
			//! ������������� ���������� �������
			bool init(ObjectsManager* om);
			//! ������ ������������� �������	 
			bool disactivate();
			//! ���������������� ����������� �������
			bool activate();
			/* ����������� � ����������� �������� */
			void registered();
			/* �������� ������ �� ����������� �������� 	*/
			void unregister();

			pid_t msgpid; // pid ������ ��������� ���������
			bool reg;
			bool active;
			bool threadcreate;
			UniSetTimer* tmr;	
			UniSetTypes::ObjectId myid;
			CORBA::Object_var oref;
			ThreadCreator<UniSetObject>* thr;

			/*! ������� ��������� ��� ������� */
			MessagesQueue queueMsg;

		 	/*! ����� ��� ������������ ����������� ������� � ������� */
			UniSetTypes::uniset_mutex qmutex;

		 	/*! ����� ��� ������������ ����������� ������� � ������� */
			UniSetTypes::uniset_mutex refmutex;

			/*! ������ ������� ��������� (��� ���������� ���������� �������) */
			unsigned int SizeOfMessageQueue;
			/*! ������� ��������� ������� ��� ������*/
			unsigned int MaxCountRemoveOfMessage;
			unsigned long recvMutexTimeout; /*!< ������� �� �������� ������������ mutex-� ��� receiveMessage */
			unsigned long pushMutexTimeout; /*!< ������� �� �������� ������������ mutex-� ��� pushMessage */
			
			// �������������� ���������� 
			unsigned long stMaxQueueMessages;	/*<! ������������ ����� ��������� ����������� � ������� */
			unsigned long stCountOfQueueFull; 	/*! ���������� ������������ ������� ��������� */
};

#endif
