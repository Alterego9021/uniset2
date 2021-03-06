#include "UConnector.h"
#include "ORepHelpers.h"
// --------------------------------------------------------------------------
using namespace std;
// --------------------------------------------------------------------------
UConnector::UConnector( UTypes::Params* p, const std::string& xfile )throw(UException):
	conf(0),
	ui(0),
	xmlfile(xfile)
{
	try
	{
		conf = UniSetTypes::uniset_init(p->argc, p->argv, xmlfile);
		ui = make_shared<UInterface>(conf);
	}
	catch( UniSetTypes::Exception& ex )
	{
		throw UException(ex.what());
	}
	catch( ... )
	{
		throw UException();
	}
}
//---------------------------------------------------------------------------
UConnector::UConnector(int argc, char** argv, const string& xfile )throw(UException):
	conf(0),
	ui(0),
	xmlfile(xfile)
{
	try
	{
		conf = UniSetTypes::uniset_init(argc, argv, xmlfile);
		ui = make_shared<UInterface>(conf);
	}
	catch( UniSetTypes::Exception& ex )
	{
		throw UException(ex.what());
	}
	catch( ... )
	{
		throw UException();
	}
}
// --------------------------------------------------------------------------
UConnector::~UConnector()
{
}
// --------------------------------------------------------------------------
string UConnector::getConfFileName()
{
	//    return xmlfile;
	if( conf )
		return conf->getConfFileName();

	return "";

}
// --------------------------------------------------------------------------
long UConnector::getValue( long id, long node )throw(UException)
{
	if( !conf || !ui )
		throw USysError();

	if( node == UTypes::DefaultID )
		node = conf->getLocalNode();

	try
	{
		return ui->getValue(id, node);
	}
	catch( UniSetTypes::Exception& ex )
	{
		throw UException(ex.what());
	}
	catch( std::exception& ex )
	{
		throw UException(ex.what());
	}
}
//---------------------------------------------------------------------------
void UConnector::setValue( long id, long val, long node )throw(UException)
{
	if( !conf || !ui )
		throw USysError();


	if( node == UTypes::DefaultID )
		node = conf->getLocalNode();

	try
	{
		ui->setValue(id, val, node);
	}
	catch( UniSetTypes::Exception& ex )
	{
		throw UException(ex.what());
	}
	catch( std::exception& ex )
	{
		throw UException(ex.what());
	}
}
//---------------------------------------------------------------------------
long UConnector::getSensorID(const string& name )
{
	if( conf )
		return conf->getSensorID(name);

	return UTypes::DefaultID;
}
//---------------------------------------------------------------------------
long UConnector::getNodeID(const string& name )
{
	if( conf )
		return conf->getNodeID(name);

	return UTypes::DefaultID;
}
//---------------------------------------------------------------------------
string UConnector::getName( long id )
{
	if( conf )
		return conf->oind->getMapName(id);

	return "";
}
//---------------------------------------------------------------------------
string UConnector::getShortName( long id )
{
	if( conf )
		return ORepHelpers::getShortName(conf->oind->getMapName(id));

	return "";
}
//---------------------------------------------------------------------------
string UConnector::getTextName( long id )
{
	if( conf )
		return conf->oind->getTextName(id);

	return "";
}
//---------------------------------------------------------------------------
