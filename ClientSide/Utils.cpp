#include "Utils.h"

bool IsValidName(const std::string& s) {
    return (s.size() >= 3 && s.size() <= 12);
}