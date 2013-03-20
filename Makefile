CL = C:/program\ files\ \(x86\)/Microsoft\ Visual\ Studio\ 10.0/VC/bin/cl.exe
LINK = C:/program\ files\ \(x86\)/Microsoft\ Visual\ Studio\ 10.0/VC/bin/link.exe
INC = C:/Program\ Files\ \(x86\)/Microsoft\ SDKs/Windows/v7.0A/Include
LIB = C:/Program\ Files\ \(x86\)/Microsoft\ SDKs/Windows/v7.0A/Lib
LIB2 = C:/Program\ Files\ \(x86\)/Microsoft\ Visual\ Studio\ 10.0/VC/lib

PINTOOL = ..\pintool
DLLS = readb4write.dll

POCSRC = $(wildcard *.c)
POCEXE = $(POCSRC:.c=.exe)

default: $(DLLS) $(POCEXE)

%.exe: %.c
	gcc -std=c99 -O0 -s -o $@ $^
	../pintool/ia32/bin/pin -t readb4write.dll -- $@

%.obj: %.cpp
	cl /c /MT /EHs- /EHa- /wd4530 /DTARGET_WINDOWS /DBIGARRAY_MULTIPLIER=1 \
		/DUSING_XED /D_CRT_SECURE_NO_DEPRECATE /D_SECURE_SCL=0 /nologo /Gy \
		/O2 /DTARGET_IA32 /DHOST_IA32 /I$(PINTOOL)\source\include \
		/I$(PINTOOL)\source\include\gen /I$(PINTOOL)\source\tools\InstLib \
		/I$(PINTOOL)\extras\xed2-ia32\include \
		/I$(PINTOOL)\extras\components\include /I$(INC) $^

%.dll: %.obj
	$(LINK) /DLL /EXPORT:main /NODEFAULTLIB /NOLOGO /INCREMENTAL:NO /OPT:REF \
		/MACHINE:x86 /ENTRY:Ptrace_DllMainCRTStartup@12 /BASE:0x55000000 \
		/LIBPATH:$(PINTOOL)\ia32\lib /LIBPATH:$(PINTOOL)\ia32\lib-ext \
		/LIBPATH:$(PINTOOL)\extras\xed2-ia32\lib /IMPLIB:tz.lib \
		/PDB:tz.pdb /OUT:$@ $^ pin.lib libxed.lib libcpmt.lib \
		libcmt.lib pinvm.lib kernel32.lib ntdll-32.lib /LIBPATH:$(LIB) \
		/LIBPATH:$(LIB2)

clean:
	rm -f '*.dll' '*.exp' '*.lib' '*.exe'
