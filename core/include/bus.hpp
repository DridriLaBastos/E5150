#ifndef BUS_HPP
#define BUS_HPP

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
		void operator= (const unsigned int newValue) { setValue(newValue); }
	
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

#endif