#include "pch.h"
#include <Windows.h>
#include <dxgi.h>
#include <tchar.h>
#include <sysinfoapi.h>
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
struct SMBIOSStruct {
	BYTE    Type;
	BYTE    Length;
	WORD    Handle;
};
struct SMBIOSStructType2 : SMBIOSStruct {
	BYTE    Manufacturer;
	BYTE    Product;
	BYTE    Version;
	BYTE    SerialNumber;
	BYTE    AssetTag;
	BYTE    FeatureFlags;
	BYTE    LocationInChassis;
	WORD    ChassisHandle;
	BYTE    BoardType;
	BYTE    NumberOfContainedObjectHandles;
	BYTE    ContainedObjectHandles[256];
};
struct RawSMBIOSData {
	BYTE    Used20CallingMethod;
	BYTE    SMBIOSMajorVersion;
	BYTE    SMBIOSMinorVersion;
	BYTE    DmiRevision;
	DWORD   Length;
	BYTE    SMBIOSTableData[];
};

struct SMBIOSStructType17 : SMBIOSStruct {
	WORD    PhysicalMemoryArrayHandle;
	WORD    MemoryErrorInformationHandle;
	WORD    TotalWidth;
	WORD    DataWidth;
	WORD    Size;
	BYTE    FormFactor;
	BYTE    DeviceSet;
	BYTE    DeviceLocator;
	BYTE    BankLocator;
	BYTE    MemoryType;
	WORD    TypeDetail;
	WORD    Speed;
	BYTE    Manufacturer;
	BYTE    SerialNumber;
	BYTE    AssetTag;
	BYTE    PartNumber;
	BYTE    Attributes;
	BYTE    ExtendedSize[4];
	WORD    ConfiguredMemoryClockSpeed;
	WORD    MinimumVoltage;
	WORD    MaximumVoltage;
	WORD    ConfiguredVoltage;
};
RawSMBIOSData* getRawData() 
{
	DWORD error = ERROR_SUCCESS;
	DWORD smBiosDataSize = 0;
	RawSMBIOSData* smBiosData = NULL;
	DWORD bytesWritten = 0;

	// Make sure that there is enough space in the heap for this table
	smBiosDataSize = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
	smBiosData = (RawSMBIOSData*)HeapAlloc(GetProcessHeap(), 0, smBiosDataSize);
	if (!smBiosData) {
		error = ERROR_OUTOFMEMORY;
		exit(1);
	}

	// Make sure that the data used is valid (by checking the amount of data received)
	bytesWritten = GetSystemFirmwareTable('RSMB', 0, smBiosData, smBiosDataSize);
	if (bytesWritten != smBiosDataSize) {
		error = ERROR_INVALID_DATA;
		exit(1);
	}

	return smBiosData;
}


SMBIOSStruct* getNextStruct(SMBIOSStruct* curStruct) {
	char* strings_begin = (char*)curStruct + curStruct->Length;
	char* next_strings = strings_begin + 1;
	while (*strings_begin != NULL || *next_strings != NULL) {
		++strings_begin;
		++next_strings;
	}
	return (SMBIOSStruct*)(next_strings + 1);
}

std::vector<SMBIOSStruct*> getStructureTable(RawSMBIOSData* rawData) {
	std::vector<SMBIOSStruct*> structure_table;
	SMBIOSStruct* curStruct = (SMBIOSStruct*)rawData->SMBIOSTableData;
	while ((char*)curStruct < (char*)rawData + rawData->Length) {
		structure_table.push_back(curStruct);
		curStruct = getNextStruct(curStruct);
	}
	return structure_table;
}
std::vector<std::string> getStrings(SMBIOSStruct* curStruct) {
	std::vector<std::string> strings;
	std::string res = "";
	strings.push_back(res);
	char* cur_char = (char*)curStruct + curStruct->Length;
	SMBIOSStruct* next_struct = getNextStruct(curStruct);

	while (cur_char < (char*)next_struct) {
		res.push_back(*cur_char);
		if (*cur_char == NULL) {
			strings.push_back(res);
			res = "";
		}
		++cur_char;
	}
	return strings;
}
void displayInformation(SMBIOSStructType17* curStruct, RawSMBIOSData* rawData) {
	std::vector<std::string> strings = getStrings(curStruct);
	std::cout << "Physics Memory Array Information (Type " << (int)curStruct->Type << ")" << std::endl;

	if (rawData->SMBIOSMajorVersion < 2 || (rawData->SMBIOSMajorVersion == 2 && rawData->SMBIOSMinorVersion < 1)) 
	{
		std::cout << "--------------------------------------------------------" << std::endl;
		return;
	}
	std::cout << "\tSize: " << (int)curStruct->Size << std::endl;

	std::cout << "\tDevice Locator: " << strings[curStruct->DeviceLocator] << std::endl;

	std::cout << "\tBank Locator: " << strings[curStruct->BankLocator] << std::endl;



	if (rawData->SMBIOSMajorVersion == 2 && rawData->SMBIOSMinorVersion < 3) {
		std::cout << std::endl;
		return;
	}


	std::cout << "\tSerial Number: " << strings[curStruct->SerialNumber] << std::endl;
	std::cout << "\tAsset Tag: " << strings[curStruct->AssetTag] << std::endl;

	if (rawData->SMBIOSMajorVersion == 2 && rawData->SMBIOSMinorVersion < 6) {
		std::cout << std::endl;
		return;
	}

	std::cout << std::endl;
}
void displayInformation(SMBIOSStructType2* curStruct, RawSMBIOSData* rawData) {
	std::vector<std::string> strings = getStrings(curStruct);
	std::cout << "Baseboard Information (Type " << (int)curStruct->Type << ")" << std::endl;

	std::cout << "\tManufacturer: " << strings[curStruct->Manufacturer] << std::endl;
	std::cout << "\tProduct: " << strings[curStruct->Product] << std::endl;
	std::cout << "\tSerial Number: " << strings[curStruct->SerialNumber] << std::endl;


	std::cout << std::endl;
}
void displayStructure(RawSMBIOSData* rawData, int id) {
	std::vector<SMBIOSStruct*> structureTable = getStructureTable(rawData);
	switch (structureTable[id]->Type) {
	case 2:
		displayInformation((SMBIOSStructType2*)structureTable[id], rawData);
		break;
	case 17:
		displayInformation((SMBIOSStructType17*)structureTable[id], rawData);
		break;
	}
}
void main()
{
	std::cout << GetCpuInfo() << std::endl;
	std::wcout << GetGPU() << std::endl;
	std::cout << GetTotalMemory() << std::endl;
	std::vector<SMBIOSStruct*> structureTable = getStructureTable(getRawData());
	for (int i = 0; i < structureTable.size(); ++i)
	{
		displayStructure(getRawData(), i);
	}

}