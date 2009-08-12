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

/*!	������� ������ CycleStorage, ���������� ��������������� �������,
	���������������� ����� ������ ��������� ��� ������ ���������� �������
*/

#include "Storages.h"

CycleStorage::CycleStorage()
{
	file=NULL;
}

CycleStorage::CycleStorage(const char* name, int inf_sz, int inf_count, int seek, bool cr):
	file(NULL)
{
	if(!open(name,inf_sz, inf_count, seek))
	{
		if(cr)
			create(name,inf_sz, inf_count, seek);
	}
}

CycleStorage::~CycleStorage()
{
	fclose(file);
}

void* CycleStorage::valPointer(void* pnt)
{
	return (char*)pnt+sizeof(CycleStorageElem);
}

void CycleStorage::filewrite(CycleStorageElem* jrn,int seek,bool needflush)
{
	fseek(file,seekpos+seek*full_size,0);
	fwrite(jrn,full_size,1,file);
	if(needflush) fflush(file);
}

/*! 	���� ������ � ����� �� ��������� jrn->status: 0 - �����, 1 - �����, 2,4 - ��� ���� ������������ ���������
	(��� 2 ����� �� ������ - 4 ������, ��� ��������), 3 - ��������� 2, 5 - ��������� 4, 6 - ��������� 1.
	��� ���������� ������|������ ������������ �������� �����
*/
void CycleStorage::findHead()
{
	CycleStorageElem *jrn = (CycleStorageElem*)new char[full_size];
	int l=-1,r=size,mid;
	iter=0;
	seekpos+=sizeof(CycleStorageAttr);
	fread(jrn,full_size,1,file);
	if(jrn->status==0)
	{
		head=-1;
		tail=-1;
	}
	else if((jrn->status==1)||(jrn->status==6))
	{
		/*! ���� ������ ������� - ������ ������ */
		head=0;
		/*! � ���� ������ ������ ����� ���� � ��������� ��������, ������� ����� ����� ������ */
		while(r - l > 1)
		{
			mid = (l+r)/2;
			fseek(file,seekpos+mid*full_size,0);
			fread(jrn,full_size,1,file);
			iter++;
			if(jrn->status==0)
				r = mid;
			else l=mid;
		}
		if(r<size)
		{
			tail=r-1;
		}
		else tail=size-1;
	}
	else
	{
		int i,j;
		/*! i ����� ��� 2-��, ��� 4-��, � ����������� �� ����� ����� ������ ������ 2-��, � ������ 4-��
		��� �������� */
		i=jrn->status-jrn->status%2;
		if(i==2) j=4; else j=2;
		while((jrn->status!=1)&&(jrn->status!=6)&&(r - l > 1))
		{
			mid = (l+r)/2;
			fseek(file,seekpos+mid*full_size,0);
			fread(jrn,full_size,1,file);
			iter++;
			if((jrn->status==i)||(jrn->status==i+1))
				l = mid;
			else if((jrn->status==j)||(jrn->status==j+1))
				r = mid;
			else
			{
				r=mid;
				break;
			}
		}
		if(r<size)
			head=r;
		else head=size-1;
		/*! ����� ������ �� 1 ����� ������, �.�. ���� ������ �� � ������, �� ������ ���� �������� */
		tail=head-1;
		if(tail<0) tail=size-1;
	}
	delete jrn;
}

bool CycleStorage::open(const char* name, int inf_sz, int inf_count, int seek)
{
	/*! 	���� ��� ��� ������ ���� � ���������� ������� ������, �� ����������� � ����������� �����
	*/

	if(file!=NULL) fclose(file);
	file = fopen(name, "r+");
	if(file==NULL) return false;

	seekpos=seek;
	if(fseek(file,seekpos,0)==-1) return false;

	/*! ������ ��������� */
	CycleStorageAttr csa;
	fread(&csa,sizeof(csa),1,file);

	int sz = inf_sz * inf_count;

	/*! ��������� ��������� �� ���������� � ������ ���������� */
	if((csa.size!=(int)((sz-sizeof(csa))/(sizeof(CycleStorageElem)+inf_sz)))||(csa.inf_size!=inf_sz)||(csa.seekpos!=seek))
		return false;

	inf_size=inf_sz;
	full_size=sizeof(CycleStorageElem)+inf_size;
	size=(sz-sizeof(csa))/full_size;
	seekpos=seek;

	seekpos+=sizeof(CycleStorageAttr);
	findHead();
	return true;
}

bool CycleStorage::create(const char* name, int inf_sz, int inf_count, int seek)
{
	if(file!=NULL) fclose(file);
	file = fopen(name, "r+");
	/*! ������� ����, ���� ��� ��� */
	if(file==NULL)
	{
		FILE*f=fopen(name,"w");
		fclose(f);
		file = fopen(name, "r+");
	}

	int sz = inf_sz * inf_count;

	inf_size=inf_sz;
	full_size=sizeof(CycleStorageElem)+inf_size;
	
	size=(sz-sizeof(CycleStorageAttr))/full_size;
	iter=0;
	seekpos=seek;

	if(fseek(file,seekpos,0)==-1) return false;

	CycleStorageElem *jrn = (CycleStorageElem*)new char[full_size];
	jrn->status=0;

	/*! ���������� ��������� ������� */
	CycleStorageAttr *csa = new CycleStorageAttr();
	csa->inf_size=inf_size;
	csa->size=size;
	csa->seekpos=seekpos;

	fwrite(csa,sizeof(CycleStorageAttr),1,file);
	fflush(file);
	seekpos+=sizeof(CycleStorageAttr);
	delete csa;

	/*! ������� ������ ������� ������� */
	for(int i=0;i<size;i++)
	{
		filewrite(jrn,i,false);
	}
	fflush(file);
	
	/*!	���������� ���������� �����, ����� ������ ������� ����� ������� �����, ������� �������� � ���������� */
	int emp = sz-size*full_size-sizeof(CycleStorageAttr);
	if(emp>0)
	{
		char* empty= new char[emp];
		fwrite(empty,emp,1,file);
		fflush(file);
		delete empty;
	}

	head=tail=-1;
	delete jrn;
	return true;
}

bool CycleStorage::addRow(void* str)
{
	if(file==NULL) return false;
	CycleStorageElem *jrn = (CycleStorageElem*)new char[full_size];
	int i=0;

	/*!	������ 2 ������ - ������ ���� (head=-1), � ������ 1 �������(head=tail=0) ������������ ��������)
	*/

	memcpy(valPointer(jrn),str,inf_size);
	if(head==-1)
	{
		jrn->status=1;
		filewrite(jrn,0);
		head=0;
		tail=0;
		delete jrn;
		return true;
	}
	if(head==tail)
	{
		jrn->status=2;
		filewrite(jrn,1);
		tail=1;
		delete jrn;
		return true;
	}
	fseek(file,seekpos+tail*full_size,0);
	fread(jrn,full_size,1,file);

	/*!	������ �������� ��������� �� �������� ���������� �������� � ������� 2, 3 -> 2; 4, 5 -> 4
	*/

	if((jrn->status==2)||(jrn->status==3)) i=2;
	else i=4;

	/*!	���� ��������� ������� ������� � ������� ������ ������� ���������� ����� �����,
		��������� � ������ ����� ����� � ����������� ������ 2->4, 4->2
	*/

	if(tail==size-1)
	{
		fseek(file,seekpos,0);
		tail=0;
		if(i==2) i=4;
		else i=2;
	}
	else tail++;
	fread(jrn,full_size,1,file);
	memcpy(valPointer(jrn),str,inf_size);
	if(jrn->status==0)
	{
		jrn->status=2;
		filewrite(jrn,tail);
	}
	else
	{
		/*!	������������ �������� ������� �����, � ������ �� ������ ������ ��������,
			�. �. �������� ������ �� 1 ������� ������
		*/

		head++;
		if(head>=size) head=0;
		jrn->status=i;
		filewrite(jrn,tail);
		fseek(file,seekpos+head*full_size,0);
		fread(jrn,full_size,1,file);
		if((jrn->status==3)||(jrn->status==5)) jrn->status=6;
		else jrn->status=1;
		filewrite(jrn,head);
	}
	delete jrn;
	return true;
}

bool CycleStorage::delRow(int row)
{
	int i=(head+row)%size,j;
	if( row >= size ) return false;
	if(file==NULL) return false;
	CycleStorageElem *jrn = (CycleStorageElem*)new char[full_size];
	fseek(file,seekpos+i*full_size,0);
	fread(jrn,full_size,1,file);

	/*!	��� �������� ������ ������ 1->6, 2->3, 4->5 ��� ���������� false
	*/

	if(jrn->status==1)
	{
		jrn->status=6;
		filewrite(jrn,i);
		delete jrn;
		return true;
	}
	if(jrn->status==2) j=3;
	else if(jrn->status==4) j=5;
	else 
	{
		delete jrn;
		return false;
	}
	jrn->status=j;
	filewrite(jrn,i);
	delete jrn;
	return true;
}

bool CycleStorage::delAllRows()
{
	/*! ������������ ������� ���� ��������� ������ */
	if(file==NULL) return false;
	CycleStorageElem *jrn = (CycleStorageElem*)new char[full_size];
	int i;
	fseek(file,seekpos,0);

	for(i=0;i<size;i++)
	{
		fread(jrn,full_size,1,file);
		if(jrn->status!=0)
		{
			jrn->status=0;
			filewrite(jrn,i,false);
		}
	}
	fflush(file);
	head=tail=-1;
	delete jrn;
	return true;
}

/*! TODO: ����� ������ �� ���������� str, ������ ���������� �������� */
void* CycleStorage::readRow(int num, void* str)
{
	if( size<=0 ) return 0;

	/*! ����������� ����� �������� �� ������ ������� */
	int j=(head+num)%size;
	if((file==NULL)||(num>=size)) return 0;

	if((head!=tail+1)&&(num>tail)) return 0;

	CycleStorageElem *jrn = (CycleStorageElem*)new char[full_size];
	fseek(file,seekpos+j*full_size,0);
	fread(jrn,full_size,1,file);

	if((jrn->status==1)||(jrn->status==2)||(jrn->status==4))
	{
		memcpy(str,valPointer(jrn),inf_size);
		delete jrn;
		return str;
	}
	delete jrn;
	return NULL;
}

int CycleStorage::getIter()
{
	return iter;
}