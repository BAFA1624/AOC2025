#include "constants.hpp"
#include "files.hpp"

#include <algorithm>
#include <bitset>
#include <cassert>
#include <map>

/*
 * Plan:
 *  - Map class -> Keeps track of shape of the map, height, width,
 *    stores the map as well.
 *     - Has (i, j) accessors for the map input to check if that
 *       location is paper or not, etc.
 *     - Performs automatic bounds checking on inputs.
 *     - Stores a count of the no. of accessible paper rolls,
 *       and automatically calculates it at construction.
 */

enum class ObjType : std::uint8_t {
    NONE = 0,
    PAPER = 1,
    ACCESSIBLE_PAPER = 2,
    INVALID = 3
};

template <>
struct std::formatter<ObjType, char>
{
    template <class FmtContext>
    FmtContext::iterator format( const ObjType type, FmtContext & ctx ) const {
        std::ostringstream out;

        switch ( type ) {
        case ObjType::NONE: out << '.'; break;
        case ObjType::PAPER: out << '@'; break;
        case ObjType::ACCESSIBLE_PAPER: out << 'X'; break;
        case ObjType::INVALID: out << '!'; break;
        }

        return std::ranges::copy( std::move( out ).str(), ctx.out() ).out;
    }
};



class Map
{
    private:
    std::uint32_t        m_width;
    std::uint32_t        m_height;
    std::vector<ObjType> m_map;
    std::vector<ObjType> m_accessible_map;
    std::uint32_t        m_accessible_paper;

    static constexpr std::pair<std::uint32_t, std::uint32_t>
    measure_dimensions( const std::string_view unprocessed_map ) {
        const bool last_char_newline{ unprocessed_map.back() == '\n' };

        const auto map_view = std::views::all( unprocessed_map );

        const auto height{ std::ranges::fold_left(
                               map_view
                                   | std::views::filter( []( const char c ) {
                                         return std::isspace( c );
                                     } ),
                               std::uint32_t{ 0 },
                               []( auto sum, [[maybe_unused]] const auto c ) {
                                   return ++sum;
                               } )
                           + !last_char_newline };
        assert( height > 0 && "Map height must not be 0." );

        const auto line_lengths{
            map_view | std::views::split( '\n' )
            | std::views::filter( []( const auto & rng ) {
                  return std::ranges::distance( rng ) != 0;
              } )
            | std::views::transform( []( const auto & rng ) {
                  return std::ranges::distance( rng );
              } )
            | std::ranges::to<std::vector<std::uint32_t>>()
        };

        // Line lengths cannot be empty
        assert( !line_lengths.empty() && "Map data cannot be empty." );
        // Check line lengths are constant
        assert( std::ranges::all_of(
                    line_lengths
                        | std::views::pairwise_transform(
                            []( const auto left, const auto right ) {
                                return left == right;
                            } )
                        | std::ranges::to<std::vector<bool>>(),
                    []( const bool result ) { return result == true; } )
                && "Map line lengths must be constant." );

        return std::pair{ line_lengths.front(), height };
    }

    static constexpr auto initialise_map( const std::uint32_t    width,
                                          const std::uint32_t    height,
                                          const std::string_view map_data ) {
        const auto map = std::views::all( map_data )
                         | std::views::filter(
                             []( const auto c ) { return !std::isspace( c ); } )
                         | std::views::transform( []( const auto c ) {
                               switch ( c ) {
                               case '.': return ObjType::NONE; break;
                               case '@': return ObjType::PAPER; break;
                               default: return ObjType::INVALID; break;
                               }
                           } )
                         | std::ranges::to<std::vector<ObjType>>();

        assert( map.size() == width * height
                && "Map dimensions must match map data." );
        return map;
    }

    public:
    [[nodiscard]] constexpr auto   width() const noexcept { return m_width; }
    [[nodiscard]] constexpr auto   height() const noexcept { return m_height; }
    [[nodiscard]] constexpr auto & map() noexcept { return m_map; }
    [[nodiscard]] constexpr auto & map() const noexcept { return m_map; }
    [[nodiscard]] constexpr auto & accessible_map() noexcept {
        return m_accessible_map;
    }
    [[nodiscard]] constexpr auto & accessible_map() const noexcept {
        return m_accessible_map;
    }
    [[nodiscard]] constexpr auto accessible_paper() const noexcept {
        return m_accessible_paper;
    }

    [[nodiscard]] constexpr auto &
    operator[]( const std::uint32_t i, const std::uint32_t j ) noexcept {
        return m_map[j * m_width + i];
    }
    [[nodiscard]] constexpr auto
    operator[]( const std::uint32_t i, const std::uint32_t j ) const noexcept {
        return m_map[j * m_width + i];
    }

    [[nodiscard]] constexpr auto & at( const std::uint32_t i,
                                       const std::uint32_t j ) {
        if ( i >= m_width )
            throw std::out_of_range(
                "[i >= m_width]: i must be less than map width." );
        if ( j >= m_height )
            throw std::out_of_range(
                "[j >= m_height]: j must be less than map height." );

        return m_map.at( j * m_width + i );
    }
    [[nodiscard]] constexpr auto & at( const std::uint32_t i,
                                       const std::uint32_t j ) const {
        if ( i >= m_width )
            throw std::out_of_range(
                "[i >= m_width]: i must be less than map width." );
        if ( j >= m_height )
            throw std::out_of_range(
                "[j >= m_height]: j must be less than map height." );

        return m_map.at( j * m_width + i );
    }

    [[nodiscard]] constexpr auto
    is_paper( const std::uint32_t i, const std::uint32_t j ) const noexcept {
        return m_map[i, j] == ObjType::PAPER;
    }

    [[nodiscard]] constexpr auto
    is_accessible_paper( const std::uint32_t i, const std::uint32_t j ) const {
        // Gain bounds check from at(i, j) rather than use is_paper(i, j)
        if ( at( i, j ) != ObjType::PAPER )
            return false;


        // 8 Positions to consider:
        //  0 1 2
        //  3 X 4
        //  5 6 7
        //  - 0: (i - 1, j - 1)
        //  - 1: (i, j - 1)
        //  - 2: (i + 1, j - 1)
        //  - 3: (i - 1, j)
        //  - 4: (i + 1, j)
        //  - 5: (i - 1, j + 1)
        //  - 6: (i, j + 1)
        //  - 7: (i + 1, j + 1)

        std::bitset<8> valid_position_mask{ 11111111 };

        // Border checks, pre-flag OOB indices as false
        // i position checks
        const auto max_i{ m_width - 1 };
        if ( i == 0 ) {
            valid_position_mask[0] = false;
            valid_position_mask[3] = false;
            valid_position_mask[5] = false;
        }
        else if ( i == max_i ) {
            valid_position_mask[2] = false;
            valid_position_mask[4] = false;
            valid_position_mask[7] = false;
        }
        // j position checks
        const auto max_j{ m_height - 1 };
        if ( j == 0 ) {
            valid_position_mask[0] = false;
            valid_position_mask[1] = false;
            valid_position_mask[2] = false;
        }
        else if ( j == max_j ) {
            valid_position_mask[5] = false;
            valid_position_mask[6] = false;
            valid_position_mask[7] = false;
        }

        valid_position_mask[0] =
            valid_position_mask[0] ? is_paper( i - 1, j - 1 ) : false;
        valid_position_mask[1] =
            valid_position_mask[1] ? is_paper( i, j - 1 ) : false;
        valid_position_mask[2] =
            valid_position_mask[2] ? is_paper( i + 1, j - 1 ) : false;
        valid_position_mask[3] =
            valid_position_mask[3] ? is_paper( i - 1, j ) : false;
        valid_position_mask[4] =
            valid_position_mask[4] ? is_paper( i + 1, j ) : false;
        valid_position_mask[5] =
            valid_position_mask[5] ? is_paper( i - 1, j + 1 ) : false;
        valid_position_mask[6] =
            valid_position_mask[6] ? is_paper( i, j + 1 ) : false;
        valid_position_mask[7] =
            valid_position_mask[7] ? is_paper( i + 1, j + 1 ) : false;

        return valid_position_mask.count() < 4;
    }

    private:
    constexpr auto process_map() {
        std::vector<ObjType> accessible_map;
        accessible_map.resize( m_map.size() );

        for ( std::uint32_t i{ 0 }; i < m_width; ++i ) {
            for ( std::uint32_t j{ 0 }; j < m_height; ++j ) {
                accessible_map[j * m_width + i] =
                    is_accessible_paper( i, j ) ? ObjType::ACCESSIBLE_PAPER :
                                                  m_map[i, j];
            }
        }

        return accessible_map;
    }

    public:
    constexpr Map() = delete;
    constexpr Map( const std::string_view map_data ) {
        const auto dimensions{ measure_dimensions( map_data ) };
        m_width = dimensions.first;
        m_height = dimensions.second;
        m_map = initialise_map( m_width, m_height, map_data );
        m_accessible_map = process_map();
        m_accessible_paper = std::ranges::fold_left(
            m_accessible_map,
            std::uint32_t{ 0 },
            []( const std::uint32_t sum, const ObjType type ) {
                return sum + ( type == ObjType::ACCESSIBLE_PAPER );
            } );
    }
    constexpr Map( const std::uint32_t width, const std::uint32_t height,
                   const std::string_view map_data ) :
        m_width( width ),
        m_height( height ),
        m_map( initialise_map( m_width, m_height, map_data ) ),
        m_accessible_map( process_map() ),
        m_accessible_paper( std::ranges::fold_left(
            m_accessible_map, std::uint32_t{ 0 },
            []( const std::uint32_t sum, const ObjType type ) {
                return sum + ( type == ObjType::ACCESSIBLE_PAPER );
            } ) ) {}

    constexpr Map( const Map & ) = default;
    constexpr Map( Map && ) noexcept = default;

    constexpr Map & operator=( const Map & ) = default;
    constexpr Map & operator=( Map && ) noexcept = default;

    constexpr ~Map() = default;
};

// template <>
// struct std::formatter<Map, char>
//{
//     template <class FmtContext>
//     FmtContext::iterator format( const Map & data, FmtContext & ctx ) const {
//         std::ostringstream out;
//
//         for ( std::uint32_t i{ 0 }; i < size; ++i ) {}
//     }
// };

std::ostream &
operator<<( std::ostream & os, const Map & map ) {
    const auto & underlying_map{ map.accessible_map() };
    // std::println( "{}", underlying_map );
    const auto width{ map.width() };
    const auto size{ width * map.height() };

    std::println(
        "map.size(): {}, accessible_map.size(): {}, width: {}, height: {}, "
        "size: {}",
        map.map().size(),
        map.accessible_map().size(),
        map.width(),
        map.height(),
        map.width() * map.height() );

    static const std::map<ObjType, char> ObjType_character_map{
        { ObjType::NONE, '.' },
        { ObjType::PAPER, '@' },
        { ObjType::ACCESSIBLE_PAPER, 'X' },
        { ObjType::INVALID, '!' }
    };

    os << std::format( "Map( {}, {}) {{\n", width, map.height() );
    for ( std::uint32_t i{ 0 }; i < size; ++i ) {
        switch ( i % width + ( i == 0 ) /* Indexing from 0 */ ) {
        case 0: os << "\n"; [[fallthrough]];
        default: os << ObjType_character_map.at( underlying_map[i] );
        }
    }
    os << "\n}";

    return os;
}

/*
 * @ -> Roll of paper
 * Can only be accessed if there are <4 rolls of paper
 * in the 8 adjacent positions.
 */

constexpr std::string_view test_input{
    "..@@.@@@@.\n"
    "@@@.@.@.@@\n"
    "@@@@@.@.@@\n"
    "@.@@@@..@.\n"
    "@@.@@@@.@@\n"
    ".@@@@@@@.@\n"
    ".@.@.@.@@@\n"
    "@.@@@.@@@@\n"
    ".@@@@@@@@.\n"
    "@.@.@@@.@.\n"
};

// Diagram showing which of the paper rolls in test_input
// are "accessible"
constexpr std::string_view test_accessible_paper_rolls{
    "..xx.xx@x.\n"
    "x@@.@.@.@@\n"
    "@@@@@.x.@@\n"
    "@.@@@@..@.\n"
    "x@.@@@@.@x\n"
    ".@@@@@@@.@\n"
    ".@.@.@.@@@\n"
    "x.@@@.@@@@\n"
    ".@@@@@@@@.\n"
    "x.x.@@@.x.\n"
};

constexpr std::uint32_t test_result_1{ 13 };

// Testing for problem_1
constexpr auto
test_problem_1() {
    // Map test{ 10, 10, test_input };
    const Map test{ test_input };

    std::cout << test << std::endl;
    std::println( "Accessible Paper: {}", test.accessible_paper() );
    assert( test.accessible_paper() == test_result_1 );
}

// Problem 1: How many of the paper rolls are accessible?
constexpr auto
problem_1( const std::string_view input ) {
    const Map map{ input };
    std::println( "Accessible Paper: {}", map.accessible_paper() );
}

// Problem 2:

int
main() {
    test_problem_1();

    const auto input{ get_input_file( 4 ) };
    problem_1( input );
}
