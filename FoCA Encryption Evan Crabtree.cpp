// Author: A.Oram (Feb 2018)
// Last revised March 2019 by E.Crabtree, CS1

char EKey = 'd';

#define StudentName "Evan Crabtree"

#define MAXCHARS 6              // max characters, minimum of 6

#define ORIGINALCHARACTERS 0    // 2D index of original character string
#define ENCRYPTEDCHARACTERS 1   // 2D index of encrypted character string
#define DECRYPTEDCHARACTERS 2   // 2D index of decrypted character string

using namespace std;
#include <string>               // for strings
#include <fstream>              // file I/O
#include <iostream>             // for cin >> and cout <<
#include <iomanip>              // for fancy output
#include "TimeUtils.h"          // for GetTime, GetDate, etc.

#define dollarchar '$'          // string terminator

char EncryptionData[3][MAXCHARS];

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
		mov eax, a_character    // move address of a_character into EAX
		mov al, [eax]           // copy a_character valeue into last 8 bits of EAX
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
		jmp wloop               // return to beginning if bad character entered
	exitLoop:
	}//--- End of Assembly code
}
//-------------------------------------------------------------------------------------------------------------

void get_original_chars(int& length)
{
  char next_char = ' ';
  length = 0;
  get_char(next_char);

  while ((length < MAXCHARS) && (next_char != dollarchar))
  {
    EncryptionData[ORIGINALCHARACTERS][length++] = next_char;
	get_char(next_char);
  }
}

//---------------------------------------------------------------------------------------------------------------
//----------------- ENCRYPTION ROUTINES -------------------------------------------------------------------------

void encrypt_chars (int lengthOfString, char EKey)
{
    __asm {
	  mov   edx, 0                      // initialise counter to 0
	  
	floop:
	  lea   eax, EncryptionData         // load address of encryption data into EAX
	  movzx ecx, byte ptr[eax+edx]      // move character into ECX

	  cmp   ecx, 90                     // compare temp_char with ascii 'Z'
	  jg    isNotUpper                  // jump to else part of if statement
	  cmp   ecx, 65                     // compare temp_char with ascii 'A'
	  jl    isNotUpper                  // jump to else part of if statement
	  add   ecx, 0x20                   // convert temp_char to lower case by adding 32

	isNotUpper:
	  lea   eax, EKey                   // move address of EKey into EAX
	  and   dword ptr[eax], 0x000000FF  // perform logical AND between Ekey and 255
	  add   [eax], 0x02                 // add 2 to Ekey
	  ror   byte ptr[eax], 2            // shifts bits of Ekey to right by 2, preserving digits by rotating them to the other side
	  mov   eax, [eax]                  // store Ekey in EDX register

	  xor   ecx, eax                    // EXCLUSIVE OR temp_char with Ekey
	  ror   cl, 1                       // shift bits of temp_char to right by 1, preserving digits by rotating them to the other side
	  
	  lea   eax, EncryptionData         // load address of encryption data into EAX
	  add   eax, MAXCHARS               // add MAXCHARS to pointer to point at ENCRYPTEDCHARS
	  mov   byte ptr[eax+edx], cl       // move temp_char into EncyptionData

          add   edx, 1                      // add 1 to counter
	  cmp   edx, lengthOfString         // compare counter with length of string
	  jl    floop                       // jump back to beginning of for loop if counter less than length
    }//--- End of Assembly code
  return;
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
		temp_char = EncryptionData[ENCRYPTEDCHARACTERS][i];

		__asm {
			lea    eax, EKey                  // move address of EKey into EAX
			and   dword ptr[eax], 0x000000FF  // perform logical AND between Ekey and 255
			add   [eax], 0x02                 // add 2 to Ekey
			ror   byte ptr[eax], 2            // shifts bits of Ekey to right by 2, preserving digits by rotating them to the other side
			mov   eax, [eax]                  // store Ekey in EDX register

			movzx ecx, temp_char              // move temp_char into ECX
			
			rol   cl, 1                       // shift bits of temp_char to left by 1, preserving digits by rotating them to the other side
			xor   ecx, eax                    // EXCLUSIVE OR temp_char with Ekey

			mov   temp_char, cl               // move last 8 bits of EAX into temp_char
		}//--- End of Assembly code
		EncryptionData[DECRYPTEDCHARACTERS][i] = temp_char;
	}
	return;
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
  cout << "\n\nOriginal string =  " << EncryptionData[ORIGINALCHARACTERS] << "\tHex = ";
  EDump<< "\n\nOriginal string =  " << EncryptionData[ORIGINALCHARACTERS] << "\tHex = ";
  for (int i = 0; i < char_count; i++)
  {
    cout << hex << setw(2) << setfill('0') << ((int(EncryptionData[ORIGINALCHARACTERS][i])) & 0xFF) << "  ";
    EDump<< hex << setw(2) << setfill('0') << ((int(EncryptionData[ORIGINALCHARACTERS][i])) & 0xFF) << "  ";
  };

  //*****************************************************
  // Encrypt the string and display/save the result
  encrypt_chars (char_count, EKey);

  cout << "\n\nEncrypted string = " << EncryptionData[ENCRYPTEDCHARACTERS] << "\tHex = ";
  EDump<< "\n\nEncrypted string = " << EncryptionData[ENCRYPTEDCHARACTERS] << "\tHex = ";
  for (int i = 0; i < char_count; i++)
  {
    cout << ((int(EncryptionData[ENCRYPTEDCHARACTERS][i])) & 0xFF) << "  ";
    EDump<< ((int(EncryptionData[ENCRYPTEDCHARACTERS][i])) & 0xFF) << "  ";
  }

  //*****************************************************
  // Decrypt the encrypted string and display/save the result
  decrypt_chars (char_count, EKey);

  cout << "\n\nDecrypted string = " << EncryptionData[DECRYPTEDCHARACTERS] << "\tHex = ";
  EDump<< "\n\nDecrypted string = " << EncryptionData[DECRYPTEDCHARACTERS] << "\tHex = ";
  for (int i = 0; i < char_count; i++)
  {
    cout << ((int(EncryptionData[DECRYPTEDCHARACTERS][i])) & 0xFF) << "  ";
    EDump<< ((int(EncryptionData[DECRYPTEDCHARACTERS][i])) & 0xFF) << "  ";
  }
  //*****************************************************

  cout << "\n\n\n";
  EDump << "\n\n-------------------------------------------------------------";
  EDump.close();

  system("PAUSE");
  return (0);
} // end of whole encryption/decryption program --------------------------------------------------------------------


