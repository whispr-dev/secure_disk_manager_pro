#include "main_controller.h"
#include <string>
#include <iostream>

CommandContext MainController::ParseArgs(int argc, char* argv[]) {
    CommandContext ctx;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--search" && i + 1 < argc) {
            ctx.search = true;
            ctx.searchQuery = argv[++i];
        } else if (arg == "--copy" && i + 1 < argc) {
            ctx.inputFile = argv[++i];
        } else if (arg == "--encrypt" && i + 1 < argc) {
            ctx.encrypt = true;
            ctx.keyFile = argv[++i];
        } else if (arg == "--decrypt" && i + 1 < argc) {
            ctx.decrypt = true;
            ctx.keyFile = argv[++i];
        } else if (arg == "--compress") {
            ctx.compress = true;
        } else if (arg == "--decompress") {
            ctx.decompress = true;
        } else if (arg == "--upload" && i + 1 < argc) {
            ctx.upload = true;
            ctx.serverUrl = argv[++i];
        } else if (arg == "--download" && i + 1 < argc) {
            ctx.download = true;
            ctx.serverUrl = argv[++i];
        } else if (arg == "--retries" && i + 1 < argc) {
            ctx.retries = std::stoi(argv[++i]);
        } else if (arg == "--shred") {
            ctx.shred = true;
        } else if (arg == "--selfdelete") {
            ctx.selfdelete = true;
        } else {
            ctx.args.push_back(arg);
        }
    }

    return ctx;
}
