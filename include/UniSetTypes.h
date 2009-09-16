/* This file is part of the UniSet project
 * Copyright (c) 2002-2005 Free Software Foundation, Inc.
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
 *  \date   $Date: 2009/01/16 23:16:42 $
 *  \version $Id: UniSetTypes.h,v 1.12 2009/01/16 23:16:42 vpashka Exp $
 *	\brief ������� ���� ���������� UniSet
*/
// -------------------------------------------------------------------------- 
#ifndef UniSetTypes_H_
#define UniSetTypes_H_
// -------------------------------------------------------------------------- 
#include <cstdlib>
#include <cstdio>
#include <string>
#include <list>
#include <limits>
#include <ostream>

#include <omniORB4/CORBA.h>
#include "UniSetTypes_i.hh"
#include "Mutex.h"
// -----------------------------------------------------------------------------------------
/*! �������� � ������������� */
inline void msleep( unsigned int m ) { usleep(m*1000); }

/*! ����������� ������� ����� ���������� UniSet */
namespace UniSetTypes
{
	typedef std::list<std::string> ListObjectName;	/*!< ������ �������� ���� ObjectName */

	typedef ObjectId SysId;
	typedef	CORBA::Object_ptr ObjectPtr;	/*!< ������ �� ������ �������������� � ObjectRepository */
	typedef	CORBA::Object_var ObjectVar;	/*!< ������ �� ������ �������������� � ObjectRepository */

	/*! ������� ������ ObjectType �� const char * (��������� const-������ � �������, ��� �����, �� �� ������� �� ������ � �ţ :) )  */
	inline static UniSetTypes::ObjectType getObjectType(const char * name) { const void *t = name;  return (UniSetTypes::ObjectType)t; }

	UniversalIO::IOTypes getIOType( const std::string s );
	std::ostream& operator<<( std::ostream& os, const UniversalIO::IOTypes t );

	/*! ������� ��� ���������� ���������� */
	enum LampCommand
	{
		lmpOFF		= 0,	/*!< ��������� */
		lmpON		= 1,	/*!< �������� */
		lmpBLINK	= 2,	/*!< ������ */
		lmpBLINK2	= 3,	/*!< ������ */
		lmpBLINK3	= 4		/*!< ������ */
	};

	static const long ChannelBreakValue = std::numeric_limits<long>::max();

	class IDList
	{
		public: 
			IDList();
			~IDList();

			void add( ObjectId id );
			void del( ObjectId id );
	
			inline int size(){ return lst.size(); }
			inline bool empty(){ return lst.empty(); }
		
			std::list<ObjectId> getList();

			// �� ������������ ��������� ������
			// �������� ����������!
			IDSeq* getIDSeq();
		
			// 
			ObjectId getFirst();
			ObjectId node;	// ����, �� ������� ��������� �������
		
		private:
			std::list<ObjectId> lst;
	};

	const ObjectId DefaultObjectId = -1;	/*!< ������������� ������� �� ��������� */

//	typedef long MessageCode;					
	const MessageCode DefaultMessageCode = 0;	/*!< ��� ������� ��������� */

	const ThresholdId DefaultThresholdId = -1;  	/*!< ������������� ������� �� ��������� */
	const ThresholdId DefaultTimerId = -1;  	/*!< ������������� ������� �� ��������� */
	
	
	/*! ���������� � ��������� */
	struct MessageInfo
	{
	   UniSetTypes::MessageCode code;	/*!< ������������� */
	   std::string text;				/*!< ����� */
	   std::string idname;				/*!< ��������� �������� �������������� */
	};

	/*! ���������� �� ����� ������� */
	struct ObjectInfo
	{
	    ObjectId id;		/*!< ������������� */
	    char* repName;		/*!< ��������� ��� ��� ����������� � ����������� */
	    char* textName;		/*!< ��������� ��� */
	};
	
	typedef std::list<NodeInfo> ListOfNode;

	/*! ����������� ��� ������������� � ������ �������� ������� */
	const char BadSymbols[]={'.','/'};

	class uniset_mutex;
	class uniset_mutex_lock;


	typedef long KeyType;	/*!< ���������� ���� ������� */
	
	/*! ��������� ����������� �������������� �����
	 *	������������ ������������� ������ ��� ���� ��������
	 *  id � node.
	 * \warning ������������ ������������� ����� ��� �� �����������, 
		 �� ��������� �� ������������� ���� �� ���� :)
	*/ 
	inline static KeyType key( UniSetTypes::ObjectId id, UniSetTypes::ObjectId node )
	{
		return KeyType((id*node)+(id+2*node));
	}

	/*! ��������� ��������� ��������� ������ 
		\param name - �������� ���������
		\param defval - ��������, ������� ����� ����������, ���� �������� �� ������
	*/
	inline std::string getArgParam( const std::string name, 
										int _argc, const char* const* _argv,
											const std::string defval="" )
	{
		for( int i=1; i < (_argc - 1) ; i++ )
		{
			if( name == _argv[i] )
				return _argv[i+1];
		}
		return defval;
	}

	/*! �������� ������� ��������� � ��������� ������
		\param name - �������� ���������
		\return ���������� -1, ���� �������� �� ������. 
			��� ������� ���������, ���� ������.
	*/
	inline int findArgParam( const std::string name, int _argc, const char* const* _argv )
	{
		for( int i=1; i<_argc; i++ )
		{
			if( name == _argv[i] )
				return i;
		}
		return -1;
	}

	/*! �������� ����������� ��������� ������������������ ��������������� ������� */
	template<typename InputIterator,
			 typename OutputIterator,
			 typename Predicate>
	OutputIterator copy_if(InputIterator begin,
							InputIterator end,
							OutputIterator destBegin,
							Predicate p)
	{
		while( begin!=end)
		{
			if( p(*begin) ) &destBegin++=*begin;
			++begin;
		}
		return destBegin;
	}

	// ������� ���������� ��������
	// raw 		- ������������� ��������
	// rawMin 	- ����������� ������� ��������� ���������
	// rawMax 	- ������������ ������� ��������� ���������
	// calMin 	- ����������� ������� �������������� ���������
	// calMin 	- ����������� ������� �������������� ���������
	// limit	- �������� �������� �������� �� �������� 
	float fcalibrate(float raw, float rawMin, float rawMax, float calMin, float calMax, bool limit=true );
	long lcalibrate(long raw, long rawMin, long rawMax, long calMin, long calMax, bool limit=true );

	// ��������� �������� � ������ ��������
	long setinregion(long raw, long rawMin, long rawMax);
	// ��������� �������� ��� ���������
	long setoutregion(long raw, long rawMin, long rawMax);


	/// �������������� ������ � ����� (������������ ������� 0, ��� 8-���, ������� 0x, ��� 16-���, ����� ��� �����. �����)
	inline int uni_atoi( const char* str )
	{
		int n = 0; // if str is NULL or sscanf failed, we return 0

		if ( str != NULL )
			std::sscanf(str, "%i", &n);
		return n;
	}
	inline int uni_atoi( const std::string str )
	{
		return uni_atoi(str.c_str());
	}

	bool file_exist( const std::string filename );
	
	IDList explode( const std::string str, char sep=',' );
}

#define atoi atoi##_Do_not_use_atoi_function_directly_Use_getIntProp90,_getArgInt_or_uni_atoi

// -----------------------------------------------------------------------------------------
#endif
