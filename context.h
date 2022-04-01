/*
* SecureWorld file context.h
contiene la estructura en C en la que se carga el contexto escrito en el fichero config.json, y que contiene la configuración de todo el sistema.
Los distintos proyectos (challenges, mirror, app descifradora etc) deben incluir este fichero para compilar pues sus DLL
se invocarán con un parámetro de tipo contexto que se definirá en context.h

Nokia Febrero 2021
*/

#ifndef context_h
#define context_h

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"
#include "json.h"
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif	//__cplusplus


	/////  DEFINITIONS  /////

#ifndef NULL
#define NULL ((void*)0)
#endif //NULL

#ifndef MAX_PATH
#define MAX_PATH 260
#endif //MAX_PATH

#ifndef SUBKEY_SIZE
#define SUBKEY_SIZE 64
#endif //SUBKEY_SIZE


#define NOOP ((void)0)
#define ENABLE_PRINTS 1					// Affects the PRINT() functions. If 0 does not print anything. If 1 traces are printed.
#define PRINT(...) do { if (ENABLE_PRINTS) printf(__VA_ARGS__); else NOOP;} while (0)
#define PRINT1(...) PRINT("    "); PRINT(__VA_ARGS__)
#define PRINT2(...) PRINT("        "); PRINT(__VA_ARGS__)
#define PRINT3(...) PRINT("            "); PRINT(__VA_ARGS__)
#define PRINT4(...) PRINT("                "); PRINT(__VA_ARGS__)
#define PRINT5(...) PRINT("                    "); PRINT(__VA_ARGS__)
#define PRINTX(DEPTH, ...) do { if (ENABLE_PRINTS) { for (int x=0; x<DEPTH; x++){ printf("    ");} printf(__VA_ARGS__); } else NOOP;} while (0)




/////  FUNCTION PROTOTYPES  /////

	static struct App* createApp();
	static void destroyApp(struct App** app);

	enum Operation getTableOperation(enum IrpOperation irp_operation, WCHAR** app_full_path, WCHAR mount_point);

	inline struct OpTable* getTable(WCHAR mount_point);
	inline struct App* getApp(WCHAR* app_full_path);
	inline WCHAR getDiskType(WCHAR* file_full_path);
	inline enum Operation* getOperations(enum AppType app_type, struct OpTable* table);
	void formatPathOLD(char** full_path);
	void formatPath(WCHAR** full_path);
	int fromDeviceToLetter(WCHAR** full_path);

	static void printContext();
	static void printChallengeGroups();
	static void printChallengeGroup(struct ChallengeEquivalenceGroup* group);
	static void printChallenge(struct Challenge* challenge);
	static void printDateNice(struct tm* time_info);
	static struct ChallengeEquivalenceGroup* getChallengeGroupById(char* group_id);
	static struct Cipher* getCipherById(char* group_id);
	static struct OpTable* getOpTableById(char* table_id);

	void printLastError(DWORD error_value);

	char* inputFile(FILE* fp, size_t size) {
		//The size is extended by the input with the value of the provisional
		char* str;
		int ch;
		size_t len = 0;
		str = realloc(NULL, sizeof(*str) * size);//size is start size
		if (!str)return str;
		while (EOF != (ch = fgetc(fp))) {//&& ch != '\n') {
			str[len++] = ch;
			if (len == size) {
				str = realloc(str, sizeof(*str) * (size += 16));
				if (!str)return str;
			}
		}
		str[len++] = '\0';

		return realloc(str, sizeof(*str) * len);
	}


	/////  CONTEXT STRUCTS AND ENUMS  /////

	enum IrpOperation {
		ON_READ,
		ON_WRITE
	};
#define NUM_IRP_OPERATIONS 2

	// FOR FUTURE USE - performance improvement: create a context for each volume (each mirror thread) and a context for each file handle (in PDOKAN_FILE_INFO.Context)
	/*
	struct VolumeContext {
		union {
			struct Folder* folder;		// Pointer to the struct folder of this volume
			struct Pendrive* pendrive;	// Pointer to the struct folder of this volume
		};
		WCHAR** sync_folders;			// Array of synchronization folders that affect this volume (after conversion from "C:\..." to "M:\...", "N:\...", etc)
		BOOL is_pendrive;				// If this volume is a pendrive (detected from USB and mounted as such)
	};

	struct FileContext {
		struct App* app;
		BOOL is_sync_folder;
	};
	*/

#pragma region Here is the context with all the associated structs and enums

	struct Context {
		struct Folder** folders;
		struct Pendrive* pendrive;
		struct ParentalControl** parentals;
		WCHAR** sync_folders;
		struct OpTable** tables;
		struct App** apps;
		struct ChallengeEquivalenceGroup** groups;
		struct Cipher** ciphers;
		struct ThirdParty** third_parties;
	} ctx;

	struct Folder {
		WCHAR* path;
		WCHAR mount_point;
		WCHAR* name;
		enum Driver driver;
		struct Protection* protection;
	};

	enum Driver {
		DOKAN,
		WINFSP
	};

	struct Protection {
		struct OpTable* op_table;
		struct ChallengeEquivalenceGroup** challenge_groups;
		struct Cipher* cipher;
		struct KeyData* key;
	};

	struct KeyData {
		byte* data;
		int size;
		time_t expires;		// In the case of the full key we will take the first expire date
		CRITICAL_SECTION critical_section;
	};

	struct Pendrive {
		WCHAR* mount_points;		// All  the possible letters to mount a pendrive, in a string like: "HIJKLMNOPQRSTUVWXYZ"
		enum Driver driver;
		struct Protection* protection;
	};

	struct ParentalControl {
		WCHAR* folder;
		WCHAR** users;
		struct ChallengeEquivalenceGroup** challenge_groups;
	};

	struct OpTable {
		char* id;
		struct Tuple** tuples;
	};

	struct Tuple {
		enum AppType app_type;
		enum Operation on_read;
		enum Operation on_write;
	};

	enum AppType {
		ANY,
		BROWSER,
		MAILER,
		BLOCKED
	};
	//const char* APP_TYPE_STRINGS[] = { "ANY, "BROWSER", "MAILER", "BLOCKED" };

	enum Operation {
		NOTHING,
		CIPHER,
		DECIPHER,
		MARK,
		UNMARK,
		IF_MARK_UNMARK_ELSE_CIPHER,
		IF_MARK_UNMARK_DECHIPHER_ELSE_NOTHING
	};

	struct App {		// We search by path and name, not by type
		WCHAR* path;
		WCHAR* name;
		enum AppType type;
	};

	struct ChallengeEquivalenceGroup {
		char* id;
		struct KeyData* subkey;		// Not obtained from json
		struct Challenge** challenges;
	};

	struct Challenge {
		WCHAR* file_name;
		HINSTANCE lib_handle;
		json_value* properties;		// Json_value of properties
	};

	struct Cipher {
		char* id;
		WCHAR* file_name;
		HINSTANCE lib_handle;
		int block_size;
		char* custom;
	};

	struct ThirdParty {
		char* id;
		struct Cipher* cipher;
		char* key;
	};

#pragma endregion




	/////  FUNCTION DEFINITIONS  /////

	/**
	* Prints all the information of the context (indented to ease the view of information).
	*
	* @return
	**/
	static void printContext() {
		PRINT("\n");
		PRINT("================================================================================\n");
		PRINT("PRINT CONTEXT BEGINS\n");

		// Folders
		PRINT("\n");
		PRINT("Folders:\n");
		for (int i = 0; i < _msize(ctx.folders) / sizeof(struct Folder*); i++) {
			PRINT1("Folder:\n");
			PRINT2("Path: %ws\n", ctx.folders[i]->path);
			PRINT2("Mount point: %wc\n", ctx.folders[i]->mount_point);
			PRINT2("Name: %ws\n", ctx.folders[i]->name);
			PRINT2("Driver: %d\n", ctx.folders[i]->driver);
			PRINT2("Protection\n");
			PRINT3("Op table: %s\n", ctx.folders[i]->protection->op_table->id);
			PRINT3("Challenge groups: ");
			for (int j = 0; j < _msize(ctx.folders[i]->protection->challenge_groups) / sizeof(char*); j++) {
				PRINT("%s%s", (char*)ctx.folders[i]->protection->challenge_groups[j]->id, (j + 1 < _msize(ctx.folders[i]->protection->challenge_groups) / sizeof(char*)) ? ", " : "\n");
			}
			PRINT3("Cipher: %s\n", ctx.folders[i]->protection->cipher->id);
		}

		// Pendrive
		PRINT("\n");
		PRINT("Pendrive:\n");
		PRINT1("Mount points: %ws\n", ctx.pendrive->mount_points);
		PRINT1("Driver: %d\n", ctx.pendrive->driver);
		PRINT1("Protection\n");
		PRINT2("Op table: %s\n", ctx.pendrive->protection->op_table->id);
		PRINT2("Challenge groups: ");
		for (int i = 0; i < _msize(ctx.pendrive->protection->challenge_groups) / sizeof(char*); i++) {
			PRINT("%s%s", (char*)ctx.pendrive->protection->challenge_groups[i]->id, (i + 1 < _msize(ctx.pendrive->protection->challenge_groups) / sizeof(char*)) ? ", " : "\n");
		}
		PRINT2("Cipher: %s\n", ctx.pendrive->protection->cipher->id);

		// Parental controls
		PRINT("\n");
		PRINT("Parental controls:\n");
		for (int i = 0; i < _msize(ctx.parentals) / sizeof(struct ParentalControl*); i++) {
			PRINT1("Parental control:\n");
			PRINT2("Folder: %ws\n", ctx.parentals[i]->folder);
			PRINT2("Challenge groups: ");
			for (int j = 0; j < _msize(ctx.parentals[i]->challenge_groups) / sizeof(char*); j++) {
				PRINT("%s%s", ctx.parentals[i]->challenge_groups[j]->id, (j + 1 < _msize(ctx.parentals[i]->challenge_groups) / sizeof(char*)) ? ", " : "\n");
			}
			PRINT2("Users: ");
			for (int j = 0; j < _msize(ctx.parentals[i]->users) / sizeof(WCHAR*); j++) {
				PRINT("%ws%s", ctx.parentals[i]->users[j], (j + 1 < _msize(ctx.parentals[i]->users) / sizeof(WCHAR*)) ? ", " : "\n");
			}
		}

		// Sync folders
		PRINT("\n");
		PRINT("Sync folders: ");
		for (int i = 0; i < _msize(ctx.sync_folders) / sizeof(WCHAR*); i++) {
			PRINT("%ws%s", ctx.sync_folders[i], (i + 1 < _msize(ctx.sync_folders) / sizeof(WCHAR*)) ? ", " : "\n");
		}

		// Operative tables
		PRINT("\n");
		PRINT("Tables:\n");
		for (int i = 0; i < _msize(ctx.tables) / sizeof(struct OpTable*); i++) {	// Iterate over tables
			PRINT1("Table id: %s \n", ctx.tables[i]->id);
			PRINT2(" _____________________________________________ \n");
			PRINT2("|       |            |           |            |\n");
			PRINT2("|  Row  |  App Type  |  On Read  |  On Write  |\n");
			PRINT2("|_______|____________|___________|____________|\n");
			PRINT2("|       |            |           |            |\n");

			for (int j = 0; j < _msize(ctx.tables[i]->tuples) / sizeof(struct Tuple*); j++) {	// Iterate over rows of each table (the so called "table tuples")
				PRINT2("|  %2d   |     %2d     |    %2d     |     %2d     |\n",
					j,
					ctx.tables[i]->tuples[j]->app_type,
					ctx.tables[i]->tuples[j]->on_read,
					ctx.tables[i]->tuples[j]->on_write
				);
			}
			PRINT2("|_______|____________|___________|____________|\n");
		}

		// Apps
		PRINT("\n");
		PRINT("Apps:\n");
		for (int i = 0; i < _msize(ctx.apps) / sizeof(struct App*); i++) {
			PRINT1("App:\n");
			PRINT2("Path: %ws \n", ctx.apps[i]->path);
			PRINT2("Name: %ws \n", ctx.apps[i]->name);
			PRINT2("Type: %d \n", ctx.apps[i]->type);
		}

		// Challenge Equivalence Groups
		printChallengeGroups(ctx);

		// Ciphers
		PRINT("\n");
		PRINT("Ciphers:\n");
		for (int i = 0; i < _msize(ctx.ciphers) / sizeof(struct Cipher*); i++) {
			PRINT1("Cipher:\n");
			PRINT2("Id: %s \n", ctx.ciphers[i]->id);
			PRINT2("Filename: %ws \n", ctx.ciphers[i]->file_name);
			PRINT2("Blocksize: %d \n", ctx.ciphers[i]->block_size);
			PRINT2("Custom: %s \n", ctx.ciphers[i]->custom);
		}

		// Third Parties
		PRINT("\n");
		PRINT("Third Parties:\n");
		for (int i = 0; i < _msize(ctx.third_parties) / sizeof(struct ThirdParty*); i++) {
			PRINT1("Third Party:\n");
			PRINT2("Id: %s \n", ctx.third_parties[i]->id);
			PRINT2("Cipher: %s \n", ctx.third_parties[i]->cipher->id);
			PRINT2("Key: '%s' \n", ctx.third_parties[i]->key);
		}

		PRINT("\n");
		PRINT("PRINT CONTEXT ENDS\n");
		PRINT("================================================================================\n");
	}


	/**
	* Prints the information of all the challenge groups (maintaining indentation from context).
	*
	* @return
	**/
	static void printChallengeGroups() {
		struct tm* time_info;

		PRINT("\n");
		PRINT("Challenge Equivalence Groups:\n");
		for (int i = 0; i < _msize(ctx.groups) / sizeof(struct ChallengeEquivalenceGroup*); i++) {
			PRINT1("EqGroup:\n");
			PRINT2("Id: %s \n", ctx.groups[i]->id);
			PRINT2("Subkey data: %s \n", ctx.groups[i]->subkey->data);
			PRINT2("Subkey size: %d \n", ctx.groups[i]->subkey->size);
			time_info = localtime(&(ctx.groups[i]->subkey->expires));
			PRINT2("Subkey expires: %lld (", ctx.groups[i]->subkey->expires); printDateNice(time_info); PRINT(")\n");
			PRINT2("Challenges: \n");
			for (int j = 0; j < _msize(ctx.groups[i]->challenges) / sizeof(struct Challenge*); j++) {
				printChallenge(ctx.groups[i]->challenges[j]);
			}
		}
	}

	/**
	* Prints the information of the ChallengeEquivalenceGroup passed by parameter (maintaining indentation from context).
	*
	* @param struct ChallengeEquivalenceGroup* group
	*		The ChallengeEquivalenceGroup which info must be printed.
	*
	* @return
	**/
	static void printChallengeGroup(struct ChallengeEquivalenceGroup* group) {
		struct tm* time_info;

		if (group == NULL) {
			PRINT("ERROR\n");
		}
		else {
			PRINT1("EqGroup:\n");
			PRINT2("Id: %s \n", group->id);
			PRINT2("Subkey data: %s \n", group->subkey->data);
			PRINT2("Subkey size: %d \n", group->subkey->size);
			time_info = localtime(&(group->subkey->expires));
			PRINT2("Subkey expires: %lld (", group->subkey->expires); printDateNice(time_info); PRINT(")\n");
			PRINT2("Challenges: \n");
			for (int j = 0; j < _msize(group->challenges) / sizeof(struct Challenge*); j++) {
				printChallenge(group->challenges[j]);
			}
		}
	}

	/**
	* Prints the information of the Challenge passed by parameter (maintaining indentation from context).
	*
	* @param struct Challenge* challenge
	*		The Challenge which info must be printed.
	*
	* @return
	**/
	static void printChallenge(struct Challenge* challenge) {
		PRINT3("Challenge:\n");
		PRINT4("Filename: %ws \n", challenge->file_name);
		PRINT4("Properties: %s \n", challenge->properties);
	}

	/**
	* Prints the date and time of the struct tm passed by parameter. The printed format is "YYYY/MM/DD - hh:mm:ss".
	* Example: "2021/03/11 - 14:21:08"
	*
	* @param struct tm *time_info
	*		The struct tm with filled fields tm_year, tm_mon, tm_mday, tm_hour, tm_min and tm_sec, to print the date and time.
	*
	* @return
	**/
	static void printDateNice(struct tm* time_info) {
		if (time_info == NULL) {
			PRINT("ERROR");
		}
		else {
			PRINT("%04d/%02d/%02d - %02d:%02d:%02d",
				time_info->tm_year + 1900,			// Year
				time_info->tm_mon + 1,				// Month
				time_info->tm_mday,					// Day
				time_info->tm_hour,					// Hours
				time_info->tm_min,					// Minutes
				time_info->tm_sec					// Seconds
			);
		}
	}

	/**
	* Returns a pointer to the ChallengeEquivalenceGroup with same id provided as parameter.
	*
	* @param char* group_id
	*		The id string to retrieve pointer from.
	*
	* @return struct ChallengeEquivalenceGroup*
	*		The pointer to the ChallengeEquivalenceGroup with given id. May be NULL if no matches were found.
	**/
	static struct ChallengeEquivalenceGroup* getChallengeGroupById(char* group_id) {
		for (int i = 0; i < _msize(ctx.groups) / sizeof(struct ChallengeEquivalenceGroup*); i++) {
			if (strcmp(ctx.groups[i]->id, group_id) == 0) {
				return ctx.groups[i];
			}
		}
		return NULL;
	}

	/**
	* Returns a pointer to the Cipher with same id provided as parameter.
	*
	* @param char* cipher_id
	*		The id string to retrieve pointer from.
	*
	* @return struct ChallengeEquivalenceGroup*
	*		The pointer to the ChallengeEquivalenceGroup with given id. May be NULL if no matches were found.
	**/
	static struct Cipher* getCipherById(char* cipher_id) {
		for (int i = 0; i < _msize(ctx.ciphers) / sizeof(struct Cipher*); i++) {
			if (strcmp(ctx.ciphers[i]->id, cipher_id) == 0) {
				return ctx.ciphers[i];
			}
		}
		return NULL;
	}

	/**
	* Returns the pointer to the OpTable with the same id as provided by parameter.
	*
	* @param char* table_id
	*		The id string to retrieve pointer from.
	*
	* @return struct ChallengeEquivalenceGroup*
	*		The pointer to the OpTable with given id. May be NULL if no matches are found.
	**/
	static struct OpTable* getOpTableById(char* table_id) {
		for (int i = 0; i < _msize(ctx.tables) / sizeof(struct OpTable*); i++) {
			if (strcmp(ctx.tables[i]->id, table_id) == 0) {
				return ctx.tables[i];
			}
		}
		return NULL;
	}

	// THIS FUNCTION MAY NOT BE NEEDED
	/**
	* The printed format is "YYYY-MM-DD - hh:mm:ss". Example: "2021-03-11 - 14:21:08"
	**/
	/*static void printDateNiceOLD(char* date) {
		if (strlen(date) != 14) {
			PRINT("ERROR");
		} else {
			PRINT("%c%c%c%c-%c%c-%c%c - %c%c:%c%c:%c%c",
				date[0], date[1], date[2], date[3],			// Year
				date[4], date[5],							// Month
				date[6], date[7],							// Day
				date[8], date[9],							// Hours
				date[10], date[11],							// Minutes
				date[12], date[13]							// Seconds
			);
		}
	}*/

	// THIS FUNCTION MAY NOT BE NEEDED
	/**
	* Transforms a time_t in the correct format string for the context ("YYYYMMDDhhmmss").
	* Allocates memory inside. Remember to free the returned char*.
	*
	* @param time_t timer
	*		The time_t value to convert into formatted string
	*
	* @return char*
	*		The time formatted as string ("YYYYMMDDhhmmss"). Memory allocated inside, remember to free.
	**/
	/*static char* getTimeFormattedString(time_t timer) {

		char* formatted_time_str = (char*)malloc(sizeof(char) * 14);
		if (formatted_time_str == NULL) {
			return NULL;
		}
		struct tm time_info;

		PRINT("Value of timer is: %lld \n", timer);
		PRINT("That is around %lld years\n", timer /(60*60*24*365));

		if (localtime_s(&time_info, &timer) == 0) {
			sprintf(formatted_time_str, "%04d%02d%02d%02d%02d%02d",
				time_info.tm_year + 1900,		// .tm_year are years since 1900
				time_info.tm_mon + 1,			// .tm_mon is in range [0-11]
				time_info.tm_mday,				// .tm_mday is in range[1-31]
				time_info.tm_hour,				// .tm_hour is in range[0-23]
				time_info.tm_min,				// .tm_min is in range[0-59]
				time_info.tm_sec				// .tm_sec is in range[0-60]. Can include leap seccond
			);
		} else {
			free(formatted_time_str);
			formatted_time_str = NULL;
		}

		return formatted_time_str;
	}*/

	// THIS FUNCTION MAY NOT BE NEEDED
	/**
	* Transforms a time formatted string into a time_t value.
	*
	* @param char* formatted_time_str
	*		The time formatted as string ("YYYYMMDDhhmmss").
	*
	* @return time_t
	*		The time_t value converted from the formatted string.
	**/
	/*static time_t getTimeFromString(char* formatted_time_str) {

		time_t timer = 0;
		struct tm time_info = {0};
		int num;
		size_t len = strlen(formatted_time_str);

		PRINT("String is: '%s' with length of %llu\n", formatted_time_str, len);

		// Check string length
		if (len != 14) {
			return timer;
		}

		// Check characters are digits
		for (size_t i = 0; i < len; i++) {
			PRINT("formatted_time_str[i] = %c\n", formatted_time_str[i]);
			if (formatted_time_str[i] < '0' || formatted_time_str[i]>'9') {
				return timer;
			}
		}

		// Fill time_info with current time to get the DST value (all other fields will be overriden)
		time_t curr_time = time(NULL);
		localtime_s(&time_info, &curr_time);

		// PARSE STRING
		// Get Year
		num = (formatted_time_str[0] - '0') * 1000 + (formatted_time_str[1] - '0') * 100 + (formatted_time_str[2] - '0') * 10 + (formatted_time_str[3] - '0');
		num -= 1900;
		if (num < 0) {
			return timer;
		}
		time_info.tm_year = num;

		// Get Month
		num = (formatted_time_str[4] - '0') * 10 + (formatted_time_str[5] - '0');
		num -= 1;
		if (num < 0 || num > 11) {
			return timer;
		}
		time_info.tm_mon = num;

		// Get Day
		num = (formatted_time_str[6] - '0') * 10 + (formatted_time_str[7] - '0');
		if (num < 1 || num > 31) {
			return timer;
		}
		time_info.tm_mday = num;

		// Get Hours
		num = (formatted_time_str[8] - '0') * 10 + (formatted_time_str[9] - '0');
		if (num < 0 || num > 23) {
			return timer;
		}
		time_info.tm_hour = num;

		// Get Minutes
		num = (formatted_time_str[10] - '0') * 10 + (formatted_time_str[11] - '0');
		if (num < 0 || num > 59) {
			return timer;
		}
		time_info.tm_min = num;

		// Get Seconds
		num = (formatted_time_str[12] - '0') * 10 + (formatted_time_str[13] - '0');
		if (num < 0 || num > 60) {
			return timer;
		}
		time_info.tm_sec = num;

		// Gets the result and fills the WeekDay and YearDay
		timer = mktime(&time_info);

		// Get WeekDay
		//time_info.tm_wday = getWeekDay(time_info.tm_year, time_info.tm_mon, time_info.tm_mday);
		PRINT("WeekDay: %d\n", time_info.tm_wday);

		// Get YearDay
		//time_info.tm_yday = getYearDay(time_info.tm_year, time_info.tm_mon, time_info.tm_mday);
		PRINT("YearDay: %d\n", time_info.tm_yday);

		return timer;
	}*/

	// THIS FUNCTION MAY NOT BE NEEDED
	/**
	* Formula "modifies" parameters (reason not to inline). Sunday=0, Monday=1, ... Saturday = 6
	**/
	/*static int getWeekDay(int y, int m, int d) {
		// https://stackoverflow.com/questions/6054016/c-program-to-find-day-of-week-given-date
		return (d += m < 3 ? y-- : y - 2, 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;
	}*/

	// THIS FUNCTION MAY NOT BE NEEDED
	/**
	* Formula "modifies" parameters (reason not to inline). Result is in range [0-365]
	**/
	/*static int getYearDay(int year, int month, int day) {
		// https://www.geeksforgeeks.org/find-the-day-number-in-the-current-year-for-the-given-date/
		int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

		if (month > 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
			++day;
		}

		while (month-- > 0) {
			day = day + days[month - 1];
		}

		return day;
	}*/

	// THIS FUNCTION MAY NOT BE NEEDED
	/**
	* Returns a list with pointers to every challenge which id matches with the id provided as parameter (independently of the group id).
	* Allocates memory inside. Remember to free the returned struct Challenge**.
	* NOTE: this function contains allocation and reallocation which may make it slow. Use it only for testing.
	*
	* @param char* challenge_id
	*		The id string to retrieve pointers from.
	*
	* @return struct Challenge**
	*		The list of pointers to challenge. Memory allocated inside, remember to free.
	*		May contain NULL pointers at the end.
	*		Can be iterated with: for (int i = 0; i < _msize(challenges) / sizeof(struct Challenge*); i++) {...}
	**/
	/*static struct Challenge** getChallengeById(char* challenge_id) {
		int num_challenges_in_list = 5;
		int challenge_index = 0;
		struct Challenge** challenges = (struct Challenge**)malloc(num_challenges_in_list * sizeof(struct Challenge*));
		if (challenges) {
			for (int i = 0; i < _msize(ctx.groups) / sizeof(struct ChallengeEquivalenceGroup*); i++) {
				ctx.groups[i]->challenges;
				for (int j = 0; j < _msize(ctx.groups[i]->challenges) / sizeof(struct Challenges*); j++) {
					if (strcmp(ctx.groups[i]->challenges[j]->id, challenge_id) == 0) {
						if (num_challenges_in_list >= challenge_index) {
							num_challenges_in_list += 5;
							challenges = (struct Challenge**)realloc(challenges, num_challenges_in_list * sizeof(struct Challenge*));
						}
						challenges[challenge_index] = ctx.groups[i]->challenges[j];
						challenge_index++;
					}
				}
			}
			return challenges;
		}
		return NULL;	// challenges could not be allocated
	}*/



#ifdef __cplusplus
}
#endif	//__cplusplus

#endif	//context_h