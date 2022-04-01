#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include "context.h"
#include <fileapi.h>
#include <time.h>
#include <sysinfoapi.h>
#include <sys/stat.h>
#include "json.h"
#define SIZE_OF_BUFF 500

#define ENABLE_PRINTS 1	
#define PRINT_HEX(BUF, BUF_SIZE) print_hex(#BUF, BUF, BUF_SIZE);
#define PRINT_SMATRIX(BUF,BUF_SIZE) print_square_matrix(BUF, BUF_SIZE);

DWORD print_hex(char* buf_name, void* buf, int size);
DWORD getFileSize(uint64_t* file_size, HANDLE handle, WCHAR* file_path);
void size_test(char* ciphered, char* deciphered);
void byte_test(DWORD b_size, char* msg, byte* cip_buf, byte* dec_buf);
int loadPropertiesjson(char* json_file, struct Challenge *ch);

DWORD print_hex(char* buf_name, void* buf, int size) {
	if (ENABLE_PRINTS) {
		printf("First %d bytes of %s contain:\n", size, buf_name);

		//byte [size*3 + size/32 + 1] str_fmt_buf;
		char* full_str = NULL;
		char* target_str = NULL;
		//int total = 0;

		// Size of string will consist on:
		//   (size*3)			 - 3 characters for every byte (2 hex characters plus 1 space). Space changed for '\n' every 32 bytes
		//   (size/8 - size/32)	 - Every 8 bytes another space is added after the space (if it is not multiple of 32, which already has '\n' instead)
		//   (1)				 - A '\n' is added at the end
		full_str = calloc((size * 3) + (size / 8 - size / 32) + (1), sizeof(char));
		if (full_str == NULL) {
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		target_str = full_str;

		for (int i = 0; i < size; i++) {
			if ((i + 1) % 32 == 0) {
				target_str += sprintf(target_str, "%02hhX\n", ((byte*)buf)[i]);
			}
			else if ((i + 1) % 8 == 0) {
				target_str += sprintf(target_str, "%02hhX  ", ((byte*)buf)[i]);
			}
			else {
				target_str += sprintf(target_str, "%02hhX ", ((byte*)buf)[i]);
			}
		}
		target_str += sprintf(target_str, "\n");
		printf(full_str);
		free(full_str);
	}
	return ERROR_SUCCESS;
}

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

void size_test(char* ciphered, char* deciphered) {

	HANDLE handle_ciphered;
	HANDLE handle_deciphered;
	DWORD error_code;

	size_t file_size_ciphered = 0;
	size_t file_size_deciphered = 0;

	handle_ciphered = CreateFileA(ciphered, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	handle_deciphered = CreateFileA(deciphered, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (handle_ciphered == INVALID_HANDLE_VALUE || handle_deciphered == INVALID_HANDLE_VALUE )
	{
		printf("\nInvalid handle.\n");
	}
	else {
		error_code = getFileSize(&file_size_ciphered, handle_ciphered, ciphered);
		if (error_code != ERROR_SUCCESS) {
			printf("\nError obtaining file size\n");
		}
		printf("The detected ciphered file size is %llub.\n", file_size_ciphered);

		error_code = getFileSize(&file_size_deciphered, handle_deciphered, deciphered);
		if (error_code != ERROR_SUCCESS) {
			printf("\nError obtaining file size\n");
		}
		printf("The detected deciphered file size is %llub.\n", file_size_deciphered);
	}
	if (file_size_ciphered == file_size_ciphered) {
		printf("\nTest passed. File sizes are the same.\n");
	}
	else {
		printf("\nTest failed. File sizes are different.\n");
	}
	CloseHandle(handle_ciphered);
	CloseHandle(handle_deciphered);

}

void byte_test(DWORD b_size, char* msg, byte* cip_buf, byte* dec_buf) { //*En vez de coger un char, coger 10 randoms(?)
	char caracter;
	char caracter_ciphered;
	char caracter_deciphered;

	caracter = msg[5];
	caracter_ciphered = cip_buf[5];
	caracter_deciphered = dec_buf[5];

	printf("Clear:%c\nCiphered:%c\nDeciphered:%c\n", caracter, caracter_ciphered, caracter_deciphered);
	if (caracter == caracter_deciphered) {
		printf("\nTest passed.\n");
	}
	else {
		printf("\nTest failed.\n");
	}

}

int loadPropertiesjson(char* file_name, struct Challenge *ch) {
	FILE* fp;
	struct stat file_status;
	int file_size;
	char* file_contents;
	json_char* json;
	json_value* value;

	// Check availability of file and get size
	if (stat(file_name, &file_status) != 0) {
		fprintf(stderr, "File %s not found\n", file_name);
		return -1;
	}
	file_size = file_status.st_size;

	// Assign space for the file contents
	file_contents = (char*)malloc(file_status.st_size);
	if (file_contents == NULL) {
		fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
		return -1;
	}

	// Try to open file
	fp = fopen(file_name, "rb");    // Read is done y binary mode. Othrewise fread() does not return (fails)
	if (fp == NULL) {
		fprintf(stderr, "Unable to open %s\n", file_name);
		//fclose(fp);        // It is not needed to close file if fopen() fails
		free(file_contents);
		return -1;
	}
	if (fread(file_contents, file_size, 1, fp) != 1) {
		fprintf(stderr, "Unable t read content of %s\n", file_name);
		fprintf(stderr, "size %d\n", file_size);
		fclose(fp);
		free(file_contents);
		return -1;
	}
	fclose(fp);

	// JSON parsing and syntax validation
	json = (json_char*)file_contents;
	value = json_parse(json, file_size);
	if (value == NULL) {
		fprintf(stderr, "Unable to parse data\n");
		free(file_contents);
		return -1;
	}

	// Fill the challenge with all the JSON data
	//getProps(value,ch->properties);
	ch->properties = NULL;
	int num_main_fields = value->u.object.length;
	for (int i = 0; i < num_main_fields; i++) {
		if (strcmp(value->u.object.values[i].name, "Props") == 0)			ch->properties = value->u.object.values[i].value;
		//else fprintf(stderr, "WARINING: the field '%s' included in config.json is not registered and will not be processed.\n", value->u.object.values[i].name);
	}
	if (ch->properties == NULL) {
		printf("Invalid json, Props is not present.\n");
		return -1;
	}
	
	// Free unnecessary pointers
	//json_value_free(value);        // This frees all the internal pointers. Be sure to have copied the data and not just pointed to it.
	free(file_contents);
	return 0;
}


BOOL ciph_menu = FALSE;
BOOL main_menu = FALSE;

char line[SIZE_OF_BUFF] = { 0 };
char dll[SIZE_OF_BUFF] = { 0 };
int result;
size_t tam;
size_t tam_test;

typedef int(__stdcall* cipher_init_func_type)(struct Cipher*);
cipher_init_func_type cipher_init_func;
typedef int(__stdcall* cipher_func_type)(LPVOID, LPCVOID, DWORD, size_t, struct KeyData*);
cipher_func_type cipher_func;
cipher_func_type decipher_func;

typedef int(__stdcall* ch_init_func_type)(struct ChallengeEquivalenceGroup*, struct Challenge*);
ch_init_func_type ch_init_func;
typedef int(__stdcall* execute_func_type)();
execute_func_type execute_func;
typedef void(__stdcall* periodicExecution_func_type)(BOOL active);
periodicExecution_func_type periodicExecution_func;

//Creation of cipher struct (and pointer)
struct Cipher cipher;
//Creation of composed key
struct KeyData* composed_key;

//Creation of challenge struct (amd pointer)
struct Challenge challenge;
//Creation of subkey
struct KeyData subkey;
//Creation of challenge equivalence group struct (amd pointer)
struct ChallengeEquivalenceGroup challenge_group;

void main() {

	//Main loop
	do {
		main_menu = FALSE;
		printf("\n\n");
		printf("  _______________________  \n");
		printf(" |                       | \n");
		printf(" |     DLL VALIDATOR     | \n");
		printf(" |_______________________| \n");
		printf("\n");
		//Request DLL file input
		printf("Introduce DLL name (0 to exit):\n");
		fgetws(line, sizeof(line), stdin);
		if (wcscmp(line, L"0\n") == 0) {
			printf("Good bye!\n");
			break;
		}
		else if (1 == swscanf(line, L"%ls", dll)) {
			//Check DLL file and load if possible
			HINSTANCE hLib;
			hLib = LoadLibraryW(dll);
			if (hLib != NULL) {
				printf("DLL loaded. \n");
				//Look for cipher or executeChallenge functions in the DLL
				cipher_func = (cipher_func_type)GetProcAddress(hLib, "cipher");
				execute_func = (execute_func_type)GetProcAddress(hLib, "executeChallenge");

				//Cipher DLL
				if (cipher_func != NULL) { //There is a Cipher function...
					printf("Cipher function exists. %ws is a cipher DLL. \n", dll);
					cipher.lib_handle = hLib;
					cipher.file_name = dll;
					cipher.block_size = 8;
					cipher.id = dll; //json name

					composed_key = malloc(1 * sizeof(struct KeyData));
					composed_key->size = 5;
					composed_key->data = malloc(composed_key->size, sizeof(byte));
					composed_key->data = (byte*)"12345";
					composed_key->expires = 0;

					//Init function call from the DLL
					cipher_init_func = (cipher_init_func_type)GetProcAddress(hLib, "init");
					if (cipher_init_func != NULL) {
						int result = cipher_init_func(&cipher);
						if (result != 0) {
							PRINT("WARNING: error \n");
						}
					}
					else {
						PRINT("WARNING: error accessing the address to the init() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
					}

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
							
					char* file_ciphered = "el_quijote_ciphered.txt";
					FILE* f_ciphered = NULL;
					byte* ciphered_buf = malloc(buf_size * sizeof(byte));

					char* file_deciphered = "el_quijote_deciphered.txt";
					FILE* f_deciphered = NULL;
					byte* deciphered_buf = malloc(buf_size * sizeof(byte));

					//Smaller buffers for specific tests
					char* message_test = "Hola mundo estoy haciendo pruebas";
					tam_test = strlen(message_test);
					DWORD buf_size_test = tam_test;
					byte* ciphered_buf_test = malloc(buf_size_test * sizeof(byte));
					byte* deciphered_buf_test = malloc(buf_size_test * sizeof(byte));

					//Cipher and decipher source buffers for testing
						
					//Ciphering smaller buffer (original -> cipher)
					result = cipher_func(ciphered_buf_test, message_test, buf_size_test, offset, composed_key);
					//Ciphering original buffer (original -> cipher)
					long long int begin = GetTickCount64();
					result = cipher_func(ciphered_buf, message, buf_size, offset, composed_key);
					long long int end = GetTickCount64();
					f_ciphered = fopen(file_ciphered, "wb");
					fwrite(ciphered_buf, 1, tam, f_ciphered);
					if (result != 0) {
						PRINT("WARNING: error ciphering '%ws' (error: %d)\n", dll, GetLastError()); ///*** Especificar error si es posible
					}
					
					//Deciphering
					decipher_func = (cipher_func_type)GetProcAddress(hLib, "decipher");
					if (decipher_func != NULL) {
						//Smaller buffer (original -> decipher)
						result = decipher_func(deciphered_buf_test, message_test, buf_size_test, offset, composed_key);
						//Original buffer (cipher -> decipher)
						result = decipher_func(deciphered_buf, ciphered_buf, buf_size, offset, composed_key);
						f_deciphered = fopen(file_deciphered, "wb");
						fwrite(deciphered_buf, 1, tam, f_deciphered);
						if (result != 0) {
							PRINT("WARNING: error deciphering '%ws' (error: %d)\n", dll, GetLastError());
						}
					}
					else {
						PRINT("WARNING: error accessing the address to the decipher() function of the cipher '%s' (error: %d)\n", dll, GetLastError());
					}
							
					do {
						ciph_menu = FALSE;
						printf("\n\n");
						printf("  _______________________  \n");
						printf(" |                       | \n");
						printf(" |      CIPHER MENU      | \n");
						printf(" |_______________________| \n");
						printf("\n");

						printf("Select an option:\n");
						printf("  1) Functionality test \n");
						printf("  2) Robustness test \n");
						printf("  0) Back to main menu \n");
						if (fgets(line, sizeof(line), stdin)) {
							if (1 == sscanf(line, "%d", &choice)) {
								switch (choice) {
								case 1:
																														
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
													//Checks if the size of ciphered and deciphered files are the same.
													size_test(file_ciphered, file_deciphered);
													break;
												case 2:
													//Checks if the cipher and decipher operations are executed correctly.***
													//Mostrar 10 bytes random
													byte_test(buf_size, message, ciphered_buf, deciphered_buf);
													break;
												case 3:
													//***PRINT_HEX(ciphered_buf_test,tam_test)
													printf("Clear text:%s\nFrom clear to ciphered:%s\nFrom clear to deciphered:%s\n", message_test, ciphered_buf_test, deciphered_buf_test);
													break;
												case 4:
													printf("Text in clear:%s\nFrom clear to deciphered:%s\n", message_test, deciphered_buf_test);
													break;
												case 5:
													printf("Running all tests...\n");
													size_test(file_ciphered, file_deciphered);
													byte_test(buf_size, message, ciphered_buf, deciphered_buf);
													printf("Clear text:%s\nFrom clear to ciphered:%s\nFrom clear to deciphered:%s\n", message_test, ciphered_buf_test, deciphered_buf_test);
													printf("Text in clear:%s\nFrom clear to deciphered:%s\n", message_test, deciphered_buf_test);
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
									double elapsed = (end - begin) * 1e-3;
									printf("Time to complete cipher: %.3f seconds.\n", elapsed);
									HANDLE handle_aux = CreateFileA(file_ciphered, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
									size_t file_size_cip = 0;
									getFileSize(&file_size_cip, handle_aux, file_ciphered);
									CloseHandle(handle_aux);
									printf("Bytes ciphered: %llub.\n", file_size_cip);
									//***Faltan los bytes/seg
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
					printf("ExecuteChallenge function exists. %ws is a challenge DLL. \n", dll);
					printf("Introduce your json parameters file for this challenge.\n");
					//fgets(line, sizeof(line), stdin);
					scanf("%s", &line);
					int resload = loadPropertiesjson(line,&challenge);
					if (resload != 0) {
						if (hLib != NULL) FreeLibrary(hLib);
						continue;
					}
					challenge_group.id = "Grupo0";
					challenge_group.challenges = malloc(sizeof(struct Challenge));
					challenge_group.subkey = malloc(sizeof(struct KeyData));
					InitializeCriticalSection(&(challenge_group.subkey->critical_section));
					challenge_group.subkey->data = calloc(1000, sizeof(char)); //Alocamos memoria para 1000B porque no conocemos la longitud que va a retornar el challenge
					//PRINT_HEX(challenge_group.subkey->data, 4);

					challenge.file_name = dll;
					challenge.lib_handle = hLib;

					challenge_group.challenges[0] = &challenge;

					periodicExecution_func = (periodicExecution_func_type)GetProcAddress(challenge_group.challenges[0]->lib_handle, "periodicExecution");
					if (periodicExecution_func == NULL) {
						printf("La funcion periodicExecution no está en la DLL\n");
						if (hLib != NULL) FreeLibrary(hLib);
						continue;
					}
					ch_init_func = (ch_init_func_type)GetProcAddress(challenge_group.challenges[0]->lib_handle, "init");
					if (ch_init_func == NULL) {
						printf("La funcion init no está en la DLL\n");
						if (hLib != NULL) FreeLibrary(hLib);
						continue;
					}
					execute_func = (execute_func_type)GetProcAddress(challenge_group.challenges[0]->lib_handle, "executeChallenge");
					if (execute_func == NULL) {
						printf("La funcion execute no está en la DLL\n");
						if (hLib != NULL) FreeLibrary(hLib);
						continue;
					}
					printf("Init y exe cargado y se va a llamar\n");
					periodicExecution_func(FALSE);
					result = ch_init_func(&challenge_group, &challenge); //Solo pruebo el init, porque este hace el primer execute
					printf("Init invocado con exito\n");
					if (0 != result) {
						printf("Error in init function\n");
						if (hLib != NULL) FreeLibrary(hLib);
						continue;
					}
					result = execute_func();
					printf("Execute invocado con exito\n");
					if (0 != result) {
						printf("Error in execute function\n");
						if (hLib != NULL) FreeLibrary(hLib);
						continue;
					}
					if (challenge_group.subkey->size > 1000 || challenge_group.subkey->size <= 0) {
						printf("Longitud de clave invalida %d.\n", challenge_group.subkey->size);
						if (hLib != NULL) FreeLibrary(hLib);
						continue;
					}
					PRINT_HEX(challenge_group.subkey->data, challenge_group.subkey->size);
					printf("Challenge validado correctamente\n");
					//Sleep(60000);
				}
				else {
				printf("There is not a Cipher or ExecuteChallenge function in your DLL.\n");
				}
			}
			else printf("DLL not loaded.\n");	
			if (hLib != NULL) FreeLibrary(hLib);
		}
	} while (TRUE);	
}