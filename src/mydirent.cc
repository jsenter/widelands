/*
 * Copyright (C) 2002 by the Widelands Development Team
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef WIN32
#include "mydirent.h"
#include <iostream.h>

Dir::Dir(const char* n)
{
	int nlen = strlen(n);
	name = new char[nlen + 1 + 4];
	strcpy(name, n);
	if (name[nlen-1] == '\\')
		name[nlen-1] = 0;
	strcat(name, "\\*.*");
	find = NULL;
}

Dir::~Dir()
{
	if (find)
		FindClose(find);
	delete name;
}

DIR* opendir(const char* n)
{
	int nlen = strlen(n);
	char* name = new char[nlen + 1];
	strcpy(name, n);
	if (name[nlen-1] == '\\' || name[nlen-1] == '/')
		name[nlen-1] = 0;

	WIN32_FIND_DATA wfd;
	HANDLE found = FindFirstFile(name, &wfd);
	delete[] name;

	if (found == INVALID_HANDLE_VALUE)
		return NULL;
	return new DIR(n);
}

dirent* readdir(DIR* dir)
{
	WIN32_FIND_DATA wfd;
	if (!dir->find)
	{
		dir->find = FindFirstFile(dir->name, &wfd);
		if (dir->find == INVALID_HANDLE_VALUE)
			return NULL;
	}
	else
		if (!FindNextFile(dir->find, &wfd))
			return NULL;

//	while (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//		if (!FindNextFile(dir->find, &wfd))
//			return NULL;

	strcpy(dir->ret.d_name, wfd.cFileName);
	return &dir->ret;
}

void closedir(DIR* dir)
{
	delete dir;
}

#endif //WIN32

