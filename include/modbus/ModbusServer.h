/*! $Id: ModbusServer.h,v 1.3 2009/01/11 19:08:46 vpashka Exp $ */
// -------------------------------------------------------------------------
#ifndef ModbusServer_H_
#define ModbusServer_H_
// -------------------------------------------------------------------------
#include <string>
#include "Debug.h"
#include "Mutex.h"
#include "Configuration.h"
#include "PassiveTimer.h"
#include "ModbusTypes.h"
// -------------------------------------------------------------------------
/*!	Modbus server interface */
class ModbusServer
{
	public:
		ModbusServer();
		virtual ~ModbusServer();

		void initLog( UniSetTypes::Configuration* conf, const std::string name, const std::string logfile="" );
		void setLog( DebugStream& dlog );


		/*! ���������� ��������� ��������� 
			\param addr 		- ����� ��� �������� ��������� ���������
			\param msecTimeout 	- ����� �������� ������� ���������� ��������� � ����.
			\return ���������� ��� ������ �� ModbusRTU::mbErrCode
		*/
		virtual ModbusRTU::mbErrCode receive( ModbusRTU::ModbusAddr addr, int msecTimeout )=0;


		/*! ��������� ����� ����� ������� 
			\return ������ ��������
		*/
		int setAfterSendPause( int msec );

		/*! ��������� �������� �� ������������ ������
			\return ������ ��������
		*/
		int setReplyTimeout( int msec );

		/*! ���������� ����� �������� �� ��������� */
		void setRecvTimeout( int msec );

		inline void setCRCNoCheckit( bool set ){ crcNoCheckit = set; }
		inline bool isCRCNoCheckit(){ return crcNoCheckit; }

		inline void setBroadcastMode( bool set ){ onBroadcast = set; }
		inline bool getBroadcastMode(){ return onBroadcast; }


		/*! ��������������� ������� ����������� ��������� ������� �� ��������� �������.
			�������� �� ������������� gettimeofday � settimeofday.
		*/
		static ModbusRTU::mbErrCode replySetDateTime( ModbusRTU::SetDateTimeMessage& query, 
														ModbusRTU::SetDateTimeRetMessage& reply,
														DebugStream* dlog=0 );


		/*! ��������������� ������� ����������� ��������� �������� ����� 
			\param fname - ������������� ����.
			\param query - ������
			\param reply - �����
		*/
		static ModbusRTU::mbErrCode replyFileTransfer( const std::string fname, 
															ModbusRTU::FileTransferMessage& query, 
															ModbusRTU::FileTransferRetMessage& reply,
															DebugStream* dlog=0 );

	protected:

		/*! ��������� ������� �� ������ ������ (0x01).
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode readCoilStatus( ModbusRTU::ReadCoilMessage& query, 
															ModbusRTU::ReadCoilRetMessage& reply )=0;
		/*! ��������� ������� �� ������ ������ (0x02).
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode readInputStatus( ModbusRTU::ReadInputStatusMessage& query, 
															ModbusRTU::ReadInputStatusRetMessage& reply )=0;
	
		/*! ��������� ������� �� ������ ������ (0x03).
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode readOutputRegisters( ModbusRTU::ReadOutputMessage& query, 
															ModbusRTU::ReadOutputRetMessage& reply )=0;

		/*! ��������� ������� �� ������ ������ (0x04).
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode readInputRegisters( ModbusRTU::ReadInputMessage& query, 
															ModbusRTU::ReadInputRetMessage& reply )=0;

		/*! ��������� ������� �� ������ ������ (0x05).
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode forceSingleCoil( ModbusRTU::ForceSingleCoilMessage& query, 
														ModbusRTU::ForceSingleCoilRetMessage& reply )=0;


		/*! ��������� ������� �� ������ ������ (0x06).
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode writeOutputSingleRegister( ModbusRTU::WriteSingleOutputMessage& query, 
														ModbusRTU::WriteSingleOutputRetMessage& reply )=0;

		/*! ��������� ������� �� ������ ������ (0x0F).
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode forceMultipleCoils( ModbusRTU::ForceCoilsMessage& query, 
														ModbusRTU::ForceCoilsRetMessage& reply )=0;

		/*! ��������� ������� �� ������ ������ (0x10).
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode writeOutputRegisters( ModbusRTU::WriteOutputMessage& query, 
														ModbusRTU::WriteOutputRetMessage& reply )=0;


		/*! ��������� ������� �� ������� (0x65)
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode journalCommand( ModbusRTU::JournalCommandMessage& query, 
															ModbusRTU::JournalCommandRetMessage& reply )=0;


		/*! ��������� ������� �� ��������� ���� � ������� (0x50)
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode setDateTime( ModbusRTU::SetDateTimeMessage& query, 
															ModbusRTU::SetDateTimeRetMessage& reply )=0;


		/*! ����� ���̣����� ������� (0x53)
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode remoteService( ModbusRTU::RemoteServiceMessage& query, 
															ModbusRTU::RemoteServiceRetMessage& reply )=0;


		/*! �������� ����� (0x66)
			\param query - ������
			\param reply - �����. ����������� � �����������.
			\return ��������� ���������
		*/
		virtual ModbusRTU::mbErrCode fileTransfer( ModbusRTU::FileTransferMessage& query, 
															ModbusRTU::FileTransferRetMessage& reply )=0;

		/*! get next data block from channel ot recv buffer 
			\param begin - get from position
			\param buf  - buffer for data
			\param len 	- size of buf
			\return real data lenght ( must be <= len ) 
		*/
		virtual int getNextData( unsigned char* buf, int len )=0;
		
		virtual ModbusRTU::mbErrCode sendData( unsigned char* buf, int len )=0;
		

		/*! set timeout for receive data */
		virtual void setChannelTimeout( int msec )=0;

		/*! ������� ���������(�����) � ����� */
		virtual ModbusRTU::mbErrCode send( ModbusRTU::ModbusMessage& buf );

		virtual ModbusRTU::mbErrCode pre_send_request( ModbusRTU::ModbusMessage& request ){ return ModbusRTU::erNoError; }
		virtual ModbusRTU::mbErrCode post_send_request( ModbusRTU::ModbusMessage& request ){ return ModbusRTU::erNoError; }

		// default processing
		virtual ModbusRTU::mbErrCode processing( ModbusRTU::ModbusMessage& buf );

		/*! ������� ��������� �� ������ */
		ModbusRTU::mbErrCode recv( ModbusRTU::ModbusAddr addr, ModbusRTU::ModbusMessage& buf, int timeout );
		ModbusRTU::mbErrCode recv_pdu( ModbusRTU::ModbusMessage& rbuf, int timeout );

		UniSetTypes::uniset_mutex recvMutex;
		int recvTimeOut_ms;		/*!< ������� �� ��ɣ� */
		int replyTimeout_ms;	/*!< ������� �� ������������ ������ */
		int aftersend_msec;		/*!< ����� ����� ������� ������ */
		bool onBroadcast;		/*!< ������� ����� ������ � broadcst-����������� */
		bool crcNoCheckit;

		void printProcessingTime();
		PassiveTimer tmProcessing;

		DebugStream dlog;

	private:

};
// -------------------------------------------------------------------------
#endif // ModbusServer_H_
// -------------------------------------------------------------------------
