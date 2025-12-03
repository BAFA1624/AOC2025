#include "constants.hpp"
#include "files.hpp"

#include <cassert>
#include <cmath>
#include <functional>
#include <numeric>

template <unsigned long long N>
class Bank
{
    private:
    unsigned long long m_joltage;

    static constexpr auto
    process_joltages( const std::vector<unsigned long long> joltages ) {
        assert( N < joltages.size() );

        // Calculate search end position so all N numbers can fit
        auto end_pos{ joltages.size() - N + 1 };

        // Find first max number
        auto first_it{ std::max_element(
            joltages.cbegin(), std::next( joltages.cbegin(), end_pos ) ) };

        bool               first_time{ true };
        unsigned long long joltage{ 0 };
        while ( first_it != joltages.cend()
                && ( std::next( joltages.cbegin(), end_pos ) != joltages.cend()
                     || first_time ) ) {
            assert( first_it != std::next( joltages.cbegin(), end_pos ) );

            first_time = false;

            joltage += *first_it
                       * static_cast<unsigned long long>(
                           std::pow( 10, joltages.size() - end_pos ) );

            // Find next max number
            first_it =
                std::max_element( std::next( first_it ),
                                  std::next( joltages.cbegin(), ++end_pos ) );
        }

        joltage += *first_it;

        return joltage;
    }

    public:
    constexpr Bank() = delete;
    constexpr explicit Bank( const std::string_view unprocessed_input ) {
        const auto joltages =
            std::views::all( unprocessed_input )
            | std::views::transform( []( const auto character ) {
                  return static_cast<unsigned long long>( character )
                         - static_cast<unsigned long long>( '0' );
              } )
            | std::ranges::to<std::vector<unsigned long long>>();

        m_joltage = process_joltages( joltages );
    };
    constexpr explicit Bank(
        const std::vector<unsigned long long> & joltages ) :
        m_joltage( process_joltages( joltage ) ) {}

    constexpr Bank( const Bank & bank ) = default;
    constexpr Bank( Bank && bank ) noexcept = default;

    constexpr Bank & operator=( const Bank & bank ) = default;
    constexpr Bank & operator=( Bank && bank ) = default;

    constexpr ~Bank() = default;

    [[nodiscard]] constexpr auto joltage() const noexcept { return m_joltage; }
};

template <unsigned long long N>
class Battery
{
    private:
    std::vector<Bank<N>> m_banks;

    public:
    constexpr Battery() = delete;
    constexpr Battery(
        const std::vector<std::string_view> & unprocessed_input ) :
        m_banks( unprocessed_input
                 | std::views::transform(
                     []( const auto view ) { return Bank<N>{ view }; } )
                 | std::ranges::to<std::vector<Bank<N>>>() ) {};
    constexpr Battery(
        const std::vector<std::vector<unsigned long long>> & joltage_banks ) :
        m_banks( joltage_banks
                 | std::views::transform(
                     []( const auto & bank ) { return Bank{ bank }; } )
                 | std::ranges::to<std::vector<Bank<N>>>() ) {};

    constexpr Battery( const Battery & battery ) = default;
    constexpr Battery( Battery && battery ) = default;

    constexpr Battery & operator=( const Battery & battery ) = default;
    constexpr Battery & operator=( Battery && battery ) noexcept = default;

    constexpr ~Battery() = default;

    [[nodiscard]] constexpr auto banks() const noexcept { return m_banks; }
    [[nodiscard]] constexpr auto joltage() const noexcept {
        return std::accumulate( m_banks.cbegin(),
                                m_banks.cend(),
                                static_cast<unsigned long long>( 0 ),
                                []( const auto sum, const auto & bank ) {
                                    return sum + bank.joltage();
                                } );
    }
};

static const std::string_view test_input{
    "987654321111111\n811111111111119\n234234234234278\n818181911112111"
};

static const std::vector<unsigned long long> test_bank_joltages_1{
    98, 89, 78, 92
};
static const unsigned long long test_joltage_sum_1{ 357 };

static const std::vector<unsigned long long> test_bank_joltages_2{
    987654321111, 811111111119, 434234234278, 888911112111
};
static const unsigned long long test_joltage_sum_2{ 3121910778619 };

constexpr void
test_function_1() {
    const Battery<2> test( std::views::all( test_input )
                           | std::views::split( '\n' )
                           | std::views::transform( []( const auto & x ) {
                                 return std::string_view{ x.data(), x.size() };
                             } )
                           | std::ranges::to<std::vector<std::string_view>>() );

    const auto bank_results{
        std::views::zip_transform(
            []( const auto bank_joltage, const auto expected_joltage ) {
                return bank_joltage == expected_joltage;
            },
            test.banks() | std::views::transform( []( const auto & bank ) {
                return bank.joltage();
            } ) | std::ranges::to<std::vector<unsigned long long>>(),
            test_bank_joltages_1 )
        | std::ranges::to<std::vector<bool>>()
    };

    std::println(
        "bank_results:     {}\nexpected_results: {}",
        test.banks() | std::views::transform( []( const auto & bank ) {
            return bank.joltage();
        } ) | std::ranges::to<std::vector<unsigned long long>>(),
        test_bank_joltages_1 );
    std::println( "Total Joltage: {}\nExpected Total Joltage: {}",
                  test.joltage(),
                  test_joltage_sum_1 );

    assert( std::all_of( bank_results.cbegin(),
                         bank_results.cend(),
                         []( const auto result ) { return result == true; } ) );
    assert( test.joltage() == test_joltage_sum_1 );
}

constexpr void
test_function_2() {
    const Battery<12> test(
        std::views::all( test_input ) | std::views::split( '\n' )
        | std::views::transform( []( const auto & x ) {
              return std::string_view{ x.data(), x.size() };
          } )
        | std::ranges::to<std::vector<std::string_view>>() );

    const auto bank_results{
        std::views::zip_transform(
            []( const auto bank_joltage, const auto expected_joltage ) {
                return bank_joltage == expected_joltage;
            },
            test.banks() | std::views::transform( []( const auto & bank ) {
                return bank.joltage();
            } ) | std::ranges::to<std::vector<unsigned long long>>(),
            test_bank_joltages_2 )
        | std::ranges::to<std::vector<bool>>()
    };

    std::println(
        "bank_results:     {}\nexpected_results: {}",
        test.banks() | std::views::transform( []( const auto & bank ) {
            return bank.joltage();
        } ) | std::ranges::to<std::vector<unsigned long long>>(),
        test_bank_joltages_2 );
    std::println( "Total Joltage: {}\nExpected Total Joltage: {}",
                  test.joltage(),
                  test_joltage_sum_2 );

    assert( std::all_of( bank_results.cbegin(),
                         bank_results.cend(),
                         []( const auto result ) { return result == true; } ) );
    assert( test.joltage() == test_joltage_sum_2 );
}

constexpr auto
problem_1( const auto input ) {
    Battery<2> battery{ std::views::all( input ) | std::views::split( '\n' )
                        | std::views::transform( []( const auto & x ) {
                              return std::string_view{ x.data(), x.size() };
                          } )
                        | std::views::filter(
                            []( const auto view ) { return !view.empty(); } )
                        | std::ranges::to<std::vector<std::string_view>>() };
    std::println( "Battery joltage: {}", battery.joltage() );
}

constexpr auto
problem_2( const auto input ) {
    Battery<12> battery{ std::views::all( input ) | std::views::split( '\n' )
                         | std::views::transform( []( const auto & x ) {
                               return std::string_view{ x.data(), x.size() };
                           } )
                         | std::views::filter(
                             []( const auto view ) { return !view.empty(); } )
                         | std::ranges::to<std::vector<std::string_view>>() };
    std::println( "Battery joltage: {}", battery.joltage() );
}

int
main() {
    const auto input{ get_input_file( 3 ) };
    test_function_1();
    problem_1( input );
    test_function_2();
    problem_2( input );
}
