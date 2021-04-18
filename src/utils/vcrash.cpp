#include "vcrash.h"
#include <cstdio>
#include <cstdlib>

// on crash mode, don't print calls made after the crash occured
// on cut setup, don't print calls made before main is called.

void stack_trace(bool crashMode, bool cutSetup,CONTEXT * context) {
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    SymInitialize(process, NULL, TRUE);

    DWORD image;
    STACKFRAME64 stackframe;
    ZeroMemory(&stackframe, sizeof(STACKFRAME64));

#ifdef _M_IX86
    image = IMAGE_FILE_MACHINE_I386;
    stackframe.AddrPC.Offset = context->Eip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context->Ebp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context->Esp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
    image = IMAGE_FILE_MACHINE_AMD64;
    stackframe.AddrPC.Offset = context->Rip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context->Rsp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context->Rsp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
    image = IMAGE_FILE_MACHINE_IA64;
    stackframe.AddrPC.Offset = context->StIIP;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context->IntSp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrBStore.Offset = context->RsBSP;
    stackframe.AddrBStore.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context->IntSp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#endif

    bool printEnable = !crashMode;

    for (size_t i = 0; i < 25; i++) {
        BOOL result = StackWalk64(
            image, process, thread,
            &stackframe, context, NULL,
            SymFunctionTableAccess64, SymGetModuleBase64, NULL);

        if (!result) { break; }

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        char buffer2[sizeof(PIMAGEHLP_LINE64)];
        PIMAGEHLP_LINE64 lineCounter = (PIMAGEHLP_LINE64)buffer2; // contains data about a location in a file (filename, line, col, ...)
        lineCounter->SizeOfStruct = sizeof(PIMAGEHLP_LINE64);

        DWORD64 displacement = 0;
        DWORD displacement32 = 0;

        if (SymFromAddr(process, stackframe.AddrPC.Offset, &displacement, symbol)) {

            if (printEnable) {
                fprintf(stderr, "[%llu] At %s",i, symbol->Name);

                // get line of symbol.
                if (SymGetLineFromAddr64(process, stackframe.AddrPC.Offset, &displacement32, lineCounter)) {
                    fprintf(stderr, " (%s:%li)", lineCounter->FileName, lineCounter->LineNumber);
                } else {
                    fprintf(stderr, " ???");
                }
                fprintf(stderr, "\n");
            }

            if (cutSetup && strcmp(symbol->Name, "main") == 0) {
                break;
            }
            if (!printEnable && (strcmp(symbol->Name, "abort") == 0 || strcmp(symbol->Name, "KiUserExceptionDispatcher") == 0)) {
                printEnable = true;
            }

        } else {
            fprintf(stderr, "[%llu] ???\n", i);
        }

    }
    SymCleanup(process);
}

void stack_trace(bool crashMode, bool cutSetup){
    CONTEXT context;
    memset(&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);
    stack_trace(crashMode,cutSetup,&context);
}

void on_process_crash(int sig) {
    fprintf(stderr, "A crash occured in thread %p\n",GetCurrentThread());
    fflush(stderr);

    stack_trace(false, false); // on crash mode, only print stack trace before crash occurs
    fflush(stderr);
    exit(sig);
}

LONG WINAPI VectoredExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo){
    fprintf(stderr, "A exception occured in thread '%p'\n",GetCurrentThread());

    DWORD ecode = pExceptionInfo->ExceptionRecord->ExceptionCode;

    switch(ecode){
        case EXCEPTION_ACCESS_VIOLATION:
        {
        	ULONG_PTR readOrWrite = pExceptionInfo->ExceptionRecord->ExceptionInformation[0];
        	ULONG_PTR address = pExceptionInfo->ExceptionRecord->ExceptionInformation[1];
            if(readOrWrite == 1){
            	fprintf(stderr,"The thread tried to write to the address '%lld' for which it does not have the appropriate access.\n",address);
            }else if(readOrWrite == 0){
            	fprintf(stderr,"The thread tried to read to the address '%lld' for which it does not have the appropriate access.\n",address);
            }else if(readOrWrite == 8){
            	fprintf(stderr,"The thread tried to execute code at the address '%lld' for which it does not have the appropriate access.\n",address);
            }else{
            	fprintf(stderr,"The thread tried to do something at the address '%lld' for which it does not have the appropriate access.\n",address);
            }
            fprintf(stderr,"You might have forgotten to initialize a pointer or used a freed ressource.\n");

            break;
        }
        case EXCEPTION_IN_PAGE_ERROR:
        {
        	ULONG_PTR readOrWrite = pExceptionInfo->ExceptionRecord->ExceptionInformation[0];
			ULONG_PTR address = pExceptionInfo->ExceptionRecord->ExceptionInformation[1];
			if(readOrWrite == 1){
				fprintf(stderr,"The thread tried to write tp a page at the address '%lld' that was not present.\n",address);
			}else if(readOrWrite == 0){
				fprintf(stderr,"The thread tried to read from a page at the address '%lld' that was not present.\n",address);
			}else if(readOrWrite == 8){
				fprintf(stderr,"The thread tried to execute code from a page at the address '%lld' that was not present.\n",address);
			}else{
				fprintf(stderr,"The thread tried to do something from a page at the address '%lld' that was not present.\n",address);
			}
			fprintf(stderr,"You might have forgotten to initialize a pointer or used a freed ressource.\n");

			break;
        }
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            fprintf(stderr,"The thread tried to divide an floating-point value by an floating-point divisor of zero.\n");
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            fprintf(stderr,"The thread tried to divide an integer value by an integer divisor of zero.\n");
            break;
        case EXCEPTION_INT_OVERFLOW:
            fprintf(stderr,"The result of an integer operation caused a carry out of the most significant bit of the result.\n");
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            fprintf(stderr,"The thread tried to execute an invalid instruction. (This might be caused by a division by zero.)\n");
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            fprintf(stderr,"The thread tried to access an array element that is out of bounds.\n");
            break;
        case EXCEPTION_BREAKPOINT: // maybe ignore this one ?
            fprintf(stderr,"A breakpoint was encountered.\n");
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            fprintf(stderr,"The thread tried to read or write data that is misaligned.\n");
            break;
        case EXCEPTION_STACK_OVERFLOW:
            // we can't really provide a stack trace in this case because the stack is too big but we can try.
            fprintf(stderr,"The thread used up its stack.\n");
            break;
        case 3221226356:{ // found by testing.
            fprintf(stderr,"The thread tried to free an address for which it did not have appropriate access. Note that this can be caused by dangling pointers: if you return a pointer that was allocated on the stack, an attempt will be made to free it at the end of the scope of your function (code -1073740940).\n");
            ULONG_PTR moreInfo = pExceptionInfo->ExceptionRecord->ExceptionInformation[0];
            fprintf(stderr," Mem offset: %lld\n",moreInfo); // not sure about this.
            break;
        }
        default:
            fprintf(stderr,"Unknown cause: exception code: %lu\n",ecode);
            for(DWORD i = 0;i < pExceptionInfo->ExceptionRecord->NumberParameters;i++){
            	fprintf(stderr,"Exception parameter %li : %lld\n",i,pExceptionInfo->ExceptionRecord->ExceptionInformation[i]);
            }
            break;
    }

    stack_trace(false, false);

    if(pExceptionInfo->ExceptionRecord->ExceptionFlags != 0){
    	// continuable
    	return pExceptionInfo->ExceptionRecord->ExceptionFlags;
    }else{
    	// non continuable
    	fprintf(stderr,"Cannot continue exception.\n");
    	fflush(stderr);
    	return pExceptionInfo->ExceptionRecord->ExceptionFlags;
    }

}

// NB: to work, you need to call this without the -mwindows argument as this argument prevents stderr from printing.
void setup_crash_handler() {
    AddVectoredExceptionHandler(1,VectoredExceptionHandler);

    //signal(SIGSEGV, on_process_crash); // catch segfaults
    //signal(SIGABRT, on_process_crash); // catch exceptions
    //signal(SIGILL, on_process_crash);
    //signal(SIGFPE, on_process_crash); // on floating point errrors
    //signal(SIGTERM, on_process_crash);
}
