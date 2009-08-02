// $Id: MBSlave.h,v 1.1 2009/01/11 19:08:45 vpashka Exp $
// -----------------------------------------------------------------------------
#ifndef _MBSlave_H_
#define _MBSlave_H_
// -----------------------------------------------------------------------------
#include <ostream>
#include <string>
#include <map>
#include <vector>
#include "UniSetObject_LT.h"
#include "modbus/ModbusTypes.h"
#include "modbus/ModbusServerSlot.h"
#include "PassiveTimer.h"
#include "Trigger.h"
#include "Mutex.h"
#include "SMInterface.h"
#include "SharedMemory.h"
#include "IOBase.h"
#include "ThreadCreator.h"
// -----------------------------------------------------------------------------
class MBSlave:
	public UniSetObject_LT
{
	public:
		MBSlave( UniSetTypes::ObjectId objId, UniSetTypes::ObjectId shmID, SharedMemory* ic=0, std::string prefix="mbs" );
		virtual ~MBSlave();
	
		/*! ���������� ������� ��� ������������� ������� */
		static MBSlave* init_mbslave( int argc, char* argv[], 
											UniSetTypes::ObjectId shmID, SharedMemory* ic=0,
											std::string prefix="mbs" );

		/*! ���������� ������� ��� ������ help-� */
		static void help_print( int argc, char* argv[] );

		static const int NoSafetyState=-1;

		enum AccessMode
		{
			amRW,
			amRO,
			amWO
		};

		struct IOProperty:
			public IOBase
		{
			ModbusRTU::ModbusData mbreg;			/*!< ������� */
			ModbusRTU::SlaveFunctionCode mbfunc;	/*!< ������� ��� ������/������ */
			AccessMode amode;

			IOProperty():	
				mbreg(0),mbfunc(ModbusRTU::fnUnknown)
			{}

			friend std::ostream& operator<<( std::ostream& os, IOProperty& p );
		};

		inline long getAskCount(){ return askCount; }

	protected:

		/*! ��������� 0x01 */
		ModbusRTU::mbErrCode readCoilStatus( ModbusRTU::ReadCoilMessage& query, 
													ModbusRTU::ReadCoilRetMessage& reply );
		/*! ��������� 0x02 */		
		ModbusRTU::mbErrCode readInputStatus( ModbusRTU::ReadInputStatusMessage& query, 
													ModbusRTU::ReadInputStatusRetMessage& reply );

		/*! ��������� 0x03 */
		ModbusRTU::mbErrCode readOutputRegisters( ModbusRTU::ReadOutputMessage& query, 
													ModbusRTU::ReadOutputRetMessage& reply );

		/*! ��������� 0x04 */
		ModbusRTU::mbErrCode readInputRegisters( ModbusRTU::ReadInputMessage& query, 
													ModbusRTU::ReadInputRetMessage& reply );

		/*! ��������� 0x05 */
		ModbusRTU::mbErrCode forceSingleCoil( ModbusRTU::ForceSingleCoilMessage& query, 
														ModbusRTU::ForceSingleCoilRetMessage& reply );

		/*! ��������� 0x0F */
		ModbusRTU::mbErrCode forceMultipleCoils( ModbusRTU::ForceCoilsMessage& query, 
													ModbusRTU::ForceCoilsRetMessage& reply );


		/*! ��������� 0x10 */
		ModbusRTU::mbErrCode writeOutputRegisters( ModbusRTU::WriteOutputMessage& query, 
														ModbusRTU::WriteOutputRetMessage& reply );

		/*! ��������� 0x06 */
		ModbusRTU::mbErrCode writeOutputSingleRegister( ModbusRTU::WriteSingleOutputMessage& query, 
														ModbusRTU::WriteSingleOutputRetMessage& reply );

		/*! ��������� �������� �� ������ ������ */
//		ModbusRTU::mbErrCode journalCommand( ModbusRTU::JournalCommandMessage& query, 
//															ModbusRTU::JournalCommandRetMessage& reply );

		/*! ��������� ������� �� ��������� ������� */
		ModbusRTU::mbErrCode setDateTime( ModbusRTU::SetDateTimeMessage& query, 
															ModbusRTU::SetDateTimeRetMessage& reply );

		/*! ��������� ������� ���̣����� ������� */
		ModbusRTU::mbErrCode remoteService( ModbusRTU::RemoteServiceMessage& query, 
															ModbusRTU::RemoteServiceRetMessage& reply );

		ModbusRTU::mbErrCode fileTransfer( ModbusRTU::FileTransferMessage& query, 
															ModbusRTU::FileTransferRetMessage& reply );


		/*! �������� ������������ �������� ����� �����������.
			���������� ��� ������� �������� �� �������� �� ������������ ������� (06 ��� 10)
		*/
		virtual ModbusRTU::mbErrCode checkRegister( ModbusRTU::ModbusData reg, ModbusRTU::ModbusData& val )
		{ return ModbusRTU::erNoError; }

		typedef std::map<ModbusRTU::ModbusData,IOProperty> IOMap;
		IOMap iomap;			/*!< ������ ������/������� */

		ModbusServerSlot* mbslot;
		ModbusRTU::ModbusAddr addr;			/*!< ����� ������� ���� */

		UniSetTypes::uniset_mutex mbMutex;

		xmlNode* cnode;
		std::string s_field;
		std::string s_fvalue;

		SMInterface* shm;

		virtual void processingMessage( UniSetTypes::VoidMessage *msg );
		void sysCommand( UniSetTypes::SystemMessage *msg );
		void sensorInfo( UniSetTypes::SensorMessage* sm );
		void askSensors( UniversalIO::UIOCommand cmd );	
		void waitSMReady();
		void execute_rtu();
		void execute_tcp();

		virtual bool activateObject();
		
		// �������� ��� ���������� ������
		virtual void sigterm( int signo );

		void initIterators();
		bool initItem( UniXML_iterator& it );
		bool readItem( UniXML& xml, UniXML_iterator& it, xmlNode* sec );

		void readConfiguration();
		bool check_item( UniXML_iterator& it );

		ModbusRTU::mbErrCode real_write( ModbusRTU::ModbusData reg, ModbusRTU::ModbusData val );
		ModbusRTU::mbErrCode real_read( ModbusRTU::ModbusData reg, ModbusRTU::ModbusData& val );

	private:
		MBSlave();
		bool initPause;
		UniSetTypes::uniset_mutex mutex_start;
		ThreadCreator<MBSlave>* thr;

		PassiveTimer ptHeartBeat;
		UniSetTypes::ObjectId sidHeartBeat;
		int maxHeartBeat;
		IOController::AIOStateList::iterator aitHeartBeat;
		UniSetTypes::ObjectId test_id;

		PassiveTimer ptTimeout;
		ModbusRTU::mbErrCode prev;
		long askCount;
		typedef std::map<ModbusRTU::mbErrCode,unsigned int> ExchangeErrorMap;
		ExchangeErrorMap errmap; 	/*!< ���������� ������ */
		
		bool activated;
		int activateTimeout;
		bool pingOK;
		int wait_msec;
		bool force;		/*!< ���� ����������, ��� ���� ��������� � SM, ���� ���� �������� �� �������� */

		bool mbregFromID;

		typedef std::map<int,std::string> FileList;
		FileList flist;
		std::string prefix;
};
// -----------------------------------------------------------------------------
#endif // _MBSlave_H_
// -----------------------------------------------------------------------------
