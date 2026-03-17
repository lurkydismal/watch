#pragma once

#include <sys/inotify.h>

#include <cstdint>
#include <functional>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace watch {

using event_t = enum class event : uint16_t {
    none,
    remove = ( IN_DELETE | IN_DELETE_SELF ),
    write = IN_CLOSE_WRITE,
    rename = IN_MOVE,
};

using eventUnderlying_t = std::underlying_type_t< event_t >;

using watch_t = struct watch {
    using callbackFile_t =
        std::function< bool( std::string_view _fileName, event_t _event ) >;

    // If event is rename then callback will be called two times with the same
    // cookie
    // Cookie is 0 on all other events
    using callbackDirectory_t = std::function<
        bool( std::string_view _fileName, event_t _event, uint32_t _cookie ) >;

    watch() = delete;

    watch( std::string_view _path, callbackFile_t&& _callback, event_t _event )
        : watch( _path,
                 static_cast< callback_t >( std::move( _callback ) ),
                 _event ) {}

    watch( std::string_view _path,
           callbackDirectory_t&& _callback,
           event_t _event )
        : watch( _path,
                 static_cast< callback_t >( std::move( _callback ) ),
                 _event ) {}

    watch( const watch& ) = delete;
    watch( watch&& ) = default;

    ~watch();

    auto operator=( const watch& ) -> watch& = delete;
    auto operator=( watch&& ) -> watch& = default;

    void check( bool _isBlocking );

    // Helper
private:
    // TODO: Improve
    using callback_t = std::variant< callbackFile_t, callbackDirectory_t >;

    watch( std::string_view _path, callback_t&& _callback, event_t _event );

    // Variable
private:
    int _inotifyDescriptor;
    int _epollDescriptor;
    std::vector< int > _watchDescriptors;
    // TODO: Multiple callbacks
    callback_t _callback;
};

} // namespace watch
