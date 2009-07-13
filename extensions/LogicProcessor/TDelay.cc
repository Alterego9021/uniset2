#include <iostream>
#include "TDelay.h"
// -------------------------------------------------------------------------
using namespace std;
// -------------------------------------------------------------------------
TDelay::TDelay( Element::ElementID id, int delayMS, int inCount):
	Element(id),
	myout(false),
	delay(delayMS)
{
	if( inCount!=0 )
	{
		// ������� �������� ���������� ������
		for( int i=1;i<=inCount;i++ )
			ins.push_front(InputInfo(i,false)); // addInput(i,st);
	}
}

TDelay::~TDelay()
{
}
// -------------------------------------------------------------------------
void TDelay::setIn( int num, bool state )
{
	bool prev = myout;
	// ���������� �����
	if( !state )
	{
		pt.setTiming(0); // reset timer
		myout = false;
		cout << this << ": set " << myout << endl;	
		if( prev != myout )
			Element::setChildOut();
		return;
	}

//	if( state )
	
	// ���������� ��� ��������
	if( delay <= 0 )
	{
		pt.setTiming(0); // reset timer
		myout = true;
		cout << this << ": set " << myout << endl;	
		if( prev != myout )
			Element::setChildOut();
		return;
	}

	// ��������, ���� �ݣ �� ���������� ������
	if( !myout && !prev  ) // �.�. !myout && prev != myout
	{
		cout << this << ": set timer " << delay << " [msec]" << endl;	
		pt.setTiming(delay);
	}
}
// -------------------------------------------------------------------------
void TDelay::tick()
{
	if( pt.getInterval()!=0 && pt.checkTime() )
	{
		myout = true;
		pt.setTiming(0); // reset timer
		cout << getType() << "(" << myid << "): TIMER!!!! myout=" << myout << endl;	
		Element::setChildOut();
	}
}
// -------------------------------------------------------------------------
bool TDelay::getOut()
{ 
	return myout; 
}
// -------------------------------------------------------------------------
void TDelay::setDelay( int timeMS )
{
	delay = timeMS;
}
// -------------------------------------------------------------------------
