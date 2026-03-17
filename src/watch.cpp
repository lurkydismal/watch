#include "watch.hpp"

#include <linux/limits.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <utility>

#include "stddebug.hpp"
#include "stdfunc.hpp"

namespace watch {

namespace {

constexpr size_t g_maxEventAmount = 8;
constexpr size_t g_maxSingleEventSize =
    ( sizeof( inotify_event ) + NAME_MAX + 1 );

} // namespace

watch::~watch() {
#if 0
    // FIX: Maybe reduntant
    // Remove watches
    {
        for ( int _descriptor : _watchDescriptors ) {
            const bool l_result =
                ( inotify_rm_watch( _inotifyDescriptor, _descriptor ) !=
                -1 );

            stdfunc::assert( l_result );
        }

        _watchDescriptors.clear();
    }
#endif

    auto l_closeDescriptor = []( int _descriptor ) -> void {
        const bool l_result = ( close( _descriptor ) != -1 );

        stdfunc::assert( l_result );
    };

    l_closeDescriptor( _inotifyDescriptor );
    l_closeDescriptor( _epollDescriptor );
};

watch::watch( std::string_view _path, callback_t&& _callback, event_t _event )
    : _inotifyDescriptor( inotify_init1( IN_NONBLOCK | IN_ONLYDIR ) ),
      _epollDescriptor( epoll_create1( 0 ) ),
      _callback( std::move( _callback ) ) {
    stdfunc::assert( _inotifyDescriptor != -1 );
    stdfunc::assert( _epollDescriptor != -1 );

    // Epoll
    {
        epoll_event l_event = {
            .events = EPOLLIN,
            .data.fd = _inotifyDescriptor,
        };

        const bool l_result =
            ( epoll_ctl( _epollDescriptor, EPOLL_CTL_ADD, _inotifyDescriptor,
                         &l_event ) == -1 );

        stdfunc::assert( l_result );
    }

    // Inotify
    {
        // TODO: Improve _path
        int l_watchDescriptor =
            inotify_add_watch( _inotifyDescriptor, std::string( _path ).c_str(),
                               static_cast< uint32_t >( _event ) );

        const bool l_result = ( l_watchDescriptor != -1 );

        stdfunc::assert( l_result, "Adding watch to path: '{}'", _path );

        _watchDescriptors.emplace_back( l_watchDescriptor );
    }
}

void watch::check( bool _isBlocking ) {
    std::array< epoll_event, g_maxEventAmount > l_events{};

    // Infinite or no timeout
    const int l_timeout = ( ( _isBlocking ) ? ( -1 ) : ( 0 ) );

    int l_eventAmount = epoll_wait( _epollDescriptor, l_events.data(),
                                    l_events.size(), l_timeout );

    stdfunc::assert( l_eventAmount != -1 );

    for ( const epoll_event& _event :
          std::span( l_events.data(), l_eventAmount ) ) {
        if ( _event.data.fd == _inotifyDescriptor ) {
            std::vector< char > l_eventsBuffer( g_maxSingleEventSize *
                                                g_maxEventAmount );

            const ssize_t l_readAmount =
                read( _inotifyDescriptor, l_eventsBuffer.data(),
                      l_eventsBuffer.size() );

            if ( !l_readAmount ) {
                continue;
            }

            {
                char* l_eventPointer = l_eventsBuffer.data();

                while ( l_eventPointer <
                        ( l_eventsBuffer.data() + l_readAmount ) ) {
                    const auto l_event =
                        reinterpret_cast< inotify_event* >( l_eventPointer );

                    event_t l_eventType = event_t::none;

                    const auto l_isEventRemove =
                        []( uint32_t _eventMask ) -> bool {
                        return (
                            ( _eventMask & ( IN_DELETE | IN_DELETE_SELF |
                                             IN_MOVED_FROM | IN_MOVE_SELF ) ) );
                    };

                    const auto l_isEventWrite =
                        [ & ]( uint32_t _eventMask ) -> bool {
                        return (
                            ( _eventMask & ( IN_MODIFY | IN_CLOSE_WRITE ) ) &&
                            !l_isEventRemove( _eventMask ) );
                    };

                    const auto l_isEventRename =
                        []( uint32_t _eventMask ) -> bool {
                        return ( _eventMask & ( IN_MOVE | IN_MOVE_SELF ) );
                    };

                    if ( l_isEventRemove( l_event->mask ) ) {
                        l_eventType = event_t::remove;

                    } else if ( l_isEventWrite( l_event->mask ) ) {
                        l_eventType = event_t::write;

                    } else if ( l_isEventRename( l_event->mask ) ) {
                        l_eventType = event_t::rename;
                    }

                    if ( l_eventType != event_t::none ) {
                        auto l_eventName =
                            static_cast< const char* >( l_event->name );

                        const bool l_result =
                            _callback.visit( stdfunc::overloadedVisit{
                                [ & ]( callbackFile_t& _function ) -> bool {
                                    return (
                                        _function( l_eventName, l_eventType ) );
                                },
                                [ & ](
                                    callbackDirectory_t& _function ) -> bool {
                                    return ( _function( l_eventName,
                                                        l_eventType,
                                                        l_event->cookie ) );
                                },
                                [ & ]( auto&& ) -> void {
                                    // TODO: Write message
                                    static_assert( false );
                                },
                            } );

                        stdfunc::assert( l_result );
                    }

                    const size_t l_eventSize =
                        ( sizeof( inotify_event ) + l_event->len );

                    l_eventPointer += l_eventSize;
                }
            }
        }
    }
}

} // namespace watch
