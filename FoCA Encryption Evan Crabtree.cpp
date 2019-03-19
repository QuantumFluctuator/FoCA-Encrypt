// Author: A.Oram (Feb 2018)
// Last revised March 2019 by E.Crabtree, CS1

char EKey = 'd';

#define StudentName "Evan Crabtree"

#define MAXCHARS 6    // minimum of 6

using namespace std;
#include <string>               // for strings
#include <fstream>              // file I/O
#include <iostream>             // for cin >> and cout <<
#include <iomanip>              // for fancy output
#include "TimeUtils.h"          // for GetTime, GetDate, etc.

#define dollarchar '$'          // string terminator

char OChars[MAXCHARS],          // Original character string
   EChars[MAXCHARS],            // Encrypted character string
   DChars[MAXCHARS] = "Soon!";  // Decrypted character string

//----------------------------- C++ Functions ----------------------------------------------------------

void get_char(char& a_character)
{
	__asm {
	wloop:
	}
	a_character=(char) _getwche();
	if (a_character=='\r' || a_character=='\n')  // allow the enter key to work as the terminating character too
	a_character=dollarchar;
	__asm {
		push eax                // backup EAX to stack
		push edx                // backup EDX to stack
		mov edx, a_character    // move address of a_character into EAX
		mov al, [eax]           // copy a_character into last 8 bits of EAX
		cmp al, '$'             // compare a_character with ascii $
		je exitLoop             // if a_character is $, exit loop successfully
		cmp al, '0'             // compare a_character with ascii 0
		jl upperLetterCheck     // if a_character is less than 48, skip rest of number check
		cmp al, '9'             // compare a_character with ascii 9
		jle exitLoop            // if a_character is between 48 and 57, exit loop successfully
	upperLetterCheck:
		cmp al, 'A'             // compare a_character with ascii A
		jl lowerLetterCheck     // if a_character is less than 65, skip rest of upper case letter check
		cmp al, 'Z'             // compare a_character with ascii Z
		jle exitLoop            // if a_character is between 65 and 90, exit loop successfully
	lowerLetterCheck:
		cmp al, 'a'             // compare a_character with ascii a
		jl retry                // if a_character is less than 97, show message and restart loop
		cmp al, 'z'             // compare a_character with ascii z
		jle exitLoop            // if a_character is between 97 and 122, exit loop successfully
	retry:
	}
	cout << "\nAlphanumeric characters only, please try again > ";
	__asm {
		pop edx                 // restore EDX backup from stack
		pop eax                 // restore EAX backup from stack
		jmp wloop               // return to beginning if bad character entered
	exitLoop:
		pop edx                 // restore EDX backup from stack
		pop eax                 // restore EAX backup from stack
	}
}
//-------------------------------------------------------------------------------------------------------------

void get_original_chars(int& length)
{
  char next_char = ' ';
  length = 0;
  get_char(next_char);

  while ((length < MAXCHARS) && (next_char != dollarchar))
  {
    OChars[length++] = next_char;
	get_char(next_char);
  }
}

//---------------------------------------------------------------------------------------------------------------
//----------------- ENCRYPTION ROUTINES -------------------------------------------------------------------------

void encrypt_chars (int length, char EKey)
{
  char temp_char;                       // Character temporary store

  for (int i = 0; i < length; i++)      // Encrypt characters one at a time
  {
    temp_char = OChars[i];              // Get the next char from Original Chars array
                                        // Note the lamentable lack of comments below!
    __asm
    {                                   
      push   eax                        // backup EAX to stack
      push   ecx                        // backup ECX to stack
      push   edx                        // backup EDX to stack
                                        
      movzx  ecx, temp_char             // move temp_char into ECX
      lea    eax, EKey                  // move address of EKey into EAX

	  push   eax                        // push address of Ekey to stack
	  push   ecx                        // push temp_char to stack

      call   encrypt_11                 // call encryption function

	  add    esp, 8                     // return stack pointer to original position prior to function call

      mov    temp_char, al              // move last 8 bits of EAX into temp_char
                                        
      pop    edx                        // revert EDX from stack
      pop    ecx                        // revert ECX from stack
      pop    eax                        // revert EAX from stack
    }
    EChars[i] = temp_char;              // store encrypted char in the Encrypted Chars array
  }
  return;

  // Inputs:  ebp+8 = 32-bit address of Ekey,
  //          ebp+12 = the character to be encrypted (in the low 8-bit field, CL).

  // Output: register EAX = the encrypted value of the source character (in the low 8-bit field, AL).

  __asm
  {
  encrypt_11:
	    push  ebp                           // put backup of base pointer in stack
		mov   ebp, esp                      // move base pointer to current position of stack pointer

		push  edx                           // backup EDX on stack
		push  ebx                           // backup EBX on stack

		mov   eax, dword ptr[ebp+12]        // move Ekey into eax from parameters

		push  dword ptr[ebp+8]              // push temp_char to stack from parameters
		and   dword ptr[eax], 0x000000FF    // perform logical AND between Ekey and 255
		add   [eax], 0x02                   // add 2 to Ekey
		ror   byte ptr[eax], 1              // shifts bits of Ekey to right by 1, preserving digits by rotating them to the other side
		ror   byte ptr[eax], 1              // shifts bits of Ekey to right by 1, preserving digits by rotating them to the other side
		mov   edx, [eax]                    // store Ekey in EDX register

		pop   ebx                           // pop temp_char from stack into EBX
		xor   ebx, edx                      // EXCLUSIVE OR temp_char with Ekey
		ror   bl, 1                         // shift bits of temp_char to right by 1, preserving digits by rotating them to the other side

		mov   eax, ebx                      // store temp_char in EAX ready for cdecl return

		pop   ebx                           // revert EBX from stack
		pop   edx                           // revert EDX from stack

		pop   ebp                           // revert base pointer to previous value
		ret                                 // return to previous address
  }

  //--- End of Assembly code
}
//*** end of encrypt_chars function
//---------------------------------------------------------------------------------------------------------------




//---------------------------------------------------------------------------------------------------------------
//----------------- DECRYPTION ROUTINES -------------------------------------------------------------------------
//
void decrypt_chars (int length, char EKey)
{
	char temp_char;

	for (int i(0); i < length; i++) {
		temp_char = EChars[i];

		__asm {
			push   eax                      // backup eax to stack
			push   ecx                      // backup ecx to stack
			push   edx                      // backup edx to stack

			movzx  ecx, temp_char           // move temp_char into ecx
			lea    eax, EKey                // move address of EKey into eax

			push   eax                      // push address of Ekey to stack
			push   ecx                      // push temp_char to stack

			call   decrypt_11               // call decrypt function

			add    esp, 8                   // return stack pointer to original position prior to function call

			mov    temp_char, al            // move last 8 bits of eax into temp_char

			pop    edx                      // revert edx from stack
			pop    ecx                      // revert ecx from stack
			pop    eax                      // revert eax from stack
		}

		DChars[i] = temp_char;
	}

	return;

	__asm {
	decrypt_11:
		push  ebp                       // put backup of base pointer in stack
		mov   ebp, esp                  // move base pointer to current position of stack pointer

		push  edx                       // backup edx on stack
		push  ebx                       // backup ebx on stack

		mov   eax, dword ptr[ebp+12]    //move EKey pointer into eax

		and   dword ptr[eax], 0x000000FF// perform logical AND between Ekey and 255
		add   [eax], 0x02               // add 2 to Ekey
		ror   byte ptr[eax], 1          // shifts bits of Ekey to right by 1, preserving digits by rotating them to the other side
		ror   byte ptr[eax], 1          // shifts bits of Ekey to right by 1, preserving digits by rotating them to the other side
		mov   edx, [eax]                // store Ekey in edx register

		mov   ebx, [ebp+8]              // move temp_char into ebx
		rol   bl, 1                     // shift bits of temp_char to left by 1, preserving digits by rotating them to the other side
		xor   ebx, edx                  // EXCLUSIVE OR temp_char with Ekey
		
		mov   eax, ebx                  // store temp_char in eax ready for cdecl return

		pop   ebx                       // revert ebx from stack
		pop   edx                       // revert edx from stack

		pop   ebp                       // revert base pointer to previous value
		ret                             // return to previous address
	}
}
//*** end of decrypt_chars function
//---------------------------------------------------------------------------------------------------------------



int main(void)
{
  int char_count (0);  // The number of actual characters entered (upto MAXCHARS limit).

  cout << "\nPlease enter upto " << MAXCHARS << " alphanumeric characters:  ";
  get_original_chars (char_count);

  ofstream EDump;
  EDump.open("EncryptDump.txt", ios::app);
  EDump << "\n\nFoCA Encryption program results (" << StudentName << ") Encryption key = '" << EKey << "'";
  EDump << "\nDate: " << GetDate() << "  Time: " << GetTime();

  // Display and save initial string
  cout << "\n\nOriginal string =  " << OChars << "\tHex = ";
  EDump<< "\n\nOriginal string =  " << OChars << "\tHex = ";
  for (int i = 0; i < char_count; i++)
  {
    cout << hex << setw(2) << setfill('0') << ((int(OChars[i])) & 0xFF) << "  ";
    EDump<< hex << setw(2) << setfill('0') << ((int(OChars[i])) & 0xFF) << "  ";
  };

  //*****************************************************
  // Encrypt the string and display/save the result
  encrypt_chars (char_count, EKey);

  cout << "\n\nEncrypted string = " << EChars << "\tHex = ";
  EDump<< "\n\nEncrypted string = " << EChars << "\tHex = ";
  for (int i = 0; i < char_count; i++)
  {
    cout << ((int(EChars[i])) & 0xFF) << "  ";
    EDump<< ((int(EChars[i])) & 0xFF) << "  ";
  }

  //*****************************************************
  // Decrypt the encrypted string and display/save the result
  decrypt_chars (char_count, EKey);

  cout << "\n\nDecrypted string = " << DChars << "\tHex = ";
  EDump<< "\n\nDecrypted string = " << DChars << "\tHex = ";
  for (int i = 0; i < char_count; i++)
  {
    cout << ((int(DChars[i])) & 0xFF) << "  ";
    EDump<< ((int(DChars[i])) & 0xFF) << "  ";
  }
  //*****************************************************

  cout << "\n\n\n";
  EDump << "\n\n-------------------------------------------------------------";
  EDump.close();

  system("PAUSE");
  return (0);
} // end of whole encryption/decryption program --------------------------------------------------------------------


