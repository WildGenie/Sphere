//
// CEncrypt.h
//

// --- Two Fish ---
#include "twofish/aes.h"

// --- Blow Fish ---
// #include "blowfish/blowfish.h"

// --- MD5 ----
#include "CMD5.h"

#define CLIENT_END 0x00000001

enum CONNECT_TYPE	// What type of client connection is this ?
{
	CONNECT_NONE,	// There is no connection.
	CONNECT_UNK,		// client has just connected. waiting for first message.
	CONNECT_CRYPT,		// It's a game or login protocol but i don't know which yet.
	CONNECT_LOGIN,			// login client protocol
	CONNECT_GAME,			// game client protocol
	CONNECT_CONSOLE,		// we at the local console.
	CONNECT_HTTP,			// We are serving web pages to this.
	CONNECT_AUTO_SERVER,	// Auto listing server request.
	CONNECT_PEER_SERVER,	// only secure listed peer servers can talk to us like this.
	CONNECT_TELNET,			// we are in telnet console mode.
	CONNECT_IRC,			// we are an IRC client.
	CONNECT_PING,			// This will be dropped immediately anyhow.
	CONNECT_GAME_PROXY,		// Just proxying this to another server. (Where the char really is)
	CONNECT_QTY,
};

enum ENCRYPTION_TYPE
{
	ENC_NONE=0,
	ENC_BFISH,
	ENC_BTFISH,
	ENC_TFISH,
	ENC_QTY,
};

// ---------------------------------------------------------------------------------------------------------------
// ===============================================================================================================

// HuffMan Compression Class

class CCompressBranch
{
	// For compressing/decompressing stuff from game server to client.
	friend class CCompressTree;
private:
	int m_Value; // -1=tree branch, 0-255=character value, 256=PROXY_BRANCH_QTY-1=end
	CCompressBranch *m_pZero;
	CCompressBranch *m_pOne;
private:
	CCompressBranch()
	{
		m_Value = -1;	// just a pass thru branch til later.
		m_pZero = NULL;
		m_pOne = NULL;
	}
	~CCompressBranch()
	{
		if ( m_pOne != NULL )
			delete m_pOne;
		if ( m_pZero != NULL )
			delete m_pZero;
	}
	bool IsLoaded() const
	{
		return( m_pZero != NULL );
	}
};

#define COMPRESS_TREE_SIZE (256+1)

class CCompressTree
{
private:
	CCompressBranch m_Root;
	const CCompressBranch * m_pDecodeCur;
	int m_iDecodeIndex;
private:
	static const WORD sm_xCompress_Base[COMPRESS_TREE_SIZE];
private:
	bool AddBranch(int Value, WORD wCode, int iBits );
public:
	void Reset()
	{
		m_pDecodeCur = &m_Root;
		m_iDecodeIndex = 0;
	}
	bool IsLoaded() const
	{
		return( m_Root.IsLoaded() );
	}
	bool IsCompletePacket() const
	{
		return( m_iDecodeIndex == 0 );
	}
	int GetIncompleteSize() const
	{
		return( m_iDecodeIndex );
	}
	
	static int Encode( BYTE * pOutput, const BYTE * pInput, int inplen );
	bool Load();
	int  Decode( BYTE * pOutput, const BYTE * pInput, int inpsize );
	// NOTE: Obsolete !!
	static void CompressXOR( BYTE * pData, int iLen, DWORD & dwIndex );
};

// ---------------------------------------------------------------------------------------------------------------
// ===============================================================================================================

union CCryptKey
{
	#define CRYPT_GAMESEED_LENGTH	8
	BYTE  u_cKey[CRYPT_GAMESEED_LENGTH];
	DWORD u_iKey[2];
};

// ---------------------------------------------------------------------------------------------------------------
// ===============================================================================================================

struct CCrypt
{

private:
	bool m_fInit;
	int m_iClientVersion;
	ENCRYPTION_TYPE m_GameEnc;

protected:
	DWORD m_MasterHi;
	DWORD m_MasterLo;

	DWORD m_CryptMaskHi;
	DWORD m_CryptMaskLo;

	DWORD m_seed;	// seed ip we got from the client.
	
	CONNECT_TYPE m_ConnectType;
	
	#define TOTAL_CLIENTS 41
	static const DWORD client_keys[TOTAL_CLIENTS+2][4];

protected:
	// --------------- Two Fish ------------------------------
	#define TFISH_RESET 0x100
	keyInstance tf_key;
	cipherInstance tf_cipher;
	BYTE tf_cipherTable[TFISH_RESET];
	int tf_position;
private:
	void InitTwoFish();
	BYTE DecryptTFByte( BYTE bEnc );
	void DecryptTwoFish( BYTE * pOutput, const BYTE * pInput, int iLen );
	// --------------- EOF TwoFish ----------------------------
	
	
protected:
	// -------------- Blow Fish ------------------------------
	#define CRYPT_GAMEKEY_COUNT	25 // CRYPT_MAX_SEQ
	#define CRYPT_GAMEKEY_LENGTH	6
	
	#define CRYPT_GAMETABLE_START	1
	#define CRYPT_GAMETABLE_STEP	3
	#define CRYPT_GAMETABLE_MODULO	11
	#define CRYPT_GAMETABLE_TRIGGER	21036
	static const BYTE sm_key_table[CRYPT_GAMEKEY_COUNT][CRYPT_GAMEKEY_LENGTH];
	static const BYTE sm_seed_table[2][CRYPT_GAMEKEY_COUNT][2][CRYPT_GAMESEED_LENGTH];
	static bool  sm_fTablesReady;
public:
	int	m_gameTable;
	int	m_gameBlockPos;		// 0-7
	int	m_gameStreamPos;	// use this to track the 21K move to the new Blowfish m_gameTable.
private:
	CCryptKey m_Key;
private:
	void InitSeed( int iTable );
	static void InitTables();
	static void PrepareKey( CCryptKey & key, int iTable );
	void DecryptBlowFish( BYTE * pOutput, const BYTE * pInput, int iLen );
	BYTE DecryptBFByte( BYTE bEnc );
	void InitBlowFish();
	// -------------- EOF BlowFish -----------------------

protected:
	// -------------------- MD5 ------------------------------
	#define MD5_RESET 0x0F
	CMD5 md5_engine;
	unsigned int md5_position;
	BYTE md5_digest[16];
protected:
	void EncryptMD5( BYTE * pOutput, const BYTE * pInput, int iLen );
	void InitMD5(BYTE * ucInitialize);
	// ------------------ EOF MD5 ----------------------------

private:
	// ------------- Old Encryption ----------------------
	void EncryptOld( BYTE * pOutput, const BYTE * pInput, int iLen  );
	void DecryptOld( BYTE * pOutput, const BYTE * pInput, int iLen  );
	// ------------- EOF Old Encryption ------------------

private:
	int GetVersionFromString( LPCTSTR pszVersion );

private:
	void SetClientVersion( int iVer )
	{
		m_iClientVersion = iVer;
	}
	
	void SetMasterKeys( DWORD hi, DWORD low )
	{
		m_MasterHi = hi;
		m_MasterLo = low;
	}
	
	void SetCryptMask( DWORD hi, DWORD low )
	{
		m_CryptMaskHi = hi;
		m_CryptMaskLo= low;
	}
	
	bool SetConnectType( CONNECT_TYPE ctWho )
	{
		if ( ctWho > CONNECT_NONE && ctWho < CONNECT_QTY )
		{
			m_ConnectType = ctWho;
			return true;
		}
		
		return false;
	}
	
	bool SetEncryptionType( ENCRYPTION_TYPE etWho )
	{
		if ( etWho >= ENC_NONE && etWho < ENC_QTY )
		{
			m_GameEnc = etWho;
			return true;
		}
		
		return false;
	}

public:
	TCHAR* WriteClientVer( TCHAR * pStr ) const;
	bool SetClientVerEnum( int iVer, bool bSetEncrypt = true );
	bool SetClientVerIndex( int iVer, bool bSetEncrypt = true );
	void SetClientVer( const CCrypt & crypt );
	bool SetClientVer( LPCTSTR pszVersion );

public:
	int GetClientVer() const
	{
		return( m_iClientVersion );
	}
	
	bool IsValid() const
	{
		return( m_iClientVersion >= 0 );
	}

	bool IsInit() const
	{
		return( m_fInit );
	}
		
	CONNECT_TYPE GetConnectType() const
	{
		return( m_ConnectType );
	}
	
	ENCRYPTION_TYPE GetEncryptionType() const
	{
		return( m_GameEnc );
	}

// --------- Basic
public:
	CCrypt();
	bool Init( DWORD dwIP, BYTE * pEvent, int iLen );
	void InitFast( DWORD dwIP, CONNECT_TYPE ctInit );
	void Decrypt( BYTE * pOutput, const BYTE * pInput, int iLen );
	void Encrypt( BYTE * pOutput, const BYTE * pInput, int iLen );
protected:
	void LoginCryptStart( DWORD dwIP, BYTE * pEvent, int iLen );
	void GameCryptStart( DWORD dwIP, BYTE * pEvent, int iLen );
	
};