//
// GRAYINST.CPP
// Copyright Menace Software (www.menasoft.com).
//

#include "graycom.h"
#include "CGrayInst.h"
#include "../graysvr/graysvr.h"

bool CGrayInstall::FindInstall()
{
#ifdef _WIN32
	// Get the install path from the registry.

	static LPCTSTR m_szKeys[] =
	{
		"Software\\Menasoft\\" GRAY_FILE,
		"Software\\Origin Worlds Online\\Ultima Online\\1.0", 
		"Software\\Origin Worlds Online\\Ultima Online Third Dawn\\1.0", 
	};
	
	HKEY hKey;
	LONG lRet;
	for ( int i=0; i<COUNTOF(m_szKeys); i++ )
	{
		lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			m_szKeys[i], // address of name of subkey to query
			0, KEY_READ, &hKey );
		if ( lRet == ERROR_SUCCESS )
			break;
	}
	if ( lRet != ERROR_SUCCESS )
	{
		return( false );
	}

	TCHAR szValue[ _MAX_PATH ];
	DWORD lSize = sizeof( szValue );
	DWORD dwType = REG_SZ;
	lRet = RegQueryValueEx(hKey, "ExePath", NULL, &dwType, (BYTE*)szValue, &lSize);

	if ( lRet == ERROR_SUCCESS && dwType == REG_SZ )
	{
		TCHAR * pSlash = strrchr( szValue, '\\' );	// get rid of the client.exe part of the name
		if ( pSlash ) * pSlash = '\0';
		m_sExePath = szValue;
	}

	// ??? Find CDROM install base as well, just in case.
	// uo.cfg CdRomDataPath=e:\uo

	lRet = RegQueryValueEx(hKey, "InstCDPath", NULL, &dwType, (BYTE*)szValue, &lSize);

	if ( lRet == ERROR_SUCCESS && dwType == REG_SZ )
	{
		m_sCDPath = szValue;
	}

	RegCloseKey( hKey );

#else
	// LINUX has no registry so we must have the INI file show us where it is installed.
#endif
	return true;
}

bool CGrayInstall::OpenFile( CGFile & file, LPCTSTR pszName, WORD wFlags )
{
	ASSERT(pszName);
	if ( !m_sPreferPath.IsEmpty() )
	{
		if ( file.Open(GetPreferPath(pszName), wFlags) )
			return true;
	}
	else
	{
		if ( file.Open(GetFullExePath(pszName), wFlags) )
			return true;
		if ( file.Open(GetFullCDPath(pszName), wFlags) )
			return true;
	}
	return false;
}

LPCTSTR CGrayInstall::GetBaseFileName( VERFILE_TYPE i ) // static
{
	static LPCTSTR const sm_szFileNames[VERFILE_QTY] =
	{
		"artidx.mul",	// Index to ART
		"art.mul",		// Artwork such as ground, objects, etc.
		"anim.idx",
		"anim.mul",		// Animations such as monsters, people, and armor.
		"soundidx.mul", // Index into SOUND
		"sound.mul",	// Sampled sounds
		"texidx.mul",	// Index into TEXMAPS
		"texmaps.mul",	// Texture map data (the ground).
		"gumpidx.mul",	// Index to GUMPART
		"gumpart.mul",	// Gumps. Stationary controller bitmaps such as windows, buttons, paperdoll pieces, etc.
		"multi.idx",
		"multi.mul",	// Groups of art (houses, castles, etc)
		"skills.idx",
		"skills.mul",
		"verdata.mul",
		"map0.mul",		// MAP(s)
		"staidx0.mul",	// STAIDX(s)
		"statics0.mul",	// Static objects on MAP(s)
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"tiledata.mul", // Data about tiles in ART. name and flags, etc
		"animdata.mul", //
		"hues.mul",		// the 16 bit color pallete we use for everything.
	};

	return ( i<0 || i>=VERFILE_QTY ) ? NULL : sm_szFileNames[i];
}

bool CGrayInstall::OpenFile( VERFILE_TYPE i )
{
	CGFile	*pFile = GetMulFile(i);
	if ( !pFile )
		return false;
	if ( pFile->IsFileOpen())
		return true;

	if ( !pFile->GetFilePath().IsEmpty() )
	{
		if ( pFile->Open(pFile->GetFilePath(), OF_READ|OF_SHARE_DENY_WRITE) )
			return true;
	}

	LPCTSTR pszTitle = GetBaseFileName((VERFILE_TYPE)i);
	if ( !pszTitle ) return false;

	return OpenFile(m_File[i], pszTitle, OF_READ|OF_SHARE_DENY_WRITE);
}

VERFILE_TYPE CGrayInstall::OpenFiles( DWORD dwMask )
{
	// Now open all the required files.
	// RETURN: VERFILE_QTY = all open success.
	int i(0);

	for ( i = 0; i < VERFILE_QTY; i++ )
	{
		if ( ! ( dwMask & ( 1 << i )) ) continue;
		if ( GetBaseFileName( (VERFILE_TYPE)i ) == NULL ) continue;

		if ( !OpenFile( (VERFILE_TYPE)i ))
		{
			//	make some MULs optional
			switch ( i )
			{
			case VERFILE_MAP:						//	map #0 is permanent and should exist!
			case VERFILE_STATICS:
			case VERFILE_STAIDX:
				memset(g_MapList.m_maps, false, sizeof(g_MapList.m_maps));
				break;
			case VERFILE_VERDATA:
				continue;
			}
			break;
		}
		else if ( i == VERFILE_MAP )
		{
			char z[256];

			//	check for map files of custom maps
			for ( int m = 0; m < 256; m++ )
			{
				if ( g_MapList.m_maps[m] )
				{
					int	index = g_MapList.m_mapnum[m];

					if ( !m_Maps[index].IsFileOpen() )
					{
						sprintf(z, "map%d.mul", index);
						OpenFile(m_Maps[index], z, OF_READ|OF_SHARE_DENY_WRITE);
					}
					if ( !m_Staidx[index].IsFileOpen() )
					{
						sprintf(z, "staidx%d.mul", index);
						OpenFile(m_Staidx[index], z, OF_READ|OF_SHARE_DENY_WRITE);
					}
					if ( !m_Statics[index].IsFileOpen() )
					{
						sprintf(z, "statics%d.mul", index);
						OpenFile(m_Statics[index], z, OF_READ|OF_SHARE_DENY_WRITE);
					}

					//	if any of the maps are not available, mark map as inavailable
					if ( !m_Maps[index].IsFileOpen() ||
						 !m_Staidx[index].IsFileOpen() ||
						 !m_Statics[index].IsFileOpen() )
					{
						if ( m_Maps[index].IsFileOpen() ) m_Maps[index].Close();
						if ( m_Staidx[index].IsFileOpen() ) m_Staidx[index].Close();
						if ( m_Statics[index].IsFileOpen() ) m_Statics[index].Close();
						g_MapList.m_maps[m] = false;
					}
				}
			}
		}
	}

	g_Log.Event(LOGM_INIT, "Supported Ultima Online expansions: T2A=%s, LBR=%s, AOS=%s, SE=%s" DEBUG_CR,
		( g_MapList.m_maps[0] ? "yes" : "no" ),
		( g_MapList.m_maps[2] ? "yes" : "no" ),
		( g_MapList.m_maps[3] ? "yes" : "no" ),
		( g_MapList.m_maps[4] ? "yes" : "no" ));

	g_MapList.Init();

	return (VERFILE_TYPE)i;
}

void CGrayInstall::CloseFiles()
{
	int i;

	for ( i = 0; i < VERFILE_QTY; i++ )
	{
		if ( m_File[i].IsFileOpen() ) m_File[i].Close();
	}

	for ( i = 0; i < 256; i++ )
	{
		if ( m_Maps[i].IsFileOpen() ) m_Maps[i].Close();
		if ( m_Statics[i].IsFileOpen() ) m_Statics[i].Close();
		if ( m_Staidx[i].IsFileOpen() ) m_Staidx[i].Close();
	}
}

bool CGrayInstall::ReadMulIndex(CGFile &file, DWORD id, CUOIndexRec &Index)
{
	LONG lOffset = id * sizeof(CUOIndexRec);

	if ( file.Seek(lOffset, SEEK_SET) != lOffset )
		return false;

	if ( file.Read((void *)&Index, sizeof(CUOIndexRec)) != sizeof(CUOIndexRec) )
		return false;

	return Index.HasData();
}

bool CGrayInstall::ReadMulData(CGFile &file, const CUOIndexRec &Index, void * pData)
{
	if ( file.Seek(Index.GetFileOffset(), SEEK_SET) != Index.GetFileOffset() )
		return false;

	DWORD dwLength = Index.GetBlockLength();
	if ( file.Read(pData, dwLength) != dwLength )
		return false;

	return true;
}

bool CGrayInstall::ReadMulIndex(VERFILE_TYPE fileindex, VERFILE_TYPE filedata, DWORD id, CUOIndexRec & Index)
{
	// Read about this data type in one of the index files.
	// RETURN: true = we are ok.
	ASSERT(fileindex<VERFILE_QTY);

	// Is there an index for it in the VerData ?
	if ( g_VerData.FindVerDataBlock(filedata, id, Index) )
		return true;

	return ReadMulIndex(m_File[fileindex], id, Index);
}

bool CGrayInstall::ReadMulData(VERFILE_TYPE filedata, const CUOIndexRec & Index, void * pData)
{
	// Use CGFile::GetLastError() for error.
	if ( Index.IsVerData() ) filedata = VERFILE_VERDATA;

	return ReadMulData(m_File[filedata], Index, pData);
}
