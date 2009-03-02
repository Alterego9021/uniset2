#include <iostream>
#include "Element.h"
// -------------------------------------------------------------------------
using namespace std;

// -------------------------------------------------------------------------
TAND::TAND(ElementID id, int num, bool st):
	TOR(id,num,st)
{
}

TAND::~TAND()
{
}
// -------------------------------------------------------------------------
void TAND::setIn( int num, bool state )
{
//	cout << this << ": input " << num << " set " << state << endl;
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
	// ��� ������� 'AND' �������� �� ������� 0
	for( InputList::iterator it=ins.begin(); it!=ins.end(); ++it )
	{
		if( !it->state )
		{
			myout = false;
			brk = true;
			break;
		}
	}
	
	if( !brk )
		myout = true;

	cout << this << ": myout " << myout << endl;	
	if( prev != myout )
		Element::setChildOut();
}
// -------------------------------------------------------------------------
