#include "Pch.h"
#include <Windows.h>
#include <sysinfoapi.h>
#include "SMBIOS.h"
 std::vector<std::string> BaseBoardInformation;
 std::string BaseBoardSerial;
 std::vector<std::string> PhysicalMemoryInformation;
 std::vector<std::string> PhysicalMemorySerials;
RawSMBIOSData* GetRawData()
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


SMBIOSStruct* GetNextStruct(SMBIOSStruct* curStruct) {
	char* strings_begin = (char*)curStruct + curStruct->Length;
	char* next_strings = strings_begin + 1;
	while (*strings_begin != NULL || *next_strings != NULL) {
		++strings_begin;
		++next_strings;
	}
	return (SMBIOSStruct*)(next_strings + 1);
}

std::vector<SMBIOSStruct*> GetStructureTable(RawSMBIOSData* rawData) {
	std::vector<SMBIOSStruct*> structure_table;
	SMBIOSStruct* curStruct = (SMBIOSStruct*)rawData->SMBIOSTableData;
	while ((char*)curStruct < (char*)rawData + rawData->Length) {
		structure_table.push_back(curStruct);
		curStruct = GetNextStruct(curStruct);
	}
	return structure_table;
}
std::vector<std::string> ConvertSMBIOSString(SMBIOSStruct* curStruct) {
	std::vector<std::string> strings;
	std::string res = "";
	strings.push_back(res);
	char* cur_char = (char*)curStruct + curStruct->Length;
	SMBIOSStruct* next_struct = GetNextStruct(curStruct);

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
void GetPhysicalMemoryInformation(SMBIOSPhysicalMemory* curStruct, RawSMBIOSData* rawData) {
	std::vector<std::string> strings = ConvertSMBIOSString(curStruct);

	if ((int)curStruct->Size == 0)
		return;

//	std::cout << "Physics Memory Array Information" << std::endl;

	if (rawData->SMBIOSMajorVersion < 2 || (rawData->SMBIOSMajorVersion == 2 && rawData->SMBIOSMinorVersion < 1))
	{
		return;
	}
	
//	std::cout << "\tSize: " << (int)curStruct->Size << std::endl;

//	std::cout << "\tDevice Locator: " << strings[curStruct->DeviceLocator] << std::endl;

	//std::cout << "\tBank Locator: " << strings[curStruct->BankLocator] << std::endl;
	PhysicalMemoryInformation.push_back(std::to_string((int)curStruct->Size));
	PhysicalMemoryInformation.push_back(strings[curStruct->BankLocator]);

	if (rawData->SMBIOSMajorVersion == 2 && rawData->SMBIOSMinorVersion < 3) {
		std::cout << std::endl;
		return;
	}
	PhysicalMemoryInformation.push_back(strings[curStruct->SerialNumber]);
	PhysicalMemoryInformation.push_back(strings[curStruct->AssetTag]);
	PhysicalMemorySerials.push_back(strings[curStruct->SerialNumber]);
	PhysicalMemoryInformation.push_back("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"); // just a seperator
//	std::cout << "\tSerial Number: " << strings[curStruct->SerialNumber] << std::endl;
	//std::cout << "\tAsset Tag: " << strings[curStruct->AssetTag] << std::endl;

	if (rawData->SMBIOSMajorVersion == 2 && rawData->SMBIOSMinorVersion < 6) {
		return;
	}
}
void GetBaseBoardInformation(SMBIOSBaseBoard* curStruct, RawSMBIOSData* rawData) {
	std::vector<std::string> strings = ConvertSMBIOSString(curStruct);
	//std::cout << "Baseboard Information" << std::endl;

//	std::cout << "\tManufacturer: " << strings[curStruct->Manufacturer] << std::endl;
//	std::cout << "\tProduct: " << strings[curStruct->Product] << std::endl;
//	std::cout << "\tSerial Number: " << strings[curStruct->SerialNumber] << std::endl;
	BaseBoardSerial = strings[curStruct->SerialNumber] + strings[curStruct->Product];  // different products could have same serial number, just a better way to deal with this.
	BaseBoardInformation.push_back(strings[curStruct->Manufacturer]);
	BaseBoardInformation.push_back(strings[curStruct->Product]);
	BaseBoardInformation.push_back(strings[curStruct->SerialNumber]);

}
void ConvertData(RawSMBIOSData* rawData, int id) {
	std::vector<SMBIOSStruct*> structureTable = GetStructureTable(rawData);
	switch (structureTable[id]->Type) {
	case 2:
		GetBaseBoardInformation((SMBIOSBaseBoard*)structureTable[id], rawData);
		break;
	case 17:
		GetPhysicalMemoryInformation((SMBIOSPhysicalMemory*)structureTable[id], rawData);
		break;
	}
}
void InitSMBIOS()
{
	std::vector<SMBIOSStruct*> structureTable = GetStructureTable(GetRawData());
	for (int i = 0; i < structureTable.size(); ++i)
	{
		ConvertData(GetRawData(), i);
	}
}