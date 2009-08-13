/* This file is part of the UniSet project
 * Copyright (c) 2009 Free Software Foundation, Inc.
 * Copyright (c) 2009 Ivan Donchevskiy
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
 *  \author Ivan Donchevskiy
 *  \date   $Date: 2009/07/15 15:55:00 $
 *  \version $Id: Jrn.h,v 1.0 2009/07/15 15:55:00 vpashka Exp $
 */
// --------------------------------------------------------------------------

#ifndef Storages_H_
#define Storages_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

/*! ��� 2 �������� ������������ � TableBlockStorage ��� ����� ��� ���� count, ����� �� ������� ������ ����
� ��������� TableBlockStorageElem */
#define EMPTY_BLOCK -5
#define EMPTY_ELEM -1

#define key_size 20

/*! ��������� ������� � ���������� ������ TableBlockStorage */
struct StorageAttr
{
	int k_size, inf_size,size,block_number;
	int lim, seekpos;
} __attribute__((__packed__));

/*! ��������� ������� � ���������� ������ CycleStorage */
struct CycleStorageAttr
{
	int size, inf_size, seekpos;
} __attribute__((__packed__));

/*! �������� ��������� ������ TableStorage */
struct TableStorageElem
{
	char status;
	char key[key_size];
} __attribute__((__packed__));

/*! �������� ��������� ������ TableBlockStorage */
struct TableBlockStorageElem
{
	int count;
} __attribute__((__packed__));

/*! �������� ��������� ������ CycleStorage */
struct CycleStorageElem
{
	char status;
} __attribute__((__packed__));

class TableStorage
{
	FILE *file;
	int size,seekpos, inf_size;
	int head;
	public:
		TableStorage(const char* name, int inf_sz, int sz, int seek);
		~TableStorage();
		int addRow(char* key, char* val);
		int delRow(char* key);
		char* findKeyValue(char* key, char* val);
};

class TableBlockStorage
{
	public:
		/*! ����������� �� ��������� �� ��������� � �� ������� ����� ������� */
		TableBlockStorage();

		/*! ����������� �������� ������� Open, � ��� ��������� create=true ������� ����� ������� ���
		������������ ���������� ��� ���������� ������ */
		TableBlockStorage(const char* name, int key_sz, int inf_sz, int sz, int block_num, int block_lim, int seek, bool create=false);

		~TableBlockStorage();

		/*! inf_sz - ������ ���� ����������, key_sz - ������ ���� �����, sz - ������ �������
		block_num - ���-�� ������ (��� ���� ������ ������ ����� = sz/block_num ������ ���� ���������� ���
		���� ����������, ������������ � �������), block_lim - ����� ����������� �� ����,
		seek - ������ �� ������ ����� (��������� �����, ��� ����������� �������) */
		bool open(const char* name, int inf_sz, int key_sz, int sz, int block_num, int block_lim, int seek);
		bool create(const char* name, int inf_sz, int key_sz, int sz, int block_num, int block_lim, int seek);

		/*! ���������� ���������� �� �����, �������� ���������� ��� ���������� ����� � ������������ */
		bool addRow(void* key, void* val);

		/*! �������� ���������� �� �����, ���������� ������������ ����� �� ����������, ��� ������ ���������� ���������*/
		bool delRow(void* key);

		/*! ����� ���������� �� �����, ��� ������� ������������ 0 */
		void* findKeyValue(void* key, void* val);

		/*! ��������� �������� ����� (��� �������� ���������) */
		int getCurBlock(void);
	protected:
		FILE *file;
		int inf_size;
	private:
		int max,cur_block;
		TableBlockStorageElem** mem;
		int k_size, lim,seekpos;
		int size,block_size,block_number,full_size;
		void filewrite(int seek, bool needflush=true);
		bool copyToNextBlock();
		bool keyCompare(int i, void* key);
		void* keyPointer(int num);
		void* valPointer(int num);
};

class CycleStorage
{
	public:
		/*! ����������� �� ��������� �� ��������� � �� ������� ������ ������� */
		CycleStorage();

		/*! ����������� �������� ������� Open, � ��� ��������� create=true ������� ����� ������ ���
		������������ ���������� ��� ���������� ������� */
		CycleStorage(const char* name, int inf_sz, int inf_count, int seek,bool create=false);

		~CycleStorage();

		/*! 
			\param inf_sz - ������ ���� ����������, 
			\param inf_count - ���������� ������� (�������� inf_sz +1 �� �������� ����)
			\param seek - ������ �� ������ ����� (��������� �����, ��� ���������� ������)

			������ ����� ������� ����� ����� inf_count*(inf_sz+1) + sizeof(CycleStorageAttr),
			��� ��������� ��������� - ������ ���������
		*/
		bool open(const char* name, int inf_sz, int inf_count, int seek);
		bool create(const char* name, int inf_sz, int inf_count, int seek);
		bool isOpen(){ return (file!=NULL); }


		/*! ���������� ���������� � ����� ������� */
		bool addRow(void* str);

		/*! �������� ���������� � ������� ���� row */
		bool delRow(int row);

		/*! ������� ������� */
		bool delAllRows(void);

		/*! \return ������� ���������� ���������� �� ���� � ������� num */
		void* readRow(int num, void* str);

		/*! ��������� ���-�� �������� ��� ������ ������/����� ������� (��� �������� ���������) */
		int getIter(void);
		
		inline int getSize(){ return size; }
		inline int getInfSize(){ return inf_size; }
		inline int getFullSize(){ return full_size; }
	protected:
		FILE *file;
		int inf_size;
		int head,tail;
	private:
		int size,seekpos, iter;
		int full_size;
		void filewrite(CycleStorageElem* jrn,int seek, bool needflush=true);
		void* valPointer(void* pnt);
		bool findHead();
};

#endif
