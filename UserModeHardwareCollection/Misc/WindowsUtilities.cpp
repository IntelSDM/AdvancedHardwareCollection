#include "pch.h"
#include "WindowsUtilities.h"

bool IsUserAdmin()
{
	BOOL isadmin = FALSE;
	SID_IDENTIFIER_AUTHORITY ntauthority = SECURITY_NT_AUTHORITY;
	PSID administratorsgroup;

	if (!AllocateAndInitializeSid(&ntauthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administratorsgroup)) {
		return false;
	}
	if (!CheckTokenMembership(NULL, administratorsgroup, &isadmin)) {
		FreeSid(administratorsgroup);
		return false;
	}
	FreeSid(administratorsgroup);
	return isadmin != FALSE;
}
std::string ExecuteConsoleCommand(std::string command)
{
	std::array<char, 128> buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(_popen(command.c_str(), "r"), _pclose);
	while (!feof(pipe.get()))
	{
		if (fgets(buffer.data(), 1024, pipe.get()) != NULL)// buffer is 1024, shouldn't be filled by any of our commands but just in case
			result += buffer.data();

	}
	return result;
}