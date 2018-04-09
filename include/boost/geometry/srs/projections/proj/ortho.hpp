// Boost.Geometry - gis-projections (based on PROJ4)

// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017, 2018.
// Modifications copyright (c) 2017-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Last updated version of proj: 5.0.0

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef BOOST_GEOMETRY_PROJECTIONS_ORTHO_HPP
#define BOOST_GEOMETRY_PROJECTIONS_ORTHO_HPP

#include <boost/config.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    struct ortho {}; // Orthographic

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace ortho
    {

            enum Mode {
                N_POLE = 0,
                S_POLE = 1,
                EQUIT  = 2,
                OBLIQ  = 3
            };

            template <typename T>
            struct par_ortho
            {
                T   sinph0;
                T   cosph0;
                int mode;
            };

            static const double EPS10 = 1.e-10;

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_ortho_spheroid : public base_t_fi<base_ortho_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_ortho<CalculationType> m_proj_parm;

                inline base_ortho_spheroid(const Parameters& par)
                    : base_t_fi<base_ortho_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    static const CalculationType half_pi = detail::half_pi<CalculationType>();

                    CalculationType coslam, cosphi, sinphi;

                    cosphi = cos(lp_lat);
                    coslam = cos(lp_lon);
                    switch (this->m_proj_parm.mode) {
                    case EQUIT:
                        if (cosphi * coslam < - EPS10) {
                            BOOST_THROW_EXCEPTION( projection_exception(-20) );
                        }
                        xy_y = sin(lp_lat);
                        break;
                    case OBLIQ:
                        if (this->m_proj_parm.sinph0 * (sinphi = sin(lp_lat)) +
                           this->m_proj_parm.cosph0 * cosphi * coslam < - EPS10) {
                            BOOST_THROW_EXCEPTION( projection_exception(-20) );
                        }
                        xy_y = this->m_proj_parm.cosph0 * sinphi - this->m_proj_parm.sinph0 * cosphi * coslam;
                        break;
                    case N_POLE:
                        coslam = - coslam;
                        BOOST_FALLTHROUGH;
                    case S_POLE:
                        if (fabs(lp_lat - this->m_par.phi0) - EPS10 > half_pi) {
                            BOOST_THROW_EXCEPTION( projection_exception(-20) );
                        }
                        xy_y = cosphi * coslam;
                        break;
                    }
                    xy_x = cosphi * sin(lp_lon);
                }

                // INVERSE(s_inverse)  spheroid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    static const CalculationType half_pi = detail::half_pi<CalculationType>();

                    CalculationType rh, cosc, sinc;

                    if ((sinc = (rh = boost::math::hypot(xy_x, xy_y))) > 1.) {
                        if ((sinc - 1.) > EPS10) {
                            BOOST_THROW_EXCEPTION( projection_exception(-20) );
                        }
                        sinc = 1.;
                    }
                    cosc = sqrt(1. - sinc * sinc); /* in this range OK */
                    if (fabs(rh) <= EPS10) {
                        lp_lat = this->m_par.phi0;
                        lp_lon = 0.0;
                    } else {
                        switch (this->m_proj_parm.mode) {
                        case N_POLE:
                            xy_y = -xy_y;
                            lp_lat = acos(sinc);
                            break;
                        case S_POLE:
                            lp_lat = - acos(sinc);
                            break;
                        case EQUIT:
                            lp_lat = xy_y * sinc / rh;
                            xy_x *= sinc;
                            xy_y = cosc * rh;
                            goto sinchk;
                        case OBLIQ:
                            lp_lat = cosc * this->m_proj_parm.sinph0 + xy_y * sinc * this->m_proj_parm.cosph0 /rh;
                            xy_y = (cosc - this->m_proj_parm.sinph0 * lp_lat) * rh;
                            xy_x *= sinc * this->m_proj_parm.cosph0;
                        sinchk:
                            if (fabs(lp_lat) >= 1.)
                                lp_lat = lp_lat < 0. ? -half_pi : half_pi;
                            else
                                lp_lat = asin(lp_lat);
                            break;
                        }
                        lp_lon = (xy_y == 0. && (this->m_proj_parm.mode == OBLIQ || this->m_proj_parm.mode == EQUIT))
                             ? (xy_x == 0. ? 0. : xy_x < 0. ? -half_pi : half_pi)
                                           : atan2(xy_x, xy_y);
                    }
                }

                static inline std::string get_name()
                {
                    return "ortho_spheroid";
                }

            };

            // Orthographic
            template <typename Parameters, typename T>
            inline void setup_ortho(Parameters& par, par_ortho<T>& proj_parm)
            {
                if (fabs(fabs(par.phi0) - geometry::math::half_pi<T>()) <= EPS10)
                    proj_parm.mode = par.phi0 < 0. ? S_POLE : N_POLE;
                else if (fabs(par.phi0) > EPS10) {
                    proj_parm.mode = OBLIQ;
                    proj_parm.sinph0 = sin(par.phi0);
                    proj_parm.cosph0 = cos(par.phi0);
                } else
                    proj_parm.mode = EQUIT;
                par.es = 0.;
            }

    }} // namespace detail::ortho
    #endif // doxygen

    /*!
        \brief Orthographic projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Azimuthal
         - Spheroid
        \par Example
        \image html ex_ortho.gif
    */
    template <typename CalculationType, typename Parameters>
    struct ortho_spheroid : public detail::ortho::base_ortho_spheroid<CalculationType, Parameters>
    {
        inline ortho_spheroid(const Parameters& par) : detail::ortho::base_ortho_spheroid<CalculationType, Parameters>(par)
        {
            detail::ortho::setup_ortho(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::ortho, ortho_spheroid, ortho_spheroid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class ortho_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<ortho_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void ortho_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("ortho", new ortho_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_ORTHO_HPP

