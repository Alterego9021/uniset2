// $Id: main.cc,v 1.8 2008/02/10 03:21:41 vpashka Exp $
// --------------------------------------------------------------------------
#include "Configuration.h"
#include "DBServer.h"
#include "ObjectsActivator.h"
#include "Debug.h"
// --------------------------------------------------------------------------
using namespace UniSetTypes;
using namespace std;
// --------------------------------------------------------------------------
static void short_usage()
{
	cout << "Usage: uniset-dbserver [--name ObjectId] [--confile configure.xml]\n";
}
// --------------------------------------------------------------------------
int main(int argc, const char** argv)
{
	try
	{
		if( argc > 1 && !strcmp(argv[1],"--help") )
		{
			short_usage();
			return 0;
		}

		uniset_init(argc,argv,"configure.xml");

		ObjectId ID = conf->getDBServer();

		// ���������� ID �������
		string name = conf->getArgParam("--name");
		if( !name.empty())
		{
			if( ID != UniSetTypes::DefaultObjectId )
			{
				unideb[Debug::WARN] << "(DBServer::main): �������������� ID ������� � " 
						<< conf->getConfFileName() << endl;
			}

			ID = conf->oind->getIdByName(conf->getServicesSection()+"/"+name);
			if( ID == UniSetTypes::DefaultObjectId )
			{
				cerr << "(DBServer::main): ������������� '" << name 
					<< "' �� ������ � ����. �����!"
					<< " � ������ " << conf->getServicesSection() << endl;
				return 1;
			}
		}
		else if( ID == UniSetTypes::DefaultObjectId )
		{
			cerr << "(DBServer::main): �� ������� ���������� ������������� �������" << endl; 
			short_usage();
			return 1;
		}

		DBServer dbs(ID);
		ObjectsActivator act;
		act.addObject(static_cast<class UniSetObject*>(&dbs));
		act.run(false);
	}
	catch(Exception& ex)
	{
		cerr << "(DBServer::main): " << ex << endl;
	}
	catch(...)
	{
		cerr << "(DBServer::main): catch ..." << endl;
	}

	return 0;
}
