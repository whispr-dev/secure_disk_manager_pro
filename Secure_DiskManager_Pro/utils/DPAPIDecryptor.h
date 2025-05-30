#ifndef DPAPI_DECRYPTOR_H
#define DPAPI_DECRYPTOR_H

#include <string>
#include <vector>

namespace DPAPIDecryptor {

    bool decrypt_from_xml(const std::string& xmlPath, std::string& outPlaintext);

}

#endif // DPAPI_DECRYPTOR_H
