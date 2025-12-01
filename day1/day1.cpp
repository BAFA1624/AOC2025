#include "constants.hpp"
#include "files.hpp"

#include <cstdint>
#include <regex>
#include <string_view>

/*
 * Dial on safe, numbered 0-99 in order.
 * Input:
 *  - Sequence of rotations, e.g: L3, R96, ...
 *  - Pattern: XY.
 *  - X: Letter indicating L -> left, R -> right.
 *  - Y: Number indicating length of rotation.
 *  - E.g. If dial is at 11, 11 + R8 -> 19, 19 + L19 -> 0.
 *  - Dial is circular -> numbers wrap both ways.
 *  - Dial starts at 50.
 *  - The real password is the no. of times the dial is left pointing
 *    at 0 after any rotation in the sequence.
 */

class Dial
{
    private:
    std::uint32_t m_zero_count{ 0 };
    std::uint32_t m_passes_zero_count{ 0 };
    std::uint32_t m_position{ 50 };

    struct Transform
    {
        bool          is_valid;
        bool          direction;
        std::uint32_t size;
    };
    void print_transform( const Transform & transform ) {
        std::println(
            "Transform {{\n\tis_valid: {},\n\tdirection: {},\n\tsize: {}\n}}",
            transform.is_valid,
            ( transform.direction ? "+" : "-" ),
            transform.size );
    }

    constexpr auto
    is_valid_transform( const std::string_view transform ) const noexcept {
        std::regex valid_transform_regex{ "^([LR])([0-9]+)$" };

        const std::string data{ transform.data(), transform.size() };

        std::smatch result{};
        if ( std::regex_match(
                 data.cbegin(), data.cend(), result, valid_transform_regex ) ) {
            return Transform{ true,
                              result[1] == std::string{ "R" },
                              static_cast<std::uint32_t>(
                                  std::stoul( result[2] ) ) };
        }
        else {
            return Transform{ false, false, 0 };
        }
    };

    constexpr auto passes_zero( const auto & transform ) {
        return transform.size
               >= ( transform.direction ? 100 - m_position : m_position );
    }

    constexpr std::uint32_t zero_passes( const auto & transform ) {
        if ( !passes_zero( transform ) )
            return 0;

        // std::println();
        // std::println( "Counting zero passes..." );
        // std::println( "current position: {}", m_position );
        // print_transform( transform );

        const std::uint32_t divisor{ transform.size / 100 };
        const std::uint32_t remainder{ transform.size % 100 };
        const std::uint32_t passes{
            divisor
            + passes_zero( Transform(
                transform.is_valid, transform.direction, remainder ) )
            - ( m_position == 0 && !transform.direction )
        };
        // std::println( "divisor: {}, remainder: {}, passes: {}",
        //               divisor,
        //               remainder,
        //               passes );
        // std::println();

        return passes;
    }

    public:
    constexpr Dial() = default;
    constexpr ~Dial() = default;
    constexpr Dial( const Dial & ) = default;
    constexpr Dial( Dial && ) noexcept = default;
    constexpr Dial & operator=( const Dial & ) = default;
    constexpr Dial & operator=( Dial && ) noexcept = default;

    constexpr auto transform( const std::string_view raw_transform ) noexcept {
        auto transform = is_valid_transform( raw_transform );
        if ( transform.is_valid ) {
            m_passes_zero_count += zero_passes( transform );

            m_position +=
                ( transform.direction ? transform.size % 100 :
                                        100 - ( transform.size % 100 ) );
            m_position %= 100;

            if ( m_position == 0 )
                m_zero_count++;
        }
        return m_position;
    }

    constexpr auto
    transform( const std::vector<std::string_view> & raw_transforms ) noexcept {
        for ( const auto & raw_transform : raw_transforms ) {
            transform( raw_transform );
        }
        return m_position;
    }
    constexpr explicit Dial(
        const std::vector<std::string_view> & raw_transforms ) {
        [[maybe_unused]] const auto position{ transform( raw_transforms ) };
    }

    constexpr auto is_zero() const noexcept { return m_position == 0; }
    constexpr auto zero_count() const noexcept { return m_zero_count; }
    constexpr auto passes_zero_count() const noexcept {
        return m_passes_zero_count;
    }
    constexpr auto position() const noexcept { return m_position; }
    constexpr void reset() noexcept {
        m_position = 50;
        m_zero_count = 0;
        m_passes_zero_count = 0;
    }

    void position( const std::uint32_t position ) { m_position = position; }
    void zero_count( const std::uint32_t zero_count ) {
        m_zero_count = zero_count;
    }
    void passes_zero_count( const std::uint32_t passes_zero_count ) {
        m_passes_zero_count = passes_zero_count;
    }
};

constexpr bool
verify_underflow() {
    Dial dial{};

    std::println( "verify_underflow | initial position: {}", dial.position() );
    dial.transform( "L50" );
    std::println(
        "verify_underflow | final position: {}, expected position: {}",
        dial.position(),
        0 );

    std::println( "verify_underflow | initial position: {}", dial.position() );
    dial.transform( "L5" );
    std::println(
        "verify_underflow | final position: {}, expected position: {}",
        dial.position(),
        95 );

    return dial.position() == 95;
}

constexpr bool
verify_overflow() {
    Dial dial{};

    std::println( "verify_overflow | initial position: {}", dial.position() );
    dial.transform( "R50" );
    std::println( "verify_overflow | final position: {}, expected position: {}",
                  dial.position(),
                  0 );

    std::println( "verify_overflow | initial position: {}", dial.position() );
    dial.transform( "R5" );
    std::println( "verify_overflow | final position: {}, expected position: {}",
                  dial.position(),
                  5 );

    return dial.position() == 5;
}

constexpr bool
verify_large_overflow() {
    Dial dial{};

    std::println( "verify_large_overflow | initial position: {}",
                  dial.position() );
    dial.transform( "R899" );
    std::println(
        "verify_large_overflow | final position: {}, expected position: {}",
        dial.position(),
        49 );

    return dial.position() == 49;
}

constexpr bool
verify_large_underflow() {
    Dial dial{};

    std::println( "verify_large_underflow | initial position: {}",
                  dial.position() );
    dial.transform( "L899" );
    std::println(
        "verify_large_underflow | final position {}, expected position: {}",
        dial.position(),
        51 );

    return dial.position() == 51;
}

constexpr bool
verify_underflow_count() {
    Dial dial{};
    dial.position( 0 );

    dial.transform( "L469" );

    return dial.passes_zero_count() == 4;
}

constexpr bool
verify_overflow_count() {
    Dial dial{};
    dial.position( 0 );

    dial.transform( "R469" );

    return dial.passes_zero_count() == 4;
}

constexpr bool
verify() {
    const std::vector<std::string_view> transforms{ "L68", "L30", "R48", "L5",
                                                    "R60", "L55", "L1",  "L99",
                                                    "R14", "L82" };
    const std::vector<std::uint32_t>    expected_positions{ 82, 52, 0, 95, 55,
                                                         0,  99, 0, 14, 32 };

    Dial dial{};

    bool result{ true };
    for ( const auto & [transform, expected_pos] :
          std::views::zip( transforms, expected_positions ) ) {
        std::println(
            "Dial position: {}, Transform: {} ->", dial.position(), transform );
        dial.transform( transform );
        std::println( "    Dial position: {}, Expected position: {}",
                      dial.position(),
                      expected_pos );

        result &= ( dial.position() == expected_pos );
    }

    std::println(
        "zero count: {}, expected zero count: {}", dial.zero_count(), 3 );

    return result && ( dial.zero_count() == 3 );
}

Dial
problem_1( const std::vector<std::string_view> & lines ) {
    Dial dial{ lines };
    std::println( "zero_count: {}", dial.zero_count() );
    return dial;
}
Dial
problem_2( const Dial & dial ) {
    std::println( "passes_zero_count: {}", dial.passes_zero_count() );
    return dial;
}

void
test() {
    Dial dial{};

    std::println();
    for ( int i{ 0 }; i < 100; ++i ) {
        std::print( "{} -> ", dial.position() );
        dial.transform( "L1" );
        std::println( "{}", dial.position() );
    }
    std::println();
}

int
main() {
    // if ( !verify_underflow() ) {
    //     std::println( "Underflow errors detected." );
    //     return 0;
    // }
    // if ( !verify_large_underflow() ) {
    //     std::println( "Large underflow errors detected." );
    //     return 0;
    // }
    // if ( !verify_overflow() ) {
    //     std::println( "Overflow errors detected." );
    //     return 0;
    // }
    // if ( !verify_large_overflow() ) {
    //     std::println( "Large overflow errors detected." );
    //     return 0;
    // }
    // if ( !verify() ) {
    //     std::println( "Errors detected." );
    //     return 0;
    // }

    // test();

    if ( !verify_underflow_count() ) {
        std::println( "Underflow counting errors." );
        return 0;
    }
    if ( !verify_overflow_count() ) {
        std::println( "Overflow counting errors." );
        return 0;
    }

    const auto input_file{ get_input_file( 1 ) };

    const auto lines{ file_lines( input_file ) };

    auto dial{ problem_1( lines ) };
    dial = problem_2( dial );
}
