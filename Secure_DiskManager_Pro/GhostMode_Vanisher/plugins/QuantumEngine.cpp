#include "QuantumEngine.h"
#include <cmath>

Qubit QuantumEngine::encodeTextAsQubit(const std::string& text) {
    double re = 0.0, im = 0.0;
    for (size_t i = 0; i < text.size(); ++i) {
        re += static_cast<double>(text[i]) * std::cos(i);
        im += static_cast<double>(text[i]) * std::sin(i);
    }
    QComplex a(re, im);
    QComplex b(im, re);
    return Qubit(a, b);
}

double QuantumEngine::similarity(const Qubit& a, const Qubit& b) {
    auto A = a.asArray();
    auto B = b.asArray();
    double dot = std::abs(std::conj(A[0]) * B[0] + std::conj(A[1]) * B[1]);
    return dot * dot; // Probability amplitude overlap
}
