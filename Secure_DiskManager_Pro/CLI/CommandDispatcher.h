#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include "command_context.h"

class CommandDispatcher {
public:
    static void Dispatch(const CommandContext& ctx);
};

#endif // COMMAND_DISPATCHER_H
