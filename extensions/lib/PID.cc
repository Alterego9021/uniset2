// $Id: PID.cc,v 1.1 2008/12/14 21:57:50 vpashka Exp $
// -----------------------------------------------------------------------------
#include <iomanip>
#include "PID.h"
// -----------------------------------------------------------------------------
using namespace std;
// -----------------------------------------------------------------------------
PID::PID():
Y(0),Kc(0),
Ti(0),Td(0),
vlim(2.0),
d0(0),
d1(0),
d2(0),
prevTs(0)
{
	reset();
}

// -----------------------------------------------------------------------------

PID::~PID()
{

}

// -----------------------------------------------------------------------------
void PID::step( double X, double Z, double Ts )
{

	// ����� �� ������������� ������������ �� ������ ����
	// ������ �����ޣ� ������ �� ���������
//	d0 		= 1+Ts/Ti+Td/Ts;
//	d1 		= -1-2*Td/Ts;
//	d2 		= Td/Ts;

//	� ������ ��������� Td � Ts �� ����� recalc �������� "��������"(PIDControl)
	if( prevTs != Ts )
	{
		prevTs = Ts;
		recalc();
	}

	sub2 	= sub1;	// ������ 2 ���� �����
	sub1 	= sub;	// ������ 1 ��� �����
	sub 	= Z-X;	// ������� ������ 
					// NOTE: � �������������� ���� "�������"(X) - "��������"(Z),
					//	�� ��������� ������ Z-X (���������!)

	// ����������� ��������(���ޣ����) ��������
	Y = Y + Kc*( d0*sub + d1*sub1 + d2*sub2 );

	if( Y > vlim ) Y = vlim;
	else if ( Y < -vlim ) Y = -vlim;
}
// -----------------------------------------------------------------------------
void PID::reset()
{
	sub2 = sub1 = sub = Y = 0;
}
// -----------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& os, PID& p )
{
	return os << "Kc=" << std::setw(4) << p.Kc
				<< "  Ti=" << std::setw(4) << p.Ti
				<< "  Td=" << std::setw(4) << p.Td
				<< "  Y=" << std::setw(4) << p.Y
				<< "  vlim=" << std::setw(4) << p.vlim
				<< "  sub2=" << setw(4) << p.sub2
				<< "  sub1=" << setw(4) << p.sub1
				<< "  sub=" << setw(4) << p.sub;
}
// --------------------------------------------------------------------------

void PID::recalc()
{
//	d0 		= 1+prevTs/Ti+Td/prevTs;
//	d1 		= -1-2*Td/prevTs;
	d2 		= Td/prevTs;
	d1 		= -1-2*d2;
	d0 		= 1+prevTs/Ti+d2;
}
// --------------------------------------------------------------------------