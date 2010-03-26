/* $Id: SharedMemory.h,v 1.2 2009/01/22 02:11:24 vpashka Exp $ */
// -----------------------------------------------------------------------------
#ifndef SharedMemory_H_
#define SharedMemory_H_
// -----------------------------------------------------------------------------
#include <string>
#include <list>
#include "IONotifyController_LT.h"
#include "Mutex.h"
#include "PassiveTimer.h"
#include "NCRestorer.h"
#include "WDTInterface.h"
// -----------------------------------------------------------------------------

/*! \page page_SharedMemory ���������� ���������� ����� ���������� ������ (SharedMemory)


      \section sec_SM_Common ������ �������� �������� SharedMemory
      
	����� SharedMemory ��������� ����� ����� ������ IONotifyController.
	��� ������������ � �������� ��������� ��. \ref page_IONotifyController

	������ �������� SM:
	- \ref sec_SM_Conf
	- \ref sec_SM_Event
	- \ref sec_SM_HeartBeat
	- \ref sec_SM_History
	- \ref sec_SM_Pulsar
	- \ref sec_SM_DBLog

	\section sec_SM_Conf ����������� ������ �������������� ��������
	  SM ��������� ���������� ������ ��������, ������� �� ����� �������������
	  ��� ������ ������ ��������. ������ ����� ����� �������� ����������� ����
	  ��� ������ "����������"(consumer) �� ������� �������, � �����
	  ���� ��� ������������ ������ ������������(depends) �� ��������.
	  ��� ��� ��������� �������� � ��������� ������

	 \par ������������ ������ �������������� ��������
	  \code
	    --s-filter-field   - ������ ����������� ���� ��� ��������
	    --s-filter-value   - ������ �������� ������������ ����. �������������� ��������.
	  \endcode
	������ ����� ��������:
	\code
	  <sensors>
	    ...
	    <item id="12" name="Sensor12" textname="xxx" .... myfilter="m1" ...>
	      <consumers>
		    <item name="Consumer1" type="object" mycfilter="c1" .../>
	      </consumers>
	    </item>  
	    ...
	    <item id="121" name="Sensor121" textname="xxx" .... myfilter="m1" ...>
	      <consumers>
		    <item name="Consumer1" type="object" mycfilter="c1" .../>
	      </consumers>
	    </item>  
	    ...
	  </sensors>
	\endcode
	��� ����, ����� SM ���������������� �� ���� ������� 12 � 121 ����������
	������� \b --s-filter-field myfilter \b --s-filter-value m1.
	
	\par ������������ ���������� (consumer)
	  \code
	    --�-filter-field   - ������ ����������� ���� ��� ���������� (consumer)
	    --�-filter-value   - ������ �������� ������������ ����. �������������� ��������.
	  \endcode
	���� ������ \b --c-filter-field mycfilter \b --c-filter-value c1, �� ��� ��������
	SM ���ӣ� � ������ ����������, ������ �� �������, � ������� ����� ������ ����������
	\a mycfilter="c1".
	
	\par ������������ ������������ (depends)
	  \code
	    --d-filter-field   - ������ ����������� ���� ��� "������������" (depends)
	    --d-filter-value   - ������ �������� ������������ ����. �������������� ��������.
	  \endcode
	...���� �� �������... ���� ������ ������� ��� ��� �� ��������...
	
		
	\note ���� ���� \b --X-filter-value �� ������� ����� ��������� ��� �������(���������,�����������)
	� ������� ���� \b --X-filter-field �� ������.
	\note ���� �� ��������� ��������� \b --X-filter-field - �� � SM ����� �������� ���� ������ 
	��������(����������, ������������) �� ������ <sensors>.
	
	
	\section sec_SM_Event ����������� � �������� SM
	� SM ���������� �������� ����������� �������� ������ �������,
	������� ����������� ����������� � ������/�������� SM.
	�������� ������������ ������, �� �������� ����������� ������ ��������
	�������� �� ��������� ������ \b --e-filter. 
	�������� \b --e-startup-pause msec - ������ ����� ����� ������ SM,
	����� ������� �������� �������� ���������� �����������.
	����� ������� ������ �������� ���������� ��������� �����������,
	���������� ��� ���� � ���������������� ����� ������� ���� 
	\b evnt_xxx="1", ��� \b xxx - ��� �������� � �������� ��������� 
	\b --e-filter.
	������:
	\code
	<objects>
	  ...
	  <item id="2342" name="MyObject" ... evnt_myfilter="1" ../>
	</objects>
	\endcode
	��� ��� � ���������� ������ SM ������ ���� ������� \b --e-filter myfilter.
	����� ��� ��ϣ� ������ SM ����̣� ������� � ��������������� 2342 �����������.
	
	� �������� ����������� �������� ����������� ��������� \b SystemMessage::WatchDog.

		
	\section sec_SM_HeartBeat �������� �� "��������" ��������
	
	  �������� �� ������������ ��������� (��������� ����������� ��������),
	  � ����� ����������� ���������� �������� "�������" ���������.
	  ...
	  ...
	  

	\section sec_SM_History �������� ���������� �����
	"��������� ����" ������������ �� ���� ����� ����������� �������
	(������ � ���������� ����� �������� �������� ����� ����. ����),
	� ������� ����������� ������� ��������� ��������� ������ ��������.
	� �������� "����������" �������� ������������� ������� (����
	��� ���������� ������, �� �������� ��������) ��� ������� ����������� ���������
	���� ������ "������������". �� ���������� ������������ ����� (��� "������")
	�������� �����������, ������� ����� ������������ ��� ������� �������������
	� ������� SharedMemory::signal_history(). ���������� ����������� �������
	�� ����������, ������, � ����� ������ �������� �� ������� ��ģ��� "�������"
	����� �� ����������.
	��������� ���������� ����� �������������� ����� ����. ����. ��� �����
	� ����������� ������ ������� SharedMemory ������ ���� ������� ���������
	"<History>".
	������:
	� ������ ������� �������� ��� "�������".
       \code
	<SharedMemory name="SharedMemory" shmID="SharedMemory">
		<History savetime="200">
			<item id="1" fuse_id="AlarmFuse1_S" fuse_invert="1" size="30" filter="a1"/>
			<item id="2" fuse_id="AlarmFuse2_AS" fuse_value="2" size="30" filter="a2"/>
		</History>
	</SharedMemory>	
       \endcode
       ���:
       \code
       savetime     - ������ ������������ ���������� ����� �������, � ����.
       id           - ������ (����������) ������������� "�������"
       fuse_id      - ������������� ������� "����������"
       fuse_value   - �������� ������������ (��� ����������� "����������")
       fuse_invert  - ������������ (��� ���������� "�����������"). 
                      �.�. ����������� �� �������� "0".
       
       size         - ���������� ����� � �������� �������
       filter       - ���� ������������ � �������� �������, ������������� �������
                      �������� � ������ �������. 
	\endcode 
       
       ������ ������ ����� ������� � ����� ���������� �������.
       
       �������� ������������� �� ��������� ������:
       
       ��� ������� ���������� ���������� ���������� ������ <History>
       � ���������� ��������������� �������� ��������. ��� ���� ����������
       ������ �� ������ <sensors> � ���� ����������� "�� ������" ���� �������� 
       � �������� ������� (\b filter), ������ ���������� � ��������������� �������.
       
       ����� ������ \b savetime ���� ���������� ������ ��������� ����� �������.
       ��� ���� � ����� ������ ����������� ����� (�������) �������� �������,
       � ���� ���������� ���������, ��� ����� ������ �������������� ����� �� �����
       \b size �����.
       
       ������ ����� � �������� ��������� ��������  (saveXXX, setXXX) �������������
       ��������� ��������� "�����������". ���� ����������� ������� ������� ���
       "������" ����� ������������ ������, � ������� ���������� ������������� �������
       � ������� ����������� ����������.
       
       \section sec_SM_Pulsar "�������" ����������� ��������
       � SM ���������� �������� ����������� ������ ����������� ���������� ������ ("�������"),
       ������� ����� � �������� �������� ������ ��ϣ ���������. ������������� �������
       �������� � ����������� ������ ���������� \b pulsar_id ��� �� ��������� ������,
       ���������� \b --pulsar-id. ������ ������� �������� ���������� \b pulsar_msec
       ��� � ��������� ������ \b --pulsar-msec. � �������� ����������� ������� �����
       ������ ����� ������ ���� DO ��� DI. �������� ������������ ��� ��������� �������
       \b pulsar_iotype ��� � ��������� ������ \b --pulsar-iotype.
       
       \section sec_SM_DBLog ���������� ����������� ��������� �������� � ��
       ��� �����������, �� ��������� � SM ��������� ���������� ������� ��������� �������� � ��
       (������������� � ������� ������ IONotifyController).
       �������� ��������� ������ \b --db-logging 1 ��������� �������� ���� ��������
       (� ���� ������� �� ������� ��������� ���������).
   
*/
class SharedMemory:
	public IONotifyController_LT
{
	public:
		SharedMemory( UniSetTypes::ObjectId id, std::string datafile );
		virtual ~SharedMemory();

		/*! ���������� ������� ��� ������������� ������� */
		static SharedMemory* init_smemory( int argc, const char* const* argv );
		/*! ���������� ������� ��� ������ help-� */
		static void help_print( int argc, const char* const* argv );

		// ������� ���������� "����������" SM � ������.
		// ������ �������������� ������� ����������, ��� ����, 
		// ����� ������, ����� ����� �������� �� SM ������.
		virtual CORBA::Boolean exist();

		void addReadItem( Restorer_XML::ReaderSlot sl );
		

		// ------------  HISTORY  --------------------
		typedef std::list<long> HBuffer;
		
		struct HistoryItem
		{
			HistoryItem():id(UniSetTypes::DefaultObjectId){}

			UniSetTypes::ObjectId id;
			HBuffer buf;

			AIOStateList::iterator ait;
			DIOStateList::iterator dit;

			void add( long val, size_t size )
			{
				buf.push_back(val);
				if( buf.size() >= size )
					buf.erase(buf.begin());
			}
		};

		typedef std::list<HistoryItem> HistoryList;

		struct HistoryInfo
		{
			HistoryInfo():
				id(0),
				size(0),filter(""),
				fuse_id(UniSetTypes::DefaultObjectId),
				fuse_invert(false),fuse_use_val(false),fuse_val(0){}
			
			long id;						// ID
			HistoryList hlst;				// history list
			int size;
			std::string filter;		// filter field
			UniSetTypes::ObjectId fuse_id; 	// fuse sesnsor
			bool fuse_invert;
			bool fuse_use_val;
			long fuse_val;
		};
		
		friend std::ostream& operator<<( std::ostream& os, const HistoryInfo& h );
		
		typedef std::list<HistoryInfo> History;

		typedef sigc::signal<void,HistoryInfo*> HistorySlot;
		HistorySlot signal_history(); /*!< ������ � ������������ ������� "������" ����� ������� */
		
	protected:
		typedef std::list<Restorer_XML::ReaderSlot> ReadSlotList;
		ReadSlotList lstRSlot;

		virtual void processingMessage( UniSetTypes::VoidMessage *msg );
		virtual void sysCommand( UniSetTypes::SystemMessage *sm );
		virtual void sensorInfo( UniSetTypes::SensorMessage *sm );
		virtual void timerInfo( UniSetTypes::TimerMessage *tm );
		virtual void askSensors( UniversalIO::UIOCommand cmd );
		virtual void sendEvent( UniSetTypes::SystemMessage& sm );

		virtual void localSaveValue( AIOStateList::iterator& it, const IOController_i::SensorInfo& si,
										CORBA::Long newvalue, UniSetTypes::ObjectId sup_id );
		virtual void localSaveState( DIOStateList::iterator& it, const IOController_i::SensorInfo& si,
										CORBA::Boolean newstate, UniSetTypes::ObjectId sup_id );
	  	virtual void localSetState( DIOStateList::iterator& it, const IOController_i::SensorInfo& si,
										CORBA::Boolean newstate, UniSetTypes::ObjectId sup_id );
		virtual void localSetValue( AIOStateList::iterator& it, const IOController_i::SensorInfo& si,
										CORBA::Long value, UniSetTypes::ObjectId sup_id );


		// �������� ��� ���������� ������
		virtual void sigterm( int signo );
		bool activateObject();
//		virtual void logging(UniSetTypes::SensorMessage& sm){}
//		virtual void dumpToDB(){}
		bool readItem( UniXML& xml, UniXML_iterator& it, xmlNode* sec );

	
		void buildEventList( xmlNode* cnode );
		void readEventList( std::string oname );
		
		UniSetTypes::uniset_mutex mutex_start;
		
		struct HeartBeatInfo
		{
			HeartBeatInfo():
				a_sid(UniSetTypes::DefaultObjectId),
				d_sid(UniSetTypes::DefaultObjectId),
				reboot_msec(UniSetTimer::WaitUpTime),
				timer_running(false),
				ptReboot(UniSetTimer::WaitUpTime)
			{}
			
			UniSetTypes::ObjectId a_sid; // ���������� �ޣ����
			UniSetTypes::ObjectId d_sid; // ���������� ������ ��������� ��������
			AIOStateList::iterator ait;
			DIOStateList::iterator dit;

			int reboot_msec; /*!< ����� � ������� ��������, ������� ������ ����������� ��ϣ �������������,
								����� ����� ����������� ������������ ����������� �� WDT (� ������ ���� �� ����ޣ�).
								���� ������ �������� �� ���������, �� "�� �������" �������� ������ ������������
								(�.�. ������ ����������� �� GUI). */

			bool timer_running;
			PassiveTimer ptReboot;
		};

		enum Timers
		{
			tmHeartBeatCheck,
			tmEvent,
			tmHistory,
			tmPulsar
		};
		
		int heartbeatCheckTime;
		std::string heartbeat_node;
		int histSaveTime;
		
		void checkHeartBeat();

		typedef std::list<HeartBeatInfo> HeartBeatList;
		HeartBeatList hlist; // ������ �������� "������������"
		UniSetTypes::uniset_mutex hbmutex;
		WDTInterface* wdt;
		bool activated;
		bool workready;

		typedef std::list<UniSetTypes::ObjectId> EventList;
		EventList elst;
		std::string e_filter;
		int evntPause;
		int activateTimeout;

		virtual void loggingInfo(UniSetTypes::SensorMessage& sm);
		virtual void dumpOrdersList(const IOController_i::SensorInfo& si, const IONotifyController::ConsumerList& lst){}
		virtual void dumpThresholdList(const IOController_i::SensorInfo& si, const IONotifyController::ThresholdExtList& lst){}

		bool dblogging;

		History hist;

		virtual void updateHistory( UniSetTypes::SensorMessage* sm );
		virtual void saveHistory();

		void buildHistoryList( xmlNode* cnode );
		void checkHistoryFilter( UniXML_iterator& it );


		DIOStateList::iterator ditPulsar;
		IOController_i::SensorInfo siPulsar;
		UniversalIO::IOTypes iotypePulsar;
		int msecPulsar;

	private:
		HistorySlot m_historySignal;
};
// -----------------------------------------------------------------------------
#endif // SharedMemory_H_
// -----------------------------------------------------------------------------
