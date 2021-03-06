// --------------------------------------------------------------------------
#include <sstream>
#include <string>
#include <cc++/socket.h>
#include "MBTCPPersistentSlave.h"
#include "Configuration.h"
#include "Debug.h"
#include "UniSetActivator.h"
#include "Extensions.h"

// --------------------------------------------------------------------------
using namespace UniSetTypes;
using namespace UniSetExtensions;
using namespace std;
// --------------------------------------------------------------------------
int main(int argc, const char** argv)
{
	//	std::ios::sync_with_stdio(false);

	if( argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) )
	{
		cout << "--smemory-id objectName  - SharedMemory objectID. Default: autodetect" << endl;
		cout << "--confile filename       - configuration file. Default: configure.xml" << endl;
		cout << "--mbs-logfile filename   - logfilename" << endl;
		cout << endl;
		MBSlave::help_print(argc, argv);
		return 0;
	}

	try
	{
		auto conf = uniset_init(argc, argv);

		string logfilename(conf->getArgParam("--mbs-logfile"));

		if( !logfilename.empty() )
		{
			std::ostringstream logname;
			string dir(conf->getLogDir());
			logname << dir << logfilename;
			ulog()->logFile( logname.str() );
			dlog()->logFile( logname.str() );
		}

		ObjectId shmID = DefaultObjectId;
		string sID = conf->getArgParam("--smemory-id");

		if( !sID.empty() )
			shmID = conf->getControllerID(sID);
		else
			shmID = getSharedMemoryID();

		if( shmID == DefaultObjectId )
		{
			cerr << sID << "? SharedMemoryID not found in " << conf->getControllersSection() << " section" << endl;
			return 1;
		}

		auto s = MBTCPPersistentSlave::init_mbslave(argc, argv, shmID);

		if( !s )
		{
			dcrit << "(mbmultislave): init не прошёл..." << endl;
			return 1;
		}

		auto act = UniSetActivator::Instance();
		act->add(s);
		SystemMessage sm(SystemMessage::StartUp);
		act->broadcast( sm.transport_msg() );

		ulogany << "\n\n\n";
		ulogany << "(main): -------------- MBTCPPersistentSlave START -------------------------\n\n";
		dlogany << "\n\n\n";
		dlogany << "(main): -------------- MBTCPPersistentSlave START -------------------------\n\n";

		act->run(false);
		return 0;
	}
	catch( const SystemError& err )
	{
		dcrit << "(mbmultislave): " << err << endl;
	}
	catch( const Exception& ex )
	{
		dcrit << "(mbmultislave): " << ex << endl;
	}
	catch( const std::exception& e )
	{
		dcrit << "(mbmultislave): " << e.what() << endl;
	}
	catch(...)
	{
		dcrit << "(mbmultislave): catch(...)" << endl;
	}

	return 1;
}
// --------------------------------------------------------------------------
