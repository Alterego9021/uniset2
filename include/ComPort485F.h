#ifndef COMPORT_485F_H_
#define COMPORT_485F_H_
// --------------------------------------------------------------------------
#include <queue>
#include "ComPort.h"
#include "PassiveTimer.h"
// --------------------------------------------------------------------------
/*!
    Класс для обмена через 485 интерфейс СПЕЦИАЛЬНО
    для контроллеров фирмы Fastwel.
    Управляет приёмо/передатчиком. Удаляет "эхо"
    посылок переданных в канал.

    kernel 2.6.12:
        module 8250_pnp
        gpio_num=5 dev: /dev/ttyS2
        gpio_num=6 dev: /dev/ttyS3
*/
class ComPort485F:
	public ComPort
{
	public:

		ComPort485F( const std::string& comDevice, int gpio_num, bool tmit_ctrl = false );

		virtual void sendByte( unsigned char x ) override;
		virtual void setTimeout( timeout_t timeout ) override;
		virtual size_t sendBlock( unsigned char* msg, size_t len ) override;

		virtual void cleanupChannel() override;
		virtual void reopen() override;

	protected:

		virtual unsigned char m_receiveByte( bool wait ) override;
		void save2queue( unsigned char* msg, int len, int bnum );
		bool remove_echo( unsigned char tb[], int len );
		void m_read( int tmsec );

		/*! просто временный буфер для считывания данных */
		unsigned char tbuf[ComPort::BufSize];

		std::queue<unsigned char> wq; /*!< хранилище байтов записанных в канал */
		std::queue<unsigned char> rq; /*!< очередь для чтения */

		int gpio_num;
		bool tmit_ctrl_on;
		PassiveTimer ptRecv;
		timeout_t tout_msec = { 2000 };
};
// --------------------------------------------------------------------------
#endif // COMPORT_485F_H_
// --------------------------------------------------------------------------
