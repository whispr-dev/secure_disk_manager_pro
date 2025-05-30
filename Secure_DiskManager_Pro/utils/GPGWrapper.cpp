#include "gpg_wrapper.h"
#include <cstdlib>
#include <iostream>

bool GpgWrapper::encryptFile(const std::string& filepath, const std::string& recipientKeyID) {
    std::string cmd = "gpg --yes --batch --output " + filepath + ".gpg --encrypt --recipient " + recipientKeyID + " " + filepath;
    return std::system(cmd.c_str()) == 0;
}

bool GpgWrapper::decryptFile(const std::string& filepath, const std::string& outputpath) {
    std::string cmd = "gpg --yes --batch --output " + outputpath + " --decrypt " + filepath;
    return std::system(cmd.c_str()) == 0;
}

bool GpgWrapper::importKey(const std::string& keyfile) {
    std::string cmd = "gpg --import " + keyfile;
    return std::system(cmd.c_str()) == 0;
}

bool GpgWrapper::keyExists(const std::string& keyID) {
    std::string cmd = "gpg --list-keys " + keyID + " > /dev/null 2>&1";
    return std::system(cmd.c_str()) == 0;
}
