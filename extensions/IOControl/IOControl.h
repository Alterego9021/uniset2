// $Id: IOControl.h,v 1.1 2008/12/14 21:57:50 vpashka Exp $
// -----------------------------------------------------------------------------
#ifndef IOControl_H_
#define IOControl_H_
// -----------------------------------------------------------------------------
#include <vector>
#include <list>
#include <string>
#include "UniXML.h"
#include "PassiveTimer.h"
#include "Trigger.h"
#include "IONotifyController.h"
#include "UniSetObject_LT.h"
#include "Mutex.h"
#include "MessageType.h"
#include "ComediInterface.h" 
#include "DigitalFilter.h" 
#include "Calibration.h" 
#include "SMInterface.h" 
#include "SingleProcess.h"
#include "IOController.h" 
#include "IOBase.h" 
#include "SharedMemory.h" 
// -----------------------------------------------------------------------------
/*!
      \page page_IOControl (IOControl) ���������� �������� �����/������
      
      - \ref sec_IOC_Comm
      - \ref sec_IOC_Conf
      - \ref sec_IOC_ConfList

      \section sec_IOC_Comm ����� �������� �������� �/�

      \section sec_IOC_Conf ���������������� �������� �/�

	--io-confnode name      - ������������ ��� ��������� ��������� xml-���� 
	--io-name name		- ID ��������. �� ��������� IOController1. 
	--io-numcards		- ���������� ���� �/�. �� ��������� 1. 
	--iodev0 dev		- ������������ ��� card='0' ��������� ���� comedi-����������. 
	--iodev1 dev		- ������������ ��� card='1' ��������� ���� comedi-����������. 
	--iodev2 dev		- ������������ ��� card='2' ��������� ���� comedi-����������. 
	--iodev3 dev		- ������������ ��� card='3' ��������� ���� comedi-����������. 
	--iodevX dev		- ������������ ��� card='X' ��������� ���� comedi-����������. 
	                          'X'  �� ������ ���� ������ --io-numcards 

	--iodevX-subdevX-type name	- ��������� ���� ������������� ��� UNIO.
	                             ���������: TBI0_24,TBI24_0,TBI16_8 

	--io-default_cardnum	- ����� ����� �� ���������. �� ��������� -1. 
	                             ���� ������, �� �� ����� ������������� ��� �������� 
	                             � ������� �� ������ ���� 'card'. 

	--io-test-lamp		- ��� ������� ���� � �������� ������� ������ '��������' ������������ ��������� ������. 
	--io-conf-field fname	- ��������� �� ����. ����� ��� ������� � ����� fname='1' 
	--io-polltime msec	- ����� ����� ������� ����. �� ��������� 200 ����. 
	--io-filtersize val	- ����������� ������� ��� ���������� ������. 
	--io-filterT val	- ���������� ������� �������. 
	--io-s-filter-field	- ������������� � configure.xml �� �������� ����������� ������ ����������� � ��� �������� �������� 
	--io-s-filter-value	- �������� �������������� �� �������� ����������� ������ ����������� � ��� �������� �������� 
	--io-blink-time msec	- ������� �������, ����. �� ��������� � configure.xml 
	--io-blink2-time msec	- ������ ������� ������� (lmpBLINK2), ����. �� ��������� � configure.xml 
	--io-blink3-time msec	- ������ ������� ������� (lmpBLINK3), ����. �� ��������� � configure.xml 
	--io-heartbeat-id	- ������ ������� ������ � ��������� ���������� heartbeat-�������. 
	--io-heartbeat-max  	- ������������ �������� heartbeat-�ޣ����� ��� ������� ��������. �� ��������� 10. 
	--io-ready-timeout	- ����� �������� ���������� SM � ������, ����. (-1 - ����� '�����')     
	--io-force		- ��������� �������� � SM, ���������� ��, ���� �������� �� �������� 
	--io-force-out		- ��������� ������ ������������� (�� �� ������) 
	--io-skip-init-output	- �� ���������������� '������' ��� ������ 
	--io-sm-ready-test-sid - ������������ ��������� ������, ��� �������� ���������� SharedMemory 

	\par ��������� ��������� �� ������� �����/������
	nofilter        - �� ������������ ������
	ioignore        - ������������ ������ ������ (��������� �������� ��������� ����/�����)
	ioinvert        - ��������������� ������ (��� DI,DO)
	default         - �������� �� ��������� (��� �������)
	noprecision     - ������������ ���� precision (�.�. ������� �/� �� ����� ��� ������������, 
	                  �� ����� ��� ��������� � SensorMessage)
	                  
	breaklim        - ��������� �������� ��� ����������� ������ ������� (������������ ��� AI).
	                  ���� �������� ���� ����� ������, �� ������������ ������� ������ �������.
	
	
	jardelay        - ������ �� ��������. �������� �� �������, ����.
	ondelay         - �������� �� ������������, ����.
	offdelay        - �������� �� ����������, ����.
	safety          - ���������� ��������. �������� ������� ����������� � ������ ���������� 
	                  ���������� ��������.
	iopriority      - ��������� ��� ������. 

	iotype          - ���  �����/������ [DI|DO|AI|AO]
	rmin            - ����������� "�����" ��������
	rmax            - ������������ "�����" ��������
	cmin            - ����������� "�������������" ��������
	cmax            - ������������ "�������������" ��������
	sensibility     - ���������������. (deprecated)
	precision       - ��������. ������ ��������� ������ ����� �������.
	                  �.�. ��� ���������� �� ������, �������� �����������
			  �� 10^precision � ��� ����� �����������.
			  � � SensorMessage ����������� ����������� precision.

	filtermedian    - �������� ��� "����������" �������
	filtersize      - �������� ��� "����������"
	filterT         - ���������� ������� �������. 
	caldiagram      - ��� ������������� ��������� �� ������ <Calibrations>.
	threshold_aid   - ������������� ����������� ������� �� �������� ����������� �����.
	                  ������������ ��� DI.
	lowlimit        - ������ ������ ������������.
	hilimit         - ������� ������ ������������.
	sensibility     - ���������������� (deprecated)
	
	card            - ����� �����
	subdev          - ����� �������������
	channel [ 0<>32 ] - ����� ������
        jack [ J1 | J2 | J3 | J4 | J5 ]  - �������� ���ߣ��. ����� �������� ������ channel
	       J1 - chanenel 0 - 15
	       J2
	lamp            - �������, ��� ������ ���������� ������ �������� "���������".
	                  �.�. �� ����� ���� ���������� �����, ������� ����� ����� ��� ���������
			  UniSetTypes::lmpOFF      - ��������
			  UniSetTypes::lmpON       - �������
			  UniSetTypes::lmpBLINK1   - ������� � �������� 1
			  UniSetTypes::lmpBLINK2   - ������� � �������� 2
			  UniSetTypes::lmpBLINK3   - ������� � �������� 3
			  
	no_iotestlamp  - ������������ ������ �������� ��� ����� ����. 
	range          - �������� ��������� ����������� ����� (��. libcomedi)
	aref           - ��� ����������� (��. libcomedi)

	\section sec_IOC_ConfList ������ �������� ��� �������� �/�
*/
// -----------------------------------------------------------------------------
#warning ������� ��������� �������� ����������....

class CardList:
	public std::vector<ComediInterface*>
{
	public:

		CardList(int size) : std::vector<ComediInterface*>(size) { }

		~CardList() {
			for( unsigned int i=0; i<size(); i++ )
				delete (*this)[i];
		}

		inline ComediInterface* getCard(int ncard) {
			if( ncard >= 0 && ncard < (int)size() )
				return (*this)[ncard];
			return NULL;
	}

};

/*! 
	������� ������ � ������� �/�.
	������:
	- ����� ���������� � ���������� ������, �������
	- �������� �� �������������
	- �������� �� ��������� (��� ���-�� ��������)
	- ������ �� ��������
	- ����������� ������������ ���������� ��������
	- ���������� ���������� ��������
	- �������������� ������ ���������� ��������
	- ����������� ����������� ��������� ������� (��� ��������� ����������)
	- ����������� ������ ������� (��� ���������� ��������)
	- ������� ����������
	- ���� ����
*/
class IOControl:
	public UniSetObject
{
	public:
		IOControl( UniSetTypes::ObjectId id, UniSetTypes::ObjectId icID, SharedMemory* ic=0, int numcards=2 );
		virtual ~IOControl();

		/*! ���������� ������� ��� ������������� ������� */
		static IOControl* init_iocontrol( int argc, const char* const* argv,
											UniSetTypes::ObjectId icID, SharedMemory* ic=0 );
		/*! ���������� ������� ��� ������ help-� */
		static void help_print( int argc, const char* const* argv );

//		inline std::string getName(){ return myname; }

		/*! ���������� � �����/������ */
		struct IOInfo:
			public IOBase
		{
			IOInfo():
				subdev(DefaultSubdev),channel(DefaultChannel),
				ncard(-1),
				aref(0),
				range(0),
				lamp(false),
				no_testlamp(false)
			{}


			int subdev;		/*!< (UNIO) ������������� (��. comedi_test ��� ���������� ����� �/�) */
			int channel;	/*!< (UNIO) ����� [0...23] */
			int ncard;		/*!< ����� ����� [1|2]. 0 - �� ����������. FIXME from Lav: -1 - �� ����������? */

			/*! ��� ����������
				0	- analog ref = analog ground
				1	- analog ref = analog common
				2	- analog ref = differential
				3	- analog ref = other (undefined)
			*/
			int aref;

			/*! ������������� ��������
				0	-  -10� - 10�
				1	-  -5� - 5�
				2	-  -2.5� - 2.5�
				3	-  -1.25� - 1.25�
			*/
			int range;

			bool lamp;		/*!< �������, ��� ������ ����� �������� ��������� (��� ��������������) */
			bool no_testlamp; /*!< ���� ���������� �� '�������� ����' */
			
			friend std::ostream& operator<<(std::ostream& os, IOInfo& inf );
		};

		struct IOPriority
		{
			IOPriority(int p, int i):
				priority(p),index(i){}
				
			int priority;
			int index;
		};

		void execute();

	protected:

		void iopoll(); /*!< ����� ���� �/� */
		void ioread( IOInfo* it );
		void check_testlamp();
		void blink();
	
		// �������� ��� ���������� ������
		virtual void processingMessage( UniSetTypes::VoidMessage* msg );
		virtual void sysCommand( UniSetTypes::SystemMessage* sm );
		virtual void askSensors( UniversalIO::UIOCommand cmd );
		virtual void sensorInfo( UniSetTypes::SensorMessage* sm );
		virtual void timerInfo( UniSetTypes::TimerMessage* tm );
		virtual void sigterm( int signo );
		virtual bool activateObject();
		
		// ��������� ������������� �������
		void initOutputs();

		// ������������� ����� (������� �/�)
		void initIOCard();

		// ������ ����� ������������
		void readConfiguration();
		bool initIOItem( UniXML_iterator& it );
		bool check_item( UniXML_iterator& it );
		bool readItem( UniXML& xml, UniXML_iterator& it, xmlNode* sec );

		void waitSM();

		bool checkCards( const std::string func="" );

//		std::string myname;
		xmlNode* cnode;			/*!< xml-���� � ����������� ����� */

		int polltime;			/*!< ������������� ���������� ������ (������ ���� �/�), [����] */
		CardList cards;			/*!< ������ ���� - ������ ��������� ComediInterface */
		bool noCards;


		typedef std::vector<IOInfo> IOMap;
		IOMap iomap;			/*!< ������ ������/������� */
		
		typedef std::list<IOPriority> PIOMap;
		PIOMap pmap;	/*!< ������ ������������ ������/������� */

		unsigned int maxItem;	/*!< ���������� ��������� (������������ �� ������ �������������) */
		unsigned int maxHalf;
		int filtersize;
		float filterT;

		std::string s_field;
		std::string s_fvalue;

		SMInterface* shm;
		UniversalInterface ui;
		UniSetTypes::ObjectId myid;

		typedef std::list<IOInfo*> BlinkList;

		void addBlink( IOInfo* it, BlinkList& lst );
		void delBlink( IOInfo* it, BlinkList& lst );
		void blink( BlinkList& lst, bool& bstate );

		// ������� �������
		BlinkList lstBlink;
		PassiveTimer ptBlink;
		bool blink_state;
		
		// ������� � ������� ��������
		BlinkList lstBlink2;
		PassiveTimer ptBlink2;
		bool blink2_state;

		// ������� � ������� ��������
		BlinkList lstBlink3;
		PassiveTimer ptBlink3;
		bool blink3_state;
		
		UniSetTypes::ObjectId testLamp_S;
		Trigger trTestLamp;
		bool isTestLamp;
		IOController::DIOStateList::iterator ditTestLamp;

		PassiveTimer ptHeartBeat;
		UniSetTypes::ObjectId sidHeartBeat;
		int maxHeartBeat;
		IOController::AIOStateList::iterator aitHeartBeat;

		bool force;			/*!< ����, ����������, ��� ���� ��������� � SM, ���� ���� �������� �� �������� */
		bool force_out;		/*!< ����, ���������� �������������� ������ ������� */
		int smReadyTimeout; 	/*!< ����� �������� ���������� SM � ������, ���� */
		int defCardNum;		/*!< ����� ����� �� ��������� */
		
		UniSetTypes::uniset_mutex iopollMutex;
		bool activated;
		bool readconf_ok;
		int activateTimeout;
		UniSetTypes::ObjectId sidTestSMReady;
		bool term;

	private:
};
// -----------------------------------------------------------------------------
#endif // IOControl_H_
// -----------------------------------------------------------------------------
