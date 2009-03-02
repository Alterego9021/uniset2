/* This file is part of the UniSet project
 * Copyright (c) 2002 Free Software Foundation, Inc.
 * Copyright (c) 2002 Pavel Vainerman <pv>
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
 *  \author Pavel Vainerman <pv>
 *  \date   $Date: 2005/08/28 20:55:53 $
 *  \version $Id: DBServer_MySQL.h,v 1.1 2005/08/28 20:55:53 vpashka Exp $
*/
// -------------------------------------------------------------------------- 
#ifndef DBServer_MySQL_H_
#define DBServer_MySQL_H_
// --------------------------------------------------------------------------
#include <map>
#include "UniSetTypes.h"
#include "DBInterface.h"
#include "DBServer.h"
//------------------------------------------------------------------------------------------
/*!
 * \brief ���������� ������� ������� �� �� ������ MySQL

	\par
		��� ��������� ���������� DBServer ������������ ( DBServer_MySQL::PingTimer ) ��������� ������� ����� � �������� ��.
	� ������ ���� ����� ������� (��� �� ���� ����������� ��� ������) DBServer �������� ����� ������������ ( DBServer::ReconnectTimer )
	���������� ����������.	��� ���� ��� ������� ������� ��������� ��� ����� � ��, ������� � ���-����.
	\warning ��� ������ ������� ������������ ���������� DBServer ������ ������ ����. ����. ������� �� ����� ����������
	����� ���������.
		
	\todo ����� �� ��������� �����, ���� ����� ���... (��� �������� � ��)
*/
class DBServer_MySQL: 
	public DBServer
{
	public:
		DBServer_MySQL( UniSetTypes::ObjectId id );
		DBServer_MySQL();
		~DBServer_MySQL();

	protected:
		typedef std::map<int, std::string> DBTableMap;

		virtual void initDB(DBInterface *db){};
		virtual void initDBTableMap(DBTableMap& tblMap){};

		virtual void processingMessage( UniSetTypes::VoidMessage *msg );
		virtual void timerInfo( UniSetTypes::TimerMessage* tm );
		virtual void sysCommand( UniSetTypes::SystemMessage* sm );

		// ������� ��������� ��������� ���������
		virtual void parse( UniSetTypes::SensorMessage* sm );
		virtual void parse( UniSetTypes::DBMessage* dbmsg );
		virtual void parse( UniSetTypes::InfoMessage* imsg );
		virtual void parse( UniSetTypes::AlarmMessage* amsg );
		virtual void parse( UniSetTypes::ConfirmMessage* cmsg );		

		bool writeToBase( const string& query );

		virtual void init();
		void createTables( DBInterface* db );
		
		inline const char* tblName(int key)
		{
			return tblMap[key].c_str();
		}

		enum Timers
		{
			PingTimer,  	/*!< ������ �� ������������� �������� ����������  � �������� �� */
			ReconnectTimer 	/*!< ������ �� ��������� ������� ���������� � �������� �� (��� �������������� �����) */
		};


		DBInterface *db;
		int PingTime;
		int ReconnectTime;
		bool connect_ok; 	/*! ������� ������� ����������� � �������� �� */

		bool activate;

	private:
		DBTableMap tblMap;

};
//------------------------------------------------------------------------------------------
#endif
