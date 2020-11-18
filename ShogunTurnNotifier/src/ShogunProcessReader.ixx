module;

#include <cstdint>

export module ShogunProcessReader;

import ProcessReader;

export class ShogunProcessReader : private ProcessReader
{
private:
	const uintptr_t MyTurnBoolAddressOffset = 0x35E31FCA;

	const wchar_t* ShogunProccessName = L"Shogun2.exe";

	void Init()
	{

	}

public:
	~ShogunProcessReader()
	{
	}

	ShogunProcessReader()
	{
		Init();
	}

	bool IsMyTurn()
	{
		bool value = ReadFromProc<bool>(ShogunProccessName, MyTurnBoolAddressOffset);
		return value;
	}
};