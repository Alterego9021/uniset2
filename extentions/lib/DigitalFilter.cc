// --------------------------------------------------------------------------
/*! $Id: DigitalFilter.cc,v 1.2 2009/01/16 23:16:42 vpashka Exp $ */
// --------------------------------------------------------------------------
#include <math.h>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include "UniSetTypes.h"
#include "DigitalFilter.h"
//--------------------------------------------------------------------------
using namespace UniSetTypes;
using namespace std;
//--------------------------------------------------------------------------
DigitalFilter::DigitalFilter( unsigned int bufsize, double T ):
	Ti(T),
	val(0),
	M(0),
	S(0),
	tmr(UniSetTimer::WaitUpTime),
	maxsize(bufsize),
	mvec(bufsize)
{
}
//--------------------------------------------------------------------------
DigitalFilter::~DigitalFilter()
{
}
//--------------------------------------------------------------------------
void DigitalFilter::setSettings( unsigned int bufsize, double T )
{
	Ti = T;
	maxsize = bufsize;
	if( maxsize < 1 )
		maxsize = 1;
	
	if( buf.size() > maxsize )
	{
		// ������� ������ (������) ��������
		int sub = buf.size() - maxsize;
		for( int i=0; i<sub; i++ )
			buf.erase( buf.begin() );
	}
}
//--------------------------------------------------------------------------
void DigitalFilter::init( int val )
{
	buf.clear();
	for( unsigned int i=0; i<maxsize; i++ )
		buf.push_back(val);

	tmr.reset();
	this->val = val;
}
//--------------------------------------------------------------------------
double DigitalFilter::firstLevel()
{
	// ������� ������� ��������������
	M=0;
	for( FIFOBuffer::iterator i=buf.begin(); i!=buf.end(); ++i )
		M = M + (*i);

	M = M/buf.size();

	// ������� ������������������ ����������
	S=0;
	double r=0;
	for( FIFOBuffer::iterator i=buf.begin(); i!=buf.end(); ++i )	
	{
		r = M-(*i);
		S = S + r*r;
	}

	S = S/buf.size();
	S = sqrt(S);

	if( S == 0 )
		return M;

	// ������� ������� �������������� ��� ����� ���������, ���������� ������� ����� ��������� ������������������
	int n = 0;
	double val = 0; // �������� ������� ��������
	for( FIFOBuffer::iterator i=buf.begin(); i!=buf.end(); ++i )	
	{
		if( fabs(M-(*i)) > S*2 )
		{
			val = val + (*i);
			n = n + 1;
		}
	}

	if( n==0 )
		return M;

	return ( val / n );
}
//--------------------------------------------------------------------------
int DigitalFilter::filterRC( int rawval )
{
	if( Ti<=0 )
		return rawval;

	return lroundf(secondLevel(rawval));
}
//--------------------------------------------------------------------------

double DigitalFilter::secondLevel( double rawval )
{
	if( Ti<=0 )
		return rawval;

	// �������� ����� � �������� ������ �������
	int dt = tmr.getCurrent();
	if( dt == 0 )
		return val;

	tmr.reset();

	// ���� ������� RC �������
	val = (rawval + Ti*val/dt) / (Ti/dt + 1);

	return val;
}
//--------------------------------------------------------------------------
int DigitalFilter::filter1( int newval )
{
	if( maxsize < 1 )
		return newval;

	add(newval);
	return lroundf(firstLevel());
}
//--------------------------------------------------------------------------
void DigitalFilter::add( int newval )
{
	// �������� ��������� �������� � �����
	// ������ ��� ���� ������ (FIFO)
	buf.push_back(newval);
	if( buf.size() > maxsize )
		buf.erase( buf.begin() );
}
//--------------------------------------------------------------------------
int DigitalFilter::current1()
{
	return lroundf(firstLevel());
}
//--------------------------------------------------------------------------
int DigitalFilter::currentRC()
{
	return lroundf(secondLevel(current1()));
}
//--------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const DigitalFilter& d )
{
	os << "(" << d.buf.size() << ")[";
	for( DigitalFilter::FIFOBuffer::const_iterator i=d.buf.begin(); i!=d.buf.end(); ++i )	
	{
		os << " " << setw(5) << (*i);
	}
	os << " ]";
	return os;
}
//--------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const DigitalFilter* d)
{
	return os << (*d);
}
//--------------------------------------------------------------------------
int DigitalFilter::median( int newval )
{
	if( maxsize < 1 )
		return newval;

	add(newval);

	// ???????? ? ??????
	FIFOBuffer::iterator it = buf.begin();
	for( unsigned int i=0; i<maxsize && it!=buf.end(); i++,it++ )
		mvec[i] = (*it);

	// ????????? ??????
	sort(mvec.begin(),mvec.end());

	return mvec[maxsize/2];
}
//--------------------------------------------------------------------------
int DigitalFilter::currentMedian()
{
	return mvec[maxsize/2];
}
//--------------------------------------------------------------------------
