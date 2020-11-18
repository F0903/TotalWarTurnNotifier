module;

#include <Windows.h>
#include <TlHelp32.h>

#include <exception>
#include <string>

export module ProcessReader;

export class ProcessReader
{
	struct ProcessInfo
	{
		~ProcessInfo()
		{
			CloseHandle(process);
		}

		HANDLE process;
		DWORD id;
		uintptr_t baseAddress;
	};

public:
	ProcessInfo* GetProcessByName(const wchar_t* str) const
	{
		DWORD pid = 0;

		PROCESSENTRY32 entry{};
		entry.dwSize = sizeof(entry);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		while (Process32Next(snapshot, &entry))
		{
			if (_wcsicmp(entry.szExeFile, str) != 0)
				continue;

			HANDLE proc = OpenProcess(PROCESS_VM_READ, FALSE, entry.th32ProcessID);

			CloseHandle(snapshot);
			return new ProcessInfo { proc, entry.th32ProcessID, 0 };
		}

		CloseHandle(snapshot);
		throw std::exception("No process was found.");
	}

	uintptr_t GetModuleBaseAddress(DWORD processId, const wchar_t* moduleName) const
	{
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
		if (snapshot == INVALID_HANDLE_VALUE)
			throw std::exception("Snapshot module handle was invalid.");

		MODULEENTRY32 entry{};
		entry.dwSize = sizeof(entry);
		while (Module32Next(snapshot, &entry))
		{
			if (_wcsicmp(moduleName, entry.szModule) != 0)
				continue;

			CloseHandle(snapshot);
			return (uintptr_t)entry.modBaseAddr;
		}
		throw std::exception("No module was found.");
	}

	ProcessInfo* GetProcessInfo(const wchar_t* processName) const
	{
		auto* info = GetProcessByName(processName);
		info->baseAddress = GetModuleBaseAddress(info->id, processName);
		return info;
	}

	template<class T>
	T ReadFromProc(const wchar_t* processName, uintptr_t addressToRead) const
	{
		auto* info = GetProcessInfo(processName);
		constexpr size_t bufSize = 1;
		char buf[bufSize];
		SIZE_T count;
		if (!ReadProcessMemory(info->process, (LPCVOID)(info->baseAddress + addressToRead), buf, bufSize, &count))
		{
			auto err = GetLastError();
			throw std::exception("Could not read from process.");
		}
		delete info;
		if (count < 1)
			throw std::exception("No bytes were read.");
		return static_cast<T>(buf[0]);
	}
};