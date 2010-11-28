// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>

#include <geometry_test_common.hpp>

#include <boost/geometry/util/reversible_view.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/util/write_dsv.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>


template <bg::iterate_direction Direction, typename Range>
void test_forward_or_reverse(Range const& range, std::string const& expected)
{
    typedef bg::reversible_view
        <
            Range const,
            Direction
        > view_type;

    view_type view(range);

    bool first = true;
    std::ostringstream out;
    for (typename boost::range_iterator<view_type const>::type
        it = boost::begin(view);
        it != boost::end(view);
        ++it, first = false)
    {
        out << (first ? "" : " ") << bg::dsv(*it);
    }
    BOOST_CHECK_EQUAL(out.str(), expected);
}



template <typename Geometry>
void test_geometry(std::string const& wkt,
            std::string const& expected_forward, std::string const& expected_reverse)
{
    Geometry geo;
    bg::read_wkt(wkt, geo);

    test_forward_or_reverse<bg::iterate_forward>(geo, expected_forward);
    test_forward_or_reverse<bg::iterate_reverse>(geo, expected_reverse);
}


template <typename P>
void test_all()
{
    test_geometry<bg::model::linestring<P> >(
            "linestring(1 1,2 2,3 3)",
            "(1, 1) (2, 2) (3, 3)",
            "(3, 3) (2, 2) (1, 1)");
}


int test_main(int, char* [])
{
    test_all<bg::model::point_2d>();
    test_all<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_all<boost::tuple<double, double> >();

    return 0;
}
