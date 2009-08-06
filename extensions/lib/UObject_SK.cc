// -----------------------------------------------------------------------------
#include "Configuration.h"
#include "Exceptions.h"
#include "Extensions.h"
#include "UObject_SK.h"
// -----------------------------------------------------------------------------
using namespace std;
using namespace UniSetTypes;
using namespace UniSetExtensions;
// -----------------------------------------------------------------------------
UObject_SK::UObject_SK():
active(false),
isTestMode(false),
idTestMode_S(DefaultObjectId),
idLocalTestMode_S(DefaultObjectId),
confnode(0),
shm(DefaultObjectId,&ui,DefaultObjectId),
smReadyTimeout(0),
activated(false)
{
	dlog[Debug::CRIT] << "UObject: init failed!!!!!!!!!!!!!!!" << endl;
	throw Exception( string(myname+": init failed!!!") );
}
// -----------------------------------------------------------------------------
UObject_SK::UObject_SK( ObjectId id, xmlNode* cnode, ObjectId shmID ):
UniSetObject(id),
sleep_msec(200),
active(true),
isTestMode(false),
idTestMode_S(conf->getSensorID("TestMode_S")),
idLocalTestMode_S(conf->getSensorID(conf->getProp(cnode,"LocalTestMode_S"))),
in_TestMode_S(false),
in_LocalTestMode_S(false),
confnode(cnode),
shm(shmID,&ui,id),
smReadyTimeout(0),
activated(false)
{
	si.node = conf->getLocalNode();

	sleep_msec = conf->getArgInt("--sleep-msec","200");
	if( sleep_msec <= 0 )
		sleep_msec = 200;

	resetMsgTime = conf->getIntProp(cnode,"resetMsgTime");
	if( resetMsgTime <= 0 )
		resetMsgTime = 200;
	ptResetMsg.setTiming(resetMsgTime);

	smReadyTimeout = conf->getArgInt("--sm-ready-timeout","");
	if( smReadyTimeout == 0 )
		smReadyTimeout = 60000;
	else if( smReadyTimeout < 0 )
		smReadyTimeout = UniSetTimer::WaitUpTime;

	activateTimeout	= conf->getArgInt("--activate-timeout"));
	if( activateTimeout <= 0 )
		activateTimeout = 20000;

	int msec = conf->getArgInt("--startup-timeout");
	if( msec <= 0 )
		msec = 10000;
	ptStartUpTimeout.setTiming(msec);
}

// -----------------------------------------------------------------------------
UObject_SK::~UObject_SK()
{
}
// -----------------------------------------------------------------------------
void UObject_SK::updateValues()
{
	in_TestMode_S  		= (idTestMode_S!=DefaultObjectId) ? shm.getState(idTestMode_S):false;
	in_LocalTestMode_S 	= (idLocalTestMode_S!=DefaultObjectId) ? shm.getState(idLocalTestMode_S):false;
}
// -----------------------------------------------------------------------------
bool UObject_SK::alarm( UniSetTypes::ObjectId code, bool state )
{
	if( code == UniSetTypes::DefaultObjectId )
	{
		dlog[Debug::CRIT]  << getName()
							<< "(alarm): ������� ������� ��������� � DefaultObjectId" 
							<< endl;
		return false;	
	}

	dlog[Debug::LEVEL1]  << getName()  << "(alarm): ";
	if( state )
		dlog(Debug::LEVEL1) << "SEND ";
	else
		dlog(Debug::LEVEL1) << "RESET ";
	
	
	
	dlog(Debug::INFO) << " not found MessgeOID?!!" << endl;
	return false;
}
// -----------------------------------------------------------------------------
void UObject_SK::resetMsg()
{
// reset messages

}
// -----------------------------------------------------------------------------
void UObject_SK::testMode( bool state )
{
	if( !state  )
		return;
}
// -----------------------------------------------------------------------------
void UObject_SK::processingMessage( UniSetTypes::VoidMessage* msg )
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

			case Message::Timer:
			{
				TimerMessage tm(msg);
				preTimerInfo(&tm);
				break;
			}

			case Message::SysCommand:
			{
				SystemMessage sm( msg );
				sysCommand( &sm );
				break;
			}

			default:
				break;
		}	
	}
	catch(Exception& ex)
	{
		cout  << myname << "(processingMessage): " << ex << endl;
	}
}
// -----------------------------------------------------------------------------
void UObject_SK::setState( UniSetTypes::ObjectId sid, bool state )
{
#warning ������� setState ��������� ��������, � �� ����� setValue
	setValue(sid, state ? 1 : 0 );
}
// -----------------------------------------------------------------------------
bool UObject_SK::checkTestMode()
{
	return (in_TestMode_S && in_LocalTestMode_S);
}
// -----------------------------------------------------------------------------
void UObject_SK::sigterm( int signo )
{
	UniSetObject::sigterm(signo);
	active = false;
}
// -----------------------------------------------------------------------------
bool UObject_SK::activateObject()
{
	// ������������ ��������� Startup 
	// ���� �� ����ģ� ������������� ��������
	// ��. sysCommand()
	{
		activated = false;
		UniSetObject::activateObject();
		activated = true;
	}

	return true;
}
// -----------------------------------------------------------------------------
void UObject_SK::askThreshold( UniSetTypes::ObjectId sid, UniSetTypes::ThresholdId tid,
							UniversalIO::UIOCommand cmd,
							CORBA::Long lowLimit, CORBA::Long hiLimit, CORBA::Long sensibility,
							UniSetTypes::ObjectId backid )
{
#warning askThreshold �� �����������...
//	shm.askThreshold( sid,tid,cmd,lowLimit,hiLimit,sensibility,backid);
}
// -----------------------------------------------------------------------------
void UObject_SK::preTimerInfo( UniSetTypes::TimerMessage* tm )
{
	timerInfo(tm);
}
// ----------------------------------------------------------------------------
void UObject_SK::waitSM( int wait_msec )
{
	if( !shm.waitSMready(wait_msec) )
	{
		ostringstream err;
		err << myname 
			<< "(waitSM): �� ��������� ����������(exist) SharedMemory � ������ � ������� " 
			<< wait_msec << " ����";

		dlog[Debug::CRIT] << err.str() << endl;
		kill(SIGTERM,getpid());	// ��������� (�������������) �������...
		throw SystemError(err.str());
	}

	if( idTestMode_S != DefaultObjectId )
	{
		if( !shm.waitSMworking(idTestMode_S,wait_msec) )
		{
			ostringstream err;
			err << myname 
				<< "(waitSM): �� ��������� ����������(work) SharedMemory � ������ � ������� " 
				<< wait_msec << " ����";
	
			dlog[Debug::CRIT] << err.str() << endl;
			kill(SIGTERM,getpid());	// ��������� (�������������) �������...
			throw SystemError(err.str());
		}
	}
}
// ----------------------------------------------------------------------------

// --------------------------------------------------------------------------
void UObject_SK::callback()
{
	if( !active )
		return;
	try
	{	
#warning ������� ������ � TestMode �� ������ ������!
		isTestMode = checkTestMode();
		if( trTestMode.change(isTestMode) )
			testMode(isTestMode);

		if( isTestMode )
		{
			if( !active )
				return;

			msleep( sleep_msec );
			return;
		}

		// �������� ��������
		checkTimers(this);

#warning ������� ������ � ResetMsg �� ������ askTimer!
		if( resetMsgTime>0 && trResetMsg.hi(ptResetMsg.checkTime()) )
		{
//			cout << myname <<  ": ********* reset messages *********" << endl;
			resetMsg();
		}

		// ��������� ��������� (�������� � �.�.)
		for( int i=0; i<30; i++ )
		{
			if( !receiveMessage(msg) )
				break;
			processingMessage(&msg);
			updateOutputs(false);
			updatePreviousValues();
		}

		// ���������� ���� ���������
		step();

		// ���������� �������
		updateOutputs(false);
		updatePreviousValues();
	}
	catch( Exception& ex )
	{
		dlog[Debug::CRIT] << myname << "(execute): " << ex << endl;
	}
	catch(CORBA::SystemException& ex)
	{
		dlog[Debug::CRIT] << myname << "(execute): �ORBA::SystemException: "
			<< ex.NP_minorString() << endl;
	}
	catch(...)
	{
		dlog[Debug::CRIT] << myname << "(execute): catch ..." << endl;
	}

	if( !active )
		return;
	
	msleep( sleep_msec );
}
// -----------------------------------------------------------------------------
void UObject_SK::askState( UniSetTypes::ObjectId sid, UniversalIO::UIOCommand cmd )
{
	shm.askSensor(sid,cmd,getId());
}
// -----------------------------------------------------------------------------
void UObject_SK::askValue( UniSetTypes::ObjectId sid, UniversalIO::UIOCommand cmd )
{
	shm.askSensor(sid,cmd,getId());
}
// -----------------------------------------------------------------------------
void UObject_SK::askSensors( UniversalIO::UIOCommand cmd )
{
	PassiveTimer ptAct(activateTimeout);
	while( !activated && !ptAct.checkTime() )
	{	
		cout << myname << "(askSensors): wait activate..." << endl;
		msleep(300);
		if( activated )
			break;
	}
			
	if( !activated )
		dlog[Debug::CRIT] << myname 
			<< "(askSensors): ************* don`t activated?! ************" << endl;

	for( ;; )
	{
		try
		{
			//
			return;
		}
		catch(SystemError& err)
		{
			dlog[Debug::CRIT] << myname << "(askSensors): " << err << endl;
		}
		catch(Exception& ex)
		{
			dlog[Debug::CRIT] << myname << "(askSensors): " << ex << endl;
		}
		catch(...)
		{
			dlog[Debug::CRIT] << myname << "(askSensors): catch(...)" << endl;
		}
#warning ������� ����� ����� �������� �������������	�� ����. �����
		msleep(2000);
	}
}
// -----------------------------------------------------------------------------
void UObject_SK::setInfo( UniSetTypes::ObjectId code, bool state )
{
	alarm( code, state );
	if( state )
	{
#warning ������� ����� �������������
		msleep(50);	// ���������� �����...
		alarm( code, false );
	}
}	
// ----------------------------------------------------------------------------
