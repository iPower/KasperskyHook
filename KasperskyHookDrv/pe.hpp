#pragma once

#include <ntifs.h>
#include <windef.h>

typedef struct _IMAGE_DOS_HEADER
{
	WORD e_magic;
	WORD e_cblp;
	WORD e_cp;
	WORD e_crlc;
	WORD e_cparhdr;
	WORD e_minalloc;
	WORD e_maxalloc;
	WORD e_ss;
	WORD e_sp;
	WORD e_csum;
	WORD e_ip;
	WORD e_cs;
	WORD e_lfarlc;
	WORD e_ovno;
	WORD e_res[4];
	WORD e_oemid;
	WORD e_oeminfo;
	WORD e_res2[10];
	LONG e_lfanew;
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;


typedef struct _IMAGE_FILE_HEADER
{
	WORD  Machine;
	WORD  NumberOfSections;
	DWORD TimeDateStamp;
	DWORD PointerToSymbolTable;
	DWORD NumberOfSymbols;
	WORD  SizeOfOptionalHeader;
	WORD  Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;


typedef struct _IMAGE_DATA_DIRECTORY
{
	DWORD VirtualAddress;
	DWORD Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;


typedef struct _IMAGE_OPTIONAL_HEADER64
{
	WORD	             Magic;
	BYTE	             MajorLinkerVersion;
	BYTE	             MinorLinkerVersion;
	DWORD	             SizeOfCode;
	DWORD	             SizeOfInitializedData;
	DWORD	             SizeOfUninitializedData;
	DWORD	             AddressOfEntryPoint;
	DWORD	             BaseOfCode;
	ULONGLONG            ImageBase;
	DWORD	             SectionAlignment;
	DWORD	             FileAlignment;
	WORD	             MajorOperatingSystemVersion;
	WORD	             MinorOperatingSystemVersion;
	WORD	             MajorImageVersion;
	WORD	             MinorImageVersion;
	WORD	             MajorSubsystemVersion;
	WORD	             MinorSubsystemVersion;
	DWORD	             Win32VersionValue;
	DWORD	             SizeOfImage;
	DWORD	             SizeOfHeaders;
	DWORD	             CheckSum;
	WORD	             Subsystem;
	WORD	             DllCharacteristics;
	ULONGLONG            SizeOfStackReserve;
	ULONGLONG            SizeOfStackCommit;
	ULONGLONG            SizeOfHeapReserve;
	ULONGLONG            SizeOfHeapCommit;
	DWORD	             LoaderFlags;
	DWORD                NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;


typedef struct _IMAGE_NT_HEADERS64
{
	ULONG                   Signature;
	IMAGE_FILE_HEADER       FileHeader;
	IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;


typedef struct _IMAGE_SECTION_HEADER
{
	char    Name[8];
	ULONG   VirtualSize;
	ULONG   VirtualAddress;
	ULONG   SizeOfRawData;
	ULONG   PointerToRawData;
	ULONG   PointerToRelocations;
	ULONG   PointerToLinenumbers;
	__int16 NumberOfRelocations;
	__int16 NumberOfLinenumbers;
	ULONG   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

namespace pe
{
	PIMAGE_SECTION_HEADER get_section_header( const uintptr_t image_base, const char* section_name );
}