/* This file is part of the UniSet project
 * Copyright (c) 2002 Free Software Foundation, Inc.
 * Copyright (c) 2002 Vitaly Lipatov
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
 *  \brief ������������ ���� ��� ����������� ��������������� �����-������.
 *  \author Vitaly Lipatov
 *  \date   $Date: 2006/12/21 14:47:42 $
 *  \version $Id: IOAccess.h,v 1.5 2006/12/21 14:47:42 vpashka Exp $
 *  \par
 *  ���� ���� ������������ ��� ����������� ������������� � �������
 *  �����-������
 */
// --------------------------------------------------------------------------
#ifndef _IOACCESS_H_
#define _IOACCESS_H_
// --------------------------------------------------------------------------
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#include "Exceptions.h"

/*! \class IOAccess
 *  \brief �������������� �������� ��� ��������������� �����-������.
 *  \author Vitaly Lipatov
 *  \date   $Date: 2006/12/21 14:47:42 $
 *  \version $Id: IOAccess.h,v 1.5 2006/12/21 14:47:42 vpashka Exp $
 *  \par
 *  ���� ����� ������������ ��� ����������� ������������� � �������
 *  �����-������
 */

class IOAccess
{
public:

	/// ��� �������� ������� ����������� ��� ����� �� ������/������
	IOAccess()
	{
		fd_port = open("/dev/port", O_RDWR | O_NDELAY);
		if ( fd_port == -1 )
			throw UniSetTypes::IOBadParam();
	}
	
	~IOAccess()
	{
		close(fd_port);
	}
	
	/*! ��������� �������� �� ��������� ������ �� port �� port+size ����.
		������������ �� ������ buf.
	*/
	void get(int port, void* buf, int size) const
	{
		if ( lseek(fd_port, port, SEEK_SET) == -1 )
			throw UniSetTypes::IOBadParam();
		ssize_t s = read(fd_port, buf, size);
		if ( s != size )
			throw UniSetTypes::IOBadParam();
	}
	
	/// ��������� ����� �� ���������� �����
	int in(int port) const
	{
		char input;
		get(port, &input, 1);
		return input;
	}
	
	/*! ������ �������� � �������� ������ �� port �� port+size ����.
		�������� ������� ������� � ������ buf.
	*/
	void put(int port, const void* buf, int size) const
	{
		if ( lseek(fd_port, port, SEEK_SET) == -1 )
			throw UniSetTypes::IOBadParam();
		ssize_t s = write(fd_port, buf, size);
		if ( s != size )
			throw UniSetTypes::IOBadParam();
	}

	/// ����� ����� value � ���� port
	void out(int port, int value) const
	{
		char output = value;
		put(port,&output,1);
	}

private:

	/// ���������� ��������� �����, ������ӣ����� � �������
	int fd_port;
	
};

#endif
