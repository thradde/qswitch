////////////////////////////////////////////////////////////
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
// you must not claim that you wrote the original software.
// If you use this software in a product, an acknowledgment
// in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
// and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "memdebug.h"

// #include <SFML/Graphics/RoundedRectangleShape.hpp>
#include "RoundedRectangleShape.h"
#include <cmath>

namespace sf
{
////////////////////////////////////////////////////////////
RoundedRectangleShape::RoundedRectangleShape(const Vector2f& size, float corner_radius, unsigned int cornerPointCount)
{
    mySize = size;
    myRadius = corner_radius;
    myCornerPointCount = cornerPointCount;
	ComputePoints();
	update();
}

////////////////////////////////////////////////////////////
void RoundedRectangleShape::setSize(const Vector2f& size)
{
    mySize = size;
	ComputePoints();
	update();
}

////////////////////////////////////////////////////////////
const Vector2f& RoundedRectangleShape::getSize() const
{
    return mySize;
}

////////////////////////////////////////////////////////////
void RoundedRectangleShape::setCornersRadius(float radius)
{
    myRadius = radius;
	ComputePoints();
	update();
}

////////////////////////////////////////////////////////////
const float RoundedRectangleShape::getCornersRadius() const
{
    return myRadius;
}

////////////////////////////////////////////////////////////
void RoundedRectangleShape::setCornerPointCount(unsigned int count)
{
    myCornerPointCount = count;
	ComputePoints();
	update();
}

////////////////////////////////////////////////////////////
std::size_t RoundedRectangleShape::getPointCount() const
{
    return myCornerPointCount * 4;
}

////////////////////////////////////////////////////////////
#if 0
sf::Vector2f RoundedRectangleShape::getPoint(std::size_t index) const
{
    if (index >= getPointCount())
        return sf::Vector2f(0,0);

    float deltaAngle = 90.0f/(myCornerPointCount-1);
    sf::Vector2f center;
    size_t centerIndex = index/myCornerPointCount;
    static const float pi = 3.141592654f;

    switch(centerIndex)
    {
        case 0: center.x = mySize.x - myRadius; center.y = myRadius; break;
        case 1: center.x = myRadius; center.y = myRadius; break;
        case 2: center.x = myRadius; center.y = mySize.y - myRadius; break;
        case 3: center.x = mySize.x - myRadius; center.y = mySize.y - myRadius; break;
    }

    return sf::Vector2f(myRadius*cos(deltaAngle*(index-centerIndex)*pi/180)+center.x,
                        myRadius*sin(deltaAngle*(index-centerIndex)*pi/180)-center.y);
}
#else
sf::Vector2f RoundedRectangleShape::getPoint(std::size_t index) const
{
	if (index >= m_Points.size())
		return sf::Vector2f(0, 0);

	return m_Points[index];
}
#endif


void RoundedRectangleShape::ComputePoints()
{
	m_Points.clear();

	float X, Y;
	float inc = myRadius / myCornerPointCount;
	float radius_2 = myRadius * myRadius;

	X = 0;
	for (unsigned int i = 0; i < myCornerPointCount; i++)
	{
		X += inc;
		Y = sqrt(radius_2 - X*X);
		m_Points.push_back(sf::Vector2f(X + mySize.x - myRadius, myRadius - Y));
	}

	Y = 0;
	for (unsigned int i = 0; i < myCornerPointCount; i++)
	{
		Y += inc;
		X = sqrt(radius_2 - Y*Y);
		m_Points.push_back(sf::Vector2f(X + mySize.x - myRadius, Y + mySize.y - myRadius));
	}

	X = 0;
	for (unsigned int i = 0; i < myCornerPointCount; i++)
	{
		X += inc;
		Y = sqrt(radius_2 - X*X);
		m_Points.push_back(sf::Vector2f(myRadius - X, Y + mySize.y - myRadius));
	}

	Y = 0;
	for (unsigned int i = 0; i < myCornerPointCount; i++)
	{
		Y += inc;
		X = sqrt(radius_2 - Y*Y);
		m_Points.push_back(sf::Vector2f(myRadius - X, myRadius - Y));
	}
}

}//namespace sf
