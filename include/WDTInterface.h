/* $Id: WDTInterface.h,v 1.1 2008/12/14 21:57:51 vpashka Exp $ */
//--------------------------------------------------------------------------
#ifndef WDTInterface_H_
#define WDTInterface_H_
/*
��� ���� ������� ����� GUI ������������ ������� wdt686.o � ������ /dev/wdt � ������� 208 � ������� 0
��� GUI ������������ i810-tco, � ������ /dev/watchdog ����������� ����� ����������(?)
*/
//--------------------------------------------------------------------------
#include <string>

class WDTInterface
{
	public:
		WDTInterface(const std::string dev);
		~WDTInterface();

		bool	ping();
		bool	stop();
	
	protected:
		const std::string dev;
};
//--------------------------------------------------------------------------
#endif 
