/* This file is part of the UniSet project
 * Copyright (c) 2002 Free Software Foundation, Inc.
 * Copyright (c) 2002 Pavel Vainerman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
// --------------------------------------------------------------------------
/*! \file
 *  \brief файл реализации DB-сервера
 *  \author Pavel Vainerman
*/
// --------------------------------------------------------------------------
#include <sys/time.h>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "ORepHelpers.h"
#include "DBServer_MySQL.h"
#include "Configuration.h"
#include "Debug.h"
#include "UniXML.h"
#include "DBLogSugar.h"
// --------------------------------------------------------------------------
using namespace UniSetTypes;
using namespace std;
// --------------------------------------------------------------------------
DBServer_MySQL::DBServer_MySQL(ObjectId id, const std::string& prefix ):
	DBServer(id, prefix),
	PingTime(300000),
	ReconnectTime(180000),
	connect_ok(false),
	activate(true),
	qbufSize(200),
	lastRemove(false)
{
	if( getId() == DefaultObjectId )
	{
		ostringstream msg;
		msg << "(DBServer_MySQL): init failed! Unknown ID!" << endl;
		throw Exception(msg.str());
	}

	db = make_shared<MySQLInterface>();

	mqbuf.setName(myname  + "_qbufMutex");
}

DBServer_MySQL::DBServer_MySQL( const std::string& prefix ):
	DBServer_MySQL(uniset_conf()->getDBServer(), prefix)
{
}
//--------------------------------------------------------------------------------------------
DBServer_MySQL::~DBServer_MySQL()
{
	if( db )
		db->close();
}
//--------------------------------------------------------------------------------------------
void DBServer_MySQL::sysCommand( const UniSetTypes::SystemMessage* sm )
{
	DBServer::sysCommand(sm);

	switch( sm->command )
	{
		case SystemMessage::StartUp:
			break;

		case SystemMessage::Finish:
		{
			activate = false;
			db->close();
		}
		break;

		case SystemMessage::FoldUp:
		{
			activate = false;
			db->close();
		}
		break;

		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
void DBServer_MySQL::confirmInfo( const UniSetTypes::ConfirmMessage* cem )
{
	try
	{
		ostringstream data;

		data << "UPDATE " << tblName(cem->type)
			 << " SET confirm='" << cem->confirm << "'"
			 << " WHERE sensor_id='" << cem->sensor_id << "'"
			 << " AND date='" << dateToString(cem->time, "-") << " '"
			 << " AND time='" << timeToString(cem->time, ":") << " '"
			 << " AND time_usec='" << cem->time_usec << " '";

		dbinfo << myname << "(update_confirm): " << data.str() << endl;

		if( !writeToBase(data.str()) )
		{
			dbcrit << myname << "(update_confirm):  db error: " << db->error() << endl;
		}
	}
	catch( const Exception& ex )
	{
		dbcrit << myname << "(update_confirm): " << ex << endl;
	}
	catch( const std::exception& ex )
	{
		dbcrit << myname << "(update_confirm): exception: " << ex.what() << endl;
	}
}
//--------------------------------------------------------------------------------------------
bool DBServer_MySQL::writeToBase( const string& query )
{
	dbinfo << myname << "(writeToBase): " << query << endl;

	//    cout << "DBServer_MySQL: " << query << endl;
	if( !db || !connect_ok )
	{
		uniset_rwmutex_wrlock l(mqbuf);
		qbuf.push(query);

		if( qbuf.size() > qbufSize )
		{
			std::string qlost;

			if( lastRemove )
				qlost = qbuf.back();
			else
				qlost = qbuf.front();

			qbuf.pop();

			dbcrit << myname << "(writeToBase): DB not connected! buffer(" << qbufSize
				   << ") overflow! lost query: " << qlost << endl;
		}

		return false;
	}

	// На всякий скидываем очередь
	flushBuffer();

	// А теперь собственно запрос..
	db->query(query);

	// Дело в том что на INSERT И UPDATE запросы
	// db->query() может возвращать false и надо самому
	// отдельно проверять действительно ли произошла ошибка
	// см. MySQLInterface::query.
	string err(db->error());

	if( err.empty() )
		return true;

	return false;
}
//--------------------------------------------------------------------------------------------
void DBServer_MySQL::flushBuffer()
{
	uniset_rwmutex_wrlock l(mqbuf);

	// Сперва пробуем очистить всё что накопилось в очереди до этого...
	while( !qbuf.empty() )
	{
		db->query( qbuf.front() );

		// Дело в том что на INSERT И UPDATE запросы
		// db->query() может возвращать false и надо самому
		// отдельно проверять действительно ли произошла ошибка
		// см. MySQLInterface::query.
		string err(db->error());

		if( !err.empty() )
			dbcrit << myname << "(writeToBase): error: " << err <<
				   " lost query: " << qbuf.front() << endl;

		qbuf.pop();
	}
}
//--------------------------------------------------------------------------------------------
void DBServer_MySQL::sensorInfo( const UniSetTypes::SensorMessage* si )
{
	try
	{
		// если время не было выставлено (указываем время сохранения в БД)
		if( !si->tm.tv_sec )
		{
			struct timezone tz;
			gettimeofday( const_cast<struct timeval*>(&si->tm), &tz);
		}

		float val = (float)si->value / (float)pow10(si->ci.precision);

		// см. DBTABLE AnalogSensors, DigitalSensors
		ostringstream data;
		data << "INSERT INTO " << tblName(si->type)
			 << "(date, time, time_usec, sensor_id, value, node) VALUES( '"
			 // Поля таблицы
			 << dateToString(si->sm_tv_sec, "-") << "','"   //  date
			 << timeToString(si->sm_tv_sec, ":") << "','"   //  time
			 << si->sm_tv_usec << "','"                //  time_usec
			 << si->id << "','"                    //  sensor_id
			 << val << "','"                //  value
			 << si->node << "')";                //  node

		dbinfo << myname << "(insert_main_history): " << data.str() << endl;

		if( !writeToBase(data.str()) )
		{
			dbcrit << myname << "(insert) sensor msg error: " << db->error() << endl;
		}
	}
	catch( const Exception& ex )
	{
		dbcrit << myname << "(insert_main_history): " << ex << endl;
	}
	catch( const std::exception& ex )
	{
		dbcrit << myname << "(insert_main_history): catch: " << ex.what() << endl;
	}
}
//--------------------------------------------------------------------------------------------
void DBServer_MySQL::initDBServer()
{
	DBServer::initDBServer();
	dbinfo << myname << "(init): ..." << endl;

	if( connect_ok )
	{
		initDBTableMap(tblMap);
		initDB(db);
		return;
	}

	auto conf = uniset_conf();

	if( conf->getDBServer() == UniSetTypes::DefaultObjectId )
	{
		ostringstream msg;
		msg << myname << "(init): DBServer OFF for this node.."
			<< " In " << conf->getConfFileName()
			<< " for this node dbserver=''";
		throw NameNotFound(msg.str());
	}

	xmlNode* node = conf->getNode("LocalDBServer");

	if( !node )
		throw NameNotFound(string(myname + "(init): section <LocalDBServer> not found.."));

	UniXML::iterator it(node);

	dbinfo << myname << "(init): init connection.." << endl;
	string dbname(conf->getProp(node, "dbname"));
	string dbnode(conf->getProp(node, "dbnode"));
	string user(conf->getProp(node, "dbuser"));
	string password(conf->getProp(node, "dbpass"));

	tblMap[UniSetTypes::Message::SensorInfo] = "main_history";
	tblMap[UniSetTypes::Message::Confirm] = "main_history";

	PingTime = conf->getIntProp(node, "pingTime");
	ReconnectTime = conf->getIntProp(node, "reconnectTime");
	qbufSize = conf->getArgPInt("--dbserver-buffer-size", it.getProp("bufferSize"), 200);

	if( findArgParam("--dbserver-buffer-last-remove", conf->getArgc(), conf->getArgv()) != -1 )
		lastRemove = true;
	else if( it.getIntProp("bufferLastRemove" ) != 0 )
		lastRemove = true;
	else
		lastRemove = false;

	if( dbnode.empty() )
		dbnode = "localhost";

	dbinfo << myname << "(init): connect dbnode=" << dbnode
		   << "\tdbname=" << dbname
		   << " pingTime=" << PingTime
		   << " ReconnectTime=" << ReconnectTime << endl;

	if( !db->nconnect(dbnode, user, password, dbname) )
	{
		//        ostringstream err;
		dbcrit << myname
			   << "(init): DB connection error: "
			   << db->error() << endl;
		//        throw Exception( string(myname+"(init): не смогли создать соединение с БД "+db->error()) );
		askTimer(DBServer_MySQL::ReconnectTimer, ReconnectTime);
	}
	else
	{
		dbinfo << myname << "(init): connect [OK]" << endl;
		connect_ok = true;
		askTimer(DBServer_MySQL::ReconnectTimer, 0);
		askTimer(DBServer_MySQL::PingTimer, PingTime);
		//        createTables(db);
		initDB(db);
		initDBTableMap(tblMap);
		flushBuffer();
	}
}
//--------------------------------------------------------------------------------------------
void DBServer_MySQL::createTables( MySQLInterface* db )
{
	auto conf = uniset_conf();

	UniXML::iterator it( conf->getNode("Tables") );

	if(!it)
	{
		dbcrit << myname << ": section <Tables> not found.." << endl;
		throw Exception();
	}

	if( !it.goChildren() )
		return;

	for( ; it; it.goNext() )
	{
		if( it.getName() != "comment" )
		{
			dbinfo << myname  << "(createTables): create " << it.getName() << endl;
			ostringstream query;
			query << "CREATE TABLE " << conf->getProp(it, "name") << "(" << conf->getProp(it, "create") << ")";

			if( !db->query(query.str()) )
				dbcrit << myname << "(createTables): error: \t\t" << db->error() << endl;
		}
	}
}
//--------------------------------------------------------------------------------------------
void DBServer_MySQL::timerInfo( const UniSetTypes::TimerMessage* tm )
{
	DBServer::timerInfo(tm);

	switch( tm->id )
	{
		case DBServer_MySQL::PingTimer:
		{
			if( !db->ping() )
			{
				dbwarn << myname << "(timerInfo): DB lost connection.." << endl;
				connect_ok = false;
				askTimer(DBServer_MySQL::PingTimer, 0);
				askTimer(DBServer_MySQL::ReconnectTimer, ReconnectTime);
			}
			else
			{
				connect_ok = true;
				dbinfo << myname << "(timerInfo): DB ping ok" << endl;
			}
		}
		break;

		case DBServer_MySQL::ReconnectTimer:
		{
			dbinfo << myname << "(timerInfo): reconnect timer" << endl;

			if( db->isConnection() )
			{
				if( db->ping() )
				{
					connect_ok = true;
					askTimer(DBServer_MySQL::ReconnectTimer, 0);
					askTimer(DBServer_MySQL::PingTimer, PingTime);
				}

				connect_ok = false;
				dbwarn << myname << "(timerInfo): DB no connection.." << endl;
			}
			else
				initDBServer();
		}
		break;

		default:
			dbwarn << myname << "(timerInfo): Unknown TimerID=" << tm->id << endl;
			break;
	}
}
//--------------------------------------------------------------------------------------------
std::shared_ptr<DBServer_MySQL> DBServer_MySQL::init_dbserver( int argc, const char* const* argv, const std::string& prefix )
{
	auto conf = uniset_conf();

	ObjectId ID = conf->getDBServer();

	string name = conf->getArgParam("--" + prefix + "-name", "");

	if( !name.empty() )
	{
		ObjectId ID = conf->getObjectID(name);

		if( ID == UniSetTypes::DefaultObjectId )
		{
			cerr << "(DBServer_MySQL): Unknown ObjectID for '" << name << endl;
			return 0;
		}
	}

	uinfo << "(DBServer_MySQL): name = " << name << "(" << ID << ")" << endl;
	return make_shared<DBServer_MySQL>(ID, prefix);
}
// -----------------------------------------------------------------------------
void DBServer_MySQL::help_print( int argc, const char* const* argv )
{
	cout << "Default: prefix='mysql'" << endl;
	cout << "--prefix-name objectID     - ObjectID. Default: 'conf->getDBServer()'" << endl;
	cout << DBServer::help_print() << endl;
}
// -----------------------------------------------------------------------------
