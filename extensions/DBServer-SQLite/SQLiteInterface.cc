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
 *  \author Pavel Vainerman
*/
// --------------------------------------------------------------------------
#include <sstream>
#include <cstdio>
#include "UniSetTypes.h"
#include "SQLiteInterface.h"
// --------------------------------------------------------------------------
using namespace std;
using namespace UniSetTypes;
// --------------------------------------------------------------------------

SQLiteInterface::SQLiteInterface():
	db(0),
	lastQ(""),
	lastE(""),
	queryok(false),
	connected(false),
	opTimeout(300),
	opCheckPause(50)
{
}

SQLiteInterface::~SQLiteInterface()
{
	close();
}

// -----------------------------------------------------------------------------------------
bool SQLiteInterface::ping()
{
	return db && ( sqlite3_db_status(db, 0, NULL, NULL, 0) == SQLITE_OK );
}
// -----------------------------------------------------------------------------------------
bool SQLiteInterface::connect( const std::string& param )
{
	std::string dbfile;
	std::string::size_type pos = param.find_first_of(":");
	dbfile = param.substr(0, pos);
	
	if( pos != std::string::npos )
	{
		std::string create_str = param.substr(pos + 1, std::string::npos);

		if( create_str == "true" )
			return connect( dbfile, true );
	}

	return connect( dbfile, false );
}
// -----------------------------------------------------------------------------------------
bool SQLiteInterface::connect( const string& dbfile, bool create )
{
	// т.к. sqlite3 по умолчанию, создаёт файл при открытии, то проверим "сами"
	//    if( !create && !UniSetTypes::file_exist(dbfile) )
	//        return false;

	int flags = create ? 0 : SQLITE_OPEN_READWRITE;

	int rc = sqlite3_open_v2(dbfile.c_str(), &db, flags, NULL);

	if( rc != SQLITE_OK )
	{
		// cerr << "SQLiteInterface::connect): rc=" << rc << " error: " << sqlite3_errmsg(db) << endl;
		lastE = "open '" + dbfile + "' error: " + string(sqlite3_errmsg(db));
		sqlite3_close(db);
		db = 0;
		connected = false;
		return false;
	}

	setOperationTimeout(opTimeout);
	connected = true;
	return true;
}
// -----------------------------------------------------------------------------------------
bool SQLiteInterface::close()
{
	if( db )
	{
		sqlite3_close(db);
		db = 0;
	}

	return true;
}
// -----------------------------------------------------------------------------------------
void SQLiteInterface::setOperationTimeout( timeout_t msec )
{
	opTimeout = msec;

	if( db )
		sqlite3_busy_timeout(db, opTimeout);
}
// -----------------------------------------------------------------------------------------
bool SQLiteInterface::insert( const string& q )
{
	if( !db )
		return false;

	// char* errmsg;
	sqlite3_stmt* pStmt;

	// Компилируем SQL запрос
	if( sqlite3_prepare(db, q.c_str(), -1, &pStmt, NULL) != SQLITE_OK )
	{
		queryok = false;
		return false;
	}

	int rc = sqlite3_step(pStmt);

	if( !checkResult(rc) && !wait(pStmt, SQLITE_DONE) )
	{
		sqlite3_finalize(pStmt);
		queryok = false;
		return false;
	}

	sqlite3_finalize(pStmt);
	queryok = true;
	return true;
}
// -----------------------------------------------------------------------------------------
bool SQLiteInterface::checkResult( int rc )
{
	if( rc == SQLITE_BUSY || rc == SQLITE_LOCKED || rc == SQLITE_INTERRUPT || rc == SQLITE_IOERR )
		return false;

	return true;
}
// -----------------------------------------------------------------------------------------
DBResult SQLiteInterface::query( const string& q )
{
	if( !db )
		return DBResult();

	// char* errmsg = 0;
	sqlite3_stmt* pStmt;

	// Компилируем SQL запрос
	sqlite3_prepare(db, q.c_str(), -1, &pStmt, NULL);
	int rc = sqlite3_step(pStmt);

	if( !checkResult(rc) && !wait(pStmt, SQLITE_ROW) )
	{
		sqlite3_finalize(pStmt);
		queryok = false;
		return DBResult();
	}

	lastQ = q;
	queryok = true;
	DBResult dbres;
	makeResult(dbres, pStmt, true);
	return dbres;
}
// -----------------------------------------------------------------------------------------
bool SQLiteInterface::wait( sqlite3_stmt* stmt, int result )
{
	PassiveTimer ptTimeout(opTimeout);

	while( !ptTimeout.checkTime() )
	{
		sqlite3_reset(stmt);
		int rc = sqlite3_step(stmt);

		if(  rc == result || rc == SQLITE_DONE )
			return true;

		msleep(opCheckPause);
	}

	return false;
}
// -----------------------------------------------------------------------------------------
const string SQLiteInterface::error()
{
	if( db )
		lastE = sqlite3_errmsg(db);

	return lastE;
}
// -----------------------------------------------------------------------------------------
const string SQLiteInterface::lastQuery()
{
	return lastQ;
}
// -----------------------------------------------------------------------------------------
double SQLiteInterface::insert_id()
{
	if( !db )
		return 0;

	return sqlite3_last_insert_rowid(db);
}
// -----------------------------------------------------------------------------------------
bool SQLiteInterface::isConnection()
{
	return connected;
}
// -----------------------------------------------------------------------------------------
void SQLiteInterface::makeResult(DBResult& dbres, sqlite3_stmt* s, bool finalize )
{
	do
	{
		int n = sqlite3_data_count(s);

		if( n <= 0 )
		{
			if( finalize )
				sqlite3_finalize(s);

			return;
		}

		DBResult::COL c;

		for( int i = 0; i < n; i++ )
		{
			char* p = (char*)sqlite3_column_text(s, i);

			if( p )
				c.push_back(p);
			else
				c.push_back("");
		}

		dbres.row().push_back(c);
	}
	while( sqlite3_step(s) == SQLITE_ROW );

	if( finalize )
		sqlite3_finalize(s);
}
// -----------------------------------------------------------------------------------------
extern "C" DBInterface* create_sqliteinterface()
{
	return new SQLiteInterface();
}
// -----------------------------------------------------------------------------------------
extern "C" void destroy_sqliteinterface(DBInterface* p)
{
	delete p;
}
// -----------------------------------------------------------------------------------------
