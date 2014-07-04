/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "MusicInfoTagLoaderNOTAG.h"
#include "MusicInfoTag.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/StringUtils.h"
#include "utils/log.h"
//#include <string>

using namespace XFILE;
using namespace MUSIC_INFO;

CMusicInfoTagLoaderNOTAG::CMusicInfoTagLoaderNOTAG()
	: IMusicInfoTagLoader()
{}

CMusicInfoTagLoaderNOTAG::~CMusicInfoTagLoaderNOTAG()
{}

// This piece of code for integrate in the library non-taggable audio files (.mlp for instance)
// File extensions needs to be added in the advancedsettings.xml
bool CMusicInfoTagLoaderNOTAG::Load(const CStdString & strFileName, CMusicInfoTag & tag, EmbeddedArt *art)
{
	try
	{
		std::string path, grandparent, parent, title;
		std::string trash, artist;

		tag.SetURL(strFileName);
		CLog::Log(LOGDEBUG, "NOTAG: work on: %s", strFileName.c_str());

		std::string strFileNameClean = strFileName.c_str();
		StringUtils::RemoveDuplicatedSpacesAndTabs(strFileNameClean);

		URIUtils::Split( strFileNameClean, path, title);
		URIUtils::RemoveSlashAtEnd(path);
		tag.SetTitle( title );
		CLog::Log(LOGDEBUG, "NOTAG: title: %s", title.c_str());

		if ( !path.empty() )
		{
			// Get the parent path
			URIUtils::Split( path, grandparent, parent);

			if ( !parent.empty() )
			{
				// occurences
				int occ = StringUtils::FindNumber( parent, " - " );
				//CLog::Log(LOGDEBUG, "NOTAG: occ: %i @ parent: %s", occ, parent.c_str());

				// set artist and album if we found the ' - ' separator
				if( occ > 0 )
				{
					const CStdString delimiter = " - ";
					CStdStringArray parentsplit = StringUtils::SplitString( parent, delimiter );
					artist = parentsplit[0].c_str();
					StringUtils::TrimRight( artist );
					tag.SetArtist( artist );
					CLog::Log(LOGDEBUG, "NOTAG: artist: %s", artist.c_str() );
					parentsplit[0].erase();

					CStdString album;
					StringUtils::JoinString( parentsplit, delimiter, album );
					StringUtils::TrimLeft( album );
					StringUtils::TrimLeft( album, "- " );
					tag.SetAlbum( album );
					CLog::Log(LOGDEBUG, "NOTAG: album: %s", album.c_str());
				} else {
					URIUtils::RemoveSlashAtEnd(grandparent);
					path = grandparent;
					URIUtils::Split( path, trash, grandparent);

					// if two dirs level are found, assume the parent is album and the grandparent is artist
					if( !grandparent.empty() ) {
						tag.SetArtist( grandparent );
						CLog::Log(LOGDEBUG, "NOTAG: artist: %s", grandparent.c_str());
						tag.SetAlbum( parent );
						CLog::Log(LOGDEBUG, "NOTAG: album: %s", parent.c_str());
					} else {
					// if only one dir is found, assume it is the artist
						tag.SetArtist( parent );
						CLog::Log(LOGDEBUG, "NOTAG: artist: %s", parent.c_str());
					}
				}
			}
		}

		tag.SetLoaded(true);
		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "Tag loader other: exception in file %s", strFileName.c_str());
	}

	return false;
}

