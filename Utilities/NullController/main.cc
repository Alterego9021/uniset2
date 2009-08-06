// $Id: main.cc,v 1.13 2008/02/10 03:21:41 vpashka Exp $
// --------------------------------------------------------------------------
#include "Configuration.h"
#include "NullController.h"
#include "ObjectsActivator.h"
#include "Debug.h"
#include "PassiveTimer.h"
// --------------------------------------------------------------------------
using namespace UniSetTypes;
using namespace std;
// --------------------------------------------------------------------------
static void short_usage()
{
	cout << "Usage: uniset-nullController"
		 << " --name ObjectId [--confile configure.xml] [--askfile filename] \n"
		 << " --s-filter-field name - ���� ��� ������������ ������ ��������\n"
		 << " --s-filter-value value - �������� ��� ���� ������������ ������ �������� \n"
		 << " --c-filter-field name - ���� ��� ������������ ������ ���������� �� ������� �������\n"
		 << " --c-filter-value value - �������� ��� ���� ������������ ������ ���������� �� ������� �������\n"
		 << " --d-filter-field name - ���� ��� ������������ ������ ������������ �� ������� �������\n"
		 << " --d-filter-value value - �������� ��� ���� ������������ ������ ������������ �� ������� �������\n"
		 << " --dbDumping [0,1] - ��������� �� dump-���� \n";
}
// --------------------------------------------------------------------------
int main(int argc, char** argv)
{
	try
	{
		if( argc <=1 )
		{
			cerr << "\n�� ������� ����������� ���������\n\n";
			short_usage();
			return 0;
		}

		if( !strcmp(argv[1],"--help") )
		{
			short_usage();
			return 0;
		}

		uniset_init(argc,argv,"configure.xml");
			

		// ���������� ID �������
		string name = conf->getArgParam("--name");
		if( name.empty())
		{
			cerr << "(nullController): �� ����� ObjectId!!! (--name)\n";
			return 0;
		}

		ObjectId ID = conf->oind->getIdByName(conf->getControllersSection()+"/"+name);	
		if( ID == UniSetTypes::DefaultObjectId )
		{
			cerr << "(nullController): ������������� '" << name 
				<< "' �� ������ � ����. �����!"
				<< " � ������ " << conf->getControllersSection() << endl;
			return 0;
		}

		// ���������� ask-����
		string askfile = conf->getArgParam("--askfile");
		if( askfile.empty())
			askfile = conf->getConfFileName();

		// ���������� ������
		string s_field = conf->getArgParam("--s-filter-field");
		string s_fvalue = conf->getArgParam("--s-filter-value");
		string c_field = conf->getArgParam("--c-filter-field");
		string c_fvalue = conf->getArgParam("--c-filter-value");
		string d_field = conf->getArgParam("--d-filter-field");
		string d_fvalue = conf->getArgParam("--d-filter-value");

		// ���� �� ������ ��������� � ��
		bool dbDumping = conf->getArgInt("--dbDumping");

		NullController nc(ID,askfile,s_field,s_fvalue,c_field,c_fvalue,d_field,d_fvalue,dbDumping);
		ObjectsActivator act;
		act.addObject(static_cast<class UniSetObject*>(&nc));
		act.run(false);
	}
	catch(Exception& ex)
	{
		cerr << "(nullController::main): " << ex << endl;
	}
	catch(...)
	{
		cerr << "(nullController::main): catch ..." << endl;
	}

	return 0;
}
