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
 *  \brief ���� ���������� Info-�������
 *  \author Pavel Vainerman
 *  \date   $Date: 2007/01/17 23:33:41 $
 *  \version $Id: InfoServer.cc,v 1.14 2007/01/17 23:33:41 vpashka Exp $
*/
// -------------------------------------------------------------------------- 
#include <string>
#include <sstream>
#include "Configuration.h"
#include "InfoServer.h"
#include "ISRestorer.h"
#include "UniXML.h"
// ------------------------------------------------------------------------------------------
using namespace UniSetTypes;
using namespace std;
// ------------------------------------------------------------------------------------------

InfoServer::InfoServer( ObjectId id, ISRestorer* d ):
	UniSetObject(id),
	restorer(d),
	dbrepeat(true)
{
	if( id == DefaultObjectId )
	{
		id = conf->getInfoServer();
		if( id == DefaultObjectId )
		{
			ostringstream msg;
			msg << "(InfoServer): ������ ����������! �� ����� ObjectId !!!!!\n";
//			unideb[Debug::CRIT] << msg.str() << endl;
			throw Exception(msg.str());
		}
		setID(id);
	}
	
	routeList.clear();

	UniXML* xml = conf->getConfXML();
	if( xml )
	{
		xmlNode* root = xml->findNode(xml->getFirstNode(),"LocalInfoServer");
		if( root )
		{
			dbrepeat = atoi(xml->getProp(root,"dbrepeat").c_str());
			if( !dbrepeat )
				unideb[Debug::INFO] << myname << "(init): dbrepeat="<< dbrepeat << endl;
			
			xmlNode* node(xml->findNode(root,"RouteList"));
			if(!node) 
				unideb[Debug::WARN] << myname << ": ������ ������ ����-�����. ��� ������� RouteList" << endl;
			else
			{
				UniXML_iterator it(node);
				if( it.goChildren() )
				{
					for( ;it.getCurrent(); it.goNext() )
					{
						string cname(xml->getProp(it,"name"));
						ConsumerInfo ci;
						ci.id =	 conf->oind->getIdByName(cname);
						if( ci.id == UniSetTypes::DefaultObjectId )
						{
							unideb[Debug::CRIT] << myname << ": �� ������ ������������� ������� -->" << cname << endl;
							continue;
						}
					
						ci.node = conf->getLocalNode();
						string cnodename(xml->getProp(it,"node"));
						if( !cnodename.empty() )
							ci.node = conf->oind->getIdByName(cnodename);
						if( ci.node == UniSetTypes::DefaultObjectId )
						{
							unideb[Debug::CRIT] << myname << ": �� ������ ������������� ���� -->" << cnodename << endl;
							continue;
						}
						routeList.push_back(ci);
					}
				}	
			}
		}
	}
}

InfoServer::~InfoServer()
{
}

// ------------------------------------------------------------------------------------------
void InfoServer::preprocessing(TransportMessage& tmsg, bool broadcast)
{
//	unideb[Debug::INFO] << myname << "... preprocessing... "<< endl;

	// ���������� �� ������ ����
	if( broadcast )
	{
		for ( UniSetTypes::ListOfNode::const_iterator it = conf->listNodesBegin();
			it != conf->listNodesEnd(); ++it )
		{
			if( it->infserver!=UniSetTypes::DefaultObjectId && it->connected && it->id != conf->getLocalNode() )
			{
				try
				{	
					ui.send(it->infserver, tmsg, it->id);
				}
				catch(...)
				{
					unideb[Debug::CRIT] << myname << "(preprocessing): �� ���� ������� ��������� ���� "<< conf->oind->getMapName(it->id)<< endl;
				}
			}
		}
	}

	

//	unideb[Debug::INFO] << myname << " ����� � ����... "<< endl;
	// ��������� � ����
	try
	{
		if( dbrepeat )
			ui.send(conf->getDBServer(), tmsg);
	}
	catch(...)
	{
		unideb[Debug::CRIT] << myname << "(preprocessing): �� ���� ������� ��������� DBServer-�" << endl;
	}

	
	// ���������� �� routeList-�
	for( list<UniSetTypes::ConsumerInfo>::const_iterator it=routeList.begin(); it!=routeList.end(); ++it )
	{
		try
		{	
			ui.send(it->id, tmsg, it->node);
		}
		catch(...)
		{
			unideb[Debug::CRIT] << myname << "(preprocessing):"
				<< " �� ���� ������� ��������� ������� "
				<< conf->oind->getNameById(it->id,it->node)<< endl;
		}
	}
}

// ------------------------------------------------------------------------------------------
void InfoServer::preprocessingConfirm(UniSetTypes::ConfirmMessage& am, bool broadcast)
{
//	unideb[Debug::INFO] << myname << "... preprocessing... "<< endl;

	TransportMessage tmsg(am.transport_msg());
	// ���������� �� ������ ����
	if( broadcast )
	{
		for ( UniSetTypes::ListOfNode::const_iterator it = conf->listNodesBegin();
			it != conf->listNodesEnd(); ++it )
		{
			if( it->infserver!=UniSetTypes::DefaultObjectId && it->connected && it->id != conf->getLocalNode() )
			{
				try
				{	
					ui.send(it->infserver, tmsg, it->id);
				}
				catch(...)
				{
					unideb[Debug::CRIT] << myname << "(preprocessing):"
						<< " �� ���� ������� ��������� ���� "
						<< conf->oind->getMapName(it->id)<< endl;
				}
			}
		}
	}

	try
	{
		if( dbrepeat )
			ui.send(conf->getDBServer(), tmsg);
	}
	catch(...)
	{
		unideb[Debug::CRIT] << myname << "(preprocessing): �� ���� ������� ��������� DBServer-�" << endl;
	}

	// ���������� �� routeList-�
	for( list<UniSetTypes::ConsumerInfo>::const_iterator it=routeList.begin(); it!=routeList.end(); ++it )
	{
		try
		{	
			ui.send(it->id, tmsg, it->node);
		}
		catch(...)
		{
			unideb[Debug::CRIT] << myname << "(preprocessing):"
					<< " �� ���� ������� ��������� ������� "
					<< conf->oind->getNameById(it->id,it->node) << endl;
		}
	}
}

// ------------------------------------------------------------------------------------------

void InfoServer::processingMessage( UniSetTypes::VoidMessage *msg )
{
	switch( msg->type )
	{
		case Message::Info:
		{
			InfoMessage im(msg);
			unideb[Debug::INFO] << myname << " InfoMessage code= "<< im.infocode << endl;
			try
			{
				// ���� ��� �� ����������� ��������� 
				// �� ������������ ���
				if( !im.route )
				{
					im.route = true;	// ���������� ������� ���������
					TransportMessage tm(im.transport_msg());
					preprocessing(tm, im.broadcast);
				}
			}
			catch(Exception& ex )
			{
				unideb[Debug::CRIT] << myname << "(info preprocessing):" << ex << endl;
			}
			catch(...)
			{
				unideb[Debug::CRIT] << myname << "(info preprocessing): catch ..." << endl;
			}

			// �������� ���� ���������� �����������
			try
			{
				event(im.infocode, im, false);
			}
			catch(Exception& ex )
			{
				unideb[Debug::CRIT] << myname << "(info event):" << ex << endl;
			}
			catch(...)
			{
				unideb[Debug::CRIT] << myname << "(info event): catch ..." << endl;
			}

			try
			{
				processing(im);
			}
			catch(Exception& ex )
			{
				unideb[Debug::CRIT] << myname << "(info processing):" << ex << endl;
			}
			catch(...)
			{
				unideb[Debug::CRIT] << myname << "(info processing) catch ..." << endl;
			}
			break;
		}

		case Message::Alarm:
		{
			AlarmMessage am(msg);
	
			unideb[Debug::INFO] << myname 
				<< " AlarmMessage code= "<< am.alarmcode
				<< " cause="<< am.causecode << endl;

			try
			{
				// ���� ��� �� ����������� ��������� 
				// �� ������������ ���
				if( !am.route )
				{
					am.route = true;	// ���������� ������� ���������
					TransportMessage tm(am.transport_msg());
					preprocessing(tm, am.broadcast);
				}
			}
			catch(Exception& ex )
			{
				unideb[Debug::CRIT] << myname << "(alarm preprocessing):" << ex << endl;
			}
			catch(...)
			{
				unideb[Debug::CRIT] << myname << "(alarm preprocessing): catch ..." << endl;
			}

			try
			{
				// �������� ���� ���������� �����������
				event(am.alarmcode, am, false);
			}
			catch(Exception& ex )
			{
				unideb[Debug::CRIT] << myname << "(alarm event):" << ex << endl;
			}
			catch(...)
			{
				unideb[Debug::CRIT] << myname << "(alarm event): catch ..." << endl;
			}
			
			try
			{
				processing(am);
			}
			catch(Exception& ex )
			{
				unideb[Debug::CRIT] << myname << "(alarm processing):" << ex << endl;
			}
			catch(...)
			{
				unideb[Debug::CRIT] << myname << "(alarm processing): catch ..." << endl;
			}

			break;
		}

		case Message::Confirm:
		{
			UniSetTypes::ConfirmMessage cm(msg);
	
			unideb[Debug::INFO] << myname << " ConfirmMessage �� ��������� code= "<< cm.code << endl;
			try
			{
				// ���� ��� �� ����������� ��������� 
				// �� ������������ ���
				if( !cm.route )
				{
					cm.route = true;
					preprocessingConfirm(cm, cm.broadcast);
				}
			}
			catch(Exception& ex )
			{
				unideb[Debug::CRIT] << myname << "(alarm processing):" << ex << endl;
			}
			catch(...)
			{
				unideb[Debug::CRIT] << myname << "(alarm processing): catch ..." << endl;
			}

		
			// �������� ���� ���������� �����������
			event(cm.code, cm, true);
			
			try
			{
				processing(cm);
			}
			catch(...){}

			break;
		}

		case Message::SysCommand:
			unideb[Debug::INFO] << myname << " system command... "<< endl;
			break;

		default:
			unideb[Debug::CRIT] << myname << ": ����������� ���������"<< endl;
			break;
	}
	
}
// ------------------------------------------------------------------------------------------
void InfoServer::ackMessage(UniSetTypes::MessageCode mid, const UniSetTypes::ConsumerInfo& ci, 
							UniversalIO::UIOCommand cmd, CORBA::Boolean acknotify)
{
	unideb[Debug::INFO] << myname << "(askMessage): �������� ����� �� "
		<< conf->oind->getNameById(ci.id, ci.node)
		<< " �� ��������� " << mid << endl;

	// �������� �� �������������
	if(!conf->mi->isExist(mid) )
	{
		unideb[Debug::CRIT] << myname << "(askMessage): ��������� � ����� "
				<< mid << " ��� � MessagesMap" << endl;

//		InfoServer_i::MsgNotFound nf;
//		nf.bad_code = mid;
//		throw nf;
	}

	{	// lock
 		uniset_mutex_lock lock(askMutex, 200);
		// � ��� ���� �������(���������) ��������� 
		ask( askList, mid, ci, cmd, acknotify);		
	}	// unlock
}
// ------------------------------------------------------------------------------------------
void InfoServer::ackMessageRange(UniSetTypes::MessageCode from, UniSetTypes::MessageCode to, 
						const UniSetTypes::ConsumerInfo& ci, UniversalIO::UIOCommand cmd, 
						CORBA::Boolean acknotify)
{
	// �������� ������������ ���������
	if( from>to )
		throw InfoServer_i::MsgBadRange();


	for( UniSetTypes::MessageCode c=from; c<=to; c++ )
		ackMessage(c,ci,cmd, acknotify);
}
// ------------------------------------------------------------------------------------------
/*!
 *	\param lst - ��������� �� ������ � ������� ���������� ������ �����������
 *	\param name - ��� ��������� �����������
 *	\note ���������� ���������� ������ ���� ������ ����������� �� ���������� � ������
*/
bool InfoServer::addConsumer(ConsumerList& lst, const ConsumerInfo& ci, CORBA::Boolean acknotify )
{
	for( ConsumerList::const_iterator li=lst.begin();li!=lst.end(); ++li )
	{
		if( li->id == ci.id && li->node == ci.node )
			return false;
	}
	
	ConsumerInfoExt cinf(ci);
	cinf.ask = acknotify;
	// �������� ������
	try
	{
		UniSetTypes::ObjectVar op = ui.resolve(ci.id,ci.node);
		cinf.ref = UniSetObject_i::_narrow(op);
	}
	catch(...){}
	
	lst.push_front(cinf);
	return true;
}

// ------------------------------------------------------------------------------------------
/*!
 *	\param lst - ��������� �� ������ �� ������� ���������� �������� �����������
 *	\param name - ��� ���������� �����������
*/
bool InfoServer::removeConsumer(ConsumerList& lst, const ConsumerInfo& cons, CORBA::Boolean acknotify )
{
	for( ConsumerList::iterator li=lst.begin();li!=lst.end();++li)
	{
		if( li->id == cons.id && li->node == cons.node  )		
		{
			lst.erase(li);
			return true;
		}
	}
	
	return false;
}

// ------------------------------------------------------------------------------------------
void InfoServer::ask(AskMap& askLst, UniSetTypes::MessageCode key, 
					const UniSetTypes::ConsumerInfo& cons, 
					UniversalIO::UIOCommand cmd,
					CORBA::Boolean acknotify)
{
	// ����� ������� � ������ 
	AskMap::iterator askIterator = askLst.find(key);

  	switch( cmd )
	{
		case UniversalIO::UIONotify: // �����
		{
   			if( askIterator==askLst.end() ) 
			{
				ConsumerList lst; // ������� ����� ������
				addConsumer(lst,cons, acknotify);	  
				askLst.insert(AskMap::value_type(key,lst));	// ����� ����������� ������(��� ������� ������� ������ ���)
				try
				{
					dumpOrdersList(key,lst);
				}
				catch(Exception& ex)
				{
					unideb[Debug::WARN] << myname << " �� ������ ������� dump: " << ex << endl;
				}
				catch(...)
				{
			    	unideb[Debug::WARN] << myname << " �� ������ ������� dump" << endl;
				}
		    }
			else
			{
				if( addConsumer(askIterator->second,cons, acknotify) )
				{  
					try
					{
						dumpOrdersList(key,askIterator->second);
					}
					catch(...)
					{	
				    	unideb[Debug::WARN] << myname << " �� ������ ������� dump" << endl;
					}
				}
		    }
			break;
		}

		case UniversalIO::UIODontNotify: 	// �����
		{
			if( askIterator!=askLst.end() )	// ����������
			{
				if( removeConsumer(askIterator->second, cons, acknotify) )
				{
					if( askIterator->second.empty() )
						askLst.erase(askIterator);	
					else
					{
						try
						{
							dumpOrdersList(key,askIterator->second);
						}
						catch(Exception& ex)
						{
							unideb[Debug::WARN] << myname << " �� ������ ������� dump: " << ex << endl;
						}
						catch(...)
						{	
				    		unideb[Debug::WARN] << myname << " �� ������ ������� dump" << endl;
						}
					}
				}
			}
			break;
		}
	
		default:
			break;
	}
}
// ------------------------------------------------------------------------------------------
void InfoServer::dumpOrdersList(UniSetTypes::MessageCode mid, const ConsumerList& lst)
{
	try
	{
		if(restorer)
			restorer->dump(this,mid,lst);
	}
	catch(Exception& ex)
	{ 
		unideb[Debug::WARN] << myname << "(dumpOrdersList): " << ex << endl;
	}
			
}
// ------------------------------------------------------------------------------------------
bool InfoServer::activateObject()
{
	UniSetObject::activateObject();
	readDump();
	return true;
}
// ------------------------------------------------------------------------------------------
void InfoServer::readDump()
{
	try
	{
		if( restorer )
			restorer->read(this);
	}
	catch(Exception& ex)
	{ 
		unideb[Debug::WARN] << myname << "(readDump): " << ex << endl;
	}
}
// ------------------------------------------------------------------------------------------
/*!
	\warning � ������ ��������� � ������� push, ����� ����������� �������� ������ ��������.
*/
template<class TMessage>
void InfoServer::send(ConsumerList& lst, TMessage& msg, CORBA::Boolean askn)
{
	for( ConsumerList::iterator li=lst.begin();li!=lst.end();++li )
	{
		// ���������� ���� ��� �� ����� ����� � �������������
		if( askn && !li->ask )
			continue;
		
		for(int i=0; i<2; i++ )	// �� ������ ������ �� ��� �������
		{
		    try
		    {
				if( CORBA::is_nil(li->ref) )
				{	
					CORBA::Object_var op = ui.resolve(li->id, li->node);			
					li->ref = UniSetObject_i::_narrow(op);
				}

				msg.consumer = li->id;
				li->ref->push( msg.transport_msg() );
//				unideb[Debug::INFO] << myname << "(send): �������� "<< conf->oind->getMapName( li->node ) << "/" << ui.getNameById( li->id ) << " notify" << endl;
				break;					
		    }
			catch(Exception& ex)
			{
			   	unideb[Debug::CRIT] << myname << "(send): " << ex << endl;
			}
		    catch( CORBA::SystemException& ex )
		    {
		    	unideb[Debug::CRIT] << myname << "(send): " 
						<< conf->oind->getNameById( li->id )
						<< " ����������!! " << ex.NP_minorString() << endl;
	    	}
			catch(...)
			{
				unideb[Debug::CRIT] << myname << "(send): "
						<< conf->oind->getNameById( li->id ) 
						<< " ����������!!(...)" << endl;
			}	
			
//			li->ref = 0;
			li->ref=UniSetObject_i::_nil();
		}
	}
}
// ------------------------------------------------------------------------------------------
template <class TMessage>
void InfoServer::event(UniSetTypes::MessageCode key, TMessage& msg, CORBA::Boolean askn)
{
	{	// lock
		uniset_mutex_lock lock(askMutex, 1000);	

		// ��c���� ��������� �� ��������� ���� ������������
		AskMap::iterator it = askList.find(key);
		if( it!=askList.end() )
			send(it->second, msg,askn);
	}	// unlock 
}
// ------------------------------------------------------------------------------------------
