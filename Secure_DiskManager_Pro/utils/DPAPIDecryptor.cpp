#include "DPAPIDecryptor.h"
#include <windows.h>
#include <wincrypt.h>
#include <tinyxml2.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

using namespace tinyxml2;

namespace DPAPIDecryptor {

    static std::vector<uint8_t> hex_string_to_bytes(const std::string& hex) {
        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteStr = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(strtol(byteStr.c_str(), nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

    bool decrypt_from_xml(const std::string& xmlPath, std::string& outPlaintext) {
        XMLDocument doc;
        if (doc.LoadFile(xmlPath.c_str()) != XML_SUCCESS) {
            std::cerr << "[DPAPI] Failed to load XML: " << xmlPath << std::endl;
            return false;
        }

        XMLElement* ssElem = doc.FirstChildElement("Objs")->FirstChildElement("SS");
        if (!ssElem) {
            std::cerr << "[DPAPI] <SS> element not found." << std::endl;
            return false;
        }

        const char* hexBlob = ssElem->GetText();
        if (!hexBlob) {
            std::cerr << "[DPAPI] Empty <SS> element." << std::endl;
            return false;
        }

        std::string hexStr(hexBlob);
        std::vector<uint8_t> encryptedData = hex_string_to_bytes(hexStr);

        DATA_BLOB inputBlob;
        inputBlob.pbData = encryptedData.data();
        inputBlob.cbData = static_cast<DWORD>(encryptedData.size());

        DATA_BLOB outputBlob;
        ZeroMemory(&outputBlob, sizeof(outputBlob));

        if (!CryptUnprotectData(&inputBlob, nullptr, nullptr, nullptr, nullptr, 0, &outputBlob)) {
            std::cerr << "[DPAPI] CryptUnprotectData failed." << std::endl;
            return false;
        }

        outPlaintext.assign(reinterpret_cast<char*>(outputBlob.pbData), outputBlob.cbData);
        LocalFree(outputBlob.pbData);
        return true;
    }

}
