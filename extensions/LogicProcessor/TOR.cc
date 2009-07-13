#include <sstream>
#include <iostream>
#include "Element.h"
// -------------------------------------------------------------------------
using namespace std;

// -------------------------------------------------------------------------
TOR::TOR(ElementID id, int num, bool st):
	Element(id),
	myout(false)
{
	if( num!=0 )
	{
		// ������� �������� ���������� ������
		for( int i=1;i<=num;i++ )
		{
			ins.push_front(InputInfo(i,st)); // addInput(i,st);
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
//	cout << getType() << "(" << myid << "):  input " << num << " set " << state << endl;
	
	for( InputList::iterator it=ins.begin(); it!=ins.end(); ++it )
	{
		if( it->num == num )
		{
			if( it->state == state )
				return; // ���� �� ������� ����� ������ �������� ��������
			
			it->state = state;	
			break;
		}
	}

	bool prev = myout;
	bool brk = false; // ������� ���������� ���������� ��������

	// ��������� ��������� �� �����
	// ��� ������� 'OR' �������� �� ������ �������
	for( InputList::iterator it=ins.begin(); it!=ins.end(); ++it )
	{
		if( it->state )
		{
			myout = true;			
			brk = true;
			break;
		}
	}
	
	if( !brk )
		myout = false;

	cout << this << ": myout " << myout << endl;	
	if( prev != myout )
		Element::setChildOut();
}
// -------------------------------------------------------------------------
