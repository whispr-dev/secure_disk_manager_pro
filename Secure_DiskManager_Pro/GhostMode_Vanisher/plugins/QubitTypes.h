#ifndef QUBIT_TYPES_H
#define QUBIT_TYPES_H

#include <array>
#include <complex>
#include <string>

using QComplex = std::complex<double>;

struct Qubit {
    QComplex alpha;
    QComplex beta;

    Qubit();
    Qubit(QComplex a, QComplex b);
    void normalize();
    std::array<QComplex, 2> asArray() const;
    std::string toString() const;
};

std::array<std::array<QComplex, 2>, 2> getPauliX();
std::array<QComplex, 2> applyGate(const std::array<std::array<QComplex, 2>, 2>& gate, const std::array<QComplex, 2>& state);

#endif
