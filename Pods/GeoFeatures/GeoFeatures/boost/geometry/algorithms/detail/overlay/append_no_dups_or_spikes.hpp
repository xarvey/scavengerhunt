// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPEND_NO_DUPS_OR_SPIKES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPEND_NO_DUPS_OR_SPIKES_HPP

#include <boost/range.hpp>

#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/detail/point_is_spike_or_equal.hpp>
#include <boost/geometry/algorithms/detail/equals/point_point.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/range.hpp>


namespace geofeatures_boost {} namespace boost = geofeatures_boost; namespace geofeatures_boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

// TODO: move this / rename this
template <typename Point1, typename Point2, typename RobustPolicy>
inline bool points_equal_or_close(Point1 const& point1,
        Point2 const& point2,
        RobustPolicy const& robust_policy)
{
    if (detail::equals::equals_point_point(point1, point2))
    {
        return true;
    }

    if (BOOST_GEOMETRY_CONDITION(! RobustPolicy::enabled))
    {
        return false;
    }

    // Try using specified robust policy
    typedef typename geometry::robust_point_type
    <
        Point1,
        RobustPolicy
    >::type robust_point_type;

    robust_point_type point1_rob, point2_rob;
    geometry::recalculate(point1_rob, point1, robust_policy);
    geometry::recalculate(point2_rob, point2, robust_policy);

    return detail::equals::equals_point_point(point1_rob, point2_rob);
}


template <typename Range, typename Point, typename RobustPolicy>
inline void append_no_dups_or_spikes(Range& range, Point const& point,
        RobustPolicy const& robust_policy)
{
#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
    std::cout << "  add: ("
        << geometry::get<0>(point) << ", " << geometry::get<1>(point) << ")"
        << std::endl;
#endif
    // The code below thies condition checks all spikes/dups
    // for geometries >= 3 points.
    // So we have to check the first potential duplicate differently
    if (geofeatures_boost::size(range) == 1
        && points_equal_or_close(*(geofeatures_boost::begin(range)), point, robust_policy))
    {
        return;
    }

    traits::push_back<Range>::apply(range, point);

    // If a point is equal, or forming a spike, remove the pen-ultimate point
    // because this one caused the spike.
    // If so, the now-new-pen-ultimate point can again cause a spike
    // (possibly at a corner). So keep doing this.
    // Besides spikes it will also avoid adding duplicates.
    while(geofeatures_boost::size(range) >= 3
            && point_is_spike_or_equal(point,
                *(geofeatures_boost::end(range) - 3),
                *(geofeatures_boost::end(range) - 2),
                robust_policy))
    {
        // Use the Concept/traits, so resize and append again
        traits::resize<Range>::apply(range, geofeatures_boost::size(range) - 2);
        traits::push_back<Range>::apply(range, point);
    }
}

template <typename Range, typename RobustPolicy>
inline void clean_closing_dups_and_spikes(Range& range,
                RobustPolicy const& robust_policy)
{
    std::size_t const minsize
        = core_detail::closure::minimum_ring_size
            <
                geometry::closure<Range>::value
            >::value;

    if (geofeatures_boost::size(range) <= minsize)
    {
        return;
    }

    typedef typename geofeatures_boost::range_iterator<Range>::type iterator_type;
    static bool const closed = geometry::closure<Range>::value == geometry::closed;

// TODO: the following algorithm could be rewritten to first look for spikes
// and then erase some number of points from the beginning of the Range

    bool found = false;
    do
    {
        found = false;
        iterator_type first = geofeatures_boost::begin(range);
        iterator_type second = first + 1;
        iterator_type ultimate = geofeatures_boost::end(range) - 1;
        if (BOOST_GEOMETRY_CONDITION(closed))
        {
            ultimate--;
        }

        // Check if closing point is a spike (this is so if the second point is
        // considered as a spike w.r.t. the last segment)
        if (point_is_spike_or_equal(*second, *ultimate, *first, robust_policy))
        {
            range::erase(range, first);
            if (BOOST_GEOMETRY_CONDITION(closed))
            {
                // Remove closing last point
                range::resize(range, geofeatures_boost::size(range) - 1);
                // Add new closing point
                range::push_back(range, range::front(range));
            }
            found = true;
        }
    } while(found && geofeatures_boost::size(range) > minsize);
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace geofeatures_boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPEND_NO_DUPS_OR_SPIKES_HPP
