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
		DigitalFilter ( unsigned int bufsize=5, double T=0, double lsq=0.2,
		                int iir_thr=10000, double iir_coeff_prev=0.5,
		                double iir_coeff_new=0.5 );
		~DigitalFilter ();
		
		// T <=0 - ��������� ������ ������� �������
		void setSettings( unsigned int bufsize, double T, double lsq,
		                  int iir_thr, double iir_coeff_prev, double iir_coeff_new );

		// ���������� � �ޣ��� ����
		// �� ���� �������� ����� ��������
		// ������������ ������������� � �ޣ��� 
		// ���������� ��������...
		int filter1( int newValue );
		
		// RC-������
		int filterRC( int newVal );
		
		// ��������� ������
		int median( int newval );
		
		// ���������� ������ �� ����� ���������� ���������
		int leastsqr( int newval );

		// ����������� ������
		int filterIIR( int newval );

		// �������� ������� ������������� ��������
		int current1();
		int currentRC();
		int currentMedian();
		int currentLS();
		int currentIIR();

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

		typedef std::vector<double> Coeff;
		Coeff w;        // ������ ������������� ��� filterIIR

		double lsparam; // �������� ��� filterIIR
		double ls;      // ��������� ��������, ������ݣ���� filterIIR

		int thr;        // ����� ��� ���������, �������������� ����������� ��������
		int prev;       // ��������� ��������, ������ݣ���� ����������� ��������

		// ������������ ��� ������������ �������
		double coeff_prev;
		double coeff_new;
};
//--------------------------------------------------------------------------
#endif // DigitalFilter_H_
//--------------------------------------------------------------------------
