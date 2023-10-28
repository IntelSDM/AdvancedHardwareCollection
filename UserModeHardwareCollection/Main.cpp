#include "pch.h"
#include <Windows.h>
#include <dxgi.h>
#include <tchar.h>

#pragma comment(lib, "dxgi.lib")
#include "SMBIOS.h"
std::string GetCpuInfo()
{
		std::array<int, 4> integerbuffer;
	constexpr size_t sizeofintegerbuffer = sizeof(int) * integerbuffer.size();

	std::array<char, 64> charbuffer = {};
	// https://docs.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=vs-2019
	constexpr std::array<int, 3> functionids = {
		// Manufacturer
		0x8000'0002,
		// Model
		0x8000'0003,
		// Clockspeed
		0x8000'0004,
	};

	std::string cpuname;

	for (int id : functionids)
	{
		__cpuid(integerbuffer.data(), id);

		std::memcpy(charbuffer.data(), integerbuffer.data(), sizeofintegerbuffer);

		cpuname += std::string(charbuffer.data());
	}
		return cpuname;
}
// This is more or less a proof of concept, We will always end up using a renderer for the GUI, D2D1, D3D11, OpenGL and all of them will provide the GPU name securely.
std::wstring GetGPU()
{
	IDXGIFactory* factory = nullptr;
	if (!SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)) && factory)
		return L"";
	IDXGIAdapter* adapter = nullptr;
	if (!SUCCEEDED(factory->EnumAdapters(0, &adapter)) && adapter)
		return L"";
	DXGI_ADAPTER_DESC adapterdesc;
	if (!SUCCEEDED(adapter->GetDesc(&adapterdesc)))
		return L"";
	return adapterdesc.Description;

	adapter->Release();
	factory->Release();

}
std::string GetTotalMemory()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	if (!GlobalMemoryStatusEx(&status))
		return "";
		return std::to_string((status.ullTotalPhys / (1024 * 1024)));
}

void main()
{
	std::cout << GetCpuInfo() << std::endl;
	std::wcout << GetGPU() << std::endl;
	std::cout << GetTotalMemory() << std::endl;
	InitSMBIOS();
	for (std::string str : BaseBoardInformation)
	{
			std::cout << str << std::endl;
	}
	std::cout << std::endl;
	for (std::string str : PhysicalMemoryInformation)
	{
		std::cout << str << std::endl;
	}

}