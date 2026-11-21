#pragma once

#include "glm/glm.hpp"
#include <cmath>

#ifndef SPACE_VIEWER_ORBITS_H
#define SPACE_VIEWER_ORBITS_H

namespace orbits {
    constexpr double AU_KM = 149597870.7;
    constexpr double PI = 3.14159265358979323846;

    inline double rad(double deg) {
        return deg * PI / 180;
    }

    //value at J2000, rate per julian century
    struct elements {
        //au, deg
        double a;
        double aR;

        double e;
        double eR;

        double I;
        double IR;

        double L;
        double LR;

        double w;
        double wR;

        double O;
        double OR;
    };

    constexpr elements MERCURY {0.38709927,  0.00000037, 0.20563593,  0.00001906, 7.00497902, -0.00594749, 252.25032350, 149472.67411175,  77.45779628,  0.16047689,  48.33076593, -0.12534081};
    constexpr elements VENUS   {0.72333566,  0.00000390, 0.00677672, -0.00004107, 3.39467605, -0.00078890, 181.97909950,  58517.81538729, 131.60246718,  0.00268329,  76.67984255, -0.27769418};
    constexpr elements EARTH   {1.00000261,  0.00000562, 0.01671123, -0.00004392, -0.00001531,-0.01294668, 100.46457166,  35999.37244981, 102.93768193,  0.32327364,   0.0,         0.0};
    constexpr elements MARS    {1.52371034,  0.00001847, 0.09339410,  0.00007882, 1.84969142, -0.00813131,  -4.55343205,  19140.30268499, -23.94362959,  0.44441088,  49.55953891, -0.29257343};
    constexpr elements JUPITER {5.20288700, -0.00011607, 0.04838624, -0.00013253, 1.30439695, -0.00183714,  34.39644051,   3034.74612775,  14.72847983,  0.21252668, 100.47390909,  0.20469106};
    constexpr elements SATURN  {9.53667594, -0.00125060, 0.05386179, -0.00050991, 2.48599187,  0.00193609,  49.95424423,   1222.49362201,  92.59887831, -0.41897216, 113.66242448, -0.28867794};
    constexpr elements URANUS  {19.18916464,-0.00196176, 0.04725744, -0.00004397, 0.77263783, -0.00242939, 313.23810451,    428.48202785, 170.95427630,  0.40805281,  74.01692503,  0.04240589};
    constexpr elements NEPTUNE {30.06992276, 0.00026291, 0.00859048,  0.00005105, 1.77004347,  0.00035372, -55.12002969,    218.45945325,  44.96476227, -0.32241464, 131.78422574, -0.00508664};

    constexpr double SATURN_EQ_KM   = 60268.0;

    // radius at the textures left edge
    constexpr double RING_INNER_KM  = 70900.0;

    // radius at the textures right edge
    constexpr double RING_OUTER_KM  = 140800.0;

    inline glm::dvec3 position(const elements& el, double T) {
        double a = el.a + el.aR * T;
        double e = el.e + el.eR * T;
        double I = rad(el.I + el.IR * T);

        double L = el.L + el.LR * T;
        double wbar = el.w + el.wR * T;
        double Om = el.O + el.OR * T;

        double M = rad(std::fmod(L - wbar, 360.0));

        //wrap to -pi, pi
        M = std::remainder(M, 2 * PI);

        //Keplers equation, newton iteration
        double E = M + e * std::sin(M);
        for (int i = 0; i < 10; i++) {
            E -= (E - e*std::sin(E) - M) / (1 - e*std::cos(E));
        }

        double xp = a*(std::cos(E) - e);
        double yp = a * std::sqrt(1 - e * e) * std::sin(E);

        double w = rad(wbar - Om);
        Om = rad(Om);

        double cw = std::cos(w);
        double sw = std::sin(w);
        double cO = std::cos(Om);
        double sO = std::sin(Om);
        double cI = std::cos(I);
        double sI = std::sin(I);

        double x = (cw * cO - sw * sO * cI) * xp + (-sw * cO - cw * sO * cI) * yp;
        double y = (cw * sO + sw * cO * cI) * xp + (-sw * sO + cw * cO * cI) * yp;
        double z = (sw * sI) * xp + (cw * sI) * yp;

        return { x, y, z };
    }

    inline double julianDate(double usecs) {
        return usecs / 86400.0 + 2440587.5;
    }

    inline double centuries(double jd) {
        return (jd - 2451545.0) / 36525.0;
    }

    inline glm::dvec3 poleEcliptic(double raDeg, double decDeg) {
        const double eps = rad(23.4392911);

        double ra = rad(raDeg);
        double dec = rad(decDeg);

        double x = std::cos(dec) * std::cos(ra);
        double y = std::cos(dec) * std::sin(ra);
        double z = std::sin(dec);

        double ceps = std::cos(eps);
        double seps = std::sin(eps);

        return {
            x,
            y * ceps + z * seps,
            -y * seps + z * ceps
        };
    }
}

#endif