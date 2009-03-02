#ifndef SingleProcess_H_
#define SingleProcess_H_
// --------------------------------------------------------------------------
#include <string>
// --------------------------------------------------------------------------
/*! ������� ����� ��� ��������� ���������.
	������������ ���������� ���������� ��������,
	���� �� ��������...
*/
class SingleProcess
{
	public:
		SingleProcess();
		virtual ~SingleProcess();
	
	protected:
		virtual void term( int signo ){}

		static void set_signals( bool ask );

	private:

		static void terminated( int signo );
		static void finishterm( int signo );

};
// --------------------------------------------------------------------------
#endif // SingleProcess_H_
// --------------------------------------------------------------------------
