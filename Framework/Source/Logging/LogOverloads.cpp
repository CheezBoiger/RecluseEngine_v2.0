
#include "Recluse/Logger.hpp"
#include "Recluse/System/DateTime.hpp"

#include "Recluse/Math/Matrix22.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Matrix43.hpp"
#include "Recluse/Math/Matrix33.hpp"
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Vector2.hpp"

#include <iomanip>
#include <sstream>

namespace Recluse {

const char* DateFormatter::kDefaultFormat = "%Y-%M-%D %h:%m:%s";


Log& Log::operator<<(const DateFormatter& formatter)
{
    data.time = formatter.getFormattedString();
    return (*this);
}


void Log::stringify(const Math::Float2& f2)
{
    std::string str = "(" + std::to_string(f2[0]) + "," + std::to_string(f2[1]) + ")";
    data.msg += str;
}


void Log::stringify(const Math::Float3& f3)
{
    std::string str = "(" + std::to_string(f3[0]) + "," + std::to_string(f3[1]) + "," + std::to_string(f3[2]) + ")";
    data.msg += str;
}


void Log::stringify(const Math::Float4& f4)
{
    std::string str = "(" + std::to_string(f4[0]) + "," + std::to_string(f4[1]) + "," + std::to_string(f4[2]) + "," + std::to_string(f4[3]) + ")";
    data.msg += str;
}


template<typename Matrix>
static void stringifyMatrix(Log& log, const Matrix& m, U32 r, U32 c)
{
    std::ostringstream oss;
    for (size_t row = 0; row < r; ++row) 
    {
        for (size_t col = 0; col < c; ++col) 
        {
            oss << std::setw(15) << m(row, col);
        }
        oss << "\n";
    }
    log.data.msg += oss.str();
}

void Log::stringify(const Math::Matrix44& m44)
{
    stringifyMatrix(*this, m44, 4, 4);
}


void Log::stringify(const Math::Matrix43& m43)
{
    stringifyMatrix(*this, m43, 4, 3);
}


void Log::stringify(const Math::Matrix22& m22)
{
    stringifyMatrix(*this, m22, 2, 2);
}


void Log::stringify(const Math::Matrix33& m33)
{
    stringifyMatrix(*this, m33, 3, 3);
}
} // Recluse