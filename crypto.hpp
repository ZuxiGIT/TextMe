#pragma once
#include <windows.h>

namespace crypto
{
    enum class KeyPairType : BYTE
    {
        Exchange = AT_KEYEXCHANGE,
        Signature = AT_SIGNATURE,
    };

    enum class ProvType : BYTE
    {
        RSA = PROV_RSA_FULL,
    };

    enum class ExportKeyType : DWORD
    {
        Session = SIMPLEBLOB,
        Public = PUBLICKEYBLOB,
        KeyPair = PRIVATEKEYBLOB,
        AnyKey = PLAINTEXTKEYBLOB
    };

    HCRYPTPROV getCryptoProv(LPCWSTR szProvName, ProvType ProvType);
    HCRYPTKEY getKeyPair(KeyPairType type, HCRYPTPROV hCryptProv = 0);
    HCRYPTKEY getSessionKey(HCRYPTPROV hCryptProv);
    DWORD exportKey(HCRYPTKEY hKey, HCRYPTKEY hXchgKey, int KeyType, BYTE** pbKeyBlob);
    HCRYPTKEY importKey(HCRYPTPROV hCryptProv, HCRYPTKEY hPubKey, const BYTE* pbData, DWORD dwDataSize);
    DWORD encryptData(HCRYPTKEY hKey, BYTE* pbDataIn, DWORD dwDataLenIn, BYTE** pbDataOut = NULL, DWORD* dwDataLenOut = NULL);
    DWORD decryptData(HCRYPTKEY hKey, BYTE* pbBuffer, DWORD dwCipherTextLen);

    BOOL destroyKey(HCRYPTKEY hKey);
    BOOL releaseCryptoProv(HCRYPTPROV hCryptProv);
}