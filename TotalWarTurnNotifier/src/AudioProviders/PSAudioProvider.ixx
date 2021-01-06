module;
#include <Windows.h>
#pragma comment(lib, "Winmm.lib")

#include <exception>
export module PSAudioProvider;
import IAudioProvider;

export class PSAudioProvider : IAudioProvider
{
	public:
	void PlayFile(const char* path) override
	{
		size_t convertNum;
		wchar_t wStr[256];
		mbstowcs_s(&convertNum, wStr, path, 256);
		if (!PlaySound(wStr, NULL, SND_FILENAME | SND_NODEFAULT))
			throw std::exception("PlaySound returned false.");
	}
};