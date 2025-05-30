#ifndef DPAPI_ENCRYPTOR_H
#define DPAPI_ENCRYPTOR_H

#include <string>

namespace DPAPIEncryptor {

    bool encrypt_to_xml(const std::string& plaintext, const std::string& outPath);

}

#endif // DPAPI_ENCRYPTOR_H
