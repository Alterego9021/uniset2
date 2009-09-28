#ifndef _COMPORT_485F_H_
#define _COMPORT_485F_H_
// --------------------------------------------------------------------------
#include <queue>
#include "ComPort.h"
#include "PassiveTimer.h"
// --------------------------------------------------------------------------
/*!
	����� ��� ������ ����� 485 ��������� ���������� 
	��� ������������ ����� Fastwel.
	��������� ��ɣ��/������������. ������� "���" 
	������� ���������� � �����.

	kernel 2.6.12: 
		module 8250_pnp
		gpio_num=5 dev: /dev/ttyS2
		gpio_num=6 dev: /dev/ttyS3
*/
class ComPort485F:
	public ComPort
{
	public:

		ComPort485F( std::string comDevice, int gpio_num, bool tmit_ctrl=false );
	
		virtual void sendByte( unsigned char x );
		virtual void setTimeout( int timeout );
		virtual int sendBlock( unsigned char*msg,int len );

	protected:

		virtual unsigned char m_receiveByte( bool wait );
		void save2queue( unsigned char*msg, int len, int bnum );
		bool remove_echo( unsigned char tb[], int len );
		void m_read( int tmsec );

		/*! ������ ��������� ����� ��� ���������� ������ */
		unsigned char tbuf[ComPort::BufSize];

		std::queue<unsigned char> wq; /*!< ��������� ������ ���������� � ����� */
		std::queue<unsigned char> rq; /*!< ������� ��� ������ */

		int gpio_num;
		bool tmit_ctrl_on;
		PassiveTimer ptRecv;
		int tout_msec;
};
// --------------------------------------------------------------------------
#endif // _COMPORT_E_H_
// --------------------------------------------------------------------------
