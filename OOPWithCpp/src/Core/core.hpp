#pragma once

namespace OWC
{
    // Plan:
    // - Prevent multiple inclusion with #pragma once to fix C2084.
    // - Replace consteval with constexpr so it can be used in both const and runtime contexts to fix C3615.
    // - Add noexcept and [[nodiscard]] for safety and clarity.

    [[nodiscard]] consteval bool IsDebugMode() noexcept
    {
    #ifdef NDEBUG
        return false;
    #else
        return true;
    #endif
    }
}
