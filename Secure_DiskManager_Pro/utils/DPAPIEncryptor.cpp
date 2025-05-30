#include "DPAPIEncryptor.h"
#include <windows.h>
#include <wincrypt.h>
#include <tinyxml2.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace tinyxml2;

namespace DPAPIEncryptor {

    static std::string bytes_to_hex_string(const uint8_t* data, size_t length) {
        std::ostringstream oss;
        for (size_t i = 0; i < length; ++i)
            oss << std::hex << std::setfill('0') << std::setw(2) << (int)data[i];
        return oss.str();
    }

    bool encrypt_to_xml(const std::string& plaintext, const std::string& outPath) {
        DATA_BLOB inputBlob, outputBlob;
        inputBlob.pbData = (BYTE*)plaintext.data();
        inputBlob.cbData = static_cast<DWORD>(plaintext.size());

        if (!CryptProtectData(&inputBlob, L"stealthmailer", nullptr, nullptr, nullptr, 0, &outputBlob)) {
            std::cerr << "[DPAPI] Encryption failed." << std::endl;
            return false;
        }

        std::string hex = bytes_to_hex_string(outputBlob.pbData, outputBlob.cbData);
        LocalFree(outputBlob.pbData);

        XMLDocument doc;
        XMLElement* root = doc.NewElement("Objs");
        doc.InsertFirstChild(root);

        XMLElement* ss = doc.NewElement("SS");
        ss->SetText(hex.c_str());
        root->InsertEndChild(ss);

        if (doc.SaveFile(outPath.c_str()) != XML_SUCCESS) {
            std::cerr << "[DPAPI] Failed to write XML: " << outPath << std::endl;
            return false;
        }

        return true;
    }

}
