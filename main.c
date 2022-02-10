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

	handle_ciphered = CreateFileA("el_quijote_cifrado.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	handle_deciphered = CreateFileA("el_quijote_descifrado.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	error_code = getFileSize(&file_size_ciphered, handle_ciphered, L"el_quijote_cifrado.txt");
	if (error_code != ERROR_SUCCESS) {
		printf("\nError obtaining file size\n");
	}
	printf("\nThe detected file size is %llu\n", file_size_ciphered);

	error_code = getFileSize(&file_size_deciphered, handle_deciphered, L"el_quijote_descifrado.txt");
	if (error_code != ERROR_SUCCESS) {
		printf("\nError obtaining file size\n");
	}
	printf("\nThe detected file size is %llu\n", file_size_deciphered);
}

void byte_test(DWORD b_size, char* msg, byte* cip_buf, byte* dec_buf) {
	char caracter;
	char caracter_ciphered;
	char caracter_deciphered;

	caracter = msg[100];
	caracter_ciphered = cip_buf[100];
	caracter_deciphered = dec_buf[100];

	printf("En claro:%c\nCifrado:%c\nDescifrado:%c\n", caracter, caracter_ciphered, caracter_deciphered);
	}

void challengeMenu() {
	char line[500] = { 0 };
	int choice = 0;

	printf("\n Select an option:\n");
	printf("  1) Run challenge \n");
	printf("  0) Back to main menu \n");
	if (fgets(line, sizeof(line), stdin)) {
		if (1 == sscanf(line, "%d", &choice)) {
			switch (choice) {
			case 1:
				printf("Running...\n");
				break;
			case 0:
				printf("Main menu... \n");
				break;
			default:
				printf("Invalid option, try again.\n");
				break;
			}
		}
	}
}

void main() {

	printf("\n\n\n");
	printf("  _______________________  \n");
	printf(" |                       | \n");
	printf(" |     DLL VALIDATOR     | \n");
	printf(" |_______________________| \n");
	printf("\n");

	BOOL ciph_menu = FALSE;
	BOOL main_menu = FALSE;

	char line[SIZE_OF_BUFF] = { 0 };
	char dll[SIZE_OF_BUFF] = { 0 };
	int result;
	size_t tam;
	size_t tam_test;

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

	void byte_test();

	
	//Creamos KeyData y puntero a Keydata
	unsigned char arr1[] = { 10, 20, 30, 40, 50 };
	struct KeyData *composed_key;

	composed_key = malloc(1 * sizeof(struct KeyData));
	composed_key->size = 5;
	composed_key->data = malloc(composed_key->size, sizeof(byte));
	composed_key->expires = 0;
	
	//Creamos estructura Cipher y puntero a Cipher
	struct Cipher* ptr_cipher,cipher;

	//Pedir input de fichero dll
	printf("Introduce DLL name:\n");
	if (fgetws(line, sizeof(line), stdin)) { //line es wchar_t
		if (1 == swscanf(line, L"%ls", dll)) {  //Para formatear la entrada por teclado.

		//Comprobar si la DLL existe, cargarla si es así.
			//hLib = LoadLibraryA(dll);
			hLib = LoadLibraryW(dll);
			if (hLib != NULL) {
				printf("DLL loaded. \n");
				//Comprobar si en la DLL existe una función cipher o execute
				cipher_func = (cipher_func_type)GetProcAddress(hLib, "cipher");
				execute_func = (execute_func_type)GetProcAddress(hLib, "executeChallenge");

				//Si existe, es una DLL de cipher
				if (cipher_func != NULL) {
					printf("%ws is a cipher DLL. \n",dll);
					//Rellenar struct Cipher
					cipher.lib_handle = hLib;
					cipher.file_name = dll;
					cipher.block_size = 8;
					cipher.id = dll; //es el nombre que tendrá en el json
					//cipher.custom;
					ptr_cipher = &cipher;
					
					//Llamar a funcion init cuando tenga todos los parametros de cipher
					cipher_init_func = (cipher_init_func_type)GetProcAddress(hLib, "init");
					if (cipher_init_func != NULL) {
						int result = cipher_init_func(ptr_cipher); //pasamos puntero a cipher
						if (result != 0) {
							PRINT("WARNING: error \n");
						}
					}
					else {
						PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
					}

					do {
						char line[SIZE_OF_BUFF] = {0};
						int choice = 0;
												
						//Creamos buffer de origen
						char* file = "el_quijote.txt";
						FILE* f_original = NULL;
						f_original = fopen(file, "r"); //suponiendo que la ruta de el fichero este en la misma ubicación que el .exe de la aplicación. Si no, pones la ruta tal cual.
						char* message;
						message = inputFile(f_original, 1);
						tam = strlen(message);
						fclose(f_original);

						//Creamos buffers de destino
						DWORD buf_size = tam;
						size_t offset = 0;
						byte* ciphered_buf = malloc(buf_size * sizeof(byte));
						byte* deciphered_buf = malloc(buf_size * sizeof(byte));

						//Buffers más pequeños para pruebas especificas
						char* message_test = "Hola mundo";
						tam_test = strlen(message_test);
						DWORD buf_size_test = tam_test;
						byte* ciphered_buf_test = malloc(buf_size_test * sizeof(byte));
						byte* deciphered_buf_test = malloc(buf_size_test * sizeof(byte));

						printf("\nSelect an option:\n");
						printf("  1) Functionality test \n");
						printf("  2) Robustness test \n");
						printf("  3) Cipher/Decipher \n");
						printf("  0) Back to main menu \n");
						if (fgets(line, sizeof(line), stdin)) {
							if (1 == sscanf(line, "%d", &choice)) {
								switch (choice) {
								case 1:
									printf("FUNCTIONALTY TEST \n");
									//Ciframos y desciframos el archivo original para poder realizar las pruebas.

									//Llamada a funcion cipher
									cipher_func = (cipher_init_func_type)GetProcAddress(hLib, "cipher");
									if (cipher_func != NULL) {
										//printf("\nTam quijote: %d\nTam buffer salida: %d\n", tam, _msize(ciphered_buf));
										result = cipher_func(ciphered_buf_test, message_test, buf_size_test, offset, composed_key);
										result = cipher_func(ciphered_buf, message, buf_size, offset, composed_key);
										FILE* f_ciphered = NULL;
										f_ciphered = fopen("el_quijote_cifrado.txt", "wb");
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
									//Llamada a funcion decipher
									decipher_func = (cipher_init_func_type)GetProcAddress(hLib, "decipher");
									if (decipher_func != NULL) {
										result = decipher_func(deciphered_buf_test, message_test, buf_size_test, offset, composed_key);
										result = decipher_func(deciphered_buf, ciphered_buf, buf_size, offset, composed_key);
										FILE* f_deciphered = NULL;
										f_deciphered = fopen("el_quijote_descifrado.txt", "wb");
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
													//Usar buffers y memcopy
													byte_test(buf_size, message, ciphered_buf, deciphered_buf);
													break;
												case 3:
													printf("Texto en claro:%s\n De claro a cifrado:%s\n De claro a descifrado:%s\n", message_test, ciphered_buf_test, deciphered_buf_test);
													break;
												case 4:
													printf("Texto en claro:%s\n De claro a descifrado:%s\n", message_test, deciphered_buf_test);
													break;
												case 5:
													printf("Running all tests...\n");
													size_test();
													byte_test(buf_size, message, ciphered_buf, deciphered_buf);
													printf("Texto en claro:%s\n De claro a cifrado:%s\n De claro a descifrado:%s\n", message_test, ciphered_buf_test, deciphered_buf_test);
													printf("Texto en claro:%s\n De claro a descifrado:%s\n", message_test, deciphered_buf_test);
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
									//Llamada a funcion cipher
									cipher_func = (cipher_init_func_type)GetProcAddress(hLib, "cipher");
									if (cipher_func != NULL) {
										printf("Tam quijote: %d\nTam quijote_cifrado: %d\n", tam, _msize(ciphered_buf));
										result = cipher_func(ciphered_buf, message, buf_size, offset, composed_key); //argumentos - propiedades
										FILE* f_ciphered = NULL;
										f_ciphered = fopen("el_quijote_cifrado.txt", "wb");
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
									//Llamada a funcion decipher
									decipher_func = (cipher_init_func_type)GetProcAddress(hLib, "decipher");
									if (decipher_func != NULL) {
										printf("Tam quijote_cifrado: %d\nTam quijote_descifrado: %d\n", _msize(ciphered_buf), _msize(deciphered_buf));
										result = decipher_func(deciphered_buf, ciphered_buf, buf_size, offset, composed_key); //argumentos - propiedades
										FILE* f_deciphered = NULL;
										f_deciphered = fopen("el_quijote_descifrado.txt", "wb");
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
				else if (execute_func != NULL ) {
					printf("%ws is a challenge DLL. \n",dll);
					//PRUEBAS CHALLENGE
				}
			}				
			else {
				printf("DLL not loaded \n");
			}
			FreeLibrary(hLib);			
		} 
	}
}

void validateChallenge() {

	//Primer paso: pedir nombre de la DLL
	char line[500] = { 0 };
	char dll[100] = { 0 };
	HINSTANCE hLib;
	//typedef int(__stdcall* init_func_type)(int);
	typedef int(__stdcall* challenge_func_type)(struct Challenge*);
	challenge_func_type challenge_func;
	int result;

	/*
	LPVOID ciphered_buf = NULL;
	LPVOID src_buf = "Probando buffers";
	DWORD buf_size = 512;
	*/

	//Creamos estructura Challenge y EqChallenge
	struct Challenge challenge;
	struct ChallengeEquivalenceGroup eqchallenge;

	//Creamos puntero a challenge
	struct Challenge *p_challenge;
	struct ChallengeEquivalenceGroup *p_eqchallenge;

	//Creamos KeyData según pdf
	struct KeyData subkey;
	subkey.data = NULL;
	subkey.expires = 0;
	subkey.size = 0;

	//Creamos puntero a KeyData
	struct Keydata *p_subkey;
	p_subkey = &subkey;

	//Pedir input de fichero dll
	printf("Introduce DLL name:\n");
	//fgetws para conseguir wchar
	if (fgetws(line, sizeof(line), stdin)) {
		if (1 == sscanf(line, "%s", dll)) {
			//Comprobar si la DLL existe.
					//Carga la DLL (si existe)
			hLib = LoadLibraryA(dll);
			if (hLib != NULL) {
				printf("DLL loaded. \n");
				//Rellenar struct Challenge
				challenge.lib_handle = hLib;
				//***Tengo el nombre de la DLL (string), necesito un WCHAR.

				//challenge.file_name = (WCHAR)dll;
				//challenge.properties = "";

				//eqchallenge.challenges = ;
				//eqchallenge.id = ;
				//eqchallenge.subkey = p_subkey;
								
				//p_challenge = &challenge;

				/*
				//Llamar a funcion init cuando tenga todos los parametros de challenge
				challenge_func = (challenge_func_type)GetProcAddress(hLib, "init");
				if (challenge_func != NULL) {
					result = challenge_func(p_eqchallenge,p_challenge); //argumentos - propiedades
					if (result != 0) {
						PRINT("WARNING: error \n");
					}
				}
				else {
					PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
				}
				sleep(2000);
				*/

			}
			else {
				printf("DLL not loaded \n");
			}

			//(key->expires > tiempo actual)

			FreeLibrary(hLib);
		}
	}


	
}

/*void menu() {

	do {

		printf("\n\n\n");
		printf("  _______________________  \n");
		printf(" |                       | \n");
		printf(" |     VALIDATOR  MENU   | \n");
		printf(" |_______________________| \n");
		printf("\n");

		printf("\n");
		printf("Select an option:\n");
		printf("  1)  \n");
		printf("  2) Validate cipher \n");
		printf("  3) Validate challenge \n");
		printf("  0) Exit\n");
		if (fgets(line, sizeof(line), stdin)) { //reads a line from the specified stream and stores it into line. stdin = keyboard
			if (1 == sscanf(line, "%d", &choice)) { //sscanf returns the number of variables filled (1 in this case)
				switch (choice) {
				case 1:
					printf("Introduce DLL name:\n");
					if (fgets(line, sizeof(line), stdin)) {
						if (1 == sscanf(line, "%s", dll)) {
							validateDLL(dll);
							//ciphered_buf = src_buf;
							//printf(ciphered_buf);
						}
					}
					break;
				case 2:
					printf("\n\n\n");
					printf("  _______________________  \n");
					printf(" |                       | \n");
					printf(" |     CIPHER  MENU      | \n");
					printf(" |_______________________| \n");
					printf("\n");
					validateCipher();
					break;
				case 3:
					printf("Case 3. \n");
					//memcpy(buf, &test, strlen(test) + 1);
					//printf("%s", &buf);
					break;
				case 0:
					printf("Exitting...\n");
					quit_menu = TRUE;
					break;
				default:
					printf("Invalid option, try again.\n");
					break;
				}
			}
		}
	} while (!quit_menu);

}*/

/*
// Get the path
	//printf("\n\tEnter the full path of the file from which you want to create a .uva file below.\n");
	//printf("\t--> ");
	if (fgetws(file_path, MAX_PATH, stdin)) {
		file_path[wcslen(file_path) - 1] = '\0';        // End the buffer with null character for the case in which fgets() filled it completely
		if (!PathFileExistsW(file_path)) {
			printf("\tThe specified path does not exist.\n");
			return;
		}
		if (PathIsDirectoryW(file_path)) {
			printf("\tThe specified path matches a directory not a file.\n");
			return;
		}
	}

*/

//LPVOID means 'Long Pointer to Void, it is simply a typedef for void*. In other words, a generic pointer which can point to anything you want.