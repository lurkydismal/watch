#include "example.hpp"

#include "test.hpp"

TEST( example, AddFunction ) {
    EXPECT_EQ( example::add( 2, 3, 4, 5, 6 ), 20 );
}
