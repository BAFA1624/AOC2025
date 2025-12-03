#pragma once

#include "constants.hpp"

#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <print>
#include <ranges>
#include <regex>
#include <sstream>
#include <string_view>
#include <vector>

constexpr std::string
read_file( const std::filesystem::directory_entry & file_obj ) {
    // Check file exists
    if ( !file_obj.exists() ) {
        std::cerr << std::format( "File {} does not exist.",
                                  file_obj.path().string() )
                  << std::endl;
        return "";
    }

    // Open file
    std::ifstream file_stream( file_obj.path() );
    if ( !file_stream.is_open() ) {
        std::cerr << std::format( "Unable to open file {}.",
                                  file_obj.path().string() )
                  << std::endl;
        return "";
    }

    // Get length of file
    file_stream.seekg( 0, std::ios_base::end );
    const auto file_size{ static_cast<std::size_t>( file_stream.tellg() ) };
    file_stream.seekg( std::ios_base::beg );
    file_stream.clear();

    // Initialise string to store file
    std::string result( file_size, '\0' );
    file_stream.read( result.data(), result.size() );

    return result;
}

constexpr std::string
get_input_file( const std::uint32_t day_no ) {
    const auto input_file_path{
        project_root / ( "day" + std::to_string( day_no ) ) / "input.txt"
    };
    return read_file( std::filesystem::directory_entry( input_file_path ) );
}

constexpr std::vector<std::string_view>
split_input( const std::string_view file,
             const std::string_view delim = "\n" ) {
    return file | std::views::split( delim )
           | std::views::transform( []( auto && rng ) {
                 return std::string_view( &*rng.cbegin(),
                                          std::ranges::distance( rng ) );
             } )
           | std::ranges::to<std::vector<std::string_view>>();
}

constexpr std::vector<std::string_view>
sanitize_input( const std::vector<std::string_view> & inputs,
                const std::regex &                    pattern ) {
    const auto filter_func = [&pattern]( const auto view ) {
        std::smatch       result;
        const std::string tmp{ view.data(), view.size() };
        return std::regex_match( tmp.cbegin(), tmp.cend(), result, pattern );
    };
    return inputs | std::views::filter( filter_func )
           | std::views::transform( []( auto && rng ) {
                 return std::string_view( &*rng.cbegin(),
                                          std::ranges::distance( rng ) );
             } )
           | std::ranges::to<std::vector<std::string_view>>();
}
