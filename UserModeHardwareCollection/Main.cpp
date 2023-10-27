#include "pch.h"
#include <Windows.h>
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
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


void main()
{
	

	std::cout << GetCpuInfo() << std::endl;
}