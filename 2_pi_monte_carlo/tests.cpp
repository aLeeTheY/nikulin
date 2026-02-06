#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/process/v1.hpp>

// протестим CPP проги
#include "pi_monte_carlo_points_generator_class.hpp"
#include "pi_convergence_check_class.hpp"
#include "true_pi_selector_class.hpp"
//#include "samples.hpp"

// протестим C проги
extern "C"
{
#include "counters.h"
};

namespace bp = boost::process::v1;

TEST( TruePISelectorClass, basic_usage_test )
{
    TruePISelector selector( 6, 1000, 2, 0.01 );

    double pi = 0;
    pi = selector.select_pi();

    ASSERT_NEAR( pi, 3.14, 0.01 );
}

#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
TEST( TruePISelectorClass, zero_total_pi_test )
{
    EXPECT_THROW(
    {
        try
        {
            TruePISelector selector( 0, 1000, 2, 0.01 );

            double pi = 0;
            pi = selector.select_pi();
        }
        catch ( std::exception const &e )
        {
            EXPECT_STREQ( "Zero total_pi", e.what() );
            throw;
        }
    }, std::exception );
}
#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif

// TODO
//void run_subprocess()
//{
//    bp::ipstream pipe_stream;
//
//#ifdef _WIN32
//    bp::child c("PIMonteCarlo-CPP.exe -t -1", bp::std_out > pipe_stream);
//#else
//    bp::child c("PIMonteCarlo-CPP -t -1", bp::std_out > pipe_stream);
//#endif
//
//    std::string line;
//    std::ostringstream out;
//
//    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
//        out << line;
//    c.wait();
//}
//
//TEST(Main, negative_total_test)
//{
//    try
//    {
//        run_subprocess();
//    }
//    catch (...)
//    {
//        SUCCEED();
//    }
//
//    FAIL() << "Fail";
//    //EXPECT_THROW(run_subprocess());
//}

TEST( counters_module, pi_single_thread_basic_usage_test )
{
    double pi = 0;
    pi = get_pi_single_thread( 6, 1000, 2, 0.01, XOR_SHIFT_64 );

    ASSERT_NEAR( pi, 3.14, 0.01 );
}

TEST( counters_module, pi_multithread_basic_usage_test )
{
    double pi = 0;
    pi = get_pi_multithread( 36, 1000, 2, 0.01, 6, XOR_SHIFT_64 );

    ASSERT_NEAR( pi, 3.14, 0.01 );
}

TEST( counters_module, pi_opencl_basic_usage_test )
{
    double pi = 0;
    pi = get_pi_opencl( 10, 1000, 2, 0.01, XOR_SHIFT_64 );

    ASSERT_NEAR( pi, 3.14, 0.01 );
}

int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    ::testing::InitGoogleMock( &argc, argv );

    return RUN_ALL_TESTS();
}
