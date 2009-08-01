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
 * \brief ���������� IONotifyController_i
 * \author Pavel Vainerman
 * \date  $Date: 2007/12/16 21:32:07 $
 * \version $Id: IONotifyController.h,v 1.23 2007/12/16 21:32:07 vpashka Exp $
*/
// -------------------------------------------------------------------------- 
#ifndef IONotifyController_H_
#define IONotifyController_H_
//---------------------------------------------------------------------------
#include <map>
#include <list>
#include <string>

#include "UniSetTypes.h"
#include "IOController_i.hh"
#include "IOController.h"

//---------------------------------------------------------------------------
class NCRestorer;
//---------------------------------------------------------------------------
/*! \class IONotifyController
 * \todo ������� ����������� ������� 
*/ 
class IONotifyController: 
	public IOController,
	public POA_IONotifyController_i
{
	public:
	
		IONotifyController(const std::string name, const std::string section, NCRestorer* dumper=0);
		IONotifyController(UniSetTypes::ObjectId id, NCRestorer* dumper=0);

	    virtual ~IONotifyController();

		virtual UniSetTypes::ObjectType getType(){ return UniSetTypes::getObjectType("IONotifyController"); }

		virtual void askSensor(const IOController_i::SensorInfo& si, const UniSetTypes::ConsumerInfo& ci, UniversalIO::UIOCommand cmd);
		virtual void askState(const IOController_i::SensorInfo& si, const UniSetTypes::ConsumerInfo& ci, UniversalIO::UIOCommand cmd);
		virtual void askValue(const IOController_i::SensorInfo& si, const UniSetTypes::ConsumerInfo& ci, UniversalIO::UIOCommand cmd);
		
		virtual void askThreshold(const IOController_i::SensorInfo& si, const UniSetTypes::ConsumerInfo& ci, 
									UniSetTypes::ThresholdId tid,
									CORBA::Long lowLimit, CORBA::Long hiLimit, CORBA::Long sensibility,
									UniversalIO::UIOCommand cmd );

		virtual void askOutput(const IOController_i::SensorInfo& si, const UniSetTypes::ConsumerInfo& ci, UniversalIO::UIOCommand cmd);

		virtual UniSetTypes::IDSeq* askSensorsSeq(const UniSetTypes::IDSeq& lst, 
													const UniSetTypes::ConsumerInfo& ci, UniversalIO::UIOCommand cmd);


		//  -----------------------------------------------
		typedef sigc::signal<void,UniSetTypes::SensorMessage*> ChangeSignal;
		ChangeSignal signal_change_state(){ return changeSignal; }

		//  -------------------- !!!!!!!!! ---------------------------------
		virtual IONotifyController_i::ThresholdsListSeq* getThresholdsList();


		/*! ���������� � ��������� */
		struct ConsumerInfoExt:
			public	UniSetTypes::ConsumerInfo
		{
			ConsumerInfoExt( const UniSetTypes::ConsumerInfo& ci,
							UniSetObject_i_ptr ref=0):
				UniSetTypes::ConsumerInfo(ci),
				ref(ref){}

			UniSetObject_i_var ref;
		};

		typedef std::list<ConsumerInfoExt> ConsumerList;

		/*! ���������� � ��������� �������� */
		struct ThresholdInfoExt:
			public IONotifyController_i::ThresholdInfo
		{
			ThresholdInfoExt( UniSetTypes::ThresholdId tid, CORBA::Long low, CORBA::Long hi, CORBA::Long sb,
								UniSetTypes::ObjectId _sid=UniSetTypes::DefaultObjectId,
								bool inv = false ):
			sid(_sid),
			inverse(inv)
			{
				id			= tid;
				hilimit		= hi;
				lowlimit	= low;
				sensibility = sb;
				state 		= IONotifyController_i::NormalThreshold;
			}

			ConsumerList clst;

			/*! ������������� ����������� �������
				���������� � ������ �������
			*/
			UniSetTypes::ObjectId sid;
			
			/*! �������� � ������ �������� 
				(��� ����������-�������� �������)
			*/
			IOController::DIOStateList::iterator itSID;
			
			/*! ��������� ������ */
			bool inverse; 
	
			inline bool operator== ( const ThresholdInfo& r ) const
			{
				return ((id == r.id) && 
						(hilimit == r.hilimit) && 
						(lowlimit == r.lowlimit) && 
						(sensibility == r.sensibility) );
			}
		};
		

		typedef std::list<ThresholdInfoExt> ThresholdExtList;

		/*! ������ ��� ������->������ ������������ */
		typedef std::map<UniSetTypes::KeyType,ConsumerList> AskMap;
		
		struct ThresholdsListInfo
		{
			ThresholdsListInfo(){}
			ThresholdsListInfo(	IOController_i::SensorInfo& si, ThresholdExtList& list, 
								UniversalIO::IOTypes t=UniversalIO::AnalogInput ):
				si(si),type(t),list(list){}
		
			IOController_i::SensorInfo si;
			AIOStateList::iterator ait;
			UniversalIO::IOTypes type;
			ThresholdExtList list;
		};
		
		/*! ������ ��� ������->������ ������� */
		typedef std::map<UniSetTypes::KeyType,ThresholdsListInfo> AskThresholdMap;

		virtual void localSaveValue( IOController::AIOStateList::iterator& it, 
										const IOController_i::SensorInfo& si,
										CORBA::Long newvalue, UniSetTypes::ObjectId sup_id );

		virtual void localSaveState( IOController::DIOStateList::iterator& it, 
										const IOController_i::SensorInfo& si,
										CORBA::Boolean newstate, UniSetTypes::ObjectId sup_id );

	  	virtual void localSetState( IOController::DIOStateList::iterator& it, 
										const IOController_i::SensorInfo& si,
										CORBA::Boolean newstate, UniSetTypes::ObjectId sup_id );

		virtual void localSetValue( IOController::AIOStateList::iterator& it, 
										const IOController_i::SensorInfo& si,
										CORBA::Long value, UniSetTypes::ObjectId sup_id );

	protected:
	    IONotifyController();
		virtual bool activateObject();

		// �������
		bool myAFilter(const UniAnalogIOInfo& ai, CORBA::Long newvalue, UniSetTypes::ObjectId sup_id);
		bool myDFilter(const UniDigitalIOInfo& ai, CORBA::Boolean newstate, UniSetTypes::ObjectId sup_id);

		//! ������� ���������� �� ��������� ��������� �������
		virtual void send(ConsumerList& lst, UniSetTypes::SensorMessage& sm);


		//! �������� ������������ ��������� ��������
		virtual void checkThreshold( AIOStateList::iterator& li, 
									const IOController_i::SensorInfo& si, bool send=true );

		//! ����� ���������� � ��������� �������
		ThresholdExtList::iterator findThreshold( UniSetTypes::KeyType k, UniSetTypes::ThresholdId tid );
		

		//! ���������� ���������� �� ��������� ��������� ������� � ����
		virtual void loggingInfo(UniSetTypes::SensorMessage& sm);

		/*! ���������� ������ ���������� 
			�� ��������� ������ dump, ���� �������� dumper.
		*/
		virtual void dumpOrdersList(const IOController_i::SensorInfo& si, const IONotifyController::ConsumerList& lst);

		/*! ���������� ������ ���������� ��������� ��������
			�� ��������� ������ dump, ���� �������� dumper.
		*/
		virtual void dumpThresholdList(const IOController_i::SensorInfo& si, const IONotifyController::ThresholdExtList& lst);

		/*! ������ dump-����� */
		virtual void readDump();

		/*! ���������� ������ ������������ �� ������� io */
		virtual void buildDependsList();

		NCRestorer* restorer;

		void onChangeUndefined( DependsList::iterator it, bool undefined );

		ChangeSignal changeSignal;

	private:
		friend class NCRestorer;

		//----------------------
		bool addConsumer(ConsumerList& lst, const UniSetTypes::ConsumerInfo& cons ); 	//!< �������� ����������� ���������
		bool removeConsumer(ConsumerList& lst, const UniSetTypes::ConsumerInfo& cons );	//!< ������� ����������� ���������
		
		//! ��������� ������ 
		void ask(AskMap& askLst, const IOController_i::SensorInfo& si, 
					const UniSetTypes::ConsumerInfo& ci, UniversalIO::UIOCommand cmd);		

 		/*! �������� ����� ����� ��� ������� */
		bool addThreshold(ThresholdExtList& lst, ThresholdInfoExt& ti, const UniSetTypes::ConsumerInfo& cons);
		/*! ������� ����� ��� ������� */
		bool removeThreshold(ThresholdExtList& lst, ThresholdInfoExt& ti, const UniSetTypes::ConsumerInfo& ci);


		AskMap askDIOList; /*!< ������ ������������ �� ���������� �������� */
		AskMap askAIOList; /*!< ������ ������������ �� ���������� �������� */
		AskThresholdMap askTMap; /*!< ������ ������� �� ���������� �������� */
		
		// ������
		AskMap askDOList; /*!< ������ ������������ �� ���������� ������� */
		AskMap askAOList; /*!< ������ ������������ �� ���������� ������� */
	
		/*! ����� ��� ������������ ����������� ������� � c����� ������������ ���������� �������� */
		UniSetTypes::uniset_mutex askDMutex; 
		/*! ����� ��� ������������ ����������� ������� � c����� ������������ ���������� �������� */
		UniSetTypes::uniset_mutex askAMutex;
		/*! ����� ��� ������������ ����������� ������� � c����� ������������ ��������� �������� */			
		UniSetTypes::uniset_mutex trshMutex;
		/*! ����� ��� ������������ ����������� ������� � c����� ������������ ���������� ������� */
		UniSetTypes::uniset_mutex askAOMutex;
		/*! ����� ��� ������������ ����������� ������� � c����� ������������ ���������� ������� */
		UniSetTypes::uniset_mutex askDOMutex;
};

#endif
