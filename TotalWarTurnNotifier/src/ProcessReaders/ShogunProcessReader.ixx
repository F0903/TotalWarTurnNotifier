module;

#include <cstdint>

export module ShogunProcessReader;

import ProcessReader;

export class ShogunProcessReader : ProcessReader
{
	private:
	const uintptr_t MyTurnBoolAddressOffset = 0; //TODO: Find better address

	const wchar_t* ProccessName = L"Shogun2.exe";

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

	const bool IsMyTurn() const
	{
		bool value = ReadFromProc<bool>(ProccessName, MyTurnBoolAddressOffset);
		return value;
	}
};