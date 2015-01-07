//
// CGString.CPP
// Copyright Menace Software (www.menasoft.com).
//

#include "graycom.h"

#define	STRING_DEFAULT_SIZE	42 // Please read the next comment before changing this
/*
Empty World! (total strings on start =480,154)
 16 bytes : memory:  8,265,143 [Mem=42,516 K] [reallocations=12,853]
 32 bytes : memory: 16,235,008 [Mem=50,916 K] [reallocations=11,232]
 36 bytes : memory: 17,868,026 [Mem=50,592 K] [reallocations=11,144]
 40 bytes : memory: 19,627,696 [Mem=50,660 K] [reallocations=11,108]
 42 bytes : memory: 20,813,400 [Mem=50,240 K] [reallocations=11,081] BEST
 48 bytes : memory: 23,759,788 [Mem=58,704 K] [reallocations=11,048]
 56 bytes : memory: 27,689,157 [Mem=57,924 K] [reallocations=11,043]
 64 bytes : memory: 31,618,882 [Mem=66,260 K] [reallocations=11,043]
128 bytes : memory: 62,405,304 [Mem=98,128 K] [reallocations=11,042] <- was in [0.55R4.0.2 - 0.56a]


Full World! (total strings on start ~1,388,081)
 16 bytes : memory:  29,839,759 [Mem=227,232 K] [reallocations=269,442]
 32 bytes : memory:  53,335,580 [Mem=250,568 K] [reallocations=224,023]
 40 bytes : memory:  63,365,178 [Mem=249,978 K] [reallocations=160,987]
 42 bytes : memory:  66,120,092 [Mem=249,896 K] [reallocations=153,181] BEST
 48 bytes : memory:  74,865,847 [Mem=272,016 K] [reallocations=142,813]
 56 bytes : memory:  87,050,665 [Mem=273,108 K] [reallocations=141,507]
 64 bytes : memory:  99,278,582 [Mem=295,932 K] [reallocations=141,388]
128 bytes : memory: 197,114,039 [Mem=392,056 K] [reallocations=141,234] <- was in [0.55R4.0.2 - 0.56a]

*/

//#define DEBUG_STRINGS
#ifdef DEBUG_STRINGS
int gAmount = 0;
ULONG gMemAmount = 0;
int gReallocs = 0;
#endif

#ifndef _WIN32
void _strupr( TCHAR * pszStr )
{
	// No portable UNIX/LINUX equiv to this.
	for ( ;pszStr[0] != '\0'; pszStr++ )
	{
		*pszStr = toupper( *pszStr );
	}
}

void _strlwr( TCHAR * pszStr )
{
	// No portable UNIX/LINUX equiv to this.
	for ( ;pszStr[0] != '\0'; pszStr++ )
	{
		*pszStr = tolower( *pszStr );
	}
}
#endif

int strcpylen( TCHAR * pDst, LPCTSTR pSrc )
{
	strcpy( pDst, pSrc );
	return( strlen( pDst ));
}

int strcpylen( TCHAR * pDst, LPCTSTR pSrc, int iMaxSize )
{
	// it does NOT include the iMaxSize element! (just like memcpy)
	// so iMaxSize=sizeof() is ok !
	ASSERT( iMaxSize );
	strncpy( pDst, pSrc, iMaxSize-1 );
	pDst[iMaxSize-1] = '\0';	// always terminate.
	return( strlen( pDst ));
}

//***************************************************************************
// -CGString

void CGString::Empty(bool bTotal)
{
	if ( bTotal )
	{
		if ( m_iMaxLength && m_pchData )
		{
#ifdef DEBUG_STRINGS
			gMemAmount -= m_iMaxLength;
#endif
			delete []m_pchData;
			m_pchData = NULL;
			m_iMaxLength = 0;
		}
	}
	else m_iLength = 0;
}

int CGString::SetLength( int iNewLength )
{
	if ( iNewLength >= m_iMaxLength )
	{
#ifdef DEBUG_STRINGS
		gMemAmount -= m_iMaxLength;
#endif
		m_iMaxLength = iNewLength+(STRING_DEFAULT_SIZE/2);	// allow grow, and always expand only
#ifdef DEBUG_STRINGS
		gMemAmount += m_iMaxLength;
		gReallocs++;
#endif
		TCHAR	*pNewData = new TCHAR[m_iMaxLength+1];
		ASSERT(pNewData);

		int iMinLength = min(iNewLength, m_iLength);
		strncpy(pNewData, m_pchData, iMinLength);

		if ( m_pchData ) delete []m_pchData;
		m_pchData = pNewData;
	}
	m_iLength = iNewLength;
	m_pchData[m_iLength] = 0;
	return m_iLength;
}

void CGString::Copy( LPCTSTR pszStr )
{
	if (( pszStr != m_pchData ) && pszStr )
	{
		SetLength(strlen(pszStr));
		strcpy(m_pchData, pszStr);
	}
}

void CGString::FormatV(LPCTSTR pszFormat, va_list args)
{
	TCHAR	*pszTemp = Str_GetTemp();
	vsprintf(pszTemp, pszFormat, args);
	Copy(pszTemp);
}

void CGString::Add( TCHAR ch )
{
	int iLen = m_iLength;
	SetLength(iLen+1);
	SetAt(iLen, ch);
}

void CGString::Add( LPCTSTR pszStr )
{
	int iLenCat = strlen(pszStr);
	if ( iLenCat )
	{
		SetLength(iLenCat + m_iLength);
		strcat(m_pchData, pszStr);
	}
}

void CGString::Reverse()
{
	STRREV(m_pchData);
}

CGString::~CGString()
{
#ifdef DEBUG_STRINGS
	gAmount--;
#endif
	Empty(true);
}

CGString::CGString()
{
#ifdef DEBUG_STRINGS
	gAmount++;
#endif
	Init();
}

CGString::CGString(LPCTSTR pStr)
{
	m_iMaxLength = m_iLength = 0;
	m_pchData = NULL;
	Copy(pStr);
}

CGString::CGString(const CGString &s)
{
	m_iMaxLength = m_iLength = 0;
	m_pchData = NULL;
	Copy(s.GetPtr());
}

bool CGString::IsValid() const
{
	if ( !m_iMaxLength ) return false;
	return( m_pchData[m_iLength] == '\0' );
}
TCHAR * CGString::GetBuffer()
{
	return m_pchData;
}
TCHAR * CGString::GetBuffer(int iMinLength)
{
	if ( iMinLength > m_iMaxLength ) SetLength(iMinLength);
	return GetBuffer();
}

int CGString::GetLength() const
{
    return (m_iLength);
}
bool CGString::IsEmpty() const
{
    return( !m_iLength );
}
TCHAR & CGString::ReferenceAt(int nIndex)       // 0 based
{
	ASSERT( nIndex < m_iLength );
	return m_pchData[nIndex];
}
TCHAR CGString::GetAt(int nIndex) const      // 0 based
{
	ASSERT( nIndex <= m_iLength );	// allow to get the null char
	return( m_pchData[nIndex] );
}
void CGString::SetAt(int nIndex, TCHAR ch)
{
	ASSERT( nIndex < m_iLength );
	m_pchData[nIndex] = ch;
	if ( !ch ) m_iLength = strlen(m_pchData);	// \0 inserted. line truncated
}
LPCTSTR CGString::GetPtr() const
{
	return( m_pchData );
}
int CGString::CopyTo( TCHAR * pStr ) const
{
    strcpy(pStr, m_pchData);
    return m_iLength;
}
void _cdecl CGString::Format( LPCTSTR pStr, ... )
{
	va_list vargs;
	va_start( vargs, pStr );
	FormatV( pStr, vargs );
	va_end( vargs );
}
void CGString::FormatVal( long iVal )
{
	Format("%d", iVal);
}
void CGString::FormatHex( DWORD dwVal )
{
	Format("0%x", dwVal);
}
int CGString::Compare( LPCTSTR pStr ) const
{
	return (strcmp(m_pchData, pStr));
}
int CGString::CompareNoCase( LPCTSTR pStr ) const
{
	return (strcmpi(m_pchData, pStr));
}

void CGString::Init()
{
	m_iMaxLength = STRING_DEFAULT_SIZE;
#ifdef DEBUG_STRINGS
	gMemAmount += m_iMaxLength;
#endif
	m_iLength = 0;
	m_pchData = new TCHAR[m_iMaxLength+1];
	m_pchData[m_iLength] = 0;
}

//***************************************************************************
// String global functions.

#define	SCRIPT_TEMP_LINES	1024	// 4mb [1024 * 4kb]

static int		Str_iTemp = 0;
static TCHAR	Str_szTemp[SCRIPT_TEMP_LINES][SCRIPT_MAX_LINE_LEN];

TCHAR * Str_GetTemp()
{
	LOCKDATA;
	// Some scratch string space, random uses
	if ( ++Str_iTemp >= SCRIPT_TEMP_LINES )
		Str_iTemp = 0;
	TCHAR *ret = Str_szTemp[Str_iTemp];
	*ret = 0;
	return ret;
}

LPCTSTR Str_GetArticleAndSpace( LPCTSTR pszWord )
{
	// NOTE: This is wrong many times.
	//  ie. some words need no article (plurals) : boots.

	ASSERT(pszWord);
	static const TCHAR sm_Vowels[] = {	'A', 'E', 'I', 'O', 'U'};
	TCHAR chName = toupper(pszWord[0]);
	for (int x = 0; x < sizeof(sm_Vowels); x++)
	{
		if ( chName == sm_Vowels[x])
			return( "an " );
	}
	return( "a " );
}

int Str_TrimEndWhitespace( TCHAR * pStr, int len )
{
	ASSERT( len >= 0 );
	while ( len>0 )
	{
		len --;
		if ( ! ISWHITESPACE( pStr[len] ))
		{
			++len;
			break;
		}
	}
	pStr[len] = '\0';
	return( len );
}

TCHAR * Str_TrimWhitespace( TCHAR * pStr )
{
	GETNONWHITESPACE( pStr );
	Str_TrimEndWhitespace( pStr, strlen(pStr));
	return( pStr );
}

bool Str_Parse( TCHAR * pLine, TCHAR ** ppLine2, LPCTSTR pszSep )
{
	// Parse a list of args. Just get the next arg.
	// similar to strtok()
	// RETURN: true = the second arg is valid.

	if ( pszSep == NULL )	// default sep.
		pszSep = "=, \t";

	// skip leading white space.
	TCHAR * pNonWhite = pLine;
	GETNONWHITESPACE(pNonWhite);
	if ( pNonWhite != pLine )
	{
		memmove( pLine, pNonWhite, strlen( pNonWhite ) + 1 );
	}

	TCHAR ch;
	for ( ;true; pLine++ )
	{
		ch = *pLine;
		if ( ch == '\0' )	// no args i guess.
		{
			if ( ppLine2 != NULL )
			{
				*ppLine2 = pLine;
			}
			return false;
		}
		if ( strchr( pszSep, ch ))
			break;
	}

	*pLine++ = '\0';
	if ( isspace( ch ))	// space seperators might have other seps as well ?
	{
		GETNONWHITESPACE( pLine );
		ch = *pLine;
		if ( ch && strchr( pszSep, ch ))
		{
			pLine++;
		}
	}

	// skip leading white space on args as well.
	if ( ppLine2 != NULL )
	{
		*ppLine2 = Str_TrimWhitespace( pLine );
	}
	return true;
}

int Str_ParseCmds( TCHAR * pszCmdLine, TCHAR ** ppCmd, int iMax, LPCTSTR pszSep )
{
	int iQty = 0;
	if ( pszCmdLine != NULL && pszCmdLine[0] != '\0' )
	{
		ppCmd[0] = pszCmdLine;
		iQty++;
		while ( Str_Parse( ppCmd[iQty-1], &(ppCmd[iQty]), pszSep ))
		{
			if ( ++iQty >= iMax )
				break;
		}
	}
	for ( int j=iQty; j<iMax; j++ )
		ppCmd[j] = NULL;	// terminate if possible.
	return( iQty );
}

int Str_ParseCmds( TCHAR * pszCmdLine, int * piCmd, int iMax, LPCTSTR pszSep )
{
	TCHAR * ppTmp[256];
	if ( iMax > COUNTOF(ppTmp))
		iMax = COUNTOF(ppTmp);
	int iQty = Str_ParseCmds( pszCmdLine, ppTmp, iMax, pszSep );
	int i;
	for ( i=0; i<iQty; i++ )
	{
		piCmd[i] = Exp_GetVal(ppTmp[i]);
	}
	for ( ;i<iMax;i++)
	{
		piCmd[i] = 0;
	}
	return( iQty );
}

static int Str_CmpHeadI( LPCTSTR pszFind, LPCTSTR pszTable )
{
	for ( int i=0; true; i++ )
	{
			//	we should always use same case as in other places. since strcmpi lowers,
			//	we should lower here as well. fucking shit!
		TCHAR ch1 = tolower(pszFind[i]);
		TCHAR ch2 = tolower(pszTable[i]);
		if ( ch2 == 0 )
		{
			if ( !isalnum(ch1))
				return 0;
			return( ch1 - ch2 );
		}
		if ( ch1 != ch2 )
		{
			return( ch1 - ch2 );
		}
	}
}

int FindTableHeadSorted( LPCTSTR pszFind, LPCTSTR const * ppszTable, int iCount, int iElemSize )
{
	// Do a binary search (un-cased) on a sorted table.
	// RETURN: -1 = not found
	int iHigh = iCount-1;
	if ( iHigh < 0 )
	{
		return -1;
	}
	int iLow = 0;
	while ( iLow <= iHigh )
	{
		int i = (iHigh+iLow)/2;
		LPCTSTR pszName = *((LPCTSTR const *) ((( const BYTE*) ppszTable ) + ( i*iElemSize )));
		int iCompare = Str_CmpHeadI( pszFind, pszName );
		if ( iCompare == 0 )
			return( i );
		if ( iCompare > 0 )
		{
			iLow = i+1;
		}
		else
		{
			iHigh = i-1;
		}
	}
	return( -1 );
}

int FindTableHead( LPCTSTR pszFind, LPCTSTR const * ppszTable, int iCount, int iElemSize )
{
	for ( int i=0; i<iCount; i++ )
	{
		int iCompare = Str_CmpHeadI( pszFind, *ppszTable );
		if ( ! iCompare )
			return( i );
		ppszTable = (LPCTSTR const *)((( const BYTE*) ppszTable ) + iElemSize );
	}
	return( -1 );
}

int FindTableSorted( LPCTSTR pszFind, LPCTSTR const * ppszTable, int iCount, int iElemSize )
{
	// Do a binary search (un-cased) on a sorted table.
	// RETURN: -1 = not found
	int iHigh = iCount-1;
	if ( iHigh < 0 )
	{
		return -1;
	}
	int iLow = 0;
	while ( iLow <= iHigh )
	{
		int i = (iHigh+iLow)/2;
		LPCTSTR pszName = *((LPCTSTR const *) ((( const BYTE*) ppszTable ) + ( i*iElemSize )));
		int iCompare = strcmpi( pszFind, pszName );
		if ( iCompare == 0 )
			return( i );
		if ( iCompare > 0 )
		{
			iLow = i+1;
		}
		else
		{
			iHigh = i-1;
		}
	}
	return( -1 );
}

int FindTable( LPCTSTR pszFind, LPCTSTR const * ppszTable, int iCount, int iElemSize )
{
	// A non-sorted table.
	for ( int i=0; i<iCount; i++ )
	{
		if ( ! strcmpi( *ppszTable, pszFind ))
			return( i );
		ppszTable = (LPCTSTR const *)((( const BYTE*) ppszTable ) + iElemSize );
	}
	return( -1 );
}

int Str_GetBare( TCHAR * pszOut, LPCTSTR pszInp, int iMaxOutSize, LPCTSTR pszStrip )
{
	// That the client can deal with. Basic punctuation and alpha and numbers.
	// RETURN: Output length.

	if ( !pszStrip )
		pszStrip = "{|}~";	// client cant print these.

	//GETNONWHITESPACE( pszInp );	// kill leading white space.

	int j=0;
	for ( int i=0; true; i++ )
	{
		TCHAR ch = pszInp[i];
		if ( ch )
		{
			if ( ch < ' ' || ch >= 127 )
				continue;	// Special format chars.
			int k=0;
			while ( pszStrip[k] && pszStrip[k] != ch )
				k++;
			if ( pszStrip[k] )
				continue;
			if ( j >= iMaxOutSize-1 )
			{
				ch = 0;
			}
		}
		pszOut[j++] = ch;
		if ( ch == 0 )
			break;
	}
	return( j-1 );
}


bool Str_Check( const TCHAR * pszIn )
{
	if ( !pszIn ) return false;
	const char *p = pszIn;
	while ( *p && ( *p != 0x0A ) && ( *p != 0x0D )) p++;
	return ( *p );
}

TCHAR * Str_MakeFiltered( TCHAR * pStr )
{
	int len = strlen( pStr );
	for ( int i=0; len; i++, len-- )
	{
		if ( pStr[i] == '\\' )
		{
			switch ( pStr[i+1] )
			{
			case 'b': pStr[i] = '\b'; break;
			case 'n': pStr[i] = '\n'; break;
			case 'r': pStr[i] = '\r'; break;
			case 't': pStr[i] = '\t'; break;
			case '\\': pStr[i] = '\\'; break;
			}
			len --;
			memmove( pStr+i+1, pStr+i+2, len );
		}
	}
	return( pStr );
}

void Str_MakeUnFiltered( TCHAR * pStrOut, LPCTSTR pStrIn, int iSizeMax )
{
	int len = strlen( pStrIn );
	int iIn = 0;
	int iOut = 0;
	for ( ; iOut < iSizeMax && iIn <= len; iIn++, iOut++ )
	{
		TCHAR ch = pStrIn[iIn];
		switch ( ch )
		{
		case '\b': ch = 'b'; break;
		case '\n': ch = 'n'; break;
		case '\r': ch = 'r'; break;
		case '\t': ch = 't'; break;
		case '\\': ch = '\\'; break;
		default:
			pStrOut[iOut] = ch;
			continue;
		}

		pStrOut[iOut++] = '\\';
		pStrOut[iOut] = ch;
	}
}

#define TOLOWER tolower

static MATCH_TYPE Str_Match_After_Star( LPCTSTR pPattern, LPCTSTR pText )
{
	// pass over existing ? and * in pattern
	for ( ; *pPattern == '?' || *pPattern == '*'; pPattern++ )
	{
		// take one char for each ? and +
		if (  *pPattern == '?' &&
			! *pText++ )		// if end of text then no match
			return MATCH_ABORT;
	}

	// if end of pattern we have matched regardless of text left
	if ( !*pPattern )
		return MATCH_VALID;

	// get the next character to match which must be a literal or '['
	TCHAR nextp = TOLOWER( *pPattern );
	MATCH_TYPE match = MATCH_INVALID;

	// Continue until we run out of text or definite result seen
	do
	{
		// a precondition for matching is that the next character
		// in the pattern match the next character in the text or that
		// the next pattern char is the beginning of a range.  Increment
		// text pointer as we go here
		if ( nextp == TOLOWER( *pText ) || nextp == '[' )
		{
			match = Str_Match(pPattern, pText);
			if ( match == MATCH_VALID )
				break;
		}

		// if the end of text is reached then no match
		if ( !*pText++ )
			return MATCH_ABORT;

	} while (
		match != MATCH_ABORT &&
		match != MATCH_PATTERN );

	return match;	// return result
}

MATCH_TYPE Str_Match( LPCTSTR pPattern, LPCTSTR pText )
{
	// case independant

	TCHAR range_start;
	TCHAR range_end;  // start and end in range

	for ( ; *pPattern; pPattern++, pText++ )
	{
		// if this is the end of the text then this is the end of the match
		if (!*pText)
		{
			return ( *pPattern == '*' && *++pPattern == '\0' ) ?
				MATCH_VALID : MATCH_ABORT;
		}
		// determine and react to pattern type
		switch ( *pPattern )
		{
		// single any character match
		case '?':
			break;
		// multiple any character match
		case '*':
			return Str_Match_After_Star( pPattern, pText );
		// [..] construct, single member/exclusion character match
		case '[':
			{
				// move to beginning of range
				pPattern++;
				// check if this is a member match or exclusion match
				bool fInvert = false;             // is this [..] or [!..]
				if ( *pPattern == '!' || *pPattern == '^')
				{
					fInvert = true;
					pPattern++;
				}
				// if closing bracket here or at range start then we have a
				// malformed pattern
				if ( *pPattern == ']' )
					return MATCH_PATTERN;

				bool fMemberMatch = false;       // have I matched the [..] construct?
				while (true)
				{
					// if end of construct then fLoop is done
					if (*pPattern == ']')
					{
						break;
					}

					// matching a '!', '^', '-', '\' or a ']'
					if ( *pPattern == '\\' )
						range_start = range_end = TOLOWER( *++pPattern );
					else
						range_start = range_end = TOLOWER( *pPattern );

					// if end of pattern then bad pattern (Missing ']')
					if (!*pPattern)
						return MATCH_PATTERN;

					// check for range bar
					if (*++pPattern == '-')
					{
						// get the range end
						range_end = TOLOWER( *++pPattern );
						// if end of pattern or construct then bad pattern
						if ( range_end == '\0' || range_end == ']')
							return MATCH_PATTERN;
						// special character range end
						if ( range_end == '\\')
						{
							range_end = TOLOWER( *++pPattern );
							// if end of text then we have a bad pattern
							if (!range_end)
								return MATCH_PATTERN;
						}
						// move just beyond this range
						pPattern++;
					}

					// if the text character is in range then match found.
					// make sure the range letters have the proper
					// relationship to one another before comparison
					TCHAR chText = TOLOWER( *pText );
					if ( range_start < range_end  )
					{
						if ( chText >= range_start && chText <= range_end)
						{
							fMemberMatch = true;
							break;
						}
					}
					else
					{
						if (chText >= range_end && chText <= range_start)
						{
							fMemberMatch = true;
							break;
						}
					}
				}	// while

				// if there was a match in an exclusion set then no match
				// if there was no match in a member set then no match
				if (( fInvert && fMemberMatch ) ||
					 !( fInvert || fMemberMatch ))
					return MATCH_RANGE;

				// if this is not an exclusion then skip the rest of the [...]
				//  construct that already matched.
				if (fMemberMatch)
				{
					while (*pPattern != ']')
					{
						// bad pattern (Missing ']')
						if (!*pPattern)
							return MATCH_PATTERN;
						// skip exact match
						if (*pPattern == '\\')
						{
							pPattern++;
							// if end of text then we have a bad pattern
							if (!*pPattern)
								return MATCH_PATTERN;
						}
						// move to next pattern char
						pPattern++;
					}
				}
			}
			break;

		// must match this character (case independant) ?exactly
		default:
			if ( TOLOWER( *pPattern ) != TOLOWER( *pText ))
                return MATCH_LITERAL;
        }
	}
	// if end of text not reached then the pattern fails
	if ( *pText )
		return MATCH_END;
	else
		return MATCH_VALID;
}
