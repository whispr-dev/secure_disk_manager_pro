#include "command_dispatcher.h"
#include <iostream>

void CommandDispatcher::Dispatch(const CommandContext& ctx) {
    if (ctx.search) {
        std::cout << "Performing search: " << ctx.searchQuery << std::endl;
        // Call search module
    }

    if (ctx.encrypt) {
        std::cout << "Encrypting with key: " << ctx.keyFile << std::endl;
        // Call encryption module
    }

    if (ctx.decrypt) {
        std::cout << "Decrypting with key: " << ctx.keyFile << std::endl;
        // Call decryption module
    }

    if (ctx.compress) {
        std::cout << "Compressing file..." << std::endl;
        // Call compression module
    }

    if (ctx.decompress) {
        std::cout << "Decompressing file..." << std::endl;
        // Call decompression module
    }

    if (ctx.upload) {
        std::cout << "Uploading to: " << ctx.serverUrl << std::endl;
        // Call upload module
    }

    if (ctx.download) {
        std::cout << "Downloading from: " << ctx.serverUrl << std::endl;
        // Call download module
    }

    if (ctx.shred) {
        std::cout << "Shredding file..." << std::endl;
        // Call shred module
    }

    if (ctx.selfdelete) {
        std::cout << "Performing self-delete..." << std::endl;
        // Call selfdelete module
    }
}
