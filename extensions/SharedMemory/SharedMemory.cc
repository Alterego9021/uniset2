#include <iomanip>
#include <sstream>
#include "UniXML.h"
#include "NCRestorer.h"
#include "SharedMemory.h"
#include "Extensions.h"
// -----------------------------------------------------------------------------
using namespace std;
using namespace UniSetTypes;
using namespace UniSetExtensions;
// -----------------------------------------------------------------------------
SharedMemory::SharedMemory( ObjectId id, string datafile ):
	IONotifyController_LT(id),
	heartbeatCheckTime(5000),
	histSaveTime(200),
	wdt(0),
	activated(false),
	workready(false),
	dblogging(false)
{
	cout << "$Id: SharedMemory.cc,v 1.4 2009/01/24 11:20:19 vpashka Exp $" << endl;

	xmlNode* cnode = conf->getNode("SharedMemory");
	if( cnode == NULL )
		throw SystemError("Not find conf-node for SharedMemory");

	UniXML_iterator it(cnode);

	// ----------------------
	buildHistoryList(cnode);
	signal_change_state().connect(sigc::mem_fun(*this, &SharedMemory::updateHistory));
	// ----------------------
	restorer = NULL;
	NCRestorer_XML* rxml = new NCRestorer_XML(datafile);
	
	string s_field = conf->getArgParam("--s-filter-field");
	string s_fvalue = conf->getArgParam("--s-filter-value");
	string c_field = conf->getArgParam("--c-filter-field");
	string c_fvalue = conf->getArgParam("--c-filter-value");
	string d_field = conf->getArgParam("--d-filter-field");
	string d_fvalue = conf->getArgParam("--d-filter-value");
	
	int lock_msec = atoi(conf->getArgParam("--lock-value-pause").c_str());
	if( lock_msec < 0 )
		lock_msec = 0;
	setCheckLockValuePause(lock_msec);
	
	heartbeat_node = conf->getArgParam("--heartbeat-node");
	if( heartbeat_node.empty() )
		dlog[Debug::WARN] << myname << "(init): --heartbeat-node NULL ===> heartbeat NOT USED..." << endl;
	else
		dlog[Debug::INFO] << myname << "(init): heartbeat-node: " << heartbeat_node << endl;

	heartbeatCheckTime = atoi(conf->getArgParam("--heartbeat-check-time","1000").c_str());

	
//	rxml->setSensorFilter(s_filterField, s_filterValue);
//#warning ��������� ��������� ��������� ������ ���������� (� ������ �������)...
	// ��� ���������� ������ ������ �������������� ���� ��� �������
//	rxml->setConsumerFilter("dummy","yes");

	rxml->setItemFilter(s_field, s_fvalue);
	rxml->setConsumerFilter(c_field, c_fvalue);
	rxml->setDependsFilter(d_field, d_fvalue);

	restorer = rxml;

	rxml->setReadItem( sigc::mem_fun(this,&SharedMemory::readItem) );
	
	string wdt_dev = conf->getArgParam("--wdt-device");
	if( !wdt_dev.empty() )
		wdt = new WDTInterface(wdt_dev);
	else
		dlog[Debug::WARN] << myname << "(init): watchdog timer NOT USED (--wdt-device NULL)" << endl;


	dblogging = atoi(conf->getArgParam("--db-logging").c_str());

	e_filter = conf->getArgParam("--e-filter");
	buildEventList(cnode);

	evntPause = atoi(conf->getArgParam("--e-startup-pause").c_str());
	if( evntPause<=0 )
		evntPause = 5000;
		
	activateTimeout	= atoi(conf->getArgParam("--activate-timeout").c_str());
	if( activateTimeout<=0 )
		activateTimeout = 10000;
}

// --------------------------------------------------------------------------------

SharedMemory::~SharedMemory()
{
	if( restorer )
	{
		delete restorer;
		restorer = NULL;
	}
	
	delete wdt;
}

// --------------------------------------------------------------------------------
void SharedMemory::processingMessage( UniSetTypes::VoidMessage *msg )
{
	try
	{
		switch( msg->type )
		{
			case Message::SensorInfo:
			{
				SensorMessage sm( msg );
				sensorInfo( &sm );
				break;
			}

			case Message::SysCommand:
			{
				SystemMessage sm( msg );
				sysCommand( &sm );
				break;
			}

			case Message::Timer:
			{
				TimerMessage tm(msg);
				timerInfo(&tm);
				break;
			}


			default:
				//dlog[Debug::WARN] << myname << ": ����������� ���������  " << msg->type << endl;
				break;
		}	
	}
	catch( Exception& ex )
	{
		dlog[Debug::CRIT]  << myname << "(processingMessage): " << ex << endl;
	}
}

// ------------------------------------------------------------------------------------------

void SharedMemory::sensorInfo( SensorMessage *sm )
{
}

// ------------------------------------------------------------------------------------------

void SharedMemory::timerInfo( TimerMessage *tm )
{
	if( tm->id == tmHeartBeatCheck )
		checkHeartBeat();
	else if( tm->id == tmEvent )
	{
		workready = true;
		// ��������� �����������, � ���, ����� ����������
		SystemMessage sm1(SystemMessage::WatchDog);
		sendEvent(sm1);
		askTimer(tm->id,0);
	}
	else if( tm->id == tmHistory )
		saveHistory();
}

// ------------------------------------------------------------------------------------------

void SharedMemory::sysCommand( SystemMessage *sm )
{
	switch( sm->command )
	{
		case SystemMessage::StartUp:
		{
			PassiveTimer ptAct(activateTimeout);
			while( !activated && !ptAct.checkTime() )
			{	
				cout << myname << "(sysCommand): wait activate..." << endl;
				msleep(100);
				if( activated )
					break;
			}
			
			if( !activated )
				dlog[Debug::CRIT] << myname << "(sysCommand): ************* don`t activate?! ************" << endl;
		
			// ��������� ���� ����ģ� �������������
			// ��. activateObject()
			UniSetTypes::uniset_mutex_lock l(mutex_start, 10000);
			askTimer(tmHeartBeatCheck,heartbeatCheckTime);
			askTimer(tmEvent,evntPause,1);
			askTimer(tmHistory,histSaveTime);
		}
		break;
		
		case SystemMessage::FoldUp:
		case SystemMessage::Finish:
			break;
		
		case SystemMessage::WatchDog:
			break;

		case SystemMessage::LogRotate:
		break;

		default:
			break;
	}
}

// ------------------------------------------------------------------------------------------

void SharedMemory::askSensors( UniversalIO::UIOCommand cmd )
{
/*
	for( History::iterator it=hist.begin();  it!=hist.end(); ++it )
	{
		if( sm->id == it->idFuse )
		{
			try
			{
				ui.askState( SID, cmd);
			}
			catch(Exception& ex)
		    {
				dlog[Debug::CRIT] << myname << "(askSensors): " << ex << endl;
			}
	}
*/	
}

// ------------------------------------------------------------------------------------------
bool SharedMemory::activateObject()
{
	PassiveTimer pt(UniSetTimer::WaitUpTime);
	bool res = true;
	// ������������ ��������� Startup 
	// ���� �� ����ģ� ������������� ��������
	// ��. sysCommand()
	{
		activated = false;
		UniSetTypes::uniset_mutex_lock l(mutex_start, 5000);
		res = IONotifyController_LT::activateObject();

		// �������������� ���������		
		for( HeartBeatList::iterator it=hlist.begin(); it!=hlist.end(); ++it )
		{
			it->ait = myaioEnd();
			it->dit = mydioEnd();
		}

		for( History::iterator it=hist.begin();  it!=hist.end(); ++it )
		{
			for( HistoryList::iterator hit=it->hlst.begin(); hit!=it->hlst.end(); ++hit )
			{
				hit->ait = myaioEnd();
				hit->dit = mydioEnd();
			}
		}

		activated = true;
	}
	cerr << "************************** activate: " << pt.getCurrent() << " msec " << endl;
	return res;
}
// ------------------------------------------------------------------------------------------
CORBA::Boolean SharedMemory::exist()
{	
//	return activated;
	return workready;
}
// ------------------------------------------------------------------------------------------
void SharedMemory::sigterm( int signo )
{
	if( signo == SIGTERM )
		wdt->stop();
//	raise(SIGKILL);
}
// ------------------------------------------------------------------------------------------
void SharedMemory::checkHeartBeat()
{
	IOController_i::SensorInfo si;
	si.node = conf->getLocalNode();

	bool wdtpingOK = true;

	for( HeartBeatList::iterator it=hlist.begin(); it!=hlist.end(); ++it )
	{
		try
		{
			si.id = it->a_sid;
			long val = localGetValue(it->ait,si);
			val --;
			if( val < -1 )
				val = -1;
			localSaveValue(it->ait,si,val,getId());

			si.id = it->d_sid;
			if( val >= 0 )
				localSaveState(it->dit,si,true,getId());
			else
				localSaveState(it->dit,si,false,getId());

			// ��������� ����� �� "������������" �� ������� �������
			if( wdt && it->ptReboot.getInterval() )
			{
				if( val > 0  )
					it->timer_running = false;
				else
				{
					if( !it->timer_running )
					{
						it->timer_running = true;
						it->ptReboot.setTiming(it->reboot_msec);
					}
					else if( it->ptReboot.checkTime() )
						wdtpingOK = false;
				}
			}
		}
		catch(Exception& ex)
	    {
			dlog[Debug::CRIT] << myname << "(checkHeartBeat): " << ex << endl;
		}
		catch(...)
		{
			dlog[Debug::CRIT] << myname << "(checkHeartBeat): ..." << endl;
		}
	}
	
	if( wdt && wdtpingOK && workready )
		wdt->ping();
}
// ------------------------------------------------------------------------------------------
bool SharedMemory::readItem( UniXML& xml, UniXML_iterator& it, xmlNode* sec )
{
	for( ReadSlotList::iterator r=lstRSlot.begin(); r!=lstRSlot.end(); ++r )
	{
		try
		{	
			(*r)(xml,it,sec);
		}
		catch(...){}
	}

	// check history filters
	checkHistoryFilter(it);


	if( heartbeat_node.empty() || it.getProp("heartbeat").empty())
		return true;

	if( heartbeat_node != it.getProp("heartbeat_node") )
		return true;

	int i = atoi(it.getProp("heartbeat").c_str());
	if( i<=0 )
		return true;

	if( it.getProp("iotype") != "AI" )
	{
		ostringstream msg;
		msg << "(SharedMemory::readItem): ��� ��� 'heartbeat' ������� (" << it.getProp("name")
			<< ") ������ ������� ("
			<< it.getProp("iotype") << ") ������ ���� 'AI'";
	
		dlog[Debug::CRIT] << msg.str() << endl;
		kill(getpid(),SIGTERM);
//		throw NameNotFound(msg.str());
	};

	HeartBeatInfo hi;
	hi.a_sid = UniSetTypes::uni_atoi( it.getProp("id").c_str() );

	if( it.getProp("heartbeat_ds_name").empty() )
	{
		if( hi.d_sid == DefaultObjectId )
		{
			ostringstream msg;
			msg << "(SharedMemory::readItem): ���������� ������ (heartbeat_ds_name) ��������� � " << it.getProp("name");
			dlog[Debug::WARN] << msg.str() << endl;
#warning ������ ������������?!			
//			dlog[Debug::CRIT] << msg.str() << endl;
//			kill(getpid(),SIGTERM);
			// throw NameNotFound(msg.str());
		}
	}
	else
	{
		hi.d_sid = conf->getSensorID(it.getProp("heartbeat_ds_name"));
		if( hi.d_sid == DefaultObjectId )
		{
			ostringstream msg;
			msg << "(SharedMemory::readItem): �� ������ ID ��� ����������� ������� (heartbeat_ds_name) ���������� � " << it.getProp("name");
			dlog[Debug::CRIT] << msg.str() << endl;
			kill(getpid(),SIGTERM);
//			throw NameNotFound(msg.str());
		}
	}

	hi.reboot_msec = UniSetTypes::uni_atoi( it.getProp("heartbeat_reboot_msec").c_str() );
	hi.ptReboot.setTiming(UniSetTimer::WaitUpTime);

	if( hi.a_sid <= 0 )
	{
		ostringstream msg;
		msg << "(SharedMemory::readItem): �� ������ id ��� " 
			<< it.getProp("name") << " ������ " << sec;

		dlog[Debug::CRIT] << msg.str() << endl;
		kill(getpid(),SIGTERM);
//		throw NameNotFound(msg.str());
	};

	// ��� �������� �� ������������ �.�. 
	// id - ����������� ������������ � ����� configure.xml
	hlist.push_back(hi);
	return true;
}
// ------------------------------------------------------------------------------------------
void SharedMemory::saveValue( const IOController_i::SensorInfo& si, CORBA::Long value,
								UniversalIO::IOTypes type, UniSetTypes::ObjectId sup_id )
{
	uniset_mutex_lock l(hbmutex);
	IONotifyController_LT::saveValue(si,value,type,sup_id);
}
// ------------------------------------------------------------------------------------------
void SharedMemory::fastSaveValue(const IOController_i::SensorInfo& si, CORBA::Long value,
					UniversalIO::IOTypes type, UniSetTypes::ObjectId sup_id )
{
	uniset_mutex_lock l(hbmutex);
	IONotifyController_LT::fastSaveValue(si,value,type,sup_id);
}					
// ------------------------------------------------------------------------------------------
SharedMemory* SharedMemory::init_smemory( int argc, char* argv[] )
{
	string dfile = conf->getArgParam("--datfile",conf->getConfFileName());

	if( dfile[0]!='.' && dfile[0]!='/' )
		dfile = conf->getConfDir() + dfile;
	
	dlog[Debug::INFO] << "(smemory): datfile = " << dfile << endl;
	
	UniSetTypes::ObjectId ID = conf->getControllerID(conf->getArgParam("--smemory-id","SharedMemory"));
	if( ID == UniSetTypes::DefaultObjectId )
	{
		cerr << "(smemory): �� ����� ������������� '" 
			<< " ��� �� ������ � " << conf->getControllersSection()
                        << endl;
		return 0;
	}
	return new SharedMemory(ID,dfile);
}
// -----------------------------------------------------------------------------
void SharedMemory::help_print( int argc, char* argv[] )
{
	cout << "--smemory-id           - SharedMemeory ID" << endl;
	cout << "--logfile fname	- �������� ���� � ���� fname. �� ��������� smemory.log" << endl;
	cout << "--datfile fname	- ���� � ������ ��������. �� ��������� configure.xml" << endl;
	cout << "--s-filter-field	- ������ ��� �������� ������ ��������." << endl;
	cout << "--s-filter-value	- �������� ������� ��� �������� ������ ��������." << endl;
	cout << "--c-filter-field	- ������ ��� �������� ������ ����������." << endl;
	cout << "--c-filter-value	- �������� ������ ��� �������� ������ ����������." << endl;
	cout << "--d-filter-field	- ������ ��� �������� ������ ������������." << endl;
	cout << "--d-filter-value	- �������� ������ ��� �������� ������ ������������." << endl;
	cout << "--wdt-device		- ������������ � �������� WDT ��������� ����." << endl;
	cout << "--heartbeat-node	- ��������� heartbeat ������� ��� ���������� ����." << endl;
	cout << "--lock-value-pause - ����� ����� ��������� spin-���������� �� ��������" << endl;
	cout << "--e-filter 		- ������ ��� ���������� <eventlist>" << endl;
	cout << "--e-startup-pause 	- ����� ����� �������� ����������� � ������ SM. (�� ���������: 1500 ����)." << endl;
	cout << "--activate-timeout - ����� �������� ����������� (�� ���������: 15000 ����)." << endl;

}
// -----------------------------------------------------------------------------
void SharedMemory::buildEventList( xmlNode* cnode )
{
	readEventList("objects");
	readEventList("controllers");
	readEventList("services");
}
// -----------------------------------------------------------------------------
void SharedMemory::readEventList( std::string oname )
{
	xmlNode* enode = conf->getNode(oname);
	if( enode == NULL )
	{
		dlog[Debug::WARN] << myname << "(readEventList): " << oname << " �� ������..." << endl;
		return;
	}

	UniXML_iterator it(enode);
	if( !it.goChildren() )
	{
		dlog[Debug::WARN] << myname << "(readEventList): <eventlist> ������..." << endl;
		return;
	}

	for( ;it.getCurrent(); it.goNext() )
	{
		if( it.getProp(e_filter).empty() )
			continue;

		ObjectId oid = UniSetTypes::uni_atoi(it.getProp("id").c_str());
		if( oid != 0 )
		{
			if( dlog.debugging(Debug::INFO) )
				dlog[Debug::INFO] << myname << "(readEventList): add " << it.getProp("name") << endl;
			elst.push_back(oid);
		}
		else
			dlog[Debug::CRIT] << myname << "(readEventList): �� ������ ID ��� " 
				<< it.getProp("name") << endl;
	}
}
// -----------------------------------------------------------------------------
void SharedMemory::sendEvent( UniSetTypes::SystemMessage& sm )
{
	TransportMessage tm(sm.transport_msg());

	for( EventList::iterator it=elst.begin(); it!=elst.end(); it++ )
	{
		bool ok = false;
		for( int i=0; i<2; i++ )
		{
			try
			{
				ui.send((*it),tm);
				ok = true;
				break;
			}
			catch(...){};
		}
		
		if(!ok)
			dlog[Debug::CRIT] << myname << "(sendEvent): ������ " << (*it) << " ����������" << endl;
	}	
}
// -----------------------------------------------------------------------------
void SharedMemory::addReadItem( Restorer_XML::ReaderSlot sl )
{
	lstRSlot.push_back(sl);
}
// -----------------------------------------------------------------------------
void SharedMemory::loggingInfo( SensorMessage& sm )
{
	if( dblogging )
		IONotifyController_LT::loggingInfo(sm);
}
// -----------------------------------------------------------------------------
void SharedMemory::buildHistoryList( xmlNode* cnode )
{
	if( dlog.debugging(Debug::INFO) )
		dlog[Debug::INFO] << myname << "(buildHistoryList): ..."  << endl;

	xmlNode* n = conf->findNode(cnode,"History");
	if( !n )
	{
		dlog[Debug::WARN] << myname << "(buildHistoryList): <History> not found " << endl;
		return;
	}

	UniXML_iterator it(n);
	
	histSaveTime = uni_atoi(it.getProp("savetime").c_str());
	if( histSaveTime < 0 )
		histSaveTime = 200;
	
	if( !it.goChildren() )
	{
		dlog[Debug::WARN] << myname << "(buildHistoryList): <History> empty..." << endl;
		return;
	}

	for( ; it.getCurrent(); it.goNext() )
	{
		HistoryInfo hi;
		hi.id 		= UniSetTypes::uni_atoi( it.getProp("id").c_str() );
		hi.size 	= UniSetTypes::uni_atoi( it.getProp("size").c_str() );
		if( hi.size <=0 )
			continue;

		hi.filter 	= it.getProp("filter");
		if( hi.filter.empty() )
			continue;

		hi.idFuse = conf->getSensorID(it.getProp("fuse"));
		if( hi.idFuse == DefaultObjectId )
		{
			dlog[Debug::WARN] << myname << "(buildHistory): not found sensor ID for " 
				<< it.getProp("idFuse")
				<< " history item id=" << it.getProp("id") << endl;
			continue;
		}

		hi.invert 	= uni_atoi(it.getProp("invert").c_str());

		// WARNING: no check duplicates...
		hist.push_back(hi);
	}

	if( dlog.debugging(Debug::INFO) )
		dlog[Debug::INFO] << myname << "(buildHistoryList): histoty size=" << hist.size() << endl;
}
// -----------------------------------------------------------------------------
void SharedMemory::checkHistoryFilter( UniXML_iterator& xit )
{
	for( History::iterator it=hist.begin();  it!=hist.end(); ++it )
	{
		if( xit.getProp(it->filter).empty() )
			continue;

		HistoryItem ai;

		if( !xit.getProp("id").empty() )
		{
			ai.id = uni_atoi(xit.getProp("id").c_str());
			it->hlst.push_back(ai);
			continue;
		}

		ai.id = conf->getSensorID(xit.getProp("name"));
		if( ai.id == DefaultObjectId )
		{
			dlog[Debug::WARN] << myname << "(checkHistoryFilter): not found sensor ID for " << xit.getProp("name") << endl;
			continue;
		}
		
		it->hlst.push_back(ai);
	}
}
// -----------------------------------------------------------------------------
SharedMemory::HistorySlot SharedMemory::signal_history()
{
	return m_historySignal;
}
// -----------------------------------------------------------------------------
void SharedMemory::saveHistory()
{
//	if( dlog.debugging(Debug::INFO) )
//		dlog[Debug::INFO] << myname << "(saveHistory): ..." << endl;

	for( History::iterator it=hist.begin();  it!=hist.end(); ++it )
	{
		for( HistoryList::iterator hit=it->hlst.begin(); hit!=it->hlst.end(); ++hit )
		{
			long val = 0;
			if(  hit->ait != myaioEnd() )
				val = localGetValue( hit->ait, hit->ait->second.si );
			else if( hit->dit != mydioEnd() )
				val = localGetState( hit->dit, hit->dit->second.si );
			else 
				continue;

			hit->add( val, it->size );
		}
	}
}
// -----------------------------------------------------------------------------
void SharedMemory::updateHistory( UniSetTypes::SensorMessage* sm )
{
	if( dlog.debugging(Debug::INFO) )
	{
		dlog[Debug::INFO] << myname << "(updateHistory): " 
			<< " sid=" << sm->id 
			<< " state=" << sm->state 
			<< " value=" << sm->value
			<< endl;
	}

	for( History::iterator it=hist.begin();  it!=hist.end(); ++it )
	{
		if( sm->id == it->idFuse )
		{
			bool st = it->invert ? !sm->state : sm->state;
			if( st )
			{
				if( dlog.debugging(Debug::INFO) )
					dlog[Debug::INFO] << myname << "(updateHistory): HISTORY EVENT for " << (*it) << endl;
			
				m_historySignal.emit( &(*it) );
			}
		}
	}
}
// -----------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& os, const SharedMemory::HistoryInfo& h )
{
	os << "id=" << h.id << " idFuse=" << h.idFuse;
	return os;
}
// ------------------------------------------------------------------------------------------
