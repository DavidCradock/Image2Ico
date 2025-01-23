#pragma once

namespace X
{
	/// \brief Template class for a dynamic array like std::vector
	template < class Datatype>
	class Array
	{
	public:
		Datatype * m_array;
		int m_size;
	};
}