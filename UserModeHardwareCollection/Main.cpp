#include "pch.h"
#include <Windows.h>
#include <dxgi.h>
#include <tchar.h>
#pragma comment(lib, "dxgi.lib")
#include "SMBIOS.h"
#include "WindowsUtilities.h"


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
std::vector<std::string> GetDriveSerialNumbers() 
{
	std::vector<std::string> serialnumbers;
	int drivenumber = 0;
	while (true) 
	{
		std::wstring path = L"\\\\.\\PhysicalDrive" + std::to_wstring(drivenumber);

		// Get a handle to the physical drive
		HANDLE h = CreateFileW(path.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (h == INVALID_HANDLE_VALUE) // End of drives
			break;
		

		// Automatic Cleanup, Smart Pointers
		std::unique_ptr<std::remove_pointer<HANDLE>::type, void(*)(HANDLE)> hDevice{ h, [](HANDLE handle) { CloseHandle(handle); } };

		STORAGE_PROPERTY_QUERY storagepropertyquery{};
		storagepropertyquery.PropertyId = StorageDeviceProperty;
		storagepropertyquery.QueryType = PropertyStandardQuery;

		STORAGE_DESCRIPTOR_HEADER storagedescriptorheader{};

		DWORD bytesret = 0;
		if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagepropertyquery, sizeof(STORAGE_PROPERTY_QUERY),&storagedescriptorheader, sizeof(STORAGE_DESCRIPTOR_HEADER), &bytesret, NULL))
			continue;
		

		const DWORD buffersize = storagedescriptorheader.Size;
		std::unique_ptr<BYTE[]> buffer{ new BYTE[buffersize]{} };

		
		if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagepropertyquery, sizeof(STORAGE_PROPERTY_QUERY),buffer.get(), buffersize, &bytesret, NULL)) 
			continue;
		

		// Read the serial number out of the output buffer
		STORAGE_DEVICE_DESCRIPTOR* devicedescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(buffer.get());
		const DWORD serialnumberoffset = devicedescriptor->SerialNumberOffset;

		if (serialnumberoffset != 0)
		{
			const char* serialnumber = reinterpret_cast<const char*>(buffer.get() + serialnumberoffset);
			serialnumbers.push_back(serialnumber);
		}
		drivenumber += 1;
	}

	return serialnumbers;
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

	std::cout << "Drive Serial Numbers:\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
	for (const std::string& serialnum : GetDriveSerialNumbers())
	{
		std::cout << serialnum << std::endl;
	}
	
	
}