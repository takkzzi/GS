#pragma once

namespace Core 
{
	template<class T> class Singleton 
	{
	protected :
		Singleton()				{};
		virtual ~Singleton()	{};
	
	public:
		static T*			Create() {
			if ( msSingleton )
				msSingleton = new T;
			return msSingleton;
		}

		static T*			Instance()		{ return msSingleton; } 
		static void			Destroy()		{
			if ( msSingleton ) {
				delete msSingleton;
				msSingleton = NULL;
			}
		}

	protected:
		static T*			msSingleton;
	};

	template<class T> T* Singleton<T>::msSingleton = NULL;
}

