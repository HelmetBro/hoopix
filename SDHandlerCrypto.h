#ifndef SD_HANDLER_CRYPTO_H
#define SD_HANDLER_CRYPTO_H

/*
Save this class for later, when I'm ready to encrypt everything.
Don't use this file, instead use SDHandler.h. Swap with this when
it's fully implemented. Putting this file on pause now.
*/


#include <Crypto.h>
// #include <CryptoLegacy.h>
#include <AES.h>
// #include <CBC.h>

// #include <Crypto.h>
// #include <CryptoLegacy.h>
// #include <AES.h>
// #include <CBC.h>
// #include <string.h>

// #include <Cipher.h>
// #include <PKCS7.h>
#include <EEPROM.h>
// #include <cstring>

template <typename T>
class SecureDataHandler {
public:

  // Note that the SD card has to be initialized before using this class
  SecureDataHandler(String filename) : filename(filename) {
    if(filename.length() < 5 || filename.length() > 12) {
        // Handle error: Filename length should be between 5 (including .bin) and 12 to fit in 8.3 format
    }
    if(filename.indexOf('/') != -1 || filename.indexOf('\\') != -1) {
        // Handle error: Filename should not contain path characters
    }
    if(!filename.endsWith(".bin")) {
        // Handle error: Filename should end with .bin extension
    }
    for (int i = 0; i < 16; i++) {
        key[i] = EEPROM.read(i);
    }
  }

  void writeData(const T &data) {
    AES256 aes;
    CBC<PKCS7> cbc(aes);

    // 1. Serialize the struct to a byte array
    auto dataBytes = serialize(data);
    int dataLength = sizeof(T);

    // Prepare a buffer for the encrypted data, size of the original data + padding
    int cipherLength = dataLength + aes.blockSize() - dataLength % aes.blockSize();
    byte cipher[cipherLength];

    // 2. Encrypt the data with the current key using CBC
    cbc.setKey(key, aes.keySize());
    cbc.encrypt(cipher, dataBytes.get(), dataLength);

    // 3. Write the encrypted data to the SD card
    myFile = SD.open(filename.c_str(), FILE_WRITE);
    if (myFile) {
      myFile.write(cipher, cipherLength);
      myFile.close();
    } else {
      // Handle error
    }

    // 4. Generate and store a new random key
    generateAndStoreNewKey();
  }

T readData() {
    AES aes;
    CBC<PKCS7> cbc(aes);

    // Determine size of encrypted data on SD card
    myFile = SD.open(filename.c_str());
    if (!myFile) {
      // Handle error
    }
    int cipherLength = myFile.size();
    myFile.close();

    // Prepare a buffer for the encrypted data
    std::unique_ptr<byte[]> cipher(new byte[cipherLength]);

    // 1. Read the encrypted data from the SD card
    myFile = SD.open(filename.c_str());
    if (myFile) {
      myFile.read(cipher.get(), cipherLength);
      myFile.close();
    } else {
      // TODO: Handle error
    }

    // Prepare a buffer for the decrypted data
    std::unique_ptr<byte[]> decryptedData(new byte[cipherLength]);

    // 2. Decrypt the data with the key using CBC
    cbc.setKey(key, aes.keySize());
    cbc.decrypt(decryptedData.get(), cipher.get(), cipherLength);

    // 3. Deserialize the byte array back into a struct
    return deserialize(decryptedData.get());
  }

private:
  byte key[16];
  File myFile;
  String filename;

  void generateAndStoreNewKey() {
    // Generate a new random key
    for (int i = 0; i < 16; i++) {
      key[i] = random(0, 256);
      EEPROM.write(i, key[i]);
    }
  }

  std::unique_ptr<byte[]> serialize(const T &data) {
    // Convert the struct to a byte array
    std::unique_ptr<byte[]> result(new byte[sizeof(T)]);
    memcpy(result.get(), &data, sizeof(T));
    return result;
  }

  T deserialize(const byte* dataBytes) {
    // Convert the byte array back into a struct
    T result;
    memcpy(&result, dataBytes, sizeof(T));
    return result;
  }
};

#endif