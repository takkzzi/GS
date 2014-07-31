#pragma once 

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

namespace Core {
	class Math
	{
	public :
		Math();
		~Math();

		static void			Init();

		//Random
		static void			SetRandSeed(DWORD seed);

		static int			Rand();
		static int			RandRange(int begin, int end);
		static float		RandFloat();		//0.0 ~ 1.0

		//TODO : Add Interpolation Functions
		//

	protected :
		
		static	bool		msbInit;
		static  DWORD		msRandSeed;

	};
}

#ifdef __cpluscplus
}
#endif // __cpluscplus