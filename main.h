
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
int loadPropertiesjson(char* json_file, struct Challenge* ch);

void mainMenuLoop();

void challengeExecutorLoop();
int execChallengeFromMainThread(struct ChallengeEquivalenceGroup* ch_group, struct Challenge* ch);
int configureExecChFromMain(struct Challenge);


