/*
 * Copyright (C) 2006-2007 by the Widelands Development Team
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

#ifndef LAYEREDFILESYSTEM_H
#define LAYEREDFILESYSTEM_H

#include "filesystem.h"

/**
 * LayeredFileSystem is a file system which basically merges several layered
 * real directory structures into a single one. The really funny thing is that
 * those directories aren't represented as absolute paths, but as nested
 * FileSystems. Are you confused yet?
 * Ultimately, this provides us with the necessary flexibility to allow file
 * overrides on a per-user-basis, nested .zip files acting as Quake-like paks
 * and so on.
 *
 * Only the top-most writable filesystem is written to. A typical
 * stack would look like this in real-life:
 *
 * ~/.widelands/
 * /where/they/installed/widelands/  <-- this is the directory that the
 * executable is in
 *
 * $CWD  <-- the current-working directory; this is useful for debugging, when
 * the executable isn't in the root of the game-data directory
 */
struct LayeredFileSystem : public FileSystem {
	LayeredFileSystem();
	virtual ~LayeredFileSystem();

	virtual void AddFileSystem(FileSystem * const fs);
	virtual void RemoveFileSystem(FileSystem * const fs);

	virtual const int FindFiles(std::string path, const std::string pattern,
	                            filenameset_t *results,
	                            uint depth=0);

	virtual const bool IsWritable() const;
	virtual const bool FileExists(const std::string path);
	virtual const bool IsDirectory(std::string path);
	virtual void EnsureDirectoryExists(const std::string dirname);
	virtual void MakeDirectory(const std::string dirname);

	virtual void * Load(const std::string & fname, size_t & length);
	virtual void Write(const std::string fname, const void * const data,
	                   const int length);

	virtual WidelandsStreamRead  * OpenWidelandsStreamRead
		(const std::string & fname);
	virtual WidelandsStreamWrite * OpenWidelandsStreamWrite
		(const std::string & fname);

	virtual FileSystem* MakeSubFileSystem(const std::string dirname);
	virtual FileSystem* CreateSubFileSystem(const std::string dirname,
	                                        const Type);
	virtual void Unlink(const std::string file);
	virtual void Rename(const std::string&, const std::string&);

	void listSubdirs() const;
	virtual const std::string getBasename() {return "";};

private:
	typedef std::vector<FileSystem*>::reverse_iterator FileSystem_rit;

	std::vector<FileSystem*> m_filesystems;
};

/// Access all game data files etc. through this FileSystem
extern LayeredFileSystem *g_fs;

#endif
