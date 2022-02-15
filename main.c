#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include "context.h"
#include <fileapi.h>
#include <time.h>
#define SIZE_OF_BUFF 500

DWORD getFileSize(uint64_t* file_size, HANDLE handle, WCHAR* file_path) {
	BOOL opened = FALSE;
	DWORD error_code = ERROR_SUCCESS;

	// Ensure handle is valid (reopen the file if necessary)
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		PRINT("Invalid file handle\n");
		handle = CreateFile(file_path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			error_code = GetLastError();
			PRINT("\tERROR creating handle to get file size (%d)\n", error_code);
			return error_code;
		}
		opened = TRUE;
	}

	// Using GetFileSizeEx() and passing directly the file_size pointer.
	// Maybe should check file_size > 0 (although that would mean that file_size > 8 EiB = 2^63 Bytes)
	if (!GetFileSizeEx(handle, (PLARGE_INTEGER)file_size)) {
		error_code = GetLastError();
		PRINT("\tcan not get a file size error = %d\n", error_code);
		if (opened)
			CloseHandle(handle);
		return error_code;
	};
	return error_code;
}

void size_test() {

	HANDLE handle_ciphered;
	HANDLE handle_deciphered;
	DWORD error_code;

	size_t file_size_ciphered = 0;
	size_t file_size_deciphered = 0;

	handle_ciphered = CreateFileA("el_quijote_ciphered.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	handle_deciphered = CreateFileA("el_quijote_deciphered.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	error_code = getFileSize(&file_size_ciphered, handle_ciphered, L"el_quijote_ciphered.txt");
	if (error_code != ERROR_SUCCESS) {
		printf("\nError obtaining file size\n");
	}
	printf("\nThe detected ciphered file size is %llu\n", file_size_ciphered);

	error_code = getFileSize(&file_size_deciphered, handle_deciphered, L"el_quijote_deciphered.txt");
	if (error_code != ERROR_SUCCESS) {
		printf("\nError obtaining file size\n");
	}
	printf("\nThe detected deciphered file size is %llu\n", file_size_deciphered);
}

void byte_test(DWORD b_size, char* msg, byte* cip_buf, byte* dec_buf) {
	char caracter;
	char caracter_ciphered;
	char caracter_deciphered;

	caracter = msg[5];
	caracter_ciphered = cip_buf[5];
	caracter_deciphered = dec_buf[5];

	printf("Clear:%c\nCiphered:%c\nDeciphered:%c\n", caracter, caracter_ciphered, caracter_deciphered);
	}

void main() {

	BOOL ciph_menu = FALSE;
	BOOL main_menu = FALSE;

	char line[SIZE_OF_BUFF] = { 0 };
	char dll[SIZE_OF_BUFF] = { 0 };
	int result;
	size_t tam;
	size_t tam_test;

	//***
	char caracter;
	char caracter_ciphered;
	char caracter_deciphered;

	HINSTANCE hLib;

	typedef int(__stdcall* cipher_init_func_type)(struct Cipher*);
	cipher_init_func_type cipher_init_func;
	typedef int(__stdcall* cipher_func_type)(LPVOID,LPCVOID,DWORD,size_t,struct KeyData*);
	cipher_func_type cipher_func;
	cipher_func_type decipher_func;

	typedef int(__stdcall* ch_init_func_type)(struct ChallengeEquivalenceGroup*, struct Challenge*);
	ch_init_func_type ch_init_func;
	typedef int(__stdcall* execute_func_type)();
	execute_func_type execute_func;
	
	//Creation of composed key (and pointer)
	unsigned char arr1[] = { 10, 20, 30, 40, 50 };
	struct KeyData *composed_key;

	composed_key = malloc(1 * sizeof(struct KeyData));
	composed_key->size = 5;
	composed_key->data = malloc(composed_key->size, sizeof(byte));
	composed_key->expires = 0;
	
	//Creation of cipher struct (and pointer)
	struct Cipher* ptr_cipher,cipher;

	//Main loop***
	do {
		printf("\n\n");
		printf("  _______________________  \n");
		printf(" |                       | \n");
		printf(" |     DLL VALIDATOR     | \n");
		printf(" |_______________________| \n");
		printf("\n");
		//Request DLL file input
		printf("Introduce DLL name:\n");
		if (fgetws(line, sizeof(line), stdin)) { //line is wchar_t
			if (1 == swscanf(line, L"%ls", dll)) {  //Format of stdin

			//Check DLL file and load if possible
				hLib = LoadLibraryW(dll); //LoadLibraryW/A?
				if (hLib != NULL) {
					printf("DLL loaded. \n");
					//Look for cipher or executeChallenge functions in the DLL
					cipher_func = (cipher_func_type)GetProcAddress(hLib, "cipher");
					execute_func = (execute_func_type)GetProcAddress(hLib, "executeChallenge");

					//Cipher DLL
					if (cipher_func != NULL) {
						printf("%ws is a cipher DLL. \n", dll);
						cipher.lib_handle = hLib;
						cipher.file_name = dll;
						cipher.block_size = 8;
						cipher.id = dll; //json name
						ptr_cipher = &cipher;

						//Init function call from the DLL
						cipher_init_func = (cipher_init_func_type)GetProcAddress(hLib, "init");
						if (cipher_init_func != NULL) {
							int result = cipher_init_func(ptr_cipher);
							if (result != 0) {
								PRINT("WARNING: error \n");
							}
						}
						else {
							PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
						}

						do {
							char line[SIZE_OF_BUFF] = { 0 };
							int choice = 0;

							//Source buffer
							char* file = "el_quijote.txt";
							FILE* f_original = NULL;
							f_original = fopen(file, "r");
							char* message;
							message = inputFile(f_original, 1);
							tam = strlen(message);
							fclose(f_original);

							//Destination buffers
							DWORD buf_size = tam;
							size_t offset = 0;
							byte* ciphered_buf = malloc(buf_size * sizeof(byte));
							byte* deciphered_buf = malloc(buf_size * sizeof(byte));

							//Smaller buffer for specific tests
							char* message_test = "Hola mundo";
							tam_test = strlen(message_test);
							DWORD buf_size_test = tam_test;
							byte* ciphered_buf_test = malloc(buf_size_test * sizeof(byte));
							byte* deciphered_buf_test = malloc(buf_size_test * sizeof(byte));

							printf("\n\n");
							printf("  _______________________  \n");
							printf(" |                       | \n");
							printf(" |      CIPHER MENU      | \n");
							printf(" |_______________________| \n");
							printf("\n");

							printf("Select an option:\n");
							printf("  1) Functionality test \n");
							printf("  2) Robustness test \n");
							printf("  3) Cipher/Decipher \n");
							printf("  0) Back to main menu \n");
							if (fgets(line, sizeof(line), stdin)) {
								if (1 == sscanf(line, "%d", &choice)) {
									switch (choice) {
									case 1:
										//Cipher and decipher source buffer for testing
										//Ciphering...
										cipher_func = (cipher_init_func_type)GetProcAddress(hLib, "cipher");
										if (cipher_func != NULL) {
											//Smaller buffer
											result = cipher_func(ciphered_buf_test, message_test, buf_size_test, offset, composed_key);
											//Original buffer
											result = cipher_func(ciphered_buf, message, buf_size, offset, composed_key);
											FILE* f_ciphered = NULL;
											f_ciphered = fopen("el_quijote_ciphered.txt", "wb");
											fwrite(ciphered_buf, 1, tam, f_ciphered);
											fclose(f_ciphered);

											//free(ciphered_buf);
											if (result != 0) {
												PRINT("WARNING: error \n");
											}
										}
										else {
											PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
										}
										//Deciphering
										decipher_func = (cipher_init_func_type)GetProcAddress(hLib, "decipher");
										if (decipher_func != NULL) {
											//Smaller buffer
											result = decipher_func(deciphered_buf_test, message_test, buf_size_test, offset, composed_key);
											//Original buffer
											result = decipher_func(deciphered_buf, ciphered_buf, buf_size, offset, composed_key);
											FILE* f_deciphered = NULL;
											f_deciphered = fopen("el_quijote_deciphered.txt", "wb");
											fwrite(deciphered_buf, 1, tam, f_deciphered);
											fclose(f_deciphered);
											if (result != 0) {
												PRINT("WARNING: error \n");
											}
										}
										else {
											PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
										}

										do {
											printf("\nSelect test to run:\n");
											printf("  1) Size \n");
											printf("  2) Byte \n");
											printf("  3) Cipher not Decipher \n");
											printf("  4) Decipher is valid \n");
											printf("  5) Run all tests \n");
											printf("  0) Back to cipher menu \n");
											if (fgets(line, sizeof(line), stdin)) {
												if (1 == sscanf(line, "%d", &choice)) {
													switch (choice) {
													case 1:
														size_test();
														break;
													case 2:
														//USE BUFFERS AND MEMCOPY (?)
														//byte_test(buf_size, message, ciphered_buf, deciphered_buf);
														caracter = message_test[5];
														caracter_ciphered = ciphered_buf_test[5];
														caracter_deciphered = deciphered_buf_test[5];

														printf("Clear:%c\nCiphered:%c\nDeciphered:%c\n", caracter, caracter_ciphered, caracter_deciphered);
														
														break;
													case 3:
														printf("Clear text:%s\n From clear to ciphered:%s\n From clear to deciphered:%s\n", message_test, ciphered_buf_test, deciphered_buf_test);
														break;
													case 4:
														printf("Text in clear:%s\n From clear to deciphered:%s\n", message_test, deciphered_buf_test);
														break;
													case 5:
														printf("Running all tests...\n");
														size_test();
														byte_test(buf_size, message, ciphered_buf, deciphered_buf);
														printf("Clear text:%s\n From clear to ciphered:%s\n From clear to deciphered:%s\n", message_test, ciphered_buf_test, deciphered_buf_test);
														printf("Text in clear:%s\n From clear to deciphered:%s\n", message_test, deciphered_buf_test);
														break;
													case 0:
														printf("Cipher menu... \n");
														ciph_menu = TRUE;
														break;
													default:
														printf("Invalid option, try again.\n");
														break;
													}
												}
											}

										} while (!ciph_menu);
										break;
									case 2:
										printf("ROBUSTNESS TEST \n");
										break;
									case 3:
										printf("CIPHER AND DECIPHER\n");
										//Cipher function
										cipher_func = (cipher_init_func_type)GetProcAddress(hLib, "cipher");
										if (cipher_func != NULL) {
											printf("Quijote size: %d\nQuijote_ciphered size: %d\n", tam, _msize(ciphered_buf));
											result = cipher_func(ciphered_buf, message, buf_size, offset, composed_key);
											FILE* f_ciphered = NULL;
											f_ciphered = fopen("el_quijote_ciphered.txt", "wb");
											fwrite(ciphered_buf, 1, tam, f_ciphered);
											fclose(f_ciphered);
											//free(ciphered_buf);
											if (result != 0) {
												PRINT("WARNING: error \n");
											}
										}
										else {
											PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
										}
										//Decipher function
										decipher_func = (cipher_init_func_type)GetProcAddress(hLib, "decipher");
										if (decipher_func != NULL) {
											printf("Quijote_ciphered size: %d\nQuijote_deciphered size: %d\n", _msize(ciphered_buf), _msize(deciphered_buf));
											result = decipher_func(deciphered_buf, ciphered_buf, buf_size, offset, composed_key);
											FILE* f_deciphered = NULL;
											f_deciphered = fopen("el_quijote_deciphered.txt", "wb");
											fwrite(deciphered_buf, 1, tam, f_deciphered);
											fclose(f_deciphered);
											if (result != 0) {
												PRINT("WARNING: error \n");
											}
										}
										else {
											PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
										}
										break;
									case 0:
										printf("Main menu... \n");
										main_menu = TRUE;
										break;
									default:
										printf("Invalid option, try again.\n");
										break;
									}
								}
							}

						} while (!main_menu);

					}
					else if (execute_func != NULL) {
						printf("%ws is a challenge DLL. \n", dll);
						//CHALLENGE TEST
					}
				}
				else {
					printf("DLL not loaded \n");
				}
				FreeLibrary(hLib);
			}
		}

	} while (TRUE);	
}