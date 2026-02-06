#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/process/v1.hpp>
namespace bp = boost::process::v1;

// протестим CPP проги
#include "point_class.hpp"
#include "cross_product_class.hpp"
#include "direction_finder_class.hpp"

// протестим C проги
extern "C"
{
#include "foo.h"
};

TEST( CrossProductClass, compute_test )
{
    double expected = -400.0;       // ожидаемый результат
    double eps =
        0.01;              // допустимое расхождение между ожидаемым и полученным значениями

    Point p1( 10.0, 10.0 ),
          p2( 20.0, 20.0 ),
          p3( 30.0, -10.0 );

    CrossProduct cp( p1, p2, p3 );
    double result = cp.compute();

    ASSERT_NEAR( result, expected, eps );
}

TEST( CrossProductClass, compute_ppp_test )
{
    double expected = 178.2;        // ожидаемый результат
    double eps =
        0.01;              // допустимое расхождение между ожидаемым и полученным значениями

    Point p1( 15.5, 37.0 ),
          p2( 22.1, 45.2 ),
          p3( -10.9, 31.2 );

    CrossProduct cp;
    double result = cp.compute( p1, p2, p3 );

    ASSERT_NEAR( result, expected, eps );
}

TEST( DirectionFinderClass, find_direction_test_non_convex_polygon_anti_clockwise_points )
{
    int expected = -1;       // ожидаемый результат

    // координаты вершин в векторе
    std::vector<Point> points;

    points.push_back( Point{ 2.0, 5.0 } );
    points.push_back( Point{ 3.5, 5.0 } );
    points.push_back( Point{ 2.5, 3.0 } );
    points.push_back( Point{ 6.0, 5.0 } );
    points.push_back( Point{ 7.5, 1.5 } );
    points.push_back( Point{ 7.5, 4.5 } );
    points.push_back( Point{ 10.5, 5.5 } );
    points.push_back( Point{ 7.5, 9.0 } );
    points.push_back( Point{ 5.5, 6.0 } );
    points.push_back( Point{ 2.0, 7.5 } );

    DirectionFinder df( points );
    int result = df.find_direction();

    ASSERT_EQ( result, expected );
}

TEST( DirectionFinderClass, find_direction_test_non_convex_polygon_clockwise_points )
{
    int expected = 1;       // ожидаемый результат

    // координаты вершин в векторе
    std::vector<Point> points;

    points.push_back( Point{ 5.5, 6.0 } );
    points.push_back( Point{ 7.5, 9.0 } );
    points.push_back( Point{ 10.5, 5.5 } );
    points.push_back( Point{ 7.5, 4.5 } );
    points.push_back( Point{ 6.5, 2.0 } );

    DirectionFinder df( points );
    int result = df.find_direction();

    ASSERT_EQ( result, expected );
}

TEST( DirectionFinderClass, find_direction_test_convex_polygon_anti_clockwise_points )
{
    int expected = -1;

    std::vector<Point> points;

    points.push_back( Point{ 5.0, -2.0 } );
    points.push_back( Point{ 2.5, 3.5 } );
    points.push_back( Point{ -4.0, 3.0 } );
    points.push_back( Point{ -3.5, -3.5 } );
    points.push_back( Point{ 1.0, -5.0 } );

    DirectionFinder df( points );
    int result = df.find_direction();

    ASSERT_EQ( result, expected );
}

TEST( DirectionFinderClass, find_direction_test_convex_polygon_clockwise_points )
{
    int expected = 1;

    std::vector<Point> points;

    points.push_back( Point{ 5.0, -2.0 } );
    points.push_back( Point{ 1.0, -5.0 } );
    points.push_back( Point{ -3.5, -3.5 } );
    points.push_back( Point{ -4.0, 3.0 } );
    points.push_back( Point{ 2.5, 3.5 } );

    DirectionFinder df( points );
    int result = df.find_direction();

    ASSERT_EQ( result, expected );
}

TEST( cross_product_module, cross_product_basic_usage_test )
{
    double expected = 0.0;
    double result = cross_product( 0.0, 0.0, 1.0, 1.0, -1.0, -1.0 );

    ASSERT_EQ( result, expected );
}

TEST( polygon_traversal_direction_module, get_traversal_direction_basic_usage_test )
{
    float x_nodes[3] = {10.0, 5.0, 2.0};
    float y_nodes[3] = {-10.0, -5.0, -2.0};

    int result = get_traversal_direction( x_nodes, y_nodes, 3 );
    int expected = -1;

    ASSERT_EQ( result, expected );
}

int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    ::testing::InitGoogleMock( &argc, argv );

    return RUN_ALL_TESTS();
}