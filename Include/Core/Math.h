#pragma once 

namespace Core {
	class Math
	{
	public :
		Math();
		~Math();

		static void			Init();

		//Random
		static volatile void		SetRandSeed(DWORD seed);
		static DWORD				GetRandSeed()			 { return msRandSeed; }
		static volatile int			Rand();
		static int					RandRange(int begin, int end);
		static float				RandFloat();		//0.0 ~ 1.0

		//TODO : Add Interpolation Functions
		//

	protected :
		
		static	bool				msbInit;
		static  volatile DWORD		msRandSeed;

	};
}
