// Пролёты МКС над Москвой, которые видно глазами.
// Орбита из iss.tle (CelesTrak), шаг 5 с на 14 дней вперёд, SGP4.

#include <libsgp4/CoordTopocentric.h>
#include <libsgp4/DecayedException.h>
#include <libsgp4/Globals.h>
#include <libsgp4/Observer.h>
#include <libsgp4/SGP4.h>
#include <libsgp4/SolarPosition.h>
#include <libsgp4/Tle.h>
#include <libsgp4/Util.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

// is_visible.cpp
bool is_visible(double station_elevation, double sun_elevation, bool station_in_sunlight);

namespace {

const double kMoscowLat = 55.7558;   // градусы
const double kMoscowLon = 37.6173;   // градусы
const double kMoscowAlt = 0.15;      // км над уровнем моря
const int kDays = 14;
const double kStepSec = 5.0;
const double kMskOffsetSec = 3 * 3600.0;   // Москва = UTC+3

std::string trim(std::string s)
{
    while (!s.empty() && (s.back() == '\r' || s.back() == ' ' || s.back() == '\n'))
        s.pop_back();
    return s;
}

// Освещена ли станция. Тень Земли считаю цилиндром, а не конусом:
// для низкой орбиты разница в секундах.
bool in_sunlight(const libsgp4::Vector& sat, const libsgp4::Vector& sun)
{
    const double sun_len = sun.Magnitude();
    const double ux = sun.x / sun_len, uy = sun.y / sun_len, uz = sun.z / sun_len;
    const double along = sat.x * ux + sat.y * uy + sat.z * uz;
    if (along > 0)
        return true;   // станция на дневной стороне Земли
    const double px = sat.x - ux * along, py = sat.y - uy * along, pz = sat.z - uz * along;
    return std::sqrt(px * px + py * py + pz * pz) > libsgp4::kXKMPER;
}

const char* compass(double azimuth_deg)
{
    static const char* names[] = {"С", "СВ", "В", "ЮВ", "Ю", "ЮЗ", "З", "СЗ"};
    int i = static_cast<int>(std::lround(azimuth_deg / 45.0)) % 8;
    return names[i];
}

std::string msk(const libsgp4::DateTime& utc)
{
    libsgp4::DateTime t = utc.AddSeconds(kMskOffsetSec);
    char buf[32];
    std::snprintf(buf, sizeof buf, "%02d.%02d  %02d:%02d", t.Day(), t.Month(), t.Hour(), t.Minute());
    return buf;
}

struct Pass
{
    libsgp4::DateTime rise;
    libsgp4::DateTime above10;     // выше 10°
    bool reached10 = false;
    double rise_az = 0;
    double set_az = 0;
    double max_el = -90;
    libsgp4::DateTime max_time;
};

}   // namespace

int main()
{
    std::ifstream f("iss.tle");
    std::string name, line1, line2;
    if (!std::getline(f, name) || !std::getline(f, line1) || !std::getline(f, line2))
    {
        std::cerr << "Нет iss.tle, запусти ./run.sh\n";
        return 1;
    }
    libsgp4::Tle tle(trim(name), trim(line1), trim(line2));
    libsgp4::SGP4 sgp4(tle);
    libsgp4::Observer moscow(kMoscowLat, kMoscowLon, kMoscowAlt);
    libsgp4::SolarPosition solar;

    std::cout << "МКС над Москвой, ближайшие " << kDays << " дней, время московское\n";
    std::cout << "Данные орбиты от " << msk(tle.Epoch()) << "\n\n";
    std::cout << " Дата   Время  Откуда → Куда   Выше всего\n";

    const libsgp4::DateTime start = libsgp4::DateTime::Now();
    const libsgp4::DateTime end = start.AddDays(kDays);
    int total = 0, shown = 0;
    bool in_pass = false;
    Pass p;

    try
    {
        for (libsgp4::DateTime t = start; t < end; t = t.AddSeconds(kStepSec))
        {
            libsgp4::CoordTopocentric look = moscow.GetLookAngle(sgp4.FindPosition(t));
            const double el = libsgp4::Util::RadiansToDegrees(look.elevation);
            const double az = libsgp4::Util::RadiansToDegrees(look.azimuth);

            if (el > 0 && !in_pass)
            {
                in_pass = true;
                p = Pass{};
                p.rise = t;
                p.rise_az = az;
            }
            if (!in_pass)
                continue;
            if (el > 10 && !p.reached10)
            {
                p.reached10 = true;
                p.above10 = t;
            }
            if (el > p.max_el)
            {
                p.max_el = el;
                p.max_time = t;
            }
            if (el <= 0)
            {
                in_pass = false;
                p.set_az = az;
                ++total;

                libsgp4::Eci sat = sgp4.FindPosition(p.max_time);
                libsgp4::Eci sun = solar.FindPosition(p.max_time);
                const double sun_el =
                    libsgp4::Util::RadiansToDegrees(moscow.GetLookAngle(sun).elevation);
                const bool lit = in_sunlight(sat.Position(), sun.Position());

                if (is_visible(p.max_el, sun_el, lit))
                {
                    ++shown;
                    std::printf(" %s   %-3s → %-3s       %2.0f°\n",
                                msk(p.reached10 ? p.above10 : p.rise).c_str(),
                                compass(p.rise_az), compass(p.set_az), p.max_el);
                }
            }
        }
    }
    catch (const libsgp4::DecayedException&)
    {
        std::cerr << "Орбита устарела, обнови iss.tle\n";
        return 1;
    }

    std::cout << "\nВсего пролётов над горизонтом: " << total << ", показано: " << shown << "\n";
    return 0;
}
