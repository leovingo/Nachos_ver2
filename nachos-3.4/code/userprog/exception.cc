// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#define MaxFileLength 32 // Do dai quy uoc cho file name
#define MAX_BUFFER_SIZE 255
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

// Tang Program Counter de tiep tuc nap lenh
void IncreasePC() {
    int counter = machine->ReadRegister(PCReg);

    machine->WriteRegister(PrevPCReg, counter);
    counter = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, counter);
    machine->WriteRegister(NextPCReg, counter + 4);
}

char* User2System(int virtAddr, int limit)
{
	int i;// index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1];//need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);
	//printf("\n Filename u2s:");
	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		//printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

int System2User(int virtAddr, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do {
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}
void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);
	
	switch (which) {
	case NoException:
		return;
	
	case PageFaultException:
       	    	DEBUG('a', "\n No valid translation found.");
       		printf("\n\n No valid translation found.");
  	    	interrupt->Halt();
            	break;

        case ReadOnlyException: 
            	DEBUG('a', "\n  Write attempted to page marked read-only.");
            	printf("\n\n Write attempted to page marked read-only.");
            	interrupt->Halt();
            	break;

       	case BusErrorException:
          	DEBUG('a', "\n Translation resulted valid physical address.");
            	printf("\n\n Translation resulted valid physical address.");
            	interrupt->Halt();
            	break;

       	case AddressErrorException:
            	DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space.");
            	printf("\n\n Unaligned reference or one that was beyond the end of the address space.");
            	interrupt->Halt();
            	break;

        case OverflowException:
            	DEBUG('a', "\nInteger overflow in add or sub.");
            	printf("\n\n Integer overflow in add or sub.");
            	interrupt->Halt();
            	break;

        case IllegalInstrException:
            	DEBUG('a', "\n Unimplemented or reserved instr.");
            	printf("\n\n Unimplemented or reserved instr.");
            	interrupt->Halt();
            	break;

        case NumExceptionTypes:
            	DEBUG('a', "\n Number exception types.");
            	printf("\n\n Number exception types.");
            	interrupt->Halt();
            	break;
	case SyscallException:
		char* buffer;
		bool isNegative = false;
		int firstIndex = 0;
		int numBytes;
		switch (type) {
		case SC_Halt:
		{
			DEBUG('a', "\n Shutdown, initiated by user program.");
			printf("\n\n Shutdown, initiated by user program.");
			interrupt->Halt();
			break;
		}
		case SC_Create:
		{
			int virtAddr;
			char* filename;
			DEBUG('a', "\n SC_Create call ...");
			DEBUG('a', "\n Reading virtual address of filename");
			// Lấy tham số tên tập tin từ thanh ghi r4
			virtAddr = machine->ReadRegister(4);
			DEBUG('a', "\n Reading filename.");
			// MaxFileLength là = 32
			filename = User2System(virtAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				printf("\n Not enough memory in system");
				DEBUG('a', "\n Not enough memory in system");
				machine->WriteRegister(2, -1); // trả về lỗi cho chương
				// trình người dùng
				delete filename;
				return;
			}
			DEBUG('a', "\n Finish reading filename.");

			if (!fileSystem->Create(filename, 0))
			{
				printf("\n Error create file '%s'", filename);
				machine->WriteRegister(2, -1);
				delete filename;
				return;
			}
			machine->WriteRegister(2, 0); // trả về cho chương trình
			// người dùng thành công
			delete filename;
			break;
		}
		case SC_ReadInt:
                {
                    // Input: khong
                    // Output: so nguyen doc duoc tu console
                    // Chuc nang: doc so nguyen tu console

                    int result = 0;

                    buffer = new char[MAX_BUFFER_SIZE + 1];

                    // Doc buffer tu console voi toi da 255 ki tu
                    numBytes = gSynchConsole->Read(buffer, MAX_BUFFER_SIZE);

                    isNegative = false;
                    firstIndex = 0; // Index cua ki tu dau tien cua so trong buffer
                    int lastIndex = 0;  // Index cua ki tu cuoi cung cua so trong buffer

                    // Bo qua ki tu space
                    while (buffer[firstIndex] == ' ') {
                        firstIndex++;
                        lastIndex++;
                    }

                    if (buffer[firstIndex] == '-') {
                        isNegative = true;
                        firstIndex++;
                        lastIndex++;
                    }

                    for (int i = firstIndex; i < numBytes; i++) {
                        // Truong hop so 0 sau dau thap phan (vd: 12.0)
                        if (buffer[i] == '.') {
                            for (int j = i + 1; j < numBytes; j++) {
                                // Toan bo phai la 0 sau dau thap phan
                                if (buffer[j] != '0') {
                                    printf("\n\n Invalid integer.");
                                    DEBUG('a', "\n Invalid integer.");

                                    machine->WriteRegister(2, 0);
                                    IncreasePC();

                                    delete buffer;
                                    return;
                                }
                            } 

                            lastIndex = i - 1; // Ngay truoc dau thap phan
                            break;
                        }
                        else if (buffer[i] < '0' || buffer[i] > '9') {
                            printf("\n\n Invalid integer. Take 0 as value.");
                            DEBUG('a', "\n Invalid integer.");

                            machine->WriteRegister(2, 0);
                            IncreasePC();

                            delete buffer;
                            return;
                        }

                        lastIndex = i;
                    }

                    // Chuyen chuoi ve so nguyen
                    const int maxInt32 = 2147483647; // truong hop tran so
                    for (int i = firstIndex; i <= lastIndex; i++) {
                        result = (result * 10 + (int)(buffer[i] - 48)) % maxInt32;
                    }

                    if (isNegative) {
                        result = -result;
                    }

                    machine->WriteRegister(2, result);
                    IncreasePC();

                    delete buffer;
                    return;

                }

                case SC_PrintInt:
                {
                    // Input: 1 so nguyen
                    // Output: khong
                    // Chuc nang: in so nguyen ra console

                    // Doc so nguyen to thanh ghi R4
                    int number = machine->ReadRegister(4);

                    // Neu la 0
                    if (number == 0) {
                        gSynchConsole->Write("0", 1);
                        IncreasePC();
                        return;
                    }

                    isNegative = false;
                    int numDigits = 0;
                    firstIndex = 0;

                    if (number < 0) {
                        isNegative = true;
                        number = -number;
                        firstIndex = 1;
                    }

                    // Dem so chu so
                    int temp = number;
                    while (temp) {
                        numDigits++;
                        temp /= 10;
                    }

                    // Tao chuoi de in ra console
                    buffer = new char[MAX_BUFFER_SIZE + 1];

                    for (int i = firstIndex + numDigits - 1; i >= firstIndex; i--) {
                        buffer[i] = (char)((number % 10) + 48);
                        number /= 10;
                    }

                    if (isNegative) {
                        buffer[0] = '-';
                        buffer[numDigits + 1] = '\0';
                        gSynchConsole->Write(buffer, numDigits + 1);

                        delete buffer;
                        IncreasePC();

                        return;
                    }
                    
                    buffer[numDigits] = '\0';
                    gSynchConsole->Write(buffer, numDigits);

                    delete buffer;
                    IncreasePC();

                    return;
                }
	     }	
            IncreasePC();
            break;
	}
}
