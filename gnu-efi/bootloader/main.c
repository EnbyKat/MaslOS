#include <efi.h>
#include <efilib.h>
#include <elf.h>

typedef unsigned long long size_t;

typedef struct
{
	void* BaseAddress;
	size_t BufferSize;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanLine;
} Framebuffer;


#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04


typedef struct
{
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;

typedef struct
{
	PSF1_HEADER* psf1_Header;
	void* glyphBuffer;
} PSF1_FONT;

typedef struct
{
	int64_t width, height;
	void* imageBuffer;
} ImageFile;



EFI_FILE* LoadFile(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_FILE* LoadedFile;
	
	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* Filesystem;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&Filesystem);


	if (Directory == NULL)
	{
		Filesystem->OpenVolume(Filesystem, &Directory);
	}

	EFI_STATUS s = Directory->Open(Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);


	if (s != EFI_SUCCESS)
	{
		return NULL;
	}

	return LoadedFile;
}






PSF1_FONT* LoadPSF1Font(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_FILE* font = LoadFile(Directory, Path, ImageHandle, SystemTable);

	if (font == NULL)
		return NULL;

	
	PSF1_HEADER* fontHeader;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void**)&fontHeader);
	
	UINTN size = sizeof(PSF1_HEADER);
	font->Read(font, &size, fontHeader);

	if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1)
		return NULL;
	
	UINTN glyphBufferSize = fontHeader->charsize * 256;

	if (fontHeader->mode == 1)
		glyphBufferSize *= 2;
	
	void* glyphBuffer;
	{
		font->SetPosition(font, sizeof(PSF1_HEADER));
		SystemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void**)&glyphBuffer);
		font->Read(font, &glyphBufferSize, glyphBuffer);

	}

	PSF1_FONT* finishedFont;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)&finishedFont);
	finishedFont->psf1_Header = fontHeader;
	finishedFont->glyphBuffer = glyphBuffer;

	return finishedFont;
}


ImageFile* LoadImage(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{

	EFI_FILE* img = LoadFile(Directory, Path, ImageHandle, SystemTable);

	if (img == NULL)
		return NULL;

	ImageFile* image;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(ImageFile), (void**)&image);
	
	// PSF1_HEADER* fontHeader;
	// SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void**)&fontHeader);
	 
	UINTN size = 3*4;
	int32_t arr[3];
	img->Read(img, &size, arr);

	// int32_t arr2[3] = {0, 0, 0};

	// for (int i = 0; i < 3; i++)
	// 	arr2[i] = (((uint32_t)arr[(i*4)+3]) << 24) | (((uint32_t)arr[(i*4)+2]) << 16) | (((uint32_t)arr[(i*4)+1]) << 8) | (((uint32_t)arr[(i*4)+0]) << 0);

	Print(L"Image Data:\n\r");
	Print(L" - Width:  %d\n\r", arr[0]);
	Print(L" - Height: %d\n\r", arr[1]);
	Print(L" - Size:   %d\n\r", arr[2]);

	image->width = arr[0];
	image->height = arr[1];


	{
		img->SetPosition(img, 4*3);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, arr[2], (void**)&image->imageBuffer);
		size = arr[2];
		img->Read(img, &size, (char*)image->imageBuffer);
	}

	// size = 10; //(UINTN)(arr[2] * 0 + 100);
	// char data[arr[2]];
	// img->Read(img, &size, data);
	//Print(L" - First Byte:   %d\n\r", *((char*)image->imageBuffer));
	return image;
}





	Framebuffer framebuffer;

	Framebuffer* InitializeGOP()
	{
		EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
		EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
		EFI_STATUS status;

		status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);

		if (EFI_ERROR(status))
		{
			Print(L"Unable to locate GOP!!!\n\r");
			return NULL;
		}
		else
		{
			Print(L"GOP Located.\n\r");
		}

		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
		UINTN SizeOfInfo, numModes, nativeMode;

		status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &SizeOfInfo, &info);

		numModes = 0;

		if (status == EFI_NOT_STARTED)
			status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
		if (EFI_ERROR(status))
			Print(L"Unable to get native resolution mode!\n\r");
		else
		{
			nativeMode = gop->Mode->Mode;
			numModes = gop->Mode->MaxMode;
		}
		UINTN MODE = nativeMode;
		for (UINTN i = 0; i < numModes; i++)
		{
			status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);
			Print(L"mode %03d width %d height %d format %x %s.\n\r",
				i,
				info->HorizontalResolution,
				info->VerticalResolution,
				info->PixelFormat,
				i == nativeMode ? "(current)" : ""			
			);
			if (info->HorizontalResolution == 1280 && info->VerticalResolution == 720)
				MODE = i;
		}



		if (EFI_ERROR(status))
		{
			Print(L"Unable to locate GOP!!!\n\r");
			return NULL;
		}
		else
		{
			Print(L"GOP Located.\n\r");
		}



		status = uefi_call_wrapper(gop->SetMode, 2, gop, MODE);

		if (EFI_ERROR(status))
		{
			Print(L"Unable to set mode %03d\n\r", 0);
			return NULL;
		}
		else
		{
			Print(L"GOP Located.\n\r");
		}

		framebuffer.BaseAddress = (void*)gop->Mode->FrameBufferBase;
		framebuffer.BufferSize = gop->Mode->FrameBufferSize;
		framebuffer.Width = gop->Mode->Info->HorizontalResolution;
		framebuffer.Height = gop->Mode->Info->VerticalResolution;
		framebuffer.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;


		return &framebuffer;
}





int memcmp(const void* aptr, const void* bptr, size_t n)
{
	const unsigned char *a = aptr, *b = bptr;

	for (size_t i = 0; i < n; i++)
	{
		if (a[i] < b[i])
			return -1;
		else if (a[i]> b[i])
			return 1;		
	}

	return 0;
}

typedef struct
{
	Framebuffer* framebuffer;
	PSF1_FONT* psf1_font;
	ImageFile* bgImage;
	ImageFile* testImage;
	EFI_MEMORY_DESCRIPTOR* mMap;
	UINTN mMapSize;
	UINTN mMapDescSize;
} BootInfo;




EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

	InitializeLib(ImageHandle, SystemTable);
	Print(L"Loading Kernel...\n\r");

	EFI_FILE* Kernel = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable);
	if (Kernel == NULL)
	{
		Print(L"Could not load Kernel!!!\n\r");
	}
	else
	{
		Print(L"Kernel was loaded successfully!\n\r");
	}

	Print(L"Loading Kernel header...\n\r");

	Elf64_Ehdr header;
	{
		UINTN FileInfoSize;
		EFI_FILE_INFO* FileInfo;
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

		UINTN size = sizeof(header);
		Kernel->Read(Kernel, &size, &header);
	}



	if  (
			memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
			header.e_ident[EI_CLASS] != ELFCLASS64 ||
			header.e_ident[EI_DATA]  != ELFDATA2LSB ||
			header.e_type != ET_EXEC ||
			header.e_machine != EM_X86_64 ||
			header.e_version != EV_CURRENT
		)
	{
		if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0)
			Print(L"Kernel header format is bad! (1)\n\r");
		if (header.e_ident[EI_CLASS] != ELFCLASS64)
			Print(L"Kernel header format is bad! (2)\n\r");
		if (header.e_ident[EI_DATA]  != ELFDATA2LSB)
			Print(L"Kernel header format is bad! (3)\n\r");
		if (header.e_type != ET_EXEC)
			Print(L"Kernel header format is bad! (4)\n\r");
		if (header.e_machine != EM_X86_64)
			Print(L"Kernel header format is bad! (5)\n\r");
		if (header.e_version != EV_CURRENT)
			Print(L"Kernel header format is bad! (6)\n\r");

		
	}
	else
	{
		Print(L"Kernel header successfully verified!\n\r");
	}


	Elf64_Phdr* phdrs;
	{
		Kernel->SetPosition(Kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**)&phdrs);
		Kernel->Read(Kernel, &size, phdrs);
	}

	for
	(
		Elf64_Phdr* phdr = phdrs;
		(char*) phdr < (char*) phdrs + header.e_phnum * header.e_phentsize;
		phdr = (Elf64_Phdr*) ((char*)phdr + header.e_phentsize)
	)
	{
		switch(phdr->p_type)
		{
			case PT_LOAD:
			{
				int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				Elf64_Addr segment = phdr->p_paddr;
				SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);

				Kernel->SetPosition(Kernel, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				Kernel->Read(Kernel, &size, (void*)segment);
				break;
			}

		}
	}

	Print(L"Kernel Loaded.\n\r");


	Framebuffer* newBuffer = InitializeGOP();

	// Print(L"Base:   %x\n\rSize:   %x\n\rWidth:  %d\n\rHeight: %d\n\rPixels: %d\n\r", 
	// newBuffer->BaseAddress, 
	// newBuffer->BufferSize, 
	// newBuffer->Width, 
	// newBuffer->Height, 
	// newBuffer->PixelsPerScanLine
	// );

	PSF1_FONT* newFont = LoadPSF1Font(NULL, L"zap-light16.psf", ImageHandle, SystemTable);

	if (newFont == NULL)
	{
		Print(L"Font was not loaded!\n\r");
	}
	else
	{
		Print(L"Font loaded. Char size: %d\n\r", newFont->psf1_Header->charsize);
	}

	ImageFile* image = LoadImage(NULL, L"test.mbif", ImageHandle, SystemTable);

	if (image == NULL)
	{
		Print(L"Image was not loaded!\n\r");
	}
	else
	{
		Print(L"Image loaded. Char size: %d\n\r", (image->height*image->width*4));
	}

	ImageFile* bgImage = LoadImage(NULL, L"bg.mbif", ImageHandle, SystemTable);

	if (bgImage == NULL)
	{
		Print(L"Image was not loaded!\n\r");
	}
	else
	{
		Print(L"Image loaded. Char size: %d\n\r", (image->height*image->width*4));
	}

	EFI_MEMORY_DESCRIPTOR* Map = NULL;
	UINTN MapSize, MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;

	{
		SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (void**)&Map);
		SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
	}


	

	void(*KernelStart)(BootInfo*) = ((__attribute__((sysv_abi)) void(*)(BootInfo*)) header.e_entry);
	
	BootInfo bootInfo;
	bootInfo.framebuffer = newBuffer;
	bootInfo.psf1_font = newFont;
	bootInfo.mMap = Map;
	bootInfo.mMapSize = MapSize;
	bootInfo.mMapDescSize = DescriptorSize;
	bootInfo.testImage = image;
	bootInfo.bgImage = bgImage;


	Print(L"Exiting EFI Bootservices...\n\r");

	SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);

	KernelStart(&bootInfo);


	return EFI_SUCCESS;
}
