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
 */
// --------------------------------------------------------------------------

#ifndef Storages_H_
#define Storages_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>

/*! Эти 2 значения используются в TableBlockStorage как флаги для поля count, чтобы не вводить лишнее поле
в структуре TableBlockStorageElem */
#define EMPTY_BLOCK -5
#define EMPTY_ELEM -1

#define key_size 20

/*! Заголовок таблицы с элементами класса TableBlockStorage */
struct StorageAttr
{
	int k_size, inf_size,size,block_number;
	int lim, seekpos;
} __attribute__((__packed__));

/*! Заголовок журнала с элементами класса CycleStorage */
struct CycleStorageAttr
{
	int size, inf_size, seekpos;
} __attribute__((__packed__));

/*! Основная структура класса TableStorage */
struct TableStorageElem
{
	char status;
	char key[key_size];
} __attribute__((__packed__));

/*! Основная структура класса TableBlockStorage */
struct TableBlockStorageElem
{
	int count;
} __attribute__((__packed__));

/*! Основная структура класса CycleStorage */
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
		/*! Конструктор по умолчанию не открывает и не создает новой таблицы */
		TableBlockStorage();

		/*! Конструктор вызывает функцию Open, а при параметре create=true создает новую таблицу при
		несовпадении заголовков или отсутствии старой */
		TableBlockStorage(const char* name, int byte_sz, int key_sz, int inf_sz, int inf_count, int block_num, int block_lim, int seek, bool create=false);

		~TableBlockStorage();

		/*!
			\param inf_sz - размер поля информации,
			\param key_sz - размер поля ключа,
			\param inf_count - кол-во записей в одном блоке таблицы (размером inf_sz+key_sz
			+sizeof(int) на ключевое поле)
			\param block_num - кол-во блоков (при этом кол-во записей во всей таблице = inf_count*block_num,
			\param block_lim - число перезаписей на блок,
			\param seek - отступ от начала файла (указывает место, где расположена таблица) 

			размер всей таблицы будет равен inf_count*block_num*(inf_sz+key_sz+sizeof(int)) +
			sizeof(StorageAttr), где последнее слагаемое - размер заголовка,
			размер можно получить, вызвав функцию getByteSize()
		*/
		bool open(const char* name, int byte_sz, int inf_sz, int key_sz, int inf_count, int block_num, int block_lim, int seek);
		bool create(const char* name, int byte_sz, int inf_sz, int key_sz, int inf_count, int block_num, int block_lim, int seek);

		/*! Добавление информации по ключу, возможна перезапись при совпадении ключа с существующим */
		bool addRow(void* key, void* val);

		/*! Удаление информации по ключу, фактически освобождения места не происходит, оно только помечается удаленным*/
		bool delRow(void* key);

		/*! Поиск информации по ключу, при неудаче возвращается 0 */
		void* findKeyValue(void* key, void* val);

		/*! Получение текущего блока (для тестовой программы) */
		int getCurBlock(void);

		inline int getByteSize() { return (size*full_size + sizeof(StorageAttr)); }

		bool checkAttr( int key_sz, int inf_sz, int sz, int block_num, int block_lim, int seek );

	protected:
		FILE *file;
		int inf_size;
	private:
		int max,cur_block;
		TableBlockStorageElem* mem;
		int k_size, lim,seekpos;
		int size,block_size,block_number,full_size;
		void filewrite(int seek, bool needflush=true);
		bool copyToNextBlock();
		bool keyCompare(int i, void* key);
		void* keyPointer(int num);
		void* valPointer(int num);
		TableBlockStorageElem* elemPointer(int num);
};

class CycleStorage
{
	public:

		class CycleStorageIterator
		{
			public:
				typedef CycleStorageIterator Self;
				CycleStorageIterator():
					str(NULL), cs(NULL), current(0) {}
				CycleStorageIterator(CycleStorage* cstor)
				{
					cs = cstor;
					current = 0;
				}
				CycleStorageIterator(CycleStorage* cstor, int num)
				{
					cs = cstor;
					if( num<0 || num>=cs->getSize() ) current = 0;
					current = num;
				}

				void* operator *() const
				{
					return str;
				}

				Self& operator++();
				Self operator++(int);

				Self& operator--();
				Self operator--(int);

				inline bool operator==(const Self& other) const
				{
					if( str==NULL || other.str==NULL )
					{
						if( str==NULL && other.str==NULL )
							return true;
						else
							return false;
					}
					if( memcmp(str, other.str, cs->getInfSize())==0 )
						return true;
					return false;
				}

				inline bool operator!=(const Self& other) const
				{
					if( str==NULL || other.str==NULL )
					{
						if( str==NULL && other.str==NULL )
							return false;
						else
							return true;
					}
					if( memcmp(str, other.str, cs->getInfSize())==0 )
						return false;
					return true;
				}
			private:
				void* str;
				CycleStorage* cs;
				int current;
		};

		typedef CycleStorageIterator iterator;

		/*! Конструктор по умолчанию не открывает и не создает нового журнала */
		CycleStorage();

		/*! Конструктор вызывает функцию Open, а при параметре create=true создает новый журнал при
		несовпадении заголовков или отсутствии старого */
		CycleStorage(const char* name, int byte_sz, int inf_sz, int inf_count, int seek,bool create=false);

		~CycleStorage();

		/*! 
			\param inf_sz - размер поля информации, 
			\param inf_count - количество записей (размером inf_sz +1 на ключевое поле)
			\param seek - отступ от начала файла (указывает место, где расположен журнал)

			размер всего журнала будет равен inf_count*(inf_sz+1) + sizeof(CycleStorageAttr),
			где последнее слагаемое - размер заголовка
			размер можно получить, вызвав функцию getByteSize()
		*/
		bool open(const char* name, int byte_sz, int inf_sz, int inf_count, int seek);
		bool create(const char* name, int byte_sz, int inf_sz, int inf_count, int seek);
		bool isOpen(){ return (file!=NULL); }


		/*! Добавление информации в конец журнала */
		bool addRow(void* str);

		/*! Удаление информации с номером ряда row */
		bool delRow(int row);

		/*! Очистка журнала */
		bool delAllRows(void);

		/*! \return Функция возвращает информацию из ряда с номером num */
		void* readRow(int num, void* str);

		/*! Получение кол-ва итерации при поиске начала/конца журнала (для тестовой программы) */
		int getIter(void);

		/*! Изменение размера журнала (количества записей в нем) */
		bool setSize(int count);

		inline int getByteSize() { return (size*full_size + sizeof(CycleStorageAttr)); }
		inline int getSize(){ return size; }
		inline int getInfSize(){ return inf_size; }
		inline int getFullSize(){ return full_size; }

		bool checkAttr(int inf_sz, int inf_count, int seek);

		inline int getHead(){ return head; }
		inline int getTail(){ return tail; }

		iterator begin()
		{
			return iterator(this);
		}

		iterator end()
		{
			return iterator(this,this->getTail());
		}

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
