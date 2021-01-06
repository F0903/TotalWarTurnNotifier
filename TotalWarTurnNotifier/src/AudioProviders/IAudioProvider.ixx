module;

export module IAudioProvider;

export class IAudioProvider
{
	public:	
	void virtual PlayFile(const char* path) = 0;
};