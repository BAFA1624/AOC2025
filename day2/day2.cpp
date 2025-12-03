#include "constants.hpp"
#include "files.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>

enum class Question { One, Two };

template <Question Q>
class Range
{
    private:
    std::uint64_t m_first;
    std::uint64_t m_last;
    bool          m_valid_range;

    static constexpr auto num_digits( std::uint64_t id ) {
        std::uint64_t digits{ 0 };
        while ( id != 0 ) {
            digits++;
            id /= 10;
        }
        return digits;
    }

    static constexpr auto id_subrange( const std::uint64_t id,
                                       const std::uint64_t left,
                                       const std::uint64_t right ) {
        const auto n_digits{ num_digits( id ) };

        assert( left <= right );
        assert( right <= n_digits );

        const auto right_shift_divisor{ static_cast<std::uint64_t>(
            std::pow( 10, n_digits - right ) ) };
        const auto shifted_remainder{ static_cast<std::uint64_t>(
            std::pow( 10, right - left ) ) };

        // Remove rightmost unnecessary digits
        const auto shifted_id{ id / right_shift_divisor };

        // Retrieve desired digits
        return shifted_id % shifted_remainder;
    }

    static constexpr auto id_chunks( const std::uint64_t id,
                                     const std::uint64_t chunk_size ) {
        const auto n_digits{ num_digits( id ) };
        assert( n_digits % chunk_size == 0 );

        std::vector<std::uint64_t> chunks( n_digits / chunk_size );

        for ( std::uint64_t i{ 0 }; i < n_digits / chunk_size; i++ ) {
            chunks[i] =
                id_subrange( id, i * chunk_size, ( i + 1 ) * chunk_size );
        }

        return chunks;
    }

    static constexpr auto valid_id( const std::uint64_t id )
        requires( Q == Question::One )
    {
        const auto n_digits{ num_digits( id ) };
        if ( n_digits % 2 != 0 )
            return true;

        constexpr auto first_half_digits = []( const auto id,
                                               const auto n_digits ) {
            return id
                   / static_cast<std::uint64_t>( std::pow( 10, n_digits / 2 ) );
        };
        constexpr auto last_half_digits = []( const auto id,
                                              const auto n_digits ) {
            return id
                   % static_cast<std::uint64_t>( std::pow( 10, n_digits / 2 ) );
        };

        const auto first_half{ first_half_digits( id, n_digits ) };
        const auto last_half{ last_half_digits( id, n_digits ) };

        return first_half != last_half;
    }

    static constexpr auto valid_id( const std::uint64_t id )
        requires( Q == Question::Two )
    {
        const auto n_digits{ num_digits( id ) };

        // Determine valid divisors
        const auto divisors{ std::ranges::iota_view( std::uint64_t{ 2 },
                                                     n_digits + 1 )
                             | std::views::filter( [&n_digits]( const auto n ) {
                                   return n_digits % n == 0;
                               } )
                             | std::ranges::to<std::vector<std::uint64_t>>() };

        // Check all chunks of divisor size for matching patterns
        for ( const auto divisor : divisors ) {
            const auto chunks{ id_chunks( id, n_digits / divisor ) };

            const auto invalid_id{ ( chunks | std::views::slide( 2 )
                                     | std::views::drop_while(
                                         []( const auto & window ) {
                                             return window[0] == window[1];
                                         } ) )
                                       .empty() };

            if ( invalid_id )
                return false;
        }

        return true;
    }

    public:
    Range() = delete;
    constexpr Range( const std::uint64_t first, const std::uint64_t last ) :
        m_first( first ), m_last( last ), m_valid_range( m_first < m_last ) {}
    Range( const std::string_view range ) {
        const std::regex pattern{ "([0-9]+)-([0-9]+)" };
        std::smatch      results;

        const std::string tmp{ range.data(), range.size() };
        m_valid_range =
            std::regex_match( tmp.cbegin(), tmp.cend(), results, pattern );

        if ( m_valid_range ) {
            m_first = static_cast<std::uint64_t>( std::stoul( results[1] ) );
            m_last = static_cast<std::uint64_t>( std::stoul( results[2] ) );
            m_valid_range &= m_first < m_last;
        }
    }

    constexpr Range( const Range & ) = default;
    constexpr Range( Range && ) noexcept = default;

    constexpr Range & operator=( const Range & ) = default;
    constexpr Range & operator=( Range && ) noexcept = default;

    ~Range() = default;

    constexpr auto first() const noexcept { return m_first; }
    constexpr auto last() const noexcept { return m_last; }

    constexpr auto is_valid() const noexcept { return m_valid_range; }

    constexpr auto ids() const noexcept {
        if ( !m_valid_range ) {
            return std::vector<std::uint64_t>{};
        }

        return std::ranges::iota_view{ m_first, m_last + 1 }
               | std::ranges::to<std::vector<std::uint64_t>>();
    }
    constexpr auto valid_ids() const noexcept {
        if ( !m_valid_range ) {
            return std::vector<std::uint64_t>{};
        }

        if constexpr ( Q == Question::One ) {
            // Early escape without filtering
            const auto n_digits_first{ num_digits( m_first ) };
            const auto n_digits_last{ num_digits( m_last ) };
            if ( n_digits_first == n_digits_last && n_digits_first % 2 != 0 ) {
                return std::ranges::iota_view{ m_first, m_last + 1 }
                       | std::ranges::to<std::vector<std::uint64_t>>();
            }
        }

        // Filtering numbers
        return std::ranges::iota_view{ m_first, m_last + 1 }
               | std::views::filter( valid_id )
               | std::ranges::to<std::vector<std::uint64_t>>();
    }
    constexpr auto invalid_ids() const noexcept {
        if ( !m_valid_range ) {
            return std::vector<std::uint64_t>{};
        }

        return std::ranges::iota_view{ m_first, m_last + 1 }
               | std::views::filter(
                   []( const auto id ) { return !valid_id( id ); } )
               | std::ranges::to<std::vector<std::uint64_t>>();
    }
};

template <Question Q>
std::ostream &
operator<<( std::ostream & os, const Range<Q> & rng ) {
    const auto ids{ rng.ids() };

    os << "Range{{ ";
    if ( ids.size() < 31 ) {
        for ( const auto id : ids ) { os << std::format( "{}, ", id ); }
    }
    else {
        os << std::format( "{} - {} ",
                           *std::min( ids.cbegin(), ids.cend() ),
                           *std::max( ids.cbegin(), ids.cend() ) );
    }
    return os << "}}";
}

constexpr auto
problem_1( const auto & inputs ) {
    // Construct ranges
    const auto ranges{ inputs | std::views::transform( []( const auto rng ) {
                           return Range<Question::One>{ rng };
                       } )
                       | std::ranges::to<std::vector<Range<Question::One>>>() };

    const auto sum{ std::accumulate(
        ranges.cbegin(),
        ranges.cend(),
        std::uint64_t{ 0 },
        []( auto sum, const auto & rng ) {
            const auto & invalid_ids{ rng.invalid_ids() };
            const auto sub_sum{ std::accumulate( invalid_ids.cbegin(),
                                                 invalid_ids.cend(),
                                                 std::uint64_t{ 0 } ) };
            return sum += sub_sum;
        } ) };

    std::println( "Problem One | Sum: {}", sum );
}

constexpr auto
problem_2( const auto & inputs ) {
    // Construct ranges
    const auto ranges{ inputs | std::views::transform( []( const auto rng ) {
                           return Range<Question::Two>{ rng };
                       } )
                       | std::ranges::to<std::vector<Range<Question::Two>>>() };

    const auto sum{ std::accumulate(
        ranges.cbegin(),
        ranges.cend(),
        std::uint64_t{ 0 },
        []( auto sum, const auto & rng ) {
            const auto & invalid_ids{ rng.invalid_ids() };
            const auto sub_sum{ std::accumulate( invalid_ids.cbegin(),
                                                 invalid_ids.cend(),
                                                 std::uint64_t{ 0 } ) };
            return sum += sub_sum;
        } ) };

    std::println( "Problem Two | Sum: {}", sum );
}



int
main() {
    // Read data
    const auto raw_input{ get_input_file( 2 ) };

    // Split into text ranges
    const auto inputs{ split_input( raw_input, "," ) };

    // Problem 1
    problem_1( inputs );

    // Problem 2
    problem_2( inputs );
}
