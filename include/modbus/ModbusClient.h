/*! $Id: ModbusClient.h,v 1.3 2009/01/11 19:08:46 vpashka Exp $ */
// -------------------------------------------------------------------------
#ifndef ModbusClient_H_
#define ModbusClient_H_
// -------------------------------------------------------------------------
#include <string>
#include "Mutex.h"
#include "Debug.h"
#include "Configuration.h"
#include "PassiveTimer.h"
#include "ModbusTypes.h"
// -------------------------------------------------------------------------
/*!	Modbus client (master) interface
*/
class ModbusClient
{
	public:

		ModbusClient();
		virtual ~ModbusClient();

		// ------------- Modbus-������� ----------------------------------------
		/*! ������ ������ ��������� (0x01) 
			\param addr - ����� slave-����
			\param start - ��������� ������� � �������� ������
			\param count - ������� ��������� ������ 
		*/
		ModbusRTU::ReadCoilRetMessage read01( ModbusRTU::ModbusAddr addr,
												ModbusRTU::ModbusData start, ModbusRTU::ModbusData count )
													throw(ModbusRTU::mbException);

		/*! ������ ������ ��������� (0x02) 
			\param addr - ����� slave-����
			\param start - ��������� ������� � �������� ������
			\param count - ������� ��������� ������ 
		*/
		ModbusRTU::ReadInputStatusRetMessage read02( ModbusRTU::ModbusAddr addr,
												ModbusRTU::ModbusData start, ModbusRTU::ModbusData count )
													throw(ModbusRTU::mbException);


		/*! ������ ������ ��������� (0x03) 
			\param addr - ����� slave-����
			\param start - ��������� ������� � �������� ������
			\param count - ������� ��������� ������ 
		*/
		ModbusRTU::ReadOutputRetMessage read03( ModbusRTU::ModbusAddr addr,
												ModbusRTU::ModbusData start, ModbusRTU::ModbusData count )
													throw(ModbusRTU::mbException);

		/*! ������ ������ ��������� (0x04) 
			\param addr - ����� slave-����
			\param start - ��������� ������� � �������� ������
			\param count - ������� ��������� ������ 
		*/
		ModbusRTU::ReadInputRetMessage read04( ModbusRTU::ModbusAddr addr,
												ModbusRTU::ModbusData start, ModbusRTU::ModbusData count )
													throw(ModbusRTU::mbException);

		/*! 0x05 
			\param addr - ����� slave-����
			\param reg - ������������ �������
			\param cmd - ������� ON | OFF
		*/
		ModbusRTU::ForceSingleCoilRetMessage write05( ModbusRTU::ModbusAddr addr,
															ModbusRTU::ModbusData reg, bool cmd )
																throw(ModbusRTU::mbException);

		/*! ������ ������ �������� (0x06) 
			\param addr - ����� slave-����
			\param reg - ������������ �������
			\param data	- ������
		*/
		ModbusRTU::WriteSingleOutputRetMessage write06( ModbusRTU::ModbusAddr addr,
															ModbusRTU::ModbusData reg, ModbusRTU::ModbusData data )
																throw(ModbusRTU::mbException);

		/*! ������ ������ ������� (0x0F) */
		ModbusRTU::ForceCoilsRetMessage write0F( ModbusRTU::ForceCoilsMessage& msg )
														throw(ModbusRTU::mbException);

		/*! ������ ������ ��������� (0x10) */
		ModbusRTU::WriteOutputRetMessage write10( ModbusRTU::WriteOutputMessage& msg )
														throw(ModbusRTU::mbException);


		/*! ���������� ��������� ����� (0x50)
			hour	- ���� [0..23]
			min		- ������ [0..59]
			sec		- ������� [0..59]
			day		- ���� [1..31]
			mon		- ����� [1..12]
			year	- ��� [0..99]
			century - �������� [19-20]
		*/
		ModbusRTU::SetDateTimeRetMessage setDateTime( ModbusRTU::ModbusAddr addr, 
							ModbusRTU::ModbusByte hour, ModbusRTU::ModbusByte min, ModbusRTU::ModbusByte sec,
							ModbusRTU::ModbusByte day, ModbusRTU::ModbusByte mon, ModbusRTU::ModbusByte year,
							ModbusRTU::ModbusByte century )
								throw(ModbusRTU::mbException);


		/*! ��������� ���� (0x66) 
			\param idFile - ������������� �����
			\param numpack - ����� ���������� �������������� ������
			\param save2filename - ��� �����, ��� ������� ����� �����Σ� ���������� ����
			\param part_timeout_msec - ������� �� ��������� ��������� ����� �����.
		*/
		ModbusRTU::FileTransferRetMessage partOfFileTransfer( ModbusRTU::ModbusAddr addr, ModbusRTU::ModbusData idFile, 
																ModbusRTU::ModbusData numpack, timeout_t part_timeout_msec=2000 )
																	throw(ModbusRTU::mbException);

		/*! ��������� ����
			\param idFile - ������������� �����
			\param save2filename - ��� �����, ��� ������� ����� �����Σ� ���������� ����
			\param part_timeout_msec - ������� �� ��������� ��������� ����� �����.
		*/
		void fileTransfer( ModbusRTU::ModbusAddr addr, ModbusRTU::ModbusData idFile, 
							const char* save2filename, timeout_t part_timeout_msec=2000 )
														throw(ModbusRTU::mbException);

		// ---------------------------------------------------------------------
		/*! ���������� ����� �������� �� ��������� */
		void setTimeout( timeout_t msec );
		
		/*! ��������� ����� ����� ������� �������
			\return ������ ��������
		*/
		int setAfterSendPause( timeout_t msec );

		void initLog( UniSetTypes::Configuration* conf, const std::string name, const std::string logfile="" );
		void setLog( DebugStream& dlog );


		inline void setCRCNoCheckit( bool set ){ crcNoCheckit = set; }
		inline bool isCRCNoCheckit(){ return crcNoCheckit; }

	protected:

		/*! get next data block from channel ot recv buffer 
			\param begin - get from position
			\param buf  - buffer for data
			\param len 	- size of buf
			\return real data lenght ( must be <= len ) 
		*/
		virtual int getNextData( unsigned char* buf, int len )=0;

		/*! set timeout for send/receive data */
		virtual void setChannelTimeout( timeout_t msec )=0;

		virtual ModbusRTU::mbErrCode sendData( unsigned char* buf, int len )=0;

		/*! ������� ������-����� */
		virtual ModbusRTU::mbErrCode query( ModbusRTU::ModbusAddr addr, ModbusRTU::ModbusMessage& msg, 
											ModbusRTU::ModbusMessage& reply, timeout_t timeout )=0;

		// -------------------------------------
		/*! ������� ������� */
		virtual ModbusRTU::mbErrCode send( ModbusRTU::ModbusMessage& msg );

		/*! ��������� ������ */
		virtual ModbusRTU::mbErrCode recv( ModbusRTU::ModbusAddr addr, ModbusRTU::ModbusByte qfunc, 
									ModbusRTU::ModbusMessage& rbuf, timeout_t timeout );

		virtual ModbusRTU::mbErrCode recv_pdu( ModbusRTU::ModbusByte qfunc, 
									ModbusRTU::ModbusMessage& rbuf, timeout_t timeout );



		ModbusRTU::ModbusMessage reply;	/*!< ����� ��� ��ɣ�� ��������� */
		ModbusRTU::ModbusMessage qbuf; 	/*!< ����� ��� ������� ��������� */

		timeout_t replyTimeOut_ms;	/*!< ������� �� �������� ������ */
		timeout_t aftersend_msec;		/*!< ����� ����� ������� ������� */
		
		bool crcNoCheckit;

		UniSetTypes::uniset_mutex sendMutex;
		DebugStream dlog;

		void printProcessingTime();
		PassiveTimer tmProcessing;

	private:
};
// -------------------------------------------------------------------------
#endif // ModbusClient_H_
// -------------------------------------------------------------------------
