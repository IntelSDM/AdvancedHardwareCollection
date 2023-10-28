#pragma once

struct SMBIOSStruct {
	BYTE    Type;
	BYTE    Length;
	WORD    Handle;
};
struct SMBIOSBaseBoard : SMBIOSStruct {
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
struct SMBIOSPhysicalMemory : SMBIOSStruct {
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
struct RawSMBIOSData {
	BYTE    Used20CallingMethod;
	BYTE    SMBIOSMajorVersion;
	BYTE    SMBIOSMinorVersion;
	BYTE    DmiRevision;
	DWORD   Length;
	BYTE    SMBIOSTableData[];
};
extern std::vector<std::string> BaseBoardInformation; // use for analysis
extern std::string BaseBoardSerial; // use for hardware locking
extern std::vector<std::string> PhysicalMemoryInformation; // use for analysis
extern std::vector<std::string> PhysicalMemorySerials; // use for hardware locking
void InitSMBIOS();