// $Id: DigitalFilter.h,v 1.1 2008/12/14 21:57:50 vpashka Exp $
//--------------------------------------------------------------------------
// �������� ������ � ����� (�����������) �������� ���������� �������
// ������ ������� ������� ��������� ��������� ��������, ���������� ����� ������
// ������ ������� - �������������� ���������� RC �������
// ����� ������ ������������ �������, ��� ���������� ��� �������������������
// ������ ������� FirstValue
//--------------------------------------------------------------------------
#ifndef DigitalFilter_H_
#define DigitalFilter_H_
//--------------------------------------------------------------------------
#include <list>
#include <vector>
#include <ostream>
#include "PassiveTimer.h"
//--------------------------------------------------------------------------
class DigitalFilter
{
	public:
		DigitalFilter ( unsigned int bufsize=5, double T=0 );
		~DigitalFilter ();
		
		// T <=0 - ��������� ������ ������� �������
		void setSettings( unsigned int bufsize, double T );

		// ���������� � �ޣ��� ����
		// �� ���� �������� ����� ��������
		// ������������ ������������� � �ޣ��� 
		// ���������� ��������...
		int filter1( int newValue );
		
		// RC-������
		int filterRC( int newVal );
		
		// ��������� ������
		int median( int newval );
		
		// �������� ������� ������������� ��������
		int current1();
		int currentRC();
		int currentMedian();

		// ������ �������� ��������� ��������
		void add( int newValue );

		void init( int val );
		
		// void init( list<int>& data );
	
		inline int size(){ return buf.size(); }

		inline double middle(){ return M; }
		inline double sko(){ return S; }

		friend std::ostream& operator<<(std::ostream& os, const DigitalFilter& d);
		friend std::ostream& operator<<(std::ostream& os, const DigitalFilter* d);
		
	private:

		// ������ ������� �������
		double firstLevel();
		// ������ ������� �������, �������������� ���������� RC �������
		double secondLevel( double val );

		double Ti;		// ���������� ������� ��� ��������������� ����� � ������������
		double val;		// ������� �������� ������ ������� �������
		double M;		// ������� ��������������
		double S;		// ������������������ ����������
		PassiveTimer tmr;
		
		typedef std::list<int> FIFOBuffer;
		FIFOBuffer buf;		
		unsigned int maxsize;
		
		typedef std::vector<int> MedianVector;
		MedianVector mvec;
};
//--------------------------------------------------------------------------
#endif // DigitalFilter_H_
//--------------------------------------------------------------------------
