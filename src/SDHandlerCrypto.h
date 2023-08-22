/*
Save this class for later, when I'm ready to encrypt everything.
Don't use this file, instead use SDHandler.h. Swap with this when
it's fully implemented. Putting this file on pause now.
*/

#ifndef SD_HANDLER_CRYPTO_H
#define SD_HANDLER_CRYPTO_H

#include <Crypto.h>
#include <AES.h>
#include <EEPROM.h>
#include "Utils.h"

template <typename T>
class SecureDataHandler {
public:
    SecureDataHandler(String filename) : filename(filename) {
        LOG("Initializing SecureDataHandler with filename: %s", filename.c_str());

        if (filename.length() < 5 || filename.length() > 12) {
            BUG("Filename length should be between 5 (including .bin) and 12 to fit in 8.3 format");
        }
        if (filename.indexOf('/') != -1 || filename.indexOf('\\') != -1) {
            BUG("Filename should not contain path characters");
        }
        if (!filename.endsWith(".bin")) {
            BUG("Filename should end with .bin extension");
        }
        for (int i = 0; i < 16; i++) {
            key[i] = EEPROM.read(i);
        }
    }

    void writeData(const T &data) {
        LOG("Writing encrypted data");

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
            LOG("Encrypted data written successfully");
        } else {
            BUG("Failed to write encrypted data");
        }

        // 4. Generate and store a new random key
        generateAndStoreNewKey();
    }

    T readData() {
        LOG("Reading encrypted data");

        AES aes;
        CBC<PKCS7> cbc(aes);

        // Determine size of encrypted data on SD card
        myFile = SD.open(filename.c_str());
        if (!myFile) {
            BUG("Failed to open file for reading");
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
            LOG("Encrypted data read successfully");
        } else {
            BUG("Failed to read encrypted data");
        }

        // Prepare a buffer for the decrypted data
        std::unique_ptr<byte[]> decryptedData(new byte[cipherLength]);

        // 2. Decrypt the data with the key using CBC
        cbc.setKey(key, aes.keySize());
        cbc.decrypt(decryptedData.get(), cipher.get(), cipherLength);

        // 3. Deserialize the byte array back into a struct
        LOG("Decryption and deserialization successful");
        return deserialize(decryptedData.get());
    }

private:
    byte key[16];
    File myFile;
    String filename;

    void generateAndStoreNewKey() {
        LOG("Generating and storing new random key");
        for (int i = 0; i < 16; i++) {
            key[i] = random(0, 256);
            EEPROM.write(i, key[i]);
        }
    }

    std::unique_ptr<byte[]> serialize(const T &data) {
        LOG("Serializing data");
        std::unique_ptr<byte[]> result(new byte[sizeof(T)]);
        memcpy(result.get(), &data, sizeof(T));
        return result;
    }

    T deserialize(const byte* dataBytes) {
        LOG("Deserializing data");
        T result;
        memcpy(&result, dataBytes, sizeof(T));
        return result;
    }
};

#endif
