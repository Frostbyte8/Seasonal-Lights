#pragma once

#ifdef _DEBUG
#define NODEFAULT assert(0)
#else
#define NODEFAULT __assume(0)
#endif // DEBUG

namespace FrostUtil {

    // At some point, I'll template this
    template <typename T>
    __forceinline T ClampInts(const T& newValue, const T& minValue, const T& maxValue) {

        if(newValue > maxValue) {
            return maxValue;
        }
        else if(newValue < minValue) {
            return minValue;
        }

        return newValue;
    }
}
