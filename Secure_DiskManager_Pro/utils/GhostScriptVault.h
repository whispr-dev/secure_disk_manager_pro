#ifndef GHOST_SCRIPT_VAULT_H
#define GHOST_SCRIPT_VAULT_H

#include <string>

namespace GhostScriptVault {

    // Decrypt the blob from XML and return its contents
    bool decrypt_script(const std::string& xmlPath, std::string& outScript);

    // Optionally execute the script via PowerShell
    bool execute_decrypted_script(const std::string& scriptContents);

}

#endif // GHOST_SCRIPT_VAULT_H
