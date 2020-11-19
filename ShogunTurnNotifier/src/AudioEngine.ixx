module;
#include <mmdeviceapi.h>
#include <Audioclient.h>

#include <exception>
export module AudioEngine;

export class AudioEngine
{
public:
	~AudioEngine()
	{
		deviceEnumerator->Release();
		selectedDevice->Release();
		audioClient->Release();
	}

private:
	static constexpr REFERENCE_TIME ReftimesPerSec = 10000000;
	static constexpr REFERENCE_TIME ReftimesPerMillisec = 10000;

	static inline void Error(const char* msg) { throw std::exception(msg); }

	IMMDeviceEnumerator* deviceEnumerator;
	IMMDevice* selectedDevice;
	IAudioClient* audioClient;

	void SelectDefaultAudioOut()
	{
		const CLSID mmDeviceEnumeratorCLSID = __uuidof(MMDeviceEnumerator);
		const IID immDeviceEnumeratorIID = __uuidof(IMMDeviceEnumerator);
		
		if (CoCreateInstance(mmDeviceEnumeratorCLSID, NULL, CLSCTX_ALL, immDeviceEnumeratorIID, (void**)&deviceEnumerator) != S_OK)
			Error("Could not create audio enumerator instance.");

		if (deviceEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, &selectedDevice) != S_OK)
			Error("Could not get default audio device.");
	}

	void CreateStream()
	{
		const IID iAudioClientIID = __uuidof(IAudioClient);
		const IID iAudioRenderClientIID = __uuidof(IAudioRenderClient);

		if (selectedDevice->Activate(iAudioClientIID, CLSCTX_ALL, NULL, (void**)&audioClient) != S_OK)
			Error("Could not activate audio device.");

		WAVEFORMATEX* mixFormat;
		if (audioClient->GetMixFormat(&mixFormat) != S_OK)
			Error("Could not get mix format.");

		if (audioClient->Initialize(AUDCLNT_SHAREMODE::AUDCLNT_SHAREMODE_SHARED, NULL, ReftimesPerSec, 0, mixFormat, NULL) != S_OK)
			Error("Could not initialize audio client.");
		
		UINT32 bufferFrameCount;
		if(audioClient->GetBufferSize(&bufferFrameCount) != S_OK)
			Error("Could not get buffer size.");

		IAudioRenderClient* renderer;
		if (audioClient->GetService(iAudioRenderClientIID, (void**)&renderer) != S_OK)
			Error("Could not get audio renderer service.");
		
		BYTE* data;
		if (renderer->GetBuffer(bufferFrameCount, &data) != S_OK)
			Error("Could not get buffer");
	}
};