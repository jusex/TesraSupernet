/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http:
 */
/** @file TransientDirectory.cpp
 * @author Marek Kotewicz <marek@ethdev.com>
 * @date 2015
 */

#include <thread>
#include <boost/filesystem.hpp>
#include "Exceptions.h"
#include "TransientDirectory.h"
#include "CommonIO.h"
#include "Log.h"
using namespace std;
using namespace dev;
namespace fs = boost::filesystem;

TransientDirectory::TransientDirectory():
	TransientDirectory((boost::filesystem::temp_directory_path() / "eth_transient" / toString(FixedHash<4>::random())).string())
{}

TransientDirectory::TransientDirectory(std::string const& _path):
	m_path(_path)
{
	
	if (boost::filesystem::exists(m_path))
		BOOST_THROW_EXCEPTION(FileError());

	if (!fs::create_directories(m_path))
		BOOST_THROW_EXCEPTION(FileError());
	DEV_IGNORE_EXCEPTIONS(fs::permissions(m_path, fs::owner_all));
}

TransientDirectory::~TransientDirectory()
{
	boost::system::error_code ec;		
	fs::remove_all(m_path, ec);
	if (!ec)
		return;

	
	
	
	
	this_thread::sleep_for(chrono::milliseconds(10));

	ec.clear();
	fs::remove_all(m_path, ec);
	if (!ec)
	{
		cwarn << "Failed to delete directory '" << m_path << "': " << ec.message();
	}
}
