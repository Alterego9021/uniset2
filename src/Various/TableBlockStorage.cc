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
 *  \version $Id: Jrn.h,v 1.0 2009/07/15 15:55:00vpashka Exp $
 */
// --------------------------------------------------------------------------

/*!	������� ������ TableBlockStorage, ������� ����-�������� � ������������ ���-��� ����������� ���
	������� ����� ������, ��� ���������� �������, ���������� ������� � ��������� ����
*/

#include "Storages.h"

TableBlockStorage::TableBlockStorage()
{
	file=NULL;
}

TableBlockStorage::TableBlockStorage(const char* name, int key_sz, int inf_sz, int inf_count, int block_num, int block_lim, int seek, bool cr)
{
	file=NULL;
	if(!open(name, key_sz, inf_sz, inf_count, block_num, block_lim, seek))
	{
		if(cr)
			create(name, key_sz, inf_sz, inf_count, block_num, block_lim, seek);
		else
			file=NULL;
	}
}

TableBlockStorage::~TableBlockStorage()
{
	delete mem;

	if(file!=NULL) fclose(file);
}

bool TableBlockStorage::keyCompare(int i, void* key)
{
	return !memcmp((char*)elemPointer(i)+sizeof(TableBlockStorageElem),key,k_size);
}

TableBlockStorageElem* TableBlockStorage::elemPointer(int num)
{
	return (TableBlockStorageElem*)((char*)mem+num*full_size);
}

void* TableBlockStorage::keyPointer(int num)
{
	return (char*)elemPointer(num) + sizeof(TableBlockStorageElem);
}

void* TableBlockStorage::valPointer(int num)
{
	return (char*)elemPointer(num) + sizeof(TableBlockStorageElem) + k_size;
}

void TableBlockStorage::filewrite(int seek, bool needflush)
{
	/*! ������ �������� � ������� i �� ������ � ������� ���� ����� */
	fseek(file,seekpos+(seek+cur_block*block_size)*full_size,0);
	fwrite(elemPointer(seek),full_size,1,file);
	if(needflush) fflush(file);
}

bool TableBlockStorage::copyToNextBlock(void)
{
	/*! ������� �� ��������� ���� ����� */
	max=-1;

	int tmp=mem->count;
	mem->count=EMPTY_BLOCK;
	filewrite(0,false);
	mem->count=tmp;

	if(cur_block>=block_number-1)
		cur_block=0;
	else
		cur_block++;

	/*! ����������� ������ ����������� �������� ������� */
	for(int i=0;i<block_size;i++)
	{
		if(elemPointer(i)->count>=0)
		{
			elemPointer(i)->count=++max;
			filewrite(i,false);
		}
	}

	fflush(file);
	/*! ���� ��������� ������������, ������������ false */ 
	if(cur_block>=block_number-1)
		return false;

	return true;
}

bool TableBlockStorage::open(const char* name, int key_sz, int inf_sz, int inf_count, int block_num, int block_lim, int seek)
{
	/*! ���� ��� ��� ������ ���� � ���������� ������� ������, �� ����������� � ����������� ����� */
	if(file!=NULL) fclose(file);

	file = fopen(name, "r+");
	if(file==NULL) return false;

	seekpos=seek;
	if(fseek(file,seekpos,0)==-1)
	{
		fclose(file);
		file=NULL;
		return false;
	}

	/*! ������ ��������� ������� */
	StorageAttr sa;
	fread(&sa,sizeof(StorageAttr),1,file);

	full_size = sizeof(TableBlockStorageElem)+key_sz+inf_sz;

	int tmpsize=inf_count*block_num;

	/*! ��������� ��������� �� ���������� � ������ ���������� */
	if((sa.k_size!=key_sz)||(sa.inf_size!=inf_sz)||(sa.size!=tmpsize)||(sa.block_number!=block_num)||(sa.lim!=block_lim)||(sa.seekpos!=seek))
	{
		fclose(file);
		file=NULL;
		return false;
	}

	k_size=key_sz;
	inf_size=inf_sz;
	lim=block_lim;
	block_number=block_num;
	block_size=inf_count;
	size=block_size*block_num;

	max=-1;

	/*! ������������� ������ */
	mem = (TableBlockStorageElem*) new char[block_size*full_size];

	TableBlockStorageElem *t = (TableBlockStorageElem*)new char[full_size];
	
	seekpos+=sizeof(StorageAttr);
	/*! ����� ��������� �����, ���� ���� ��� ������, ������� ��������������� 0 */
	for(cur_block=0; cur_block < block_num; cur_block++)
	{
		fseek(file,seekpos+cur_block*block_size*(full_size),0);
		fread(t,(full_size),1,file);
		if(t->count >= 0) 
			break;
	}

	/*! ������ � ������ �� ������� ����� */
	fseek(file,seekpos+(cur_block*block_size)*(full_size),0);
	for(int i=0;i<block_size;i++)
	{
		fread(elemPointer(i),(full_size),1,file);
		if(elemPointer(i)->count>max) max=elemPointer(i)->count;
	}
	delete t;
	return true;
}

bool TableBlockStorage::create(const char* name, int key_sz, int inf_sz, int inf_count, int block_num, int block_lim, int seek)
{
	if(file!=NULL) fclose(file);
	file = fopen(name, "r+");
	if(file==NULL)
	{
		FILE*f=fopen(name,"w");
		fclose(f);
		file = fopen(name, "r+");
	}
	k_size=key_sz;
	inf_size=inf_sz;
	seekpos=seek;
	lim=block_lim;
	full_size = sizeof(TableBlockStorageElem)+k_size+inf_size;
	int i;


	block_number=block_num;
	block_size=inf_count;
	size=block_size*block_num;
	max=-1;

	if(fseek(file,seekpos,0)==-1) return false;

	/*! ������������� ������ */
	mem = (TableBlockStorageElem*) new char[block_size*full_size];

	for( i=0; i<block_size*full_size; i++ )
		*((char*)mem+i) = 0;

	StorageAttr sa;
	sa.k_size=k_size;
	sa.inf_size=inf_size;
	sa.size=size;
	sa.block_number=block_number;
	sa.lim=lim;
	sa.seekpos=seekpos;

	/*! ������ ��������� ������� */
	cur_block=0;
	fwrite(&sa,sizeof(StorageAttr),1,file);
	fflush(file);
	seekpos+=sizeof(StorageAttr);

	/*!	���� �������� ������� ��� �������� ������ ������ �� �������������� ����� � �� ������� ������ ������:
		EMPTY_BLOCK=(-5) - ����������� ������ �������� ������� �����, ���� ��� ������ ��������, �� ���� ���� ������������, EMPTY_ELEM=(-1) - ��� ��������� ������ ������
	*/

	mem->count=EMPTY_BLOCK;
	for(i=1;i<block_size;i++)
		elemPointer(i)->count=EMPTY_ELEM;

	/*!	���� �������������� ��� ����� � �����*/
	for(i=0;i<size;i++) 
	{
		if((i!=0)&&(i%block_size==0)) cur_block++;
		filewrite(i%block_size,false);
	}
	cur_block=0;
	fflush(file);

	return true;
}

bool TableBlockStorage::addRow(void* key, void* value)
{
	int i=0,pos=-1,empty=-1;
	if(file==NULL) return false;

	if(max==lim-1) copyToNextBlock();
	for(i=0;i<block_size;i++)
	{
		if(elemPointer(i)->count>=0)
			if(keyCompare(i,key)) pos = i;
		if((elemPointer(i)->count<0)&&(empty<0)) empty=i;
	}

	/*! ���� ����� ���������� �����, �� pos>=0, ���������� �� ��� �����, ����� ����� �� ������ ����� empty */
	if(pos>=0) empty=pos;
	else
	{
		if( empty<0 ) return false; /*! ���������� false, ���� ����� � ����� ����������� */
		memcpy(keyPointer(empty),key,k_size);
	}

	elemPointer(empty)->count=++max;
	memcpy(valPointer(empty),value,inf_size);
	filewrite(empty);
	return true;
}

bool TableBlockStorage::delRow(void* key)
{
	int i;
	if(file==NULL) return false;

	/*! ��� �������� ������� ����������� ����� ������������� */
	if(max==lim-1) copyToNextBlock();
	for(i=0;i<block_size;i++)
	{
		if(elemPointer(i)->count < 0)
			continue;
		if(keyCompare(i,key))
		{
			elemPointer(i)->count=++max;
			memset(keyPointer(i),0,k_size);
			filewrite(i);
			return true;
		}
	}
	return false;
}

/*! TODO: ����� ������ �� ���������� val, ������ ���������� �������� */
void* TableBlockStorage::findKeyValue(void* key, void* val)
{
	int i;
	if(file==NULL) return 0;
	for(i=0;i<block_size;i++)
	{
		/*! ���������� ����� ������ ���� ������� >= 0, �.�. ������ ���������� */
		if(elemPointer(i)->count < 0)
			continue;
		if(keyCompare(i,key))
		{
			memcpy(val,valPointer(i),inf_size);
			return val;
		}
	}
	return NULL;
}

int TableBlockStorage::getCurBlock()
{
	return cur_block;
}

