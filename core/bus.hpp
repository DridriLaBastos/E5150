#ifndef BUS_HPP
#define BUS_HPP

namespace E5150
{
	template <unsigned int LINE_NUMBER>
	class BUS
	{
		public:
			BUS(const unsigned int initialValue = 0)
			{
				mMask = computeMask();
				setValue(initialValue);
			}

			operator unsigned int (void) { return mValue; }
			operator const unsigned int (void) const { return mValue; }
			template <typename DATA_TYPE>
			void operator= (const DATA_TYPE& newValue)
			{ if constexpr (sizeof(DATA_TYPE) * 8 <= LINE_NUMBER) { mValue = newValue; } else { setValue(newValue); } }
		
		private:
			unsigned int computeMask() const
			{
				unsigned int mask = 0;
				for (int i = 0; i < LINE_NUMBER; ++i)
					mask |= 1 << i;
				return mask;
			}

			void setValue (const unsigned int newValue)
			{ mValue = newValue & mMask; }

		private:
			unsigned int mMask;
			unsigned int mValue;
	};
}

#endif