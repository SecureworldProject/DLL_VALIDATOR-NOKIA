#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include "context.h"
#include <fileapi.h>
#include <time.h>
#define SIZE_OF_BUFF 500

void cipherMenu() {

	char line[500] = { 0 };
	int choice = 0;

	printf("\n Select an option:\n");
	printf("  1) Cipher (type key) \n");
	printf("  2) Decipher (type key) \n");
	printf("  0) Back to main menu \n");
	if (fgets(line, sizeof(line), stdin)) {
		if (1 == sscanf(line, "%d", &choice)) {
			switch (choice) {
			case 1:
				printf("Ciphering...\n");
				break;
			case 2:
				printf("Deciphering...\n");
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

BOOL validateDLL(char library[]) {

	HINSTANCE hLib;
	typedef void(__stdcall* funcion_importada)(); //__stdcall que sirve para llamadas a funciones y se usa en Windows para llamar a funciones de la API deWin32
	funcion_importada funcion_dll; //funcion_importada es el tipo de la variable funcion_dll

	hLib = LoadLibraryA(library);
	if (hLib != NULL) {
		printf("DLL loaded. \n");
		funcion_dll = (funcion_importada)GetProcAddress(hLib, "saludar");
		funcion_dll();
		FreeLibrary(hLib);
	}
	else {
		printf("DLL not loaded \n");
	}
	
	return (hLib != NULL);

}

void validateCipher() {

	printf("\n\n\n");
	printf("  _______________________  \n");
	printf(" |                       | \n");
	printf(" |     CIPHER  MENU      | \n");
	printf(" |_______________________| \n");
	printf("\n");

	BOOL func_menu = FALSE;
	BOOL ciph_menu = FALSE;
	BOOL main_menu = FALSE;

	char line[SIZE_OF_BUFF] = { 0 };
	char dll[SIZE_OF_BUFF] = { 0 };
	int result;
	size_t tam;

	HINSTANCE hLib;

	typedef int(__stdcall* init_func_type)(struct Cipher*);
	init_func_type init_func;

	typedef int(__stdcall* cipher_func_type)(LPVOID,LPCVOID,DWORD,size_t,struct KeyData*);
	cipher_func_type cipher_func;
	cipher_func_type decipher_func;
	
	//Creamos KeyData y puntero a Keydata
	unsigned char arr1[] = { 10, 20, 30, 40, 50 };
	struct KeyData *composed_key;

	composed_key = malloc(1 * sizeof(struct KeyData));
	composed_key->size = 5;
	composed_key->data = malloc(composed_key->size, sizeof(byte));
	composed_key->expires = 0;
	

	//ptr_composed_key = &composed_key;

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
				//Comprobar si en la DLL existe una función cipher
				cipher_func = (cipher_func_type)GetProcAddress(hLib, "cipher");
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
					init_func = (init_func_type)GetProcAddress(hLib, "init");
					if (init_func != NULL) {
						int result = init_func(ptr_cipher); //pasamos puntero a cipher
						if (result != 0) {
							PRINT("WARNING: error \n");
						}
					}
					else {
						PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
					}

					do {
						char line[SIZE_OF_BUFF] = {0};
						//char *file_cip;
						int choice = 0;

						printf("\n Select an option:\n");
						printf("  1) Functionality test \n");
						printf("  2) Robustness test \n");
						printf("  3) Cipher/Decipher \n");
						printf("  0) Back to main menu \n");
						if (fgets(line, sizeof(line), stdin)) {
							if (1 == sscanf(line, "%d", &choice)) {
								switch (choice) {
								case 1:
									printf("FUNCTIONALTY TEST \n");
									do {
										printf("\n Select an option:\n");
										printf("  1) Size \n");
										printf("  2) Byte \n");
										printf("  3) Cipher not Decipher \n");
										printf("  4) Decipher is valid \n");
										printf("  0) Back to main menu \n");
										func_menu = FALSE;
									} while (!func_menu);
									break;
								case 2:
									printf("ROBUSTNESS TEST \n");
									break;
								case 3:
									printf("CIPHER or DECIPHER\n");
									//PEDIR ARCHIVO									
									do {
										/*printf("Enter file's path:");
										//fgets(file, SIZE_OF_BUFF,stdin);
										printf("Looking for the '.' character in \"%s\"...\n", file);
										//file_cip = strchr(file, '.');
										
										while (file_cip != NULL)
										{
											printf("found at %d\n", file_cip - file + 1);
											file_cip = strchr(file_cip + 1, '.');
										}
										char *match = '\0';
										printf("Terminado \n");
										char* resultado[20];
										int fileExtPos = IndexOf(file, ".");
										memcpy(resultado, &file[0],sizeof(file)-fileExtPos);
										resultado[sizeof(resultado)+1] = '\0';
										printf(resultado);

										*/


										//Buffer de tipo archivo (origen)
										//byte* src_buf = ""; Usamos un archivo como buffer de entrada
										char* file = "el_quijote.txt";
										FILE* fp = NULL;
										fp = fopen(file, "r"); //suponiendo que la ruta de el fichero este en la misma ubicación que el .exe de la aplicación. Si no, pones la ruta tal cual.
										char* message;
										message = inputFile(fp, 1);
										tam = strlen(message);
										
										fclose(fp);

										//Creamos buffers destino
										DWORD buf_size = tam;
										size_t offset = 0;
										byte* dst_buf = malloc(buf_size * sizeof(byte));
										byte* deciphered_buf = malloc(buf_size * sizeof(byte));

										printf("\n Select an option:\n");
										printf("  1) Cipher \n");
										printf("  2) Decipher \n");
										printf("  0) Back to main menu \n");
										if (fgets(line, sizeof(line), stdin)) {
											if (1 == sscanf(line, "%d", &choice)) {
												switch (choice) {
												case 1:
													//Llamada a funcion cipher
													cipher_func = (init_func_type)GetProcAddress(hLib, "cipher");
													if (cipher_func != NULL) {
														printf("\nTam quijote: %d\nTam buffer salida: %d\n", tam,_msize(dst_buf));
														result = cipher_func(dst_buf, message, buf_size, offset, composed_key); //argumentos - propiedades
														printf("\nResult: %d\n", result);
														FILE* fq = NULL;
														fq = fopen("el_quijote_cifrado.txt", "w");
														//fputs(dst_buf, fq);
														fwrite(dst_buf, 1, tam, fq);
														fclose(fq);

														//free(dst_buf);
														//printf("\n(%s)\n", dst_buf);
														if (result != 0) {
															PRINT("WARNING: error \n");
														}
													}
													else {
														PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
													}
													decipher_func = (init_func_type)GetProcAddress(hLib, "decipher");
													if (decipher_func != NULL) {
														result = decipher_func(deciphered_buf, dst_buf, buf_size, offset, composed_key); //argumentos - propiedades
														//printf("\n(%s)\n", dst_buf);
														FILE* fr = NULL;
														fr = fopen("el_quijote_descifrado.txt", "w");
														//fputs(deciphered_buf, fr);
														fwrite(deciphered_buf, 1, tam, fr);
														fclose(fr);
														if (result != 0) {
															PRINT("WARNING: error \n");
														}
													}
													else {
														PRINT("WARNING: error accessing the address to the cipher() function of the cipher '%ws' (error: %d)\n", dll, GetLastError());
													}
													break;
												case 2:
													decipher_func = (init_func_type)GetProcAddress(hLib, "decipher");
													if (decipher_func != NULL) {
														result = decipher_func(deciphered_buf,dst_buf, buf_size, offset, composed_key); //argumentos - propiedades
														//printf("\n(%s)\n", dst_buf);
														FILE* fr = NULL;
														fr = fopen("el_quijote_descifrado.txt", "w");
														//fputs(deciphered_buf, fr);
														fwrite(deciphered_buf, 1, tam, fr);
														fclose(fr);
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
				else {
					printf("%s is not a cipher DLL. \n",dll);
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
	LPVOID dst_buf = NULL;
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

void main() {

	//////////////////////////////////////////
	LPVOID src_buf = "Probando buffers";
	LPVOID dst_buf = NULL;
	//char* test = "Probando buffers";
	/////////////////////////////////////////

	char line[150] = { 0 };
	int choice = 0;
	char dll[150] = { 0 };
	BOOL quit_menu = FALSE;
	//HINSTANCE hLib;
			
	do {

		printf("\n\n\n");
		printf("  _______________________  \n");
		printf(" |                       | \n");
		printf(" |     VALIDATOR  MENU   | \n");
		printf(" |_______________________| \n");
		printf("\n");

		printf("\n");
		printf("Select an option:\n");
		printf("  1) Validate dll \n");
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
							//dst_buf = src_buf;
							//printf(dst_buf);
						}
					}
					break;
				case 2:
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
							//dst_buf = src_buf;
							//printf(dst_buf);
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