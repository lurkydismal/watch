#pragma once

#include <concepts>

namespace example {

template < std::integral... Arguments >
[[nodiscard]] constexpr auto add( Arguments... _arguments ) -> int {
    return ( _arguments + ... );
}

} // namespace example
