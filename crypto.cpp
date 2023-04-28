#include "crypto.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <type_traits>
#undef UNICODE
#include <tchar.h>
#include <wincrypt.h>

namespace{
    
    void MyHandleError(LPCTSTR psz)
    {
        _ftprintf(stderr, TEXT("An error occurred in the program. \n"));
        _ftprintf(stderr, TEXT("%s\n"), psz);
        _ftprintf(stderr, TEXT("Error number 0x%lX.\n"), GetLastError());
        _ftprintf(stderr, TEXT("Program terminating. \n"));
        exit(1);
    } // End of MyHandleError.
}

namespace crypto
{

    HCRYPTPROV getCryptoProv(LPCSTR pszKeyContainerName, ProvType dProvType)
    {
        (void)dProvType;
        
        // Handle for the cryptographic provider context.
        HCRYPTPROV hCryptProv = 0;        
        
        // The name of the container.
        if(pszKeyContainerName == nullptr)
            pszKeyContainerName = TEXT("Key Container for Encoding/Decoding");
    
        //---------------------------------------------------------------
        // Begin processing. Attempt to acquire a context by using the 
        // specified key container.
        if(CryptAcquireContext(
            &hCryptProv,
            pszKeyContainerName,
            MS_ENHANCED_PROV,
            PROV_RSA_FULL,
            0))
        {
            _tprintf(
                TEXT("A crypto context with the \"%s\" key container ")
                TEXT("has been acquired.\n"), 
                pszKeyContainerName);
        }
        else
        { 
            //-----------------------------------------------------------
            // Some sort of error occurred in acquiring the context. 
            // This is most likely due to the specified container 
            // not existing. Create a new key container.
            if(GetLastError() == static_cast<DWORD>(NTE_BAD_KEYSET))
            {
                if(CryptAcquireContext(
                    &hCryptProv, 
                    pszKeyContainerName, 
                    MS_ENHANCED_PROV, 
                    PROV_RSA_FULL, 
                    CRYPT_NEWKEYSET)) 
                {
                    _tprintf(TEXT("A new key container has been ")
                             TEXT("created.\n"));
                }
                else
                {
                    MyHandleError(TEXT("Could not create a new key ")
                                  TEXT("container.\n"));
                }
            }
            else
            {
                MyHandleError(TEXT("CryptAcquireContext failed.\n"));
            }
        }
        return hCryptProv;
    }


    HCRYPTKEY getKeyPair(KeyPairType type, HCRYPTPROV hCryptProv)
    {    
        //---------------------------------------------------------------
        // A context with a key container is available.
        // Attempt to get the handle to the key. 
    
        // Public/private key handle.
        HCRYPTKEY hKey;  

        DWORD dwKeySpec = static_cast<std::underlying_type_t<KeyPairType>>(type) ;

        if(CryptGetUserKey(
            hCryptProv,
            dwKeySpec,
            &hKey))
        {
            _tprintf(TEXT("A key is available.\n"));
        }
        else
        {
            _tprintf(TEXT("No key is available.\n"));
            if(GetLastError() == static_cast<DWORD>(NTE_NO_KEY)) 
            {
                //-------------------------------------------------------
                // The error was that there is a container but no key.
                 // Create a signature key pair. 
                _tprintf(TEXT("The key does not exist.\n"));
                _tprintf(TEXT("Create a key pair.\n")); 
                if(CryptGenKey(
                    hCryptProv,
                    dwKeySpec,
                    0,
                    &hKey)) 
                {
                    _tprintf(TEXT("Created a key pair.\n"));
                }
                else
                {
                    MyHandleError(TEXT("Error occurred creating a key\n")); 
                }
            }
            else
            {
                MyHandleError(TEXT("An error other than NTE_NO_KEY ")
                              TEXT("getting a key.\n"));
            }
        }
        
        _tprintf(TEXT("A key pair existed, or one was ")
                 TEXT("created.\n\n"));
        
        return hKey;
    }

    HCRYPTKEY getSessionKey(HCRYPTPROV hCryptProv)
    {
        HCRYPTKEY hKey = 0;
        if (CryptGenKey(     
            hCryptProv,      
            CALG_RC4,      
            CRYPT_EXPORTABLE, 
            &hKey))
        {   
            _tprintf(TEXT("Original session key is created. \n"));
        }
        else
        {
           MyHandleError(TEXT("ERROR -- CryptGenKey."));
        }
        return hKey;
    }

    DWORD exportKey(HCRYPTKEY hKey, HCRYPTKEY hXchgKey, ExportKeyType KeyType, BYTE** pbKeyBlob)
    {
        DWORD dwBlobLen = 0;
        DWORD dwBlobType = static_cast<std::underlying_type_t<ExportKeyType>>(KeyType);

        if(CryptExportKey(
            hKey, 
            hXchgKey, 
            dwBlobType, 
            0, 
            NULL, 
            &dwBlobLen)) 
        {
             _tprintf(TEXT("Size of the BLOB for the session key determined. \n"));
        }
        else
        {
            MyHandleError(TEXT("Error computing BLOB length."));
        }

        if((*pbKeyBlob = new BYTE [dwBlobLen])) 
        {
            _tprintf(TEXT("Memory has been allocated for the BLOB. \n"));
        }
        else
        {
            MyHandleError(TEXT("Out of memory. \n"));
        }

        if(CryptExportKey(
            hKey, 
            hXchgKey, 
            dwBlobType, 
            0, 
            *pbKeyBlob, 
            &dwBlobLen))
        {
            _tprintf(TEXT("Contents have been written to the BLOB. \n"));
        }
        else
        {
            MyHandleError(TEXT("Error during CryptExportKey."));
        }
    
        return dwBlobLen;
    }

    HCRYPTKEY importKey(HCRYPTPROV hCryptProv, HCRYPTKEY hPubKey, const BYTE* pbData, DWORD dwDataSize)
    {
        HCRYPTKEY hKey = 0;
        if (!CryptImportKey(
            hCryptProv,
            pbData,
            dwDataSize,
            0,
            CRYPT_EXPORTABLE,
            &hKey ))
        {
            _tprintf(TEXT("Error 0x%08lx in importing the key \n"), GetLastError());
        }
        return hKey;
    }

    DWORD encryptData(HCRYPTKEY hKey, BYTE* pbDataIn, DWORD dwDataLenIn, BYTE** pbDataOut, DWORD* dwDataLenOut)
    {
        DWORD dwCipherTextSize = dwDataLenIn;

        if(!CryptEncrypt(
            hKey, 
            0, 
            TRUE,
            0, 
            pbDataIn, 
            &dwCipherTextSize, 
            dwDataLenIn))
        { 
            MyHandleError(TEXT("Error occured in encryptData\n")); 
        }

        _tprintf(TEXT("%d bytes is needed to store the ciphertext\n"), dwCipherTextSize);
        _tprintf(TEXT("data was encrypted successfully\n"));

        return dwCipherTextSize;
    }

    DWORD decryptData(HCRYPTKEY hKey, BYTE* pbBuffer, DWORD dwCipherTextLen)
    {
        if(!CryptDecrypt(
            hKey, 
            0, 
            TRUE, 
            0, 
            pbBuffer, 
            &dwCipherTextLen))
        {
            MyHandleError(TEXT("Error occured in decryptData!\n")); 
        }

        return dwCipherTextLen;
    }

    BOOL destroyKey(HCRYPTKEY hKey)
    {
        return CryptDestroyKey(hKey);
    }

    BOOL releaseCryptoProv(HCRYPTPROV hCryptProv)
    {
        return CryptReleaseContext(hCryptProv, 0);
    }
} /* namespace crypto */