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
 * \brief ���������� IOController_i
 * \author Pavel Vainerman <pv>
 * \date $Date: 2008/11/29 21:24:25 $
 * \version $Id: IOController.h,v 1.28 2008/11/29 21:24:25 vpashka Exp $
*/
// -------------------------------------------------------------------------- 
#ifndef IOController_H_
#define IOController_H_
//---------------------------------------------------------------------------
#include <map>
#include <list>
#include <sigc++/sigc++.h>
#include "IOController_i.hh"
#include "UniSetTypes.h"
#include "ObjectsManager.h"
#include "Configuration.h"
#include "Mutex.h"
//---------------------------------------------------------------------------
/*! ���������� ���������� IOController-� */ 
class IOController: 
		public ObjectsManager,
		public POA_IOController_i
{
	public:
	
		IOController(const std::string name, const std::string section);
		IOController(UniSetTypes::ObjectId id);
		~IOController();

		virtual UniSetTypes::ObjectType getType(){ return UniSetTypes::getObjectType("IOController"); }

		virtual CORBA::Boolean getState( const IOController_i::SensorInfo& si );
	  	virtual CORBA::Long getValue( const IOController_i::SensorInfo& si );	

//     -------------------- !!!!!!!!! ---------------------------------
//		����������� ���������� i/o ������������
//		�� ��������� ������ ���������� ���� �������
	  	virtual void setState( const IOController_i::SensorInfo& si, CORBA::Boolean state,
								UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId );
		virtual void setValue( const IOController_i::SensorInfo& si, CORBA::Long value,
								UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId );

	  	virtual void fastSetState( const IOController_i::SensorInfo& si, CORBA::Boolean state,
								UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId );
		virtual void fastSetValue( const IOController_i::SensorInfo& si, CORBA::Long value,
								UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId );

//     ----------------------------------------------------------------

		/*! \warning �� ������� ��������, ��������������� �� ����� ������ */
		virtual void saveState(const IOController_i::SensorInfo& si, CORBA::Boolean state,
								UniversalIO::IOTypes type = UniversalIO::DigitalInput,
								UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId );

		virtual void fastSaveState(const IOController_i::SensorInfo& si, CORBA::Boolean state,
								UniversalIO::IOTypes type = UniversalIO::DigitalInput,
								UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId );

		/*! \warning �� ������� ��������, ��������������� �� ����� ������ */								
	    virtual void saveValue(const IOController_i::SensorInfo& si, CORBA::Long value,
								UniversalIO::IOTypes type = UniversalIO::AnalogInput,
								UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId );

	    virtual void fastSaveValue(const IOController_i::SensorInfo& si, CORBA::Long value,
								UniversalIO::IOTypes type = UniversalIO::AnalogInput,
								UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId );

		virtual void setUndefinedState(const IOController_i::SensorInfo& si, 
										CORBA::Boolean undefined, 
										UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId );


		virtual IOController_i::ASensorInfoSeq* getSensorSeq(const UniSetTypes::IDSeq& lst);
		virtual UniSetTypes::IDSeq* setOutputSeq(const IOController_i::OutSeq& lst, UniSetTypes::ObjectId sup_id);

//     ----------------------------------------------------------------
		virtual UniversalIO::IOTypes getIOType(const IOController_i::SensorInfo& si);		

		virtual IOController_i::ASensorInfoSeq* getAnalogSensorsMap();
		virtual IOController_i::DSensorInfoSeq* getDigitalSensorsMap();

		virtual IOController_i::DigitalIOInfo getDInfo(const IOController_i::SensorInfo& si);
		virtual IOController_i::AnalogIOInfo getAInfo(const IOController_i::SensorInfo& si);


		virtual CORBA::Long getRawValue(const IOController_i::SensorInfo& si);
		virtual void calibrate(const IOController_i::SensorInfo& si, 
									const IOController_i::CalibrateInfo& ci,
									UniSetTypes::ObjectId adminId );
		
		IOController_i::CalibrateInfo getCalibrateInfo(const IOController_i::SensorInfo& si);

		inline IOController_i::SensorInfo SensorInfo(UniSetTypes::ObjectId id, 
								UniSetTypes::ObjectId node=UniSetTypes::conf->getLocalNode())
		{
			IOController_i::SensorInfo si;
			si.id = id;
			si.node = node;
			return si;
		};

		UniSetTypes::Message::Priority getPriority(const IOController_i::SensorInfo& si, 
													UniversalIO::IOTypes type);
													
	public:

		struct DependsInfo;
		typedef std::list<DependsInfo> DependsList;

		/*! ���� ��� ����������� ������� ���������� ��� ��������� ��������� ������� 
			\param it 	- ��������� �� DependsList
			\param bool	- ������� ��������� undefined (TRUE|FALSE)
		*/
		typedef sigc::slot<void,DependsList::iterator,bool> DependsSlot;
		
		/*! \warning � ������ ���������� call-back ������� ������ ����!
			����� ����� ����� ������� �� ������ (���� AFilter � DFilter)
		*/
		void setDependsSlot( DependsSlot sl );
		void setBlockDependsSlot( DependsSlot sl );

		// ��������� ��� ����������� �������� ���������� �� ��������
		struct UniDigitalIOInfo:
			public IOController_i::DigitalIOInfo
		{
			UniDigitalIOInfo():any(0),dlst_lock(false),block_state(false)
				{ undefined = false; blocked=false; }
			virtual ~UniDigitalIOInfo(){}
			
			UniDigitalIOInfo(IOController_i::DigitalIOInfo& r);
			UniDigitalIOInfo(const IOController_i::DigitalIOInfo& r);
			UniDigitalIOInfo(IOController_i::DigitalIOInfo* r);

			UniDigitalIOInfo& operator=(IOController_i::DigitalIOInfo& r);
			UniDigitalIOInfo& operator=(IOController_i::DigitalIOInfo* r);
			const UniDigitalIOInfo& operator=(const IOController_i::DigitalIOInfo& r);
		
			void* any; 			/*!< ���������� ��� ����������� �������� ����� ���������� */
			
			DependsList dlst; 	/*!< ������ io ��������� �� ������� */
			bool dlst_lock;		/*!< ���� ����������� ������ �� ������� */
			bool block_state;

			UniSetTypes::uniset_spin_mutex val_lock; /*!< ���� ����������� ������ �� ��������� */
		};

		struct UniAnalogIOInfo:
			public IOController_i::AnalogIOInfo
		{
			UniAnalogIOInfo():any(0),dlst_lock(false),block_value(0)
				{ undefined = false; blocked=false; }
			virtual ~UniAnalogIOInfo(){}

			UniAnalogIOInfo(IOController_i::AnalogIOInfo& r);
			UniAnalogIOInfo(IOController_i::AnalogIOInfo* r);
			UniAnalogIOInfo(const IOController_i::AnalogIOInfo& r);

			UniAnalogIOInfo& operator=(IOController_i::AnalogIOInfo& r);
			const UniAnalogIOInfo& operator=(const IOController_i::AnalogIOInfo& r);
			UniAnalogIOInfo& operator=(IOController_i::AnalogIOInfo* r);
		
			void* any; 			/*!< ���������� ��� ����������� �������� ����� ���������� */
			DependsList dlst; 	/*!< ������ io ��������� �� ������� (��� ����������� ���� undefined) */
			bool dlst_lock; 	/*!< ���� ����������� ������ �� ������� */
			long block_value;

			UniSetTypes::uniset_spin_mutex val_lock; /*!< ���� ����������� ������ �� ��������� */
		};


		// ������� ������ �� �������� �������� (��� ��������� 'const')
		typedef std::map<UniSetTypes::KeyType, UniDigitalIOInfo> DIOStateList;
		typedef std::map<UniSetTypes::KeyType, UniAnalogIOInfo> AIOStateList;
	
		inline DIOStateList::iterator dioBegin(){ return dioList.begin(); }
		inline DIOStateList::iterator dioEnd(){ return dioList.end(); }
		inline DIOStateList::iterator dfind(UniSetTypes::KeyType k){ return dioList.find(k); }
		inline int dioCount(){ return dioList.size(); }		

		inline AIOStateList::iterator aioBegin(){ return aioList.begin(); }
		inline AIOStateList::iterator aioEnd(){ return aioList.end(); }
		inline AIOStateList::iterator afind(UniSetTypes::KeyType k){ return aioList.find(k); }
		inline int aioCount(){ return aioList.size(); }

		struct DependsInfo
		{
			DependsInfo( bool init=false );
			DependsInfo( IOController_i::SensorInfo& si,
						 DIOStateList::iterator& dit, AIOStateList::iterator& ait );
			
			IOController_i::SensorInfo si;
			DIOStateList::iterator dit;
			AIOStateList::iterator ait;
			bool block_invert;	/*!< �������������� ������ ��� ������������ */
			bool init;
		};

		// ������ � ��������� ����� ��������
		virtual void localSaveValue( AIOStateList::iterator& it, const IOController_i::SensorInfo& si,
										CORBA::Long newvalue, UniSetTypes::ObjectId sup_id );
		virtual void localSaveState( DIOStateList::iterator& it, const IOController_i::SensorInfo& si,
										CORBA::Boolean newstate, UniSetTypes::ObjectId sup_id );

	  	virtual void localSetState( DIOStateList::iterator& it, const IOController_i::SensorInfo& si,
										CORBA::Boolean newstate, UniSetTypes::ObjectId sup_id );

		virtual void localSetValue( AIOStateList::iterator& it, const IOController_i::SensorInfo& si,
										CORBA::Long value, UniSetTypes::ObjectId sup_id );

		virtual bool localGetState( DIOStateList::iterator& it, const IOController_i::SensorInfo& si );
	  	virtual long localGetValue( AIOStateList::iterator& it, const IOController_i::SensorInfo& si );
		

		/*! ������� ����������� �������� ��������̣����� ��������� ��� ���������� �������� 
			// ��� ���������� �������� ������������� ��� �������� ������� ���.
			// ��. ������ ����������� � ������� localSaveState
		*/
		virtual void localSetUndefinedState( AIOStateList::iterator& it, bool undefined,
												const IOController_i::SensorInfo& si );


	protected:
			// �������������� ��� ���������� ������ ����������� ��������
			virtual bool disactivateObject();
			virtual bool activateObject();

			/*! ����������� ��������, �� ���������� � ������� �������� ������ IOController */
		    virtual void sensorsRegistration(){};
			/*! �������� �� ����������� �������� �� ���������� � ������� �������� ������ IOController */
			virtual void sensorsUnregistration();
	
			/*! ����������� ����������� ������� 
				force=true - �� ��������� �� ������������ (�����������)
			*/
			void dsRegistration( const UniDigitalIOInfo&, bool force=false );
			

			/*! ����������� ����������� �������
				force=true - �� ��������� �� ������������ (�����������)
			*/
			void asRegistration( const UniAnalogIOInfo&, bool force=false );

			
			/*! �������������� ������� */
			void sUnRegistration(const IOController_i::SensorInfo& si);
			
			
			UniSetTypes::Message::Priority getMessagePriority(UniSetTypes::KeyType k, UniversalIO::IOTypes type);
			
			// ------------------------------
			inline IOController_i::DigitalIOInfo
					DigitalIOInfo(bool s, UniversalIO::IOTypes t, const IOController_i::SensorInfo& si, 
									UniSetTypes::Message::Priority p = UniSetTypes::Message::Medium,
									bool defval=false )
			{
				IOController_i::DigitalIOInfo di;
				di.si = si;
				di.state = s;
				di.real_state = s;
				di.type = t;
				di.priority = p;
				di.default_val = defval;
				di.blocked = false;
				return di;
			};

			inline IOController_i::AnalogIOInfo
				AnalogIOInfo(long v, UniversalIO::IOTypes t, const IOController_i::SensorInfo& si, 
								UniSetTypes::Message::Priority p = UniSetTypes::Message::Medium,
								long defval=0, IOController_i::CalibrateInfo* ci=0 )
			{
				IOController_i::AnalogIOInfo ai;
				ai.si = si;
				ai.type = t;
				ai.value = v;
				ai.priority = p;
				ai.default_val = defval;
				ai.real_value = v;
				ai.blocked = false;
				if( ci!=0 )
					ai.ci = *ci;
				else
				{
					ai.ci.minRaw = 0;
					ai.ci.maxRaw = 0;
					ai.ci.minCal = 0;
					ai.ci.maxCal = 0;
					ai.ci.sensibility = 0;
					ai.ci.precision = 0;
				}
				return ai;	
			};


			//! ���������� ���������� �� ��������� ��������� �������
			virtual void logging(UniSetTypes::SensorMessage& sm);
			
			//! ���������� ��������� ���� �������� � ��
			virtual void dumpToDB();


		IOController();	

		// ������ � ������ c ���������� ������ ��� �����
		DIOStateList::iterator mydioBegin();
		DIOStateList::iterator mydioEnd();
		AIOStateList::iterator myaioBegin();
		AIOStateList::iterator myaioEnd();
		AIOStateList::iterator myafind(UniSetTypes::KeyType k);
		DIOStateList::iterator mydfind(UniSetTypes::KeyType k);

		// --------------------------
		// ������������
		// 
		typedef sigc::slot<bool,const UniAnalogIOInfo&, CORBA::Long, UniSetTypes::ObjectId> AFilterSlot;
		typedef sigc::slot<bool,const UniDigitalIOInfo&, CORBA::Boolean, UniSetTypes::ObjectId> DFilterSlot;
		typedef std::list<AFilterSlot> AFilterSlotList;
		typedef std::list<DFilterSlot> DFilterSlotList;

		/*
			����������� ������� ������ ����������:
			TRUE - ���� �������� '����������'
			FALSE - ���� �������� �� �������� (�������������)
		
			������ �������������: 
				addAFilter( sigc::mem_fun(my,&MyClass::my_filter) );
		*/
		AFilterSlotList::iterator addAFilter( AFilterSlot sl, bool push_front=false );
		DFilterSlotList::iterator addDFilter( DFilterSlot sl, bool push_front=false );
		void eraseAFilter(AFilterSlotList::iterator& it);
		void eraseDFilter(DFilterSlotList::iterator& it);

		// ������ �������� �������� ��������
		bool checkDFilters( const UniDigitalIOInfo& ai, CORBA::Boolean newstate, UniSetTypes::ObjectId sup_id );
		bool checkAFilters( const UniAnalogIOInfo& ai, CORBA::Long& newvalue, UniSetTypes::ObjectId sup_id );

		inline bool afiltersEmpty(){ return afilters.empty(); }
		inline bool dfiltersEmpty(){ return dfilters.empty(); }
		inline int afiltersSize(){ return afilters.size(); }
		inline int dfiltersSize(){ return dfilters.size(); }

		// ---------------------------
		// note: ������� ���������� ����������!!!
		void updateDepends( IOController::DependsList& lst, bool undefined, bool& lock );
		void updateBlockDepends( IOController::DependsList& lst, bool blk_state, bool& lock );

		void setCheckLockValuePause( int msec );
		inline int getCheckLockValuePause(){ return checkLockValuePause; }
	
	private:		
		friend class AskDumper;
	
		DIOStateList dioList;	/*!< ������ � ������� ���������� ���������� ������/������� */
		AIOStateList aioList;	/*!< ������ � ������� ���������� ���������� ������/������� */
		
		UniSetTypes::uniset_mutex dioMutex;	/*!< ����� ��� ������������ ����������� ������� � dioList */
		UniSetTypes::uniset_mutex aioMutex; /*!< ����� ��� ������������ ����������� ������� � aioList */
		
		bool isPingDBServer;	// ���� ����� � DBServer-�� 

		AFilterSlotList afilters; /*!< ������ �������� ��� ���������� �������� */
		DFilterSlotList dfilters; /*!< ������ �������� ��� ���������� �������� */

		DependsSlot dslot; /*!< undefined depends slot */
		DependsSlot bslot; /*!< block depends slot */
		int checkLockValuePause;
};

#endif
