#include "crypto.hpp"
#include "net.hpp"
#include <stdio.h>
#include <tchar.h>

#define UNUSED(x) (void)(x)


int main(void) 
{ 

    HCRYPTPROV hCryptoProv  = crypto::getCryptoProv("Key Containter for Encoding/Decoding", crypto::ProvType::RSA);
    HCRYPTKEY hExchKey = crypto::getKeyPair(crypto::KeyPairType::Exchange,hCryptoProv);
    HCRYPTKEY hSigKey = crypto::getKeyPair(crypto::KeyPairType::Signature, hCryptoProv);
    HCRYPTKEY hSesKey = crypto::getSessionKey(hCryptoProv);

    BYTE* pbBlob = NULL;

    DWORD dwBlobSize = crypto::exportKey(hSesKey, hExchKey, crypto::ExportKeyType::Session, &pbBlob);

    printf("Blob size is %li\n", dwBlobSize);
    printf("Blob:\n");
    for(DWORD i = 0; i < dwBlobSize; ++i)
        printf("%hhx ", pbBlob[i]);
    printf("\n");

    HCRYPTKEY hImpSesKey = crypto::importKey(hCryptoProv, hExchKey, pbBlob, dwBlobSize);
    BYTE* pbBlob2 = NULL;

    DWORD dwBlobSize2 = crypto::exportKey(hImpSesKey, hExchKey, crypto::ExportKeyType::Session, &pbBlob2);
    printf("Blob size is %li\n", dwBlobSize2);
    printf("Blob:\n");
    for(DWORD i = 0; i < dwBlobSize2; ++i)
        printf("%hhx ", pbBlob2[i]);
    printf("\n");



    unsigned char test[] = "hello";
    printf("before encryption:\n");
    for(DWORD i = 0; i < sizeof(test); ++i)
        printf("0x%hhX ", test[i]);
    printf("\n");

    crypto::encryptData(hSesKey, test, sizeof(test));
    
    printf("\nafter encryption and before decryption:\n");
    for(DWORD i = 0; i < sizeof(test); ++i)
        printf("0x%hhX ", test[i]);
    printf("\n");
    
    crypto::decryptData(hSesKey, test, sizeof(test));
    
    printf("\nafter decryption:\n");
    for(DWORD i = 0; i < sizeof(test); ++i)
        printf("0x%hhX ", test[i]);
    printf("\n");

    delete [] pbBlob;
    delete [] pbBlob2;

#define STR(x) #x
    printf(__FUNCTION__);
    
    crypto::destroyKey(hExchKey);
    crypto::destroyKey(hSigKey);
    crypto::destroyKey(hSesKey);
    crypto::destroyKey(hImpSesKey);
    crypto::releaseCryptoProv(hCryptoProv);

    _tprintf(TEXT("Everything is okay."));  
} 