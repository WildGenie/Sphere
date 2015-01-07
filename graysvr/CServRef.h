//
// CServerDef.h
//

#ifndef _INC_CSERVERDEF_H
#define _INC_CSERVERDEF_H
#pragma once

enum SERV_STAT_TYPE
{
	SERV_STAT_CLIENTS,	// How many clients does it say it has ? (use as % full)
	SERV_STAT_CHARS,
	SERV_STAT_ITEMS,
	SERV_STAT_ACCOUNTS,
	SERV_STAT_QTY,
	SERV_STAT_MEM = SERV_STAT_QTY,	// virtual
};

enum ACCAPP_TYPE	// types of new account applications.
{
	ACCAPP_Closed = 0,	// 0=Closed. Not accepting more.
	ACCAPP_Unused1,
	ACCAPP_Free,		// 2=Anyone can just log in and create a full account.
	ACCAPP_GuestAuto,	// You get to be a guest and are automatically sent email with u're new password.
	ACCAPP_GuestTrial,	// You get to be a guest til u're accepted for full by an Admin.
	ACCAPP_Unused5,
	ACCAPP_Unspecified,	// Not specified.
	ACCAPP_Unused7,
	ACCAPP_Unused8,
	ACCAPP_QTY,
};

class CServerDef	: public CScriptObj, public CThreadLockableObj	// Array of servers we point to.
{	// NOTE: g_Serv = this server.
	static LPCTSTR const sm_szLoadKeys[];

private:
	CGString m_sName;	// What the name should be. Fill in from ping.
	CServTime  m_timeLastPoll;		// Last poll time in CServTime::GetCurrentTime() (if polling)
	CServTime  m_timeLastValid;	// Last valid poll time in CServTime::GetCurrentTime()
	CGTime	m_dateLastValid;

	CServTime  m_timeCreate;	// When added to the list ? 0 = at start up.

	// Status read from returned string.
	CGString m_sStatus;	// last returned status string.

	// statistics
	DWORD m_dwStat[ SERV_STAT_QTY ];

	// Moving these chars to this server.
	CThread m_TaskCharMove;
	CCharRefArray m_MoveChars;

protected:
	int m_iClientsAvg;	// peak per day of clients.

public:
	CGString m_sServVersion;	// Version string this server is using.
	CSocketAddress m_ip;	// socket and port.
	//OLD: CCryptBase m_ClientVersion;		// GRAY_CLIENT_VER 12537 = Base client crypt we are using.
	CCrypt m_ClientVersion;

	// Breakdown the string. or filled in locally.
	signed char m_TimeZone;	// Hours from GMT. +5=EST
	CGString m_sEMail;		// Admin email address.
	CGString m_sURL;			// URL for the server.
	CGString m_sLang;
	CGString m_sNotes;		// Notes to be listed for the server.
	ACCAPP_TYPE m_eAccApp;	// types of new account applications.
	CGString m_sRegisterPassword;	// The password  the server uses to id itself to the registrar.

private:
	void SetStatusFail( LPCTSTR pszTrying );

public:
	CServerDef( LPCTSTR pszName, CSocketAddressIP dwIP );

	LPCTSTR GetStatus() const
	{
		return(m_sStatus);
	}
	LPCTSTR GetServerVersion() const
	{
		return( m_sServVersion );
	}
	CServTime GetCreateTime() const
	{
		return m_timeCreate;
	}

	DWORD StatGet( SERV_STAT_TYPE i ) const;

	void StatInc( SERV_STAT_TYPE i )
	{
		ASSERT( i>=0 && i<SERV_STAT_QTY );
		m_dwStat[i]++;
	}
	void StatDec( SERV_STAT_TYPE i )
	{
		ASSERT( i>=0 && i<SERV_STAT_QTY );
		DEBUG_CHECK( m_dwStat[i] );
		m_dwStat[i]--;
	}
	void StatChange( SERV_STAT_TYPE i, int iChange )
	{
		ASSERT( i>=0 && i<SERV_STAT_QTY );
		m_dwStat[i] += iChange;
	}
	void SetStat( SERV_STAT_TYPE i, DWORD dwVal )
	{
		ASSERT( i>=0 && i<SERV_STAT_QTY );
		m_dwStat[i] = dwVal;
	}

	LPCTSTR GetName() const { return( m_sName ); }
	void SetName( LPCTSTR pszName );

	bool IsValidStatus() const;
	bool ParseStatus( LPCTSTR pszStatus, bool fStore );
	bool PollStatus();	// Do this on a seperate thread.

	virtual int GetAgeHours() const;

	bool IsSame( const CServerDef * pServNew ) const
	{
		if ( m_sRegisterPassword.IsEmpty())
			return( true );
		return( ! strcmpi( m_sRegisterPassword, pServNew->m_sRegisterPassword ));
	}

	void SetValidTime();
	int GetTimeSinceLastValid() const;
	void SetPollTime();
	int GetTimeSinceLastPoll() const;

	void ClientsInc()
	{
		StatInc( SERV_STAT_CLIENTS );
		if ( StatGet(SERV_STAT_CLIENTS) > m_iClientsAvg )
		{
			m_iClientsAvg = StatGet(SERV_STAT_CLIENTS);
		}
	}
	int GetClientsAvg() const
	{
		if ( m_iClientsAvg )
			return( m_iClientsAvg );
		return( StatGet(SERV_STAT_CLIENTS));
	}

	virtual void r_DumpVerbKeys( CTextConsole * pSrc )
	{
		CScriptObj::r_DumpVerbKeys(pSrc);
	}
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual bool r_LoadVal( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pKey, CGString &sVal, CTextConsole * pSrc = NULL );
	virtual void r_WriteData( CScript & s );
	void r_WriteCreated( CScript & s );

	bool IsConnected() const
	{
		return( m_timeLastValid.IsTimeValid() && ( m_timeLastPoll <= m_timeLastValid ));
	}
	void addToServersList( CCommand & cmd, int iThis, int jArray ) const;
};

#endif // _INC_CSERVERDEF_H