/* This file is part of the UniSet project
 * Copyright (c) 2002 Free Software Foundation, Inc.
 * Copyright (c) 2002 Pavel Vainerman, Vitaly Lipatov
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
 *  \brief ����� ������ � �������������
 *  \author Vitaly Lipatov, Pavel Vainerman
 *  \date   $Date: 2008/02/21 19:59:57 $
 *  \version $Id: Configuration.h,v 1.25 2008/02/21 19:59:57 vpashka Exp $
 */
// --------------------------------------------------------------------------
#ifndef Configuration_H_
#define Configuration_H_
// --------------------------------------------------------------------------
// ������ UniXML.h, ������� unixml �� ���������!!!!!!!!
#include <string>
#include <ostream>
#include "UniXML.h"
#include "UniSetTypes.h"
#include "ObjectIndex.h"
#include "IORFile.h"
#include "MessageInterface.h"
#include "Debug.h"

class SystemGuard;
/*
	� ������� main ����� ������� ����������� ������ Configuration
	fileConf - �������� ����� ������������, ������� ����� �����������
	�� � �������� conf ���� ��������, ������ ���� �������� ���������.
	getTopDir ��������� �������� �������, ������ �������� ���������
*/

namespace UniSetTypes
{
	/*!
		 ������������ ������� 
		 \note � ������ ����������� ����������� ������ � ����������� ����� �����. 
		 ������������ ���������� � ���������� ������.
	*/
	class Configuration
	{
	public:
			virtual ~Configuration();

			/*!	���������������� xml-������ ( ���������������� ������ )	*/
			Configuration( int argc, char** argv, const std::string xmlfile="" );

			/*!	���������������� xml-������	*/
			Configuration( int argc, char** argv, ObjectIndex* oind, const std::string xmlfile="" );

			/*! ���������� �������, ��� ��������� ������ �������� */
			Configuration( int argc, char** argv, 
							const std::string fileConf, UniSetTypes::ObjectInfo* objectsMap );

		/// �������� �������� ����� � ��ԣ� path
		std::string getField(const std::string path);
		xmlNode* findNode(xmlNode* node, const std::string searchnode, const std::string name = "" );
		
		// �������� ����
		xmlNode* getNode(const std::string& path);
		// �������� ��������� �������� ����
		std::string getProp(xmlNode*, const std::string name);
		// �������� ��������� �������� �� ����� ����
		std::string getPropByNodeName(const std::string& nodename, const std::string& prop);

		static std::ostream& help(std::ostream& os);

		std::string getRootDir(); /*!< ��������� ��������, � ������� ��������� ������������� ��������� */
		inline int getArgc(){ return _argc; }
		inline char** getArgv() const { return _argv; } 
		inline ObjectId getTimerService() const { return localTimerService; } /*!< ��������� �������������� TimerServic-� */
		inline ObjectId getDBServer() const { return localDBServer; }		/*!< ��������� �������������� DBServer-� */
		inline ObjectId getInfoServer() const { return localInfoServer; }	/*!< ��������� �������������� InfoServer-� */
		inline ObjectId getLocalNode() const { return localNode; }		/*!< ��������� �������������� ���������� ���� */
		inline const std::string getNSName() const { return NSName; }		
	
		// repository
		inline std::string getRootSection() const { return secRoot; }
		inline std::string getSensorsSection() const { return secSensors; }
		inline std::string getObjectsSection() const { return secObjects; }
		inline std::string getControllersSection() const { return secControlles; }
		inline std::string getServicesSection() const { return secServices; }
		// xml
		xmlNode* getXMLSensorsSection();
		xmlNode* getXMLObjectsSection();
		xmlNode* getXMLControllersSection();
		xmlNode* getXMLServicesSection();
	
		// net
		inline unsigned int getCountOfNet() const { return countOfNet; }
		inline unsigned int getRepeatTimeout() const { return repeatTimeout; }
		inline unsigned int getRepeatCount() const { return repeatCount; }

		UniSetTypes::ObjectId getSensorID( const std::string name );
		UniSetTypes::ObjectId getControllerID( const std::string name );
		UniSetTypes::ObjectId getObjectID( const std::string name );
		UniSetTypes::ObjectId getServiceID( const std::string name );
		UniSetTypes::ObjectId getNodeID( const std::string name, const std::string alias="" );

		inline const std::string getConfFileName() const { return fileConfName; }
		inline std::string getImagesDir() const { return imagesDir; }	// ��������

		// dirs
		inline const std::string getConfDir() const { return confDir; }
		inline const std::string getDataDir() const { return dataDir; }
		inline const std::string getBinDir() const { return binDir; }		
		inline const std::string getLogDir() const { return logDir; }
		inline const std::string getLockDir() const { return lockDir; }
		inline const std::string getDocDir() const { return docDir; }


		inline bool isLocalIOR(){ return localIOR; }
		inline bool isTransientIOR(){ return transientIOR; }
		
		std::string getArgParam(const std::string name, const std::string defval="");
		xmlNode* initDebug( DebugStream& deb, const std::string& nodename );

		UniSetTypes::ListOfNode::const_iterator listNodesBegin()
		{
			return lnodes.begin();
		}

		inline UniSetTypes::ListOfNode::const_iterator listNodesEnd()
		{
			return lnodes.end();
		}
		
		/*! ��������� � ����� ���������	*/
		MessageInterface* mi;
		
		/*! ��������� � ����� �������� */
		ObjectIndex* oind;
		
		/*! ��������� � ������ � ����������� ior-������� */
		IORFile iorfile;

		/*! ��������� �� ���������������� xml */
		inline UniXML* getConfXML(){ return &unixml; }

		CORBA::ORB_ptr getORB() { return CORBA::ORB::_duplicate(orb); }
		CORBA::PolicyList getPolicy() const { return policyList; }

	protected:
		Configuration();

		virtual void initConfiguration(int argc, char** argv);
		
		void createNodesList();
		virtual void initNode( UniSetTypes::NodeInfo& ninfo, UniXML_iterator& it);

		void initRepSections();
		std::string getRepSectionName(const std::string sec, xmlNode* secnode=0 );
		void setConfFileName(const std::string fn="");
		void initParameters();
		void setLocalNode( std::string nodename );
		
		std::string getPort();
			
		friend class ::SystemGuard;
//		friend bool SystemGuard::pingNode();
		std::string rootDir;
		UniXML unixml;

		int _argc;
		char** _argv;
		CORBA::ORB_var orb;
		CORBA::PolicyList policyList;
		
		const std::string NSName;		/*!< ��� ������� ���������� �� ������ ������ (������ "NameService") */
		unsigned int countOfNet;	/*!< ���������� ��������� ������� */
		unsigned int repeatCount;	/*!< ���������� ������� �������� ������ � ���������� �������
											������ ��� ����� ���������� ���������� TimeOut.		*/

		unsigned int repeatTimeout;	/*!< ����� ����� ��������� [��] */

		UniSetTypes::ListOfNode lnodes;

		// repository
		std::string secRoot;
		std::string secSensors;
		std::string secObjects;
		std::string secControlles; 
		std::string secServices;

		// xml
		static xmlNode* xmlSensorsSec;
		static xmlNode* xmlObjectsSec;
		static xmlNode* xmlControllersSec;
		static xmlNode* xmlServicesSec;

		ObjectId localTimerService;
		ObjectId localDBServer;
		ObjectId localInfoServer;
		ObjectId localNode;
		
		std::string fileConfName;
		std::string imagesDir;

		std::string confDir;
		std::string dataDir;
		std::string binDir;
		std::string logDir;
		std::string docDir;
		std::string lockDir;
		bool localIOR;
		bool transientIOR;
	};

	/*! ���������� ��������� �� ������������ */
	extern Configuration* conf;
	
	/*! ���������� ������ ��� ������ ����� */
	extern DebugStream unideb;
	
	
	// ������������� UniSetTypes::conf.
	// ( ����������� ��������� ��������� ������ --confile � --id-from-config )
	void uniset_init( int argc, char* argv[], const std::string xmlfile="configure.xml" );
	
	
}	// end of UniSetTypes namespace

#endif // Configuration_H_
