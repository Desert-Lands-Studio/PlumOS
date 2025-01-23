[Defines]
PLATFORM_NAME           = PlumLoader
PLATFORM_GUID           = e558a0d8-abf7-4eca-8ed0-ca619562d16e
PLATFORM_VERSION        = 1.0
DSC_SPECIFICATION       = 0x00010005
OUTPUT_DIRECTORY        = Build/PlumOS
SUPPORTED_ARCHITECTURES = AARCH64|X64
BUILD_TARGETS           = DEBUG|RELEASE
SKUID_IDENTIFIER        = DEFAULT

[LibraryClasses]
UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
UefiLib|MdePkg/Library/UefiLib/UefiLib.inf

[Components]
PlumOSPkg/PlumOS.inf
