#include "QubitTypes.h"
#include <cmath>
#include <sstream>

Qubit::Qubit() : alpha(1.0, 0.0), beta(0.0, 0.0) {}

Qubit::Qubit(QComplex a, QComplex b) : alpha(a), beta(b) {
    normalize();
}

void Qubit::normalize() {
    double mag = std::sqrt(std::norm(alpha) + std::norm(beta));
    if (mag != 0) {
        alpha /= mag;
        beta /= mag;
    }
}

std::array<QComplex, 2> Qubit::asArray() const {
    return { alpha, beta };
}

std::string Qubit::toString() const {
    std::ostringstream oss;
    oss << "(" << alpha << ")|0> + (" << beta << ")|1>";
    return oss.str();
}

std::array<std::array<QComplex, 2>, 2> getPauliX() {
    return {{
        { {0, 0}, {1, 0} },
        { {1, 0}, {0, 0} }
    }};
}

std::array<QComplex, 2> applyGate(const std::array<std::array<QComplex, 2>, 2>& gate, const std::array<QComplex, 2>& state) {
    return {
        gate[0][0] * state[0] + gate[0][1] * state[1],
        gate[1][0] * state[0] + gate[1][1] * state[1]
    };
}
