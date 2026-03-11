#include "Interval.hpp"


namespace OWC
{
	const Interval Interval::Empty(std::numeric_limits<f32>::max(), -std::numeric_limits<f32>::max());
	const Interval Interval::Univers(-std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max());
}
