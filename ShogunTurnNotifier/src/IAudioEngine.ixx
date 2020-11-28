module;

export module IAudioEngine;

export class IAudioEngine
{
	public:	
	void virtual PlayFile(const char* path) = 0;
};