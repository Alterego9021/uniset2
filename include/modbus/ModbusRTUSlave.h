/*! $Id: ModbusRTUSlave.h,v 1.3 2009/02/24 20:27:25 vpashka Exp $ */
// -------------------------------------------------------------------------
#ifndef ModbusRTUSlave_H_
#define ModbusRTUSlave_H_
// -------------------------------------------------------------------------
#include <string>
#include "Mutex.h"
#include "Debug.h"
#include "Configuration.h"
#include "PassiveTimer.h"
#include "ComPort.h"
#include "ModbusTypes.h"
#include "ModbusServer.h"
// -------------------------------------------------------------------------
/*!	Modbus RTU slave mode  
	����� �� ��������������� � �������� "�����" ����������� �������
	��� ���������� ������� �� �������.

	\todo ���������� � ��� ��� �������� �� ������������ �������! 
		������ �������� ���������!!! ���� ������!!!
	\todo ���������� terminate, ����� ����� ���� �������� ��������
*/
class ModbusRTUSlave:
	public ModbusServer
{
	public:
		ModbusRTUSlave( const std::string dev, bool use485=false );
		ModbusRTUSlave( ComPort* com );
		virtual ~ModbusRTUSlave();
		
		void setSpeed( ComPort::Speed s );
		void setSpeed( const std::string s );
		ComPort::Speed getSpeed();

		virtual ModbusRTU::mbErrCode receive( ModbusRTU::ModbusAddr addr, int msecTimeout );
		
	protected:

		// realisation (see ModbusServer.h)
		virtual int getNextData( unsigned char* buf, int len );
		virtual void setChannelTimeout( int msec );
		virtual ModbusRTU::mbErrCode sendData( unsigned char* buf, int len );

		std::string dev;	/*!< ���������� */
		ComPort* port;		/*!< ���������� ��� ������ � COM-������ */
		bool myport;

	private:

};
// -------------------------------------------------------------------------
#endif // ModbusRTUSlave_H_
// -------------------------------------------------------------------------
