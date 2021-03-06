#include <string>
#include "Debug.h"
#include "UniSetActivator.h"
#include "Configuration.h"
#include "IOControl.h"
#include "Extensions.h"
// --------------------------------------------------------------------------
using namespace UniSetTypes;
using namespace std;
using namespace UniSetExtensions;
// --------------------------------------------------------------------------
int main(int argc, const char** argv)
{
	//	std::ios::sync_with_stdio(false);

	if( argc > 1 && strcmp(argv[1], "--help") == 0 )
	{
		cout << "--io-confile    - Использовать указанный конф. файл. По умолчанию configure.xml" << endl;
		cout << "--io-logfile fname    - выводить логи в файл fname. По умолчанию iocontrol.log" << endl;
		IOControl::help_print(argc, argv);
		return 0;
	}

	try
	{
		auto conf = uniset_init(argc, argv);

		string logfilename = conf->getArgParam("--io-logfile", "iocontrol.log");
		string logname( conf->getLogDir() + logfilename );
		dlog()->logFile( logname );
		ulog()->logFile( logname );

		ObjectId shmID = DefaultObjectId;
		string sID = conf->getArgParam("--smemory-id");

		if( !sID.empty() )
			shmID = conf->getControllerID(sID);
		else
			shmID = getSharedMemoryID();

		if( shmID == DefaultObjectId )
		{
			cerr << sID << "? SharedMemoryID not found in "
				 << conf->getControllersSection() << " section" << endl;
			return 1;
		}


		auto ic = IOControl::init_iocontrol(argc, argv, shmID);

		if( !ic )
		{
			dcrit << "(iocontrol): init не прошёл..." << endl;
			return 1;
		}

		auto act = UniSetActivator::Instance();
		act->add(ic);

		SystemMessage sm(SystemMessage::StartUp);
		act->broadcast( sm.transport_msg() );

		ulogany << "\n\n\n";
		ulogany << "(main): -------------- IOControl START -------------------------\n\n";
		dlogany << "\n\n\n";
		dlogany << "(main): -------------- IOControl START -------------------------\n\n";
		act->run(true);
		msleep(500);
		ic->execute();
		return 0;
	}
	catch( const std::exception& ex )
	{
		dcrit << "(iocontrol): " << ex.what() << endl;
	}
	catch(...)
	{
		dcrit << "(iocontrol): catch(...)" << endl;
	}

	return 1;
}
