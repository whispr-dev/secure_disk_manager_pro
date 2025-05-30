#ifndef QUANTUM_ENGINE_H
#define QUANTUM_ENGINE_H

#include <string>
#include <vector>
#include "QubitTypes.h"

class QuantumEngine {
public:
    static Qubit encodeTextAsQubit(const std::string& text);
    static double similarity(const Qubit& a, const Qubit& b);
};

#endif
