#include <sstream>
#include <iostream>
#include "Extensions.h"
#include "Element.h"
// -------------------------------------------------------------------------
using namespace std;
using namespace UniSetExtensions;
// -------------------------------------------------------------------------
TOR::TOR(ElementID id, unsigned int num, bool st):
	Element(id),
	myout(false)
{
	if( num != 0 )
	{
		// создаём заданное количество входов
		for( unsigned int i = 1; i <= num; i++ )
		{
			ins.push_front(InputInfo(i, st)); // addInput(i,st);

			if( st == true )
				myout = true;
		}
	}
}

TOR::~TOR()
{
}
// -------------------------------------------------------------------------
void TOR::setIn( int num, bool state )
{
	//    cout << getType() << "(" << myid << "):  input " << num << " set " << state << endl;

	for( auto& it : ins )
	{
		if( it.num == num )
		{
			if( it.state == state )
				return; // вход не менялся можно вообще прервать проверку

			it.state = state;
			break;
		}
	}

	bool prev = myout;
	bool brk = false; // признак досрочного завершения проверки

	// проверяем изменился ли выход
	// для тригера 'OR' проверка до первой единицы
	for( auto& it : ins )
	{
		if( it.state )
		{
			myout = true;
			brk = true;
			break;
		}
	}

	if( !brk )
		myout = false;

	dinfo << this << ": myout " << myout << endl;

	if( prev != myout )
		Element::setChildOut();
}
// -------------------------------------------------------------------------
