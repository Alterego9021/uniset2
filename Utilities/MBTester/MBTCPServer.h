/*! $Id: MBTCPServer.h,v 1.1 2008/11/22 23:22:23 vpashka Exp $ */
// -------------------------------------------------------------------------
#ifndef MBTCPServer_H_
#define MBTCPServer_H_
// -------------------------------------------------------------------------
//#include <map>
#include <string>
#include "modbus/ModbusTCPServerSlot.h"

// -------------------------------------------------------------------------
/*! ������ �� �������� ���������� MBTCPServer ��� ������������ */
class MBTCPServer
{
	public:
		MBTCPServer( ModbusRTU::ModbusAddr myaddr, const std::string inetaddr, int port=502, bool verbose=false );
		~MBTCPServer();

		inline void setVerbose( bool state )
		{
			verbose = state;
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


		/*! ��������� ModbusSlave ��� ������ �� RS */
		ModbusTCPServerSlot* sslot;
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
		
	private:
		
};
// -------------------------------------------------------------------------
#endif // MBTCPServer_H_
// -------------------------------------------------------------------------
