//
// CAtom.cpp
//

#include "graycom.h"
#include "CAtom.h"

struct CAtomManager : public CGObSortArray < CAtomDef *, LPCTSTR >
{
	// Create an alpha sorted string lookup table.
	int CompareKey( LPCTSTR pszKey, CAtomDef * pVal, bool fNoSpaces ) const
	{
		ASSERT(pszKey);
		ASSERT(pVal);
		return( strcmp( pszKey, * ( static_cast <CGString*>( pVal ))));
	}
};

static CAtomManager g_AtomManager;

//*********************************
// CAtomRef

void CAtomRef::ClearRef()
{
	if ( m_pDef )
	{
		ASSERT(m_pDef->m_iUseCount);
		if ( ! --m_pDef->m_iUseCount )
		{
			g_AtomManager.DeleteOb(m_pDef);
		}
		m_pDef = NULL;
	}
}

void CAtomRef::Copy( const CAtomRef & atom )
{
	// Copy's are fast.
	if ( m_pDef == atom.m_pDef )
		return;
	ClearRef();
	m_pDef = atom.m_pDef;
	m_pDef->m_iUseCount++;
}

void CAtomRef::SetStr( LPCTSTR pszText )
{
	ClearRef();
	if ( pszText == NULL )
		return;

	// Find it in the atom table first.
	int iCompareRes;
	int index = g_AtomManager.FindKeyNear( pszText, iCompareRes );
	if ( !iCompareRes )
	{
		// already here just increment useage.
		m_pDef = g_AtomManager.GetAt( index );
		m_pDef->m_iUseCount++;
	}
	else
	{
		// Insertion sort.
		m_pDef = new CAtomDef( pszText );
		g_AtomManager.AddPresorted( index, iCompareRes, m_pDef );
	}
}

