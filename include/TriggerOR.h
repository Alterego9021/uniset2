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
 *  \date   $Date: 2008/06/01 21:36:19 $
 *  \version $Id: TriggerOR.h,v 1.7 2008/06/01 21:36:19 vpashka Exp $
*/
// --------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifndef TRIGGER_OR_H_
#define TRIGGER_OR_H_
//---------------------------------------------------------------------------
#include <map>
//---------------------------------------------------------------------------
/*!
	������� \b "���", �� ���������� ������.
	������ ��������� ���������:
	- "1" �� ����� ����� ���� �� ������ "1"
	- "0" �� \b ���� ������ ���� �� ������ "0"	

	� ������������ ����������� �������, ������� ����� ���������� ��� \b ��������� ��������� ������.

	\warning ��� ������������ ����������� �������(�� ��������� �� ������ � ������������� �����).

	\par ������ �������������
	\code
	#include "TriggerOR.h"
	class MyClass
	{
		public:
			MyClass(){};
			~MyClass(){};
			void out( bool newstate){ cout << "OR out state="<< newstate <<endl;}
		...
	};

	...
	MyClass rec;
	// ��������
	TriggerOR<MyClass, int> tr_or(&rec, &MyClass::out);
	
	// ���������� '������'
	tr_or.add(1,true);
	tr_or.add(2,false);
	tr_or.add(3,false);
	tr_or.add(4,false);
	...
	// �������������
	// ������ �� ���� N1 "0"
	// ����� ����, ��� ��������� ��������� '������' ����� ������� ������� MyClass::out, � ������� ������������ 
	// ����������� ��������� '��������� ���������'
	tr_or.commit(1,false);
	\endcode
*/
template<class Caller, typename InputType>
class TriggerOR
{
	public:

		/*! 
			�������� ������� ������ 
			\prarm newstate - ����� ��������� '������'
		*/
		typedef void(Caller::* Action)(bool newstate);	
	
		TriggerOR(Caller* r, Action a);
		~TriggerOR();
		
		inline bool state(){ return out; }
		

		bool getState(InputType in);
		bool commit(InputType in, bool state);	

		void add(InputType in, bool state);		
		void remove(InputType in);				

		typedef std::map<InputType, bool> InputMap;

		inline typename InputMap::const_iterator begin()
		{
			return inputs.begin();
		}

		inline typename InputMap::const_iterator end()
		{
			return inputs.end();
		}

		void update();		
		void reset();

	protected:
		void check();

		InputMap inputs; // ������ ������
		bool out;
		Caller* cal;
		Action act;
		
};

//---------------------------------------------------------------------------
#include "TriggerOR_template.h"
//---------------------------------------------------------------------------
#endif
