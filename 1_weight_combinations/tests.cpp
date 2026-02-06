/**
 * @file tests.cpp
 * @author Vladimir Nikulin (mail.jorey@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-11-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/process/v1.hpp>

// протестим CPP проги
#include "weight_combinator.hpp"

// протестим C проги
extern "C"
{
#include "wc_module.h"
};

/// @brief Псевдоним для boost::process
namespace bp = boost::process::v1;

/**
 * @brief Тест на правильность работы класса
 * должен выдавать две комбинации: {100, 200} и {300}
 * при номенклатуре весов {100, 200, 300, 500, 1000, 1200, 1400, 1500, 2000, 3000}
 * и целевом весе target = 300
 */
TEST( WeightCombinatorClass, combine_basic_test )
{
    WeightCombinator::Combinations expected =
    {
        {100, 200},
        {300}
    };

    WeightCombinator::Weights weights = {100, 200, 300, 500, 1000, 1200, 1400, 1500, 2000, 3000};
    WeightCombinator::Combinations result;
    int target = 300;

    WeightCombinator combinator;
    combinator.combine( target, result, weights );

    ASSERT_THAT( result, expected );
}

/**
 * @brief Тест метода combinator.combine(target, result, weights)
 * должен выдать ошибку при нулевом значении target
 */
TEST( WeightCombinatorClass, combine_zero_target_test )
{
    WeightCombinator::Weights weights = { 100, 200, 300 };
    WeightCombinator::Combinations result;
    int target = 0;

    #ifdef __GNUC__
    EXPECT_THROW(
    {
        try
        {
            WeightCombinator combinator;
            combinator.combine( target, result, weights );
        }
        catch ( std::runtime_error const &e )
        {
            EXPECT_STREQ( "Zero target weight", e.what() );
            throw;
        }
    }, std::runtime_error );
    #else
    EXPECT_THROW(
    {
        try
        {
            WeightCombinator combinator;
            combinator.combine( target, result, weights );
        }
        catch ( std::exception const &e )
        {
            EXPECT_STREQ( "Zero target weight", e.what() );
            throw;
        }
    }, std::exception );
    #endif
}

/// @brief Сокращённый вариант предыдущего теста TEST(WeightCombinatorClass, combine_zero_target_test)
TEST( WeightCombinatorClass, combine_zero_target_test_2 )
{

    WeightCombinator::Weights weights = {100, 200, 300};
    WeightCombinator::Combinations result;
    int target = 0;

    WeightCombinator combinator;

    #ifdef __GNUC__
    EXPECT_THROW( combinator.combine( target, result, weights ), std::runtime_error );
    #else
    EXPECT_THROW( combinator.combine( target, result, weights ), std::exception );
    #endif
}

/**
 * @brief Тест метода WeightCombinator::combine(unsigned int target, WeightCombinator::Combinations &combinations, WeightCombinator::Weights &weights).
 * При пустой номенклатуре весов должна быть выдана ошибка плохого аргумента
 */
TEST( WeightCombinatorClass, combine_empty_collection_test )
{
    WeightCombinator::Weights weights;          // empty
    WeightCombinator::Combinations result;
    int target = 123;

    #ifdef __GNUC__
    EXPECT_THROW(
    {
        try
        {
            WeightCombinator combinator;
            combinator.combine( target, result, weights );
        }
        catch ( std::runtime_error const &e )
        {
            EXPECT_STREQ( "Invalid weights size", e.what() );
            throw;
        }
    }, std::runtime_error );
    #else
    EXPECT_THROW(
    {
        try
        {
            WeightCombinator combinator;
            combinator.combine( target, result, weights );
        }
        catch ( std::exception const &e )
        {
            EXPECT_STREQ( "Invalid weights size", e.what() );
            throw;
        }
    }, std::exception );
    #endif
}

/**
 * @brief Тест метода WeightCombinator::combine(unsigned int target, WeightCombinator::Combinations &combinations, WeightCombinator::Weights &weights).
 * Если номенклатура больше, чем количество бит в переменной combinator (> 32)
 * (биты отвечают за кол-во возможных вариантов наборов гирь)
 * должна быть выдана ошибка плохого аргумента
 */
TEST( WeightCombinatorClass, combine_big_collection_test )
{
    WeightCombinator::Weights weights( 100 );
    WeightCombinator::Combinations result;
    int target = 123;

    EXPECT_THROW(
    {
        try
        {
            WeightCombinator combinator;
            combinator.combine( target, result, weights );
        }
        catch ( std::exception const &e )
        {
            EXPECT_STREQ( "Invalid weights size", e.what() );
            throw;
        }
    }, std::exception );
}

/**
 * @brief Тест поведения при одном элементе в номенклатуре весов.
 * Ожидается возврат одной комбинации при совпадении target и weights[0]
 */
TEST( WeightCombinatorClass, combine_one_element_in_collection_test )
{
    WeightCombinator::Weights weights = {100};
    WeightCombinator::Combinations result;
    int target = 100;

    WeightCombinator combinator;
    combinator.combine( target, result, weights );

    ASSERT_THAT( result, WeightCombinator::Combinations{ {100} } );
}

/**
 * @brief Тест противоположный тесту TEST(WeightCombinatorClass, combine_one_element_in_collection_test).
 * Ожидается возврат 0 при различных target и weights[0]
 */
TEST( WeightCombinatorClass, combine_one_element_in_collection_bad_target_value_test )
{
    WeightCombinator::Weights weights = {100};
    WeightCombinator::Combinations result;
    int target = 200;

    WeightCombinator combinator;
    combinator.combine( target, result, weights );

    ASSERT_THAT( result, WeightCombinator::Combinations() );
}

/**
 * @brief
 * Ожидается возврат 0 при различных target = 2637 при несоответствующей номенклатуре весов
 */
TEST( WeightCombinatorClass, combine_zero_combination_in_result_test )
{
    WeightCombinator::Combinations expected;    // empty

    WeightCombinator::Weights weights = {100, 200, 300, 500, 1000, 1200, 1400, 1500, 2000, 3000};
    WeightCombinator::Combinations result;
    int target = 2637;

    WeightCombinator combinator;
    combinator.combine( target, result, weights );

    ASSERT_THAT( result, expected );
}

/**
 * @brief
 * Ожидается возврат 1, максимальный вес "поддерживаемый" номенклатурой == сумме всех весов из номенклатуры.
 * Это одна комбинация всегда
 */
TEST( WeightCombinatorClass, combine_max_combination_in_result_test )
{
    WeightCombinator::Combinations expected =
    {
        {100, 200, 300, 500, 1000, 1200, 1400, 1500, 2000, 3000}
    };

    WeightCombinator::Weights weights = {100, 200, 300, 500, 1000, 1200, 1400, 1500, 2000, 3000};
    WeightCombinator::Combinations result;
    int target = 11200;

    WeightCombinator combinator;
    combinator.combine( target, result, weights );

    ASSERT_THAT( result, expected );
}

/**
 * @brief
 * Программа должна работать, даже если номенклатура имеет одинаковые веса.
 * При номенклатуре {100, 100, 100, 100, 100, 100, 100}, т.е. 7 гирь
 * заданный вес target = 700 -> сумма всех семи гирь из номенклатуры даёт комбинацию
 */
TEST( WeightCombinatorClass, combine_all_weights_are_same_one_combination_in_result_test )
{
    WeightCombinator::Combinations expected =
    {
        {100, 100, 100, 100, 100, 100, 100}
    };

    WeightCombinator::Weights weights = {100, 100, 100, 100, 100, 100, 100};
    WeightCombinator::Combinations result;
    int target = 700;

    WeightCombinator combinator;
    combinator.combine( target, result, weights );

    ASSERT_THAT( result, expected );
}

/**
 * @brief
 * Усложнение предыдущего теста TEST(WeightCombinatorClass, combine_all_weights_are_same_one_combination_in_result_test).
 * Вместо одной одинаковый гири -> две одинаковые
 */
TEST( WeightCombinatorClass, combine_all_weights_are_same_two_combination_in_result_test )
{
    WeightCombinator::Combinations expected =
    {
        {100, 100, 100},
        {100, 200},
        {100, 200},
        {100, 200},
        {100, 200},
        {100, 200},
        {100, 200},
        {100, 200},
        {100, 200},
        {100, 200}
    };

    WeightCombinator::Weights weights = {100, 100, 200, 200, 200, 100};
    WeightCombinator::Combinations result;
    int target = 300;

    WeightCombinator combinator;
    combinator.combine( target, result, weights );

    ASSERT_THAT( result, expected );
}

/**
 * @brief Усложнение предыдущего теста TEST(WeightCombinatorClass, combine_all_weights_are_same_two_combination_in_result_test)
 * Вместо одной одинаковый гири -> несколько одинаковых
 */
TEST( WeightCombinatorClass, combine_all_weights_are_same_some_combination_in_result_test )
{
    WeightCombinator::Combinations expected =
    {
        {100, 100, 100},
        {100, 100, 100},
        {100, 100, 100},
        {100, 100, 100},
        {300},
        {300},
        {300}
    };

    WeightCombinator::Weights weights = {100, 100, 100, 300, 300, 100, 300};
    WeightCombinator::Combinations result;
    int target = 300;

    WeightCombinator combinator;
    combinator.combine( target, result, weights );

    ASSERT_THAT( result, expected );
}

/**
 * @brief Тестируем через main()
 *
 */
TEST( Main, basic_usage_test )
{
    bp::ipstream pipe_stream;

    #ifdef _WIN32
    bp::child c( "WeightCombinations-CPP.exe --log-disable", bp::std_out > pipe_stream );
    #else
    bp::child c( "WeightCombinations-CPP --log-disable", bp::std_out > pipe_stream );
    #endif

    std::string line;
    std::ostringstream out;

    while ( pipe_stream && std::getline( pipe_stream, line ) && !line.empty() )
        out << line;

    c.wait();

    #ifdef _WIN32
    ASSERT_EQ( out.str(), "100 200 300 500 1000 1200 1400 1500 2000 3000 \r" );
    #else
    ASSERT_EQ( out.str(), "100 200 300 500 1000 1200 1400 1500 2000 3000 " );
    #endif
}

// Тесты C кода

/// @brief Тест битового варианта
TEST( get_number_of_combinations, basic_bits_test )
{
    unsigned int expected_number_of_combinations = 2;

    unsigned int nomenclature_size = 10;
    unsigned int *nomenclature = ( unsigned int * )calloc( nomenclature_size, sizeof( unsigned int ) );

    if ( !nomenclature )
    {
        FAIL() << "Cannot create '*nomenclature' dynamic array\n";
        return;
    }

    //ASSERT_TRUE(nomenclature != nullptr);

    nomenclature[0] = 100;
    nomenclature[1] = 200;
    nomenclature[2] = 300;
    nomenclature[3] = 500;
    nomenclature[4] = 1000;
    nomenclature[5] = 1200;
    nomenclature[6] = 1400;
    nomenclature[7] = 1500;
    nomenclature[8] = 2000;
    nomenclature[9] = 3000;

    unsigned int target = 300;
    enum Solution solution = BITS;

    unsigned int **result;
    int number_of_combinations = get_number_of_combinations( &solution, nomenclature, &nomenclature_size, &target,
                                                             &result );

    ASSERT_EQ( number_of_combinations, expected_number_of_combinations );

    ASSERT_EQ( result[0][0], 100 );
    ASSERT_EQ( result[0][1], 200 );
    ASSERT_EQ( result[1][0], 300 );
}

/// @brief Тест рекурсивного варианта
TEST( get_number_of_combinations, basic_recursive_test )
{
    unsigned int expected_number_of_combinations = 2;

    unsigned int nomenclature_size = 10;
    unsigned int *nomenclature = ( unsigned int * )calloc( nomenclature_size, sizeof( unsigned int ) );

    if ( !nomenclature )
    {
        FAIL() << "Cannot create '*nomenclature' dynamic array\n";
        return;
    }

    nomenclature[0] = 100;
    nomenclature[1] = 200;
    nomenclature[2] = 300;
    nomenclature[3] = 500;
    nomenclature[4] = 1000;
    nomenclature[5] = 1200;
    nomenclature[6] = 1400;
    nomenclature[7] = 1500;
    nomenclature[8] = 2000;
    nomenclature[9] = 3000;

    unsigned int target = 300;
    enum Solution solution = RECURSIVE;

    unsigned int **result;
    int number_of_combinations = get_number_of_combinations( &solution, nomenclature, &nomenclature_size, &target,
                                                             &result );

    ASSERT_EQ( number_of_combinations, expected_number_of_combinations );

    ASSERT_EQ( result[0][0], 100 );
    ASSERT_EQ( result[0][1], 200 );
    ASSERT_EQ( result[1][0], 300 );
}

/// @brief Тест, когда все гири одинаковы. Битовый вариант
TEST( get_number_of_combinations, combine_all_weights_are_same_bits_test )
{
    unsigned int expected_number_of_combinations = 4;

    unsigned int nomenclature_size = 4;
    unsigned int *nomenclature = ( unsigned int * )calloc( nomenclature_size, sizeof( unsigned int ) );

    if ( !nomenclature )
    {
        FAIL() << "Cannot create '*nomenclature' dynamic array\n";
        return;
    }

    nomenclature[0] = 100;
    nomenclature[1] = 100;
    nomenclature[2] = 100;
    nomenclature[3] = 100;

    unsigned int target = 300;
    enum Solution solution = BITS;

    unsigned int **result;
    int number_of_combinations = get_number_of_combinations( &solution, nomenclature, &nomenclature_size, &target,
                                                             &result );

    ASSERT_EQ( number_of_combinations, expected_number_of_combinations );

    ASSERT_EQ( result[0][0], 100 );
    ASSERT_EQ( result[0][1], 100 );
    ASSERT_EQ( result[0][2], 100 );

    ASSERT_EQ( result[1][0], 100 );
    ASSERT_EQ( result[1][1], 100 );
    ASSERT_EQ( result[1][2], 100 );

    ASSERT_EQ( result[2][0], 100 );
    ASSERT_EQ( result[2][1], 100 );
    ASSERT_EQ( result[2][2], 100 );

    ASSERT_EQ( result[3][0], 100 );
    ASSERT_EQ( result[3][1], 100 );
    ASSERT_EQ( result[3][2], 100 );
}

/// @brief Тест, когда все гири одинаковы. Рекурсивный вариант
TEST( get_number_of_combinations, combine_all_weights_are_same_recursive_test )
{
    unsigned int expected_number_of_combinations = 4;

    unsigned int nomenclature_size = 4;
    unsigned int *nomenclature = ( unsigned int * )calloc( nomenclature_size, sizeof( unsigned int ) );

    if ( !nomenclature )
    {
        FAIL() << "Cannot create '*nomenclature' dynamic array\n";
        return;
    }

    nomenclature[0] = 100;
    nomenclature[1] = 100;
    nomenclature[2] = 100;
    nomenclature[3] = 100;

    unsigned int target = 300;
    enum Solution solution = RECURSIVE;

    unsigned int **result;
    int number_of_combinations = get_number_of_combinations( &solution, nomenclature, &nomenclature_size, &target,
                                                             &result );

    ASSERT_EQ( number_of_combinations, expected_number_of_combinations );

    ASSERT_EQ( result[0][0], 100 );
    ASSERT_EQ( result[0][1], 100 );
    ASSERT_EQ( result[0][2], 100 );

    ASSERT_EQ( result[1][0], 100 );
    ASSERT_EQ( result[1][1], 100 );
    ASSERT_EQ( result[1][2], 100 );

    ASSERT_EQ( result[2][0], 100 );
    ASSERT_EQ( result[2][1], 100 );
    ASSERT_EQ( result[2][2], 100 );

    ASSERT_EQ( result[3][0], 100 );
    ASSERT_EQ( result[3][1], 100 );
    ASSERT_EQ( result[3][2], 100 );
}

/// @brief Тест, когда есть несколько гирь, но значения всего два. Битовый вариант
TEST( get_number_of_combinations, all_weights_are_same_two_values_bits_test )
{
    unsigned int expected_number_of_combinations = 7;

    unsigned int nomenclature_size = 7;
    unsigned int *nomenclature = ( unsigned int * )calloc( nomenclature_size, sizeof( unsigned int ) );

    if ( !nomenclature )
    {
        FAIL() << "Cannot create '*nomenclature' dynamic array\n";
        return;
    }

    nomenclature[0] = 100;
    nomenclature[1] = 100;
    nomenclature[2] = 100;
    nomenclature[3] = 300;
    nomenclature[4] = 300;
    nomenclature[5] = 100;
    nomenclature[6] = 300;

    unsigned int target = 300;
    enum Solution solution = BITS;

    unsigned int **result;
    int number_of_combinations = get_number_of_combinations( &solution, nomenclature, &nomenclature_size, &target,
                                                             &result );

    ASSERT_EQ( number_of_combinations, expected_number_of_combinations );

    //for (int i = 0; i < number_of_combinations; i++)
    //{
    //    std::cout << "\n";
    //    for (unsigned int j = 0; j < nomenclature_size; j++)
    //    {
    //        std::cout << result[i][j] << " ";
    //    }
    //}

    ASSERT_EQ( result[0][0], 100 );
    ASSERT_EQ( result[0][1], 100 );
    ASSERT_EQ( result[0][2], 100 );

    ASSERT_EQ( result[1][0], 300 );

    ASSERT_EQ( result[2][0], 300 );

    ASSERT_EQ( result[3][0], 100 );
    ASSERT_EQ( result[3][1], 100 );
    ASSERT_EQ( result[3][2], 100 );

    ASSERT_EQ( result[4][0], 100 );
    ASSERT_EQ( result[4][1], 100 );
    ASSERT_EQ( result[4][2], 100 );

    ASSERT_EQ( result[5][0], 100 );
    ASSERT_EQ( result[5][1], 100 );
    ASSERT_EQ( result[5][2], 100 );

    ASSERT_EQ( result[6][0], 300 );
}

/// @brief Тест, когда есть несколько гирь, но значения всего два. Рекурсивный вариант
TEST( get_number_of_combinations, all_weights_are_same_two_values_recursive_test )
{
    unsigned int expected_number_of_combinations = 7;

    unsigned int nomenclature_size = 7;
    unsigned int *nomenclature = ( unsigned int * )calloc( nomenclature_size, sizeof( unsigned int ) );

    if ( !nomenclature )
    {
        FAIL() << "Cannot create '*nomenclature' dynamic array\n";
        return;
    }

    nomenclature[0] = 100;
    nomenclature[1] = 100;
    nomenclature[2] = 100;
    nomenclature[3] = 300;
    nomenclature[4] = 300;
    nomenclature[5] = 100;
    nomenclature[6] = 300;

    unsigned int target = 300;
    enum Solution solution = RECURSIVE;

    unsigned int **result;
    int number_of_combinations = get_number_of_combinations( &solution, nomenclature, &nomenclature_size, &target,
                                                             &result );

    ASSERT_EQ( number_of_combinations, expected_number_of_combinations );

    //for (int i = 0; i < number_of_combinations; i++)
    //{
    //    std::cout << "\n";
    //    for (unsigned int j = 0; j < nomenclature_size; j++)
    //    {
    //        std::cout << result[i][j] << " ";
    //    }
    //}

    ASSERT_EQ( result[0][0], 100 );
    ASSERT_EQ( result[0][1], 100 );
    ASSERT_EQ( result[0][2], 100 );

    ASSERT_EQ( result[1][0], 100 );
    ASSERT_EQ( result[1][1], 100 );
    ASSERT_EQ( result[1][2], 100 );

    ASSERT_EQ( result[2][0], 100 );
    ASSERT_EQ( result[2][1], 100 );
    ASSERT_EQ( result[2][2], 100 );

    ASSERT_EQ( result[3][0], 100 );
    ASSERT_EQ( result[3][1], 100 );
    ASSERT_EQ( result[3][2], 100 );

    ASSERT_EQ( result[4][0], 300 );

    ASSERT_EQ( result[5][0], 300 );

    ASSERT_EQ( result[6][0], 300 );
}

/// @brief Тест на выдачу ошибки, при нулевом целевом весе
TEST( get_number_of_combinations, zero_target_test )
{
    int expected_number_of_combinations = -1;

    unsigned int nomenclature_size = 10;
    unsigned int *nomenclature = ( unsigned int * )calloc( nomenclature_size, sizeof( unsigned int ) );

    if ( !nomenclature )
    {
        FAIL() << "Cannot create '*nomenclature' dynamic array\n";
        return;
    }

    nomenclature[0] = 100;
    nomenclature[1] = 200;
    nomenclature[2] = 300;
    nomenclature[3] = 500;
    nomenclature[4] = 1000;
    nomenclature[5] = 1200;
    nomenclature[6] = 1400;
    nomenclature[7] = 1500;
    nomenclature[8] = 2000;
    nomenclature[9] = 3000;

    unsigned int target = 0;
    enum Solution solution = RECURSIVE;

    ::testing::internal::CaptureStderr();   // перехватываем err msg из stderr

    unsigned int **result;
    int number_of_combinations = get_number_of_combinations( &solution, nomenclature, &nomenclature_size, &target,
                                                             &result );

    std::string errMsg = ::testing::internal::GetCapturedStderr();

    ASSERT_EQ( number_of_combinations, expected_number_of_combinations );
    ASSERT_EQ( errMsg, "Zero target weight\n" );
}

/// @brief Тест на выдачу ошибки, при слишком большой номенклатуре. Битовый вариант
TEST( get_number_of_combinations, bits_solution_big_nomenclature_test )
{
    int expected_number_of_combinations = -1;

    unsigned int nomenclature_size = ( unsigned int )_allowed_bits + 1;
    unsigned int *nomenclature = ( unsigned int * )calloc( nomenclature_size, sizeof( unsigned int ) );

    if ( !nomenclature )
    {
        FAIL() << "Cannot create '*nomenclature' dynamic array\n";
        return;
    }

    for ( unsigned int *ptr = nomenclature, *end = nomenclature + nomenclature_size; ptr != end; ptr++ )
        *ptr = 100;

    unsigned int target = 500;
    enum Solution solution = BITS;

    ::testing::internal::CaptureStderr();   // перехватываем err msg из stderr

    unsigned int **result;
    int number_of_combinations = get_number_of_combinations( &solution, nomenclature, &nomenclature_size, &target,
                                                             &result );

    std::string errMsg = ::testing::internal::GetCapturedStderr();

    ASSERT_EQ( number_of_combinations, expected_number_of_combinations );
    ASSERT_EQ( errMsg, "Nomenclature size exceeds allowable value: 33\nMax allowed bits: 32\n" );
}

/// @brief Тест программы целиком, из дочернего процесса
TEST( Main_C, set_target_and_nomenclature_test )
{
    bp::ipstream pipe_stream;

    #ifdef _WIN32
    bp::child c( "WeightCombinations-C.exe -t 300 -n 100 100 100 100", bp::std_out > pipe_stream );
    #else
    bp::child c( "WeightCombinations-C -t 300 -n 100 100 100 100", bp::std_out > pipe_stream );
    #endif

    std::string line;
    std::ostringstream out;

    while ( pipe_stream && std::getline( pipe_stream, line ) && !line.empty() )
        out << line;

    c.wait();

    #ifdef _WIN32
    ASSERT_EQ( out.str(), "4\r" );
    #else
    ASSERT_EQ( out.str(), "4" );
    #endif
}

/// @brief Тест печати комбинаций
TEST( Main_C, get_combinations_values_test )
{
    bp::ipstream pipe_stream;

    #ifdef _WIN32
    bp::child c( "WeightCombinations-C.exe -c -t 1000", bp::std_out > pipe_stream );
    #else
    bp::child c( "WeightCombinations-C -c -t 1000", bp::std_out > pipe_stream );
    #endif

    #ifdef _WIN32
    std::string line;
    std::ostringstream out;

    while ( pipe_stream && std::getline( pipe_stream, line ) && !line.empty() )
        out << line;

    c.wait();

    ASSERT_EQ( out.str(), "2\r \r200 300 500  \r1000  \r" );
    #else
    std::vector<std::string> data;
    std::string line;

    while ( c.running() && std::getline( pipe_stream, line ) && !line.empty() )
        data.push_back( line );

    c.wait();

    ASSERT_THAT( data, ::testing::ElementsAre( "2", " ", "200 300 500  ", "1000  " ) );
    #endif
}

/**
 * @brief Точка входа для тестирования
 *
 * @param argc Количество аргументов
 * @param argv Сами аргументы
 * @return int Статус код ошибки
 */
int main( int argc, char **argv )
{
    spdlog::set_level(spdlog::level::off);

    ::testing::InitGoogleTest( &argc, argv );
    ::testing::InitGoogleMock( &argc, argv );

    return RUN_ALL_TESTS();
}
