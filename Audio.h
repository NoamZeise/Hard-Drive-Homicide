#pragma once
#ifndef AUDIO_H
#define AUDIO_H


#if defined(_WIN32) || defined(WIN32)

#include <Windows.h>

#include <string>
#include <thread>
class Audio
{
public:
	Audio()
	{
	}
	~Audio()
	{
	}
private:

	static void play()
	{
		//mciSendString(L"play \"porch.wav\"", NULL, 0, 0);
	}
	std::string name;
};




#elif defined(__unix__)  //windows end



#endif //os end
#endif; //header guard