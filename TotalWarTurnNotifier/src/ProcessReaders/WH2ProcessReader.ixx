module;
#include <cstdint>
export module WH2ProcessReader;

import ProcessReader;

export class WH2ProcessReader : ProcessReader
{
	const uintptr_t AttentionRequiredAddress = 0x7FF4DAAA0404; //TODO: Find better address

	const wchar_t* ProccessName = L"Warhammer2.exe";

	public:
	const bool IsMyTurn() const
	{
		bool value = ReadFromProc<bool>(ProccessName, AttentionRequiredAddress);
		return value;
	}
};