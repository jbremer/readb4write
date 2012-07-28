#include <stdio.h>
#include <string.h>
#include "pin.h"

namespace W {
    #include <windows.h>
}

// we use 0x8000 chunks of 128k to taint
UINT32 *taint[0x8000];

inline UINT32 *taint_get(ADDRINT addr)
{
    // allocate memory to taint these memory pages
    if(taint[addr / 0x20000] == NULL) {
        // we need an 16kb page to track 128k of memory
        taint[addr / 0x20000] = (UINT32 *) W::VirtualAlloc(NULL, 0x20000 / 8,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }

    return taint[addr / 0x20000];
}

// taint this chunk of memory as undefined
void taint_undefined(ADDRINT addr, UINT32 size)
{
    // taint the addresses as undefined, pretty slow, but easiest to implement
    for (int i = 0; i < size; i++) {
        UINT32 *t = taint_get(addr + i);
        UINT32 index = (addr + i) % 0x20000;

        // undefine this bit
        t[index / 32] &= ~(1 << (index % 32));
    }
}

// check if we can read this memory address, i.e. if it has been written to
// already
void taint_check(ADDRINT ip, ADDRINT addr, UINT32 size)
{
    // check if the addresses are defined
    for (int i = 0; i < size; i++) {
        UINT32 *t = taint_get(addr + i);
        UINT32 index = (addr + i) % 0x20000;

        // check if this address is undefined and check if this instruction
        // pointer lies within our executable
        if((t[index / 32] & (1 << (index % 32))) == 0 &&
                (ip & 0xfff00000) == 0x00400000) {
            printf("untainted address 0x%08x is being read @ 0x%08x..\n",
                addr + i, ip);
            return;
        }
    }
}

// taint this address as written
void taint_define(ADDRINT addr, UINT32 size)
{
    // taint the addresses as defined, pretty slow, but easiest to implement
    for (int i = 0; i < size; i++) {
        UINT32 *t = taint_get(addr + i);
        UINT32 index = (addr + i) % 0x20000;

        // define this bit
        t[index / 32] |= 1 << (index % 32);
    }
}

void instruction(INS ins, void *v)
{
    // check if the stack pointer is altered (i.e. memory is allocated on the
    // stack by subtracting an immediate from the stack pointer)
    if(INS_Opcode(ins) == XED_ICLASS_SUB &&
            INS_OperandReg(ins, 0) == REG_STACK_PTR &&
            INS_OperandIsImmediate(ins, 1)) {
        // insert call after, so we can pass the stack pointer directly
        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR) &taint_undefined,
            IARG_REG_VALUE, REG_STACK_PTR, IARG_ADDRINT,
            (UINT32) INS_OperandImmediate(ins, 1), IARG_END);
    }

    for (int i = 0; i < INS_MemoryOperandCount(ins); i++) {
        // we want to read memory
        if(INS_MemoryOperandIsRead(ins, i)) {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) &taint_check,
                IARG_INST_PTR, IARG_MEMORYOP_EA, i, IARG_MEMORYREAD_SIZE,
                IARG_END);
        }
        // we want to write memory
        if(INS_MemoryOperandIsWritten(ins, i)) {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) &taint_define,
                IARG_MEMORYOP_EA, i, IARG_MEMORYWRITE_SIZE, IARG_END);
        }
    }
}

TLS_KEY alloc_key;

void alloc_before(THREADID tid, UINT32 size)
{
    // temporary store the size to be allocated in TLS because we cannot pass
    // function arguments to IPOINT_BEFORE callbacks
    PIN_SetThreadData(alloc_key, (void *) size, tid);
}

void alloc_after(THREADID tid, ADDRINT addr)
{
    UINT32 size = (UINT32) PIN_GetThreadData(alloc_key, tid);
    if(addr != 0 && size != 0) {
        taint_undefined(addr, size);
    }
}

void image(IMG img, void *v)
{
    // functions such as malloc() are wrappers around RtlAllocateHeap()
    for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym)) {
        if(PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY) ==
                "RtlAllocateHeap") {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if(RTN_Valid(rtn)) {
                RTN_Open(rtn);

                // second parameter defines the size to be allocated
                RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR) &alloc_before,
                    IARG_THREAD_ID, IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                    IARG_END);

                // return value is the address that has been allocated
                RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR) &alloc_after,
                    IARG_THREAD_ID, IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

                RTN_Close(rtn);
            }
        }
    }

    // we assume that the entire image is initialized data (to ignore a lot
    // of false positives)
    taint_define(IMG_LowAddress(img),
        IMG_HighAddress(img) - IMG_LowAddress(img));
}

int main(int argc, char *argv[])
{
    if(PIN_Init(argc, argv)) {
        printf("Pintool to detect read before write access\n");
        return 0;
    }

    PIN_InitSymbols();

    alloc_key = PIN_CreateThreadDataKey(0);

    INS_AddInstrumentFunction(&instruction, NULL);
    IMG_AddInstrumentFunction(&image, NULL);

    PIN_StartProgram();
    return 0;
}
