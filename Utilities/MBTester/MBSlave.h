/*! $Id: MBSlave.h,v 1.6 2009/02/24 20:27:24 vpashka Exp $ */
// -------------------------------------------------------------------------
#ifndef MBSlave_H_
#define MBSlave_H_
// -------------------------------------------------------------------------
#include <map>
#include <string>
#include "modbus/ModbusRTUSlaveSlot.h"

// -------------------------------------------------------------------------
/*! ������ �� �������� ���������� MBSlave ��� ������������ */
class MBSlave
{
	public:
		MBSlave( ModbusRTU::ModbusAddr addr, const std::string dev, const std::string speed, bool use485=false );
		~MBSlave();

		inline void setVerbose( bool state )
		{
			verbose = state;
		}

		inline void setReply( long val )
		{
			replyVal = val;
		}

		void execute();	/*!< �������� ���� ������ */


		void setLog( DebugStream& dlog );

	protected:
		// �������� ��� ���������� ������
		void sigterm( int signo );

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
		ModbusRTU::mbErrCode journalCommand( ModbusRTU::JournalCommandMessage& query, 
															ModbusRTU::JournalCommandRetMessage& reply );

		/*! ��������� ������� �� ��������� ������� */
		ModbusRTU::mbErrCode setDateTime( ModbusRTU::SetDateTimeMessage& query, 
															ModbusRTU::SetDateTimeRetMessage& reply );

		/*! ��������� ������� ���̣����� ������� */
		ModbusRTU::mbErrCode remoteService( ModbusRTU::RemoteServiceMessage& query, 
															ModbusRTU::RemoteServiceRetMessage& reply );

		ModbusRTU::mbErrCode fileTransfer( ModbusRTU::FileTransferMessage& query, 
															ModbusRTU::FileTransferRetMessage& reply );


		/*! ��������� ModbusRTUSlave ��� ������ �� RS */
		ModbusRTUSlaveSlot* rscomm;
		ModbusRTU::ModbusAddr addr;			/*!< ����� ������� ���� */

		bool verbose;
#if 0		
		typedef std::map<ModbusRTU::mbErrCode,unsigned int> ExchangeErrorMap;
		ExchangeErrorMap errmap; 	/*!< ���������� ������ */
		ModbusRTU::mbErrCode prev;


		// ����� ���� �� ������� unsigned, �� ���������� ������� � ��� ����� 
		// ��� long. � ��� ����� ���������� � ������� � ���� ����������� �������
		long askCount;	/*!< ���������� �������� �������� */


		typedef std::map<int,std::string> FileList;
		FileList flist;
#endif 
		long replyVal;
	private:
		
};
// -------------------------------------------------------------------------
#endif // MBSlave_H_
// -------------------------------------------------------------------------
