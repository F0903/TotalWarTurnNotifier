module;
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <functiondiscoverykeys_devpkey.h>

#include <exception>
#include <iostream>
#include <fstream>
export module AudioEngine;

export class AudioEngine
{
public:
	~AudioEngine()
	{
		deviceEnumerator->Release();
		selectedDevice->Release();
		audioClient->Release();
		CoUninitialize();
	}

	AudioEngine()
	{
		InitializeCOM();
		SelectDefaultAudioOut();
	}

private:
	static constexpr REFERENCE_TIME ReftimesPerSec = 10000000;
	static constexpr REFERENCE_TIME ReftimesPerMillisec = ReftimesPerSec / 1000;

	static constexpr CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	static constexpr IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	static constexpr IID IID_IAudioClient = __uuidof(IAudioClient);
	static constexpr IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

	IMMDeviceEnumerator* deviceEnumerator;
	IMMDevice* selectedDevice;
	IAudioClient* audioClient;

	static inline void Error(const char* msg) { throw std::exception(msg); }

	void InitializeCOM()
	{
		if (CoInitializeEx(NULL, COINIT::COINIT_MULTITHREADED) != S_OK)
			Error("Could not initialize COM");
	}

	void SelectDefaultAudioOut()
	{
		if (CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&deviceEnumerator) != S_OK)
			Error("Could not create audio enumerator instance.");

		if (deviceEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, &selectedDevice) != S_OK)
			Error("Could not get default audio device.");

		IPropertyStore* properties;
		selectedDevice->OpenPropertyStore(STGM_READ, &properties);

		PROPVARIANT prop;
		PropVariantInit(&prop);
		properties->GetValue(PKEY_Device_FriendlyName, &prop);

		std::wcout << "Selected device: " << prop.pwszVal << '\n';

		PropVariantClear(&prop);
		properties->Release();
	}

	//Ref: https://docs.microsoft.com/en-us/windows/win32/coreaudio/rendering-a-stream
	void OutputStream(std::istream& writeStream)
	{
		if (selectedDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&audioClient) != S_OK)
			Error("Could not activate audio device.");

		WAVEFORMATEX* mixFormat;
		if (audioClient->GetMixFormat(&mixFormat) != S_OK)
			Error("Could not get mix format.");

		constexpr auto shareMode = AUDCLNT_SHAREMODE::AUDCLNT_SHAREMODE_SHARED;

		WAVEFORMATEX* closestMatch;
		const auto result = audioClient->IsFormatSupported(shareMode, mixFormat, &closestMatch);
		switch (result)
		{
			case S_OK:
				break;
			case S_FALSE:
				CoTaskMemFree(mixFormat);
				mixFormat = closestMatch;
				break;
			default:
				Error("Audio format is not supported, and a closest match could not be found.");
				break;
		}

		//AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
		if (audioClient->Initialize(shareMode, 0, ReftimesPerSec, 0, mixFormat, NULL) != S_OK)
			Error("Could not initialize audio client.");

		// Get the actual size of the allocated buffer.
		UINT32 bufferFrameCount;
		if (audioClient->GetBufferSize(&bufferFrameCount) != S_OK)
			Error("Could not get buffer size.");

		IAudioRenderClient* renderClient;
		if (audioClient->GetService(IID_IAudioRenderClient, (void**)&renderClient) != S_OK)
			Error("Could not get audio renderer service.");

		// Grab the entire buffer for the initial fill operation.
		BYTE* data;
		if (renderClient->GetBuffer(bufferFrameCount, &data) != S_OK)
			Error("Could not get buffer");

		// Load the initial data into the shared buffer.
		const auto writeAmount = bufferFrameCount * static_cast<UINT32>(mixFormat->nBlockAlign);
		writeStream.read(reinterpret_cast<char*>(data), writeAmount);

		DWORD bufferFlags = NULL;
		if (renderClient->ReleaseBuffer(bufferFrameCount, bufferFlags) != S_OK)
			Error("Could not release buffer");

		// Calculate the actual duration of the allocated buffer.
		const REFERENCE_TIME actualDuration = (double)ReftimesPerSec * bufferFrameCount / mixFormat->nSamplesPerSec;

		if (audioClient->Start() != S_OK)
			Error("Could not start playing.");

		const DWORD sleepDuration = (DWORD)(actualDuration / ReftimesPerMillisec / 2);

		UINT32 currentFramesPadding = 0;
		UINT32 currentFramesAvailable = 0;
		while (writeStream.good())
		{
			// Sleep for half of the buffer duration.
			Sleep(sleepDuration);

			// See how much buffer space is available.
			if (audioClient->GetCurrentPadding(&currentFramesPadding) != S_OK)
				Error("Could not get current padding.");

			currentFramesAvailable = bufferFrameCount - currentFramesPadding;

			// Grab all the available space in the shared buffer.
			if (renderClient->GetBuffer(currentFramesAvailable, &data) != S_OK)
				Error("Could not get buffer (2)");

			const auto writeAmount = currentFramesAvailable * static_cast<UINT32>(mixFormat->nBlockAlign);
			writeStream.read(reinterpret_cast<char*>(data), writeAmount);

			if (renderClient->ReleaseBuffer(currentFramesAvailable, bufferFlags) != S_OK)
				Error("Could not release stream (2)");
		}

		// Wait for last data in buffer to play before stopping.
		Sleep(sleepDuration);

		if (audioClient->Stop() != S_OK)
			Error("Could not stop audio client.");

		CoTaskMemFree(mixFormat);
		renderClient->Release();

		const auto err = GetLastError();
		if (err != S_OK)
		{
			std::cerr << "OutputStream terminated with error code: " << err << '\n';
			return;
		}
	}

public:
	void PlayFile(const char* path)
	{
		std::ifstream f(path, std::ios_base::in | std::ios_base::binary);
		if (!f)
			Error("Something went wrong whilst opening the audio file.");
		OutputStream(f);
	}
};