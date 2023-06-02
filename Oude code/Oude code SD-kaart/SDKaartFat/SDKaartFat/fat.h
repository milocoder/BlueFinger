#ifndef FAT_H_
#define FAT_H_

/*************************************************************
	INCLUDES
**************************************************************/
#include <avr/io.h>
#include <string.h>
#include "utils.h"
#include "sd.h"

/*************************************************************
	USER DEFINED SETTINGS
**************************************************************/
#define FAT_SUPPORT_FAT32			1	// set to 0 to support only FAT16 and exclude FAT32

// FAT supports file names up to 260 characters including path 
// but that would take a lot of space so
// shorter file names could be used instead
#define FAT_MAX_FILENAME_LENGTH		30

/*************************************************************
	SYSTEM DEFINES
**************************************************************/
#define true	1
#define false	0

#define FAT_TASK_SEARCH_SFN			1
#define FAT_TASK_TABLE_COUNT_FREE	2
#define FAT_TASK_TABLE_FIND_FREE	3
#define FAT_TASK_MKDIR				4
#define FAT_TASK_MKFILE				5
#define FAT_TASK_TABLE_SET			6
#define FAT_TASK_TABLE_GET_NEXT		7
#define FAT_TASK_OPEN_DIR			8
#define FAT_TASK_COUNT_ITEMS		9
#define FAT_TASK_FIND_FILE			10
#define FAT_TASK_SET_FILESIZE		11
#define FAT_TASK_SET_START_CLUSTER	12
#define FAT_TASK_TABLE_READ_SET		13

// Directory entry
#define FAT_DIR_FREE_SLOT						0x00	// root directory entry is free
#define FAT_FILE_DELETED						0xE5	// the filename has been used, but the file has been deleted
#define FAT_FILE_E5_CHAR						0x05	// the first character of the filename is actually 0xE5
#define FAT_LFN_MAX_CHARS						13		// a LFN entry can hold 13 characters

// FAT Table
#define FAT16_FREE_CLUSTER						0x0000		// the cluster is free
#define FAT16_BAD_CLUSTER						0xFFF7		// indicates a bad (defective) cluster
#define FAT16_EOF_CLUSTER						0xFFFF		// cluster is allocated and is the final cluster for the file (indicates end-of-file)

#define FAT32_FREE_CLUSTER						0x0000000	// the cluster is free
#define FAT32_BAD_CLUSTER						0xFFFFFF7	// indicates a bad (defective) cluster
#define FAT32_EOF_CLUSTER						0x0FFFFFFF	// cluster is allocated and is the final cluster for the file (indicates end-of-file)
// NOTE: No FAT32 volume should ever be configured containing cluster numbers available for allocation >= 0xFFFFFF7.
#define FAT32_MAX_CLUSTER						0xFFFFFF7

// The maximum Windows filename length to the operating system is 260 characters, 
// however that includes a number of required characters that lower the effective number.
// From the 260, you must allow room for the following: 
// Drive letter, Colon after drive letter, Backslash after drive letter, End-of-Line character,
// Backslashes that are part of the filename path (e.g. c:\dir-name\dir-name\filename).
// So, that takes the 260 down to 256 characters as an absolute maximum.
// Absolute (relative) maximum file length - including path - is 256 characters.
#define FS_MAX_PATH					260 - 4 - 1 - 8 // subtract 1 so max_length is 255 to fit in 1 byte minus 8 for drive label

/* File system type (FATFS.fs_type) */
#define FS_FAT12	0
#define FS_FAT16	1 // must be 1 so bit shifting << will multiply by 2
#define FS_FAT32	2 // must be 2 so bit shifting << will multiply by 4
#define FS_EXFAT	3

/* Offsets */
/* MBR */
#define FAT_MBR_PARTITION_ENTRY_OFFSET					0x1BE	// offset at which partition information starts
#define FAT_MBR_PARTITION_TYPE							0x04	// partition type offset relative to each 16-bit partition slot
#define FAT_MBR_PARTITION_START							0x08	// relative offset to the partition in sectors (LBA)
#define FAT_MBR_PARTITION_SIZE							0x0C	// size of the partition in sectors

/* Boot Record */
#define FAT_BPB_BYTES_PER_SECTOR						0x0B	// sector size in bytes
#define FAT_BPB_SECTORS_PER_CLUSTER						0x0D	// number of sectors per cluster
#define FAT_BPB_RESERVED_SECTORS						0x0E	// reserved sectors (including the boot sector)
#define FAT_BPB_NR_OF_FATS								0x10	// number of FATs
#define FAT_BPB_ROOT_DIR_ENTRIES						0x11	// number of directory entries in the root directory (N.A. for FAT32)
#define FAT_BPB_TOT_SEC_16								0x13	// total number of sectors on the disk/partition
#define FAT_BPB_FAT_SZ_16								0x16	// number of sectors occupied by a FAT (N.A. for FAT32)
#define FAT_BPB_TOT_SEC_32								0x20	// total number of sectors of the FAT volume in new 32-bit field

#define FAT16_BS_VOL_ID									39		// volume serial number (FAT12/16)

#define FAT32_BPB_FAT_SZ_32								36		// this field is the FAT32 32-bit count of sectors occupied by one FAT
#define FAT32_BPB_EXT_FLAFS								40		// indicates how many FATs are used and if only 1 which one
#define FAT32_BPB_FS_VER								42		// the version number of the FAT32 volume
#define FAT32_BPB_ROOT_CLUST							44		// cluster number of the first cluster of the root directory
#define FAT32_BPB_FS_INFO								48		// sector number of FSINFO structure in the reserved area of the FAT32 volume. Usually 1
#define FAT32_BPB_BK_BOOT_SECTOR						50		// if non-zero, indicates the sector number in the reserved area of the volume of a copy of the boot record
#define FAT32_BPB_RESERVED								52		// must be set to 0x0
#define FAT32_BS_DRV_NUM								64		// set value to 0x80 or 0x00
#define FAT32_BS_RESERVED1								65		// must be set to 0x0
#define FAT32_BS_BOOT_SIG								66		// extended boot signature
#define FAT32_BS_VOL_ID									67		// volume serial number (FAT32)
#define FAT32_BS_VOL_LABEL								71		// volume label (FAT32)
#define FAT32_BS_FS_TYPE								82		// set to the string:"FAT32 "

/* Directory Entry */
#define FAT_DIR_NAME									0x00
#define FAT_DIR_ATTR									11
#define FAT_DIR_NT_RES									12
#define FAT_DIR_CREAT_TIME_MILLIS						13
#define FAT_DIR_CREAT_TIME								14
#define FAT_DIR_CREAT_DATE								16
#define FAT_DIR_LAST_ACC_DATE							18
#define FAT_DIR_FIRST_CLUS_HIGH							20
#define FAT_DIR_WRITE_TIME								22
#define FAT_DIR_WRITE_DATE								24
#define FAT_DIR_FIRST_CLUS_LOW							26
#define FAT_DIR_FILE_SIZE								28

/* File attributes Offsets and Masks */
#define FAT_FILE_ATTR_READ_ONLY							0x01
#define FAT_FILE_ATTR_HIDDEN							0x02
#define FAT_FILE_ATTR_SYSTEM							0x04
#define FAT_FILE_ATTR_VOLUME_ID							0x08
#define FAT_FILE_ATTR_DIRECTORY							0x10
#define FAT_FILE_ATTR_ARCHIVE							0x20
#define FAT_FILE_ATTR_LONG_NAME							(FAT_FILE_ATTR_READ_ONLY | FAT_FILE_ATTR_HIDDEN | FAT_FILE_ATTR_SYSTEM | FAT_FILE_ATTR_VOLUME_ID)
#define FAT_FILE_ATTR_LONG_NAME_MASK					(FAT_FILE_ATTR_READ_ONLY | FAT_FILE_ATTR_HIDDEN | FAT_FILE_ATTR_SYSTEM |FAT_FILE_ATTR_VOLUME_ID | FAT_FILE_ATTR_DIRECTORY | FAT_FILE_ATTR_ARCHIVE)

/* FAT Long Directory Entry Structure Offsets and Masks */													
#define FAT_LONG_DIR_ORDER								0		// the order of this entry in the sequence of long dir entries
#define FAT_LAST_LONG_ENTRY_MASK						0x40
#define FAT_LONG_DIR_NAME								1		// characters 1-5 of the long-name sub-component in this dir entry
#define FAT_LONG_DIR_ATTR								11		// attributes - must be ATTR_LONG_NAME
#define FAT_LONG_DIR_TYPE								12		// if zero, indicates a directory entry that is a sub-component of a long name
#define FAT_LONG_DIR_CHECKSUM							13		// checksum of name in the short dir entry at the end of the long dir set
#define FAT_LONG_DIR_NAME2								14		// characters 6-11 of the long-name sub-component in this dir entry
#define FAT_LONG_DIR_FIRST_CLST_LOW						26		// must be ZERO. This is an artifact of the FAT "first cluster"
#define FAT_LONG_DIR_NAME3								28		// characters 12-13 of the long-name sub-component in this dir entry


/*************************************************************
	GLOBALS
**************************************************************/
/* Function pointers to low lever card interface functions */
//uint8_t (*card_read_single_block) (uint32_t addr, uint8_t *buf, uint8_t *token);

typedef uint32_t FSIZE_t;			// file size in bytes
typedef uint32_t SECTSIZE_t;		// sectors

#if FAT_SUPPORT_FAT32 == 0
	typedef uint16_t CLSTSIZE_t;	// clusters
#else
	typedef uint32_t CLSTSIZE_t;	// clusters
#endif

/* Volume mounting return codes (MOUNT_RESULT) */
typedef enum {
	MR_OK = 0,				// Succeeded
	MR_DEVICE_INIT_FAIL,	// An error occurred during device initialization
	MR_ERR,					// An error occurred in the low level disk I/O layer
	MR_NO_PARTITION,		// No partition has been found
	MR_FAT_ERR,				// General FAT error
	MR_UNSUPPORTED_FS,		// Unsupported file system
	MR_UNSUPPORTED_BS		// Unsupported block size
} MOUNT_RESULT;


/* File function return codes (FRESULT) */
typedef enum {
	FR_OK = 0,				// Succeeded
	FR_EOF,					// End of file
	FR_NOT_FOUND,			// Could not find the file
	FR_NO_PATH,				// Could not find the path
	FR_NO_SPACE,			// Not enough space to create file
	FR_EXIST,				// File exists
	FR_INCORRECT_ENTRY,		// Some entry parameters are incorrect
	FR_DENIED,				// Access denied due to prohibited access or directory full
	FR_PATH_LENGTH_EXCEEDED,// Path too long
	FR_NOT_A_DIRECTORY,
	FR_ROOT_DIR,			// When going back the directory path this is returned when the active dir is root
	FR_INDEX_OUT_OF_RANGE,
	FR_DEVICE_ERR			// A hard error occurred in the low level disk I/O layer
} FRESULT;


/* File system object structure (FAT) */
typedef struct {
	uint32_t	fs_partition_offset;	// relative offset to the partition in sectors (LBA)
	uint32_t	BPB_TotSec32;			// total number of sectors of the FAT volume in new 32-bit field
	uint32_t	EOC;					// End Of Chain cluster value
	CLSTSIZE_t	CountofClusters;		// count of clusters
	uint32_t	FATSz;					// number of sectors for one FAT table
	uint16_t	BPB_BytsPerSec;			// sector size in bytes 
	uint16_t	RootDirSectors;			// count of sectors occupied by the root directory (N.A for FAT32)
	uint32_t	RootFirstCluster;		// first cluster of the root directory (FAT32 only)
	uint16_t	RootFirstSector;		// first sector of the root	directory
	uint16_t	FirstDataSector;		// this sector number is relative to the first sector of the volume that contains the BPB
	uint16_t	Fat1StartSector;
	uint16_t	Fat2StartSector;
	uint8_t		FATDataSize;			// 2-bytes if FAT16, 4-bytes if FAT32
	uint8_t		entries_per_sector;		// number of entries in a sector given a 32 bytes entry. For a 512 bytes per sector: 512 / 32 = 16	
	uint8_t		fs_low_level_code;		// low level status code from card controller
	uint8_t		fs_type;				// file system type (0:not mounted)
	uint8_t		BPB_SecPerClus;			// sectors per cluster
} FAT;


/* Directory object structure (DIR) */
typedef struct {
	bool		dir_open;
	bool		dir_open_by_idx;
	uint16_t	dir_nr_of_entries;			// number of files and folders inside a directory
	CLSTSIZE_t	dir_start_cluster;
	CLSTSIZE_t	dir_active_cluster;
	SECTSIZE_t	dir_start_sector;
	uint16_t	dir_active_sector;			// max 2^16 sectors per cluster
	uint16_t	dir_active_item;			// index of selected item inside directory starting from 1
	uint8_t		dir_entry_offset;			// entry offset inside the sector of selected item
	uint16_t	dir_nr_of_items;
	uint16_t	find_by_index;
	uint8_t		filename_length;
	const char*	ptr_path_buff;
} DIR;


/* File information structure (FILE) */
typedef struct {
	uint8_t		file_err;									// abort flag (error code)
	FSIZE_t		fptr;										// file read/write pointer (zeroed on file open)
	uint8_t		file_attrib;								// file attribute
	//uint16_t	file_last_access_date;						// this is the date of last read or write
	uint16_t	file_write_time;							// time of last write. Note that file creation is considered a write
	uint16_t	file_write_date;							// date of last write. Note that file creation is considered a write
	FSIZE_t		file_size;									// file size in bytes
	CLSTSIZE_t	file_start_cluster;							// entry's first cluster number
	CLSTSIZE_t	file_active_cluster;
	uint32_t	file_start_sector;
	uint16_t	file_active_sector;							// incremented after each sector read
	uint16_t	buffer_idx;
	SECTSIZE_t	entry_start_sector;							// sector number that holds the file entry
	uint8_t		entry_offset;								// offset inside the sector where the entry starts
	bool		file_update_size;
	bool		file_open;
	bool		w_sec_changed;								// write sector changed
	bool		eof;
} FILE;


// Access a long variable as 4, 8-bit integer values
typedef union lng {
	uint32_t Long;
	uint8_t Int[4];
} lng;


static char FAT_filename[FAT_MAX_FILENAME_LENGTH + 1];		// file name
static DIR* bufferModBy;
static bool set_null = false;			// used to add null terminator to long file names
static bool null_is_set = false;		//

static FAT fat_obj;						// card object
static FAT* fat = &fat_obj;


/*************************************************************
	MACRO FUNCTIONS
**************************************************************/
/* Character code support macros */
#define IsUpper(c)		((c) >= 'A' && (c) <= 'Z')
#define IsLower(c)		((c) >= 'a' && (c) <= 'z')
#define IsDigit(c)		((c) >= '0' && (c) <= '9')
#define IsSeparator(c)	((c) == '/' || (c) == '\\')


/*************************************************************
	FUNCTION PROTOTYPES
**************************************************************/
// Volume
MOUNT_RESULT FAT_mountVolume(void);
uint64_t FAT_volumeFreeSpace(void);
uint64_t FAT_volumeCapacity(void);
float FAT_volumeCapacityKB(void);
float FAT_volumeCapacityMB(void);
float FAT_volumeCapacityGB(void);
FRESULT FAT_getLabel(char* label, uint32_t* vol_sn);

// Other
uint8_t FAT_createTimeMilli(void);
uint16_t FAT_createTime(void);
uint16_t FAT_createDate(void);

// Directory
FRESULT FAT_makeDir(const char* path);
FRESULT FAT_openDir(DIR* dir_p, const char* path);
FRESULT FAT_openDirByIndex(DIR* dir_p, FILE* finfo_p, uint16_t idx);
FRESULT FAT_dirBack(DIR* dir_p);
FRESULT FAT_findByIndex(DIR* dir_p, FILE* finfo_p, uint16_t idx);
FRESULT FAT_findNext(DIR* dir_p, FILE* finfo_p);
uint16_t FAT_dirCountItems(DIR* dir_p);
static FRESULT _FAT_dirRegister(const char* path, uint8_t task);

// File
FRESULT FAT_makeFile(const char* path);
FRESULT FAT_fwriteFloat(FILE* fp, float nr, uint8_t nrOfDecimals);
FRESULT FAT_fwriteInt(FILE* fp, INT_SIZE nr);
FRESULT FAT_fwriteString(FILE* fp, const char* string);
FRESULT FAT_fwrite(FILE* fp, const void* buff, uint16_t btw, uint16_t* bw);
FRESULT FAT_ftruncate(FILE* fp);
FRESULT FAT_fsync(FILE* fp);
FRESULT FAT_fopen(DIR* dir_p, FILE* file_p, char *file_name);
FRESULT FAT_fopenByIndex(DIR* dir_p, FILE* file_p, uint16_t idx);
static void _FAT_freset(FILE* file_p);
uint8_t* FAT_fread(FILE* file_p);
FSIZE_t FAT_getFptr(FILE* fp);
void FAT_fseekEnd(FILE* fp);
void FAT_fseek(FILE* fp, FSIZE_t fptr);
bool FAT_feof(FILE* fp);
uint8_t FAT_ferror(FILE* fp);
void FAT_fclear_error(FILE* fp);
char* FAT_getFilename(void);
uint16_t FAT_getItemIndex(DIR* dir_p);
FSIZE_t FAT_getFileSize(FILE* finfo_p);
uint16_t FAT_getWriteYear(FILE* finfo_p);
uint8_t FAT_getWriteMonth(FILE* finfo_p);
uint8_t FAT_getWriteDay(FILE* finfo_p);
uint8_t FAT_getWriteHour(FILE* finfo_p);
uint8_t FAT_getWriteMinute(FILE* finfo_p);
uint8_t FAT_getWriteSecond(FILE* finfo_p);
bool FAT_attrIsFolder(FILE* finfo_p);
bool FAT_attrIsFile(FILE* finfo_p);
bool FAT_attrIsHidden(FILE* finfo_p);
bool FAT_attrIsSystem(FILE* finfo_p);
bool FAT_attrIsReadOnly(FILE* finfo_p);
bool FAT_attrIsArchive(FILE* finfo_p);
static uint8_t _FAT_nextFileCluster(FILE* file_p);

// System
static void _FAT_removeChain(CLSTSIZE_t cluster);
static FRESULT _FAT_updateFileInfo(FILE* fp, uint8_t task);
static FRESULT _FAT_allocateCluster(CLSTSIZE_t* new_cluster, CLSTSIZE_t new_cluster_val);
static void _FAT_moveWindow(DIR* dir_p, CLSTSIZE_t start_cluster);
static FRESULT _FAT_clearCluster(CLSTSIZE_t cluster, bool first_sector);
static CLSTSIZE_t FAT_tableFindFree(uint8_t task);
static unsigned char ChkSum(unsigned char *pFcbName);
static void _FAT_stringLFN(uint16_t start_idx, uint16_t length, const char** data, uint8_t data_size);
static void _FAT_fillBufferArray(uint16_t start_idx, uint16_t length, uint8_t value);
static void _FAT_getSFN(uint8_t entry_pos, uint8_t task, uint8_t file_attrib);
static FRESULT _FAT_followPath(DIR* dir_p, const char* path, uint8_t task);
static FRESULT _FAT_createSFN(DIR* dir_p, const char* source, char dest[], bool* needs_lfn);
static FRESULT _FAT_getFileNextSector(DIR* dir_p);
static uint16_t MATH_remainder(uint16_t divident, uint16_t divisor, uint16_t quotient);
static SECTSIZE_t _FAT_clusterToSector(CLSTSIZE_t cluster);
static CLSTSIZE_t _FAT_tableReadSet(CLSTSIZE_t cluster, CLSTSIZE_t new_value, uint8_t task);
static void _FAT_parse_long_names(char buff[], uint16_t sector_offset);
static uint8_t _FAT_parse_long_names_(char buff[], uint8_t start_idx, uint16_t sector_offset, uint8_t buff_idx, uint8_t len);


/*************************************************************
	FUNCTIONS
**************************************************************/
MOUNT_RESULT FAT_mountVolume(void){
	lng temp_long;
	uint8_t fs_partition_type = 0;	// 4, 6 or 14 for FAT16
	uint8_t	BPB_NumFATs = 0;		// number of FATs
	uint16_t i;
	uint16_t BPB_RsvdSecCnt = 0;	// reserved sector count
	uint16_t BPB_RootEntCnt = 0;	// number of directory entries in the root directory (N.A. for FAT32)
	uint16_t BPB_TotSec16 = 0;		// total number of sectors on the disk/partition
	uint16_t BPB_FATSz16 = 0;		// number of sectors occupied by a FAT (N.A. for FAT32)
	float DataSec;					// count of sectors in the data region of the volume
	
	fat->RootFirstCluster = 0;
	fat->fs_partition_offset = 0;
	fat->fs_type = 0;
	temp_long.Long = 0;
	
	// Assign function to pointer
	//card_read_single_block = &sd_read_single_block;
	
	// Card initialization
	fat->fs_low_level_code = sd_init();
	if(fat->fs_low_level_code){
		return MR_DEVICE_INIT_FAIL;
	}
	
	// Read the first sector that could be MBR or Boot Sector
	fat->fs_low_level_code = sd_read_single_block(0, SD_Buffer);
	if(fat->fs_low_level_code) return MR_ERR;
	
	// Check Boot sector signature, must be 0x55AA
	temp_long.Int[0] = SD_Buffer[511];
	temp_long.Int[1] = SD_Buffer[510];
	if(temp_long.Long != 0x55AA) return MR_FAT_ERR;
	
	// Check if this is the MBR (with partition(s)) or boot sector (no partition).
	temp_long.Int[0] = SD_Buffer[444];
	temp_long.Int[1] = SD_Buffer[445];
	
	// MBR
	if((temp_long.Long == 0x5A5A) || (temp_long.Long == 0)){
		// There are 4 16-bit slots that can have 4 partitions. Search each slot for a valid partition.
		for(i = 0; i < 4; i++){
			fs_partition_type = SD_Buffer[FAT_MBR_PARTITION_ENTRY_OFFSET + FAT_MBR_PARTITION_TYPE + (16 * i)];
			if(fs_partition_type) break; // select the first available partition
		}
	
		// Get partition offset
		temp_long.Long = 0;
		temp_long.Int[0] = SD_Buffer[FAT_MBR_PARTITION_ENTRY_OFFSET + FAT_MBR_PARTITION_START + (16 * i)];
		temp_long.Int[1] = SD_Buffer[FAT_MBR_PARTITION_ENTRY_OFFSET + FAT_MBR_PARTITION_START + (16 * i) + 1];
		temp_long.Int[2] = SD_Buffer[FAT_MBR_PARTITION_ENTRY_OFFSET + FAT_MBR_PARTITION_START + (16 * i) + 2];
		temp_long.Int[3] = SD_Buffer[FAT_MBR_PARTITION_ENTRY_OFFSET + FAT_MBR_PARTITION_START + (16 * i) + 3];
		fat->fs_partition_offset = temp_long.Long;
	
		// Check for valid partition
		// Ignore fs_partition_type? and check only fs_partition_offset?
		if((fs_partition_type == 0) || (fat->fs_partition_offset == 0)) return MR_NO_PARTITION;
	}
	
	
	
	// Read the Boot Record of detected partition or sector 0 including the Boot Sector
	// fs_partition_offset will be 0 if there is no MBR
	fat->fs_low_level_code = sd_read_single_block(fat->fs_partition_offset, SD_Buffer);
	if(fat->fs_low_level_code) return MR_ERR;
	
	// Check Boot sector signature, must be 0x55AA
	temp_long.Int[0] = SD_Buffer[511];
	temp_long.Int[1] = SD_Buffer[510];
	if(temp_long.Long != 0x55AA) return MR_FAT_ERR;
	
	// Read the Boot Record
	// Bytes per sector
	temp_long.Int[0] = SD_Buffer[FAT_BPB_BYTES_PER_SECTOR];
	temp_long.Int[1] = SD_Buffer[FAT_BPB_BYTES_PER_SECTOR + 1];
	fat->BPB_BytsPerSec = temp_long.Long;
	
	// Working with block sizes other than 512 would complicate the code too much.
	// Modern cards only work with 512 block size so the FAT must be formated accordingly.
	if(fat->BPB_BytsPerSec != 512) return MR_UNSUPPORTED_BS;
	
	// Sectors per cluster
	fat->BPB_SecPerClus = SD_Buffer[FAT_BPB_SECTORS_PER_CLUSTER];
	
	// Reserved sectors
	temp_long.Int[0] = SD_Buffer[FAT_BPB_RESERVED_SECTORS];
	temp_long.Int[1] = SD_Buffer[FAT_BPB_RESERVED_SECTORS + 1];
	BPB_RsvdSecCnt = temp_long.Long;
	
	// Number of FATs
	BPB_NumFATs = SD_Buffer[FAT_BPB_NR_OF_FATS];
	
	// Root entries
	temp_long.Int[0] = SD_Buffer[FAT_BPB_ROOT_DIR_ENTRIES];
	temp_long.Int[1] = SD_Buffer[FAT_BPB_ROOT_DIR_ENTRIES + 1];
	BPB_RootEntCnt = temp_long.Long;
	
	// Total sectors 16
	temp_long.Int[0] = SD_Buffer[FAT_BPB_TOT_SEC_16];
	temp_long.Int[1] = SD_Buffer[FAT_BPB_TOT_SEC_16 + 1];
	BPB_TotSec16 = temp_long.Long;
	
	// This field is the FAT12/FAT16 16-bit count of sectors occupied by one FAT.
	// On FAT32 volumes	this field must be 0, and BPB_FATSz32 contains the FAT size count.
	temp_long.Int[0] = SD_Buffer[FAT_BPB_FAT_SZ_16];
	temp_long.Int[1] = SD_Buffer[FAT_BPB_FAT_SZ_16 + 1];
	BPB_FATSz16 = temp_long.Long;
	
	// Total sectors 32
	temp_long.Int[0] = SD_Buffer[FAT_BPB_TOT_SEC_32];
	temp_long.Int[1] = SD_Buffer[FAT_BPB_TOT_SEC_32 + 1];
	temp_long.Int[2] = SD_Buffer[FAT_BPB_TOT_SEC_32 + 2];
	temp_long.Int[3] = SD_Buffer[FAT_BPB_TOT_SEC_32 + 3];
	
	fat->BPB_TotSec32 = temp_long.Long;
	
	// This field must not be 0 because the boot sector itself 
	// contains this BPB in the reserved area
	if(BPB_RsvdSecCnt == 0) return MR_FAT_ERR;
	
	// Determine count of sectors occupied by the root directory
	// Note that on a FAT32 volume, the BPB_RootEntCnt value is always 0. 
	// Therefore, on a FAT32 volume, RootDirSectors is always 0.
	fat->RootDirSectors = (BPB_RootEntCnt * 32) / fat->BPB_BytsPerSec;
	
	// Determine count of sectors occupied by one FAT depending of FAT type
	if(BPB_FATSz16 != 0){
		fat->FATSz = BPB_FATSz16; // FAT16
	}else{
		temp_long.Int[0] = SD_Buffer[FAT32_BPB_FAT_SZ_32];
		temp_long.Int[1] = SD_Buffer[FAT32_BPB_FAT_SZ_32 + 1];
		temp_long.Int[2] = SD_Buffer[FAT32_BPB_FAT_SZ_32 + 2];
		temp_long.Int[3] = SD_Buffer[FAT32_BPB_FAT_SZ_32 + 3];
		fat->FATSz = temp_long.Long; // FAT32
	}
	
	// First Data sector
	// NOTE: This sector number is relative to the first sector of the volume that contains the BPB (the
	// sector that contains the BPB is sector number 0). This does not necessarily map directly onto the
	// drive, because sector 0 of the volume is not necessarily sector 0 of the drive due to partitioning.
	fat->FirstDataSector = BPB_RsvdSecCnt + (BPB_NumFATs * fat->FATSz) + fat->RootDirSectors;
	
	// Determine the count of sectors in the data region of the volume
	if(BPB_TotSec16 != 0) temp_long.Long = BPB_TotSec16;
	else temp_long.Long = fat->BPB_TotSec32;
	DataSec = temp_long.Long - fat->FirstDataSector;
	
	// Now we determine the count of clusters
	// This computation rounds down
	fat->CountofClusters = (DataSec / fat->BPB_SecPerClus) - 0.5;

	// Determine the FAT type
	if(fat->CountofClusters < 4085){
		// Volume is FAT12
		fat->fs_type = FS_FAT12;
	}else if(fat->CountofClusters < 65525){
		// Volume is FAT16
		fat->fs_type = FS_FAT16;
		fat->FATDataSize = 2;
		fat->EOC = FAT16_EOF_CLUSTER;
	}else{
		// Volume is FAT32
		fat->fs_type = FS_FAT32;
		fat->FATDataSize = 4;
		fat->EOC = FAT32_EOF_CLUSTER;
		
		// Root directory first cluster
		temp_long.Int[0] = SD_Buffer[FAT32_BPB_ROOT_CLUST];
		temp_long.Int[1] = SD_Buffer[FAT32_BPB_ROOT_CLUST + 1];
		temp_long.Int[2] = SD_Buffer[FAT32_BPB_ROOT_CLUST + 2];
		temp_long.Int[3] = SD_Buffer[FAT32_BPB_ROOT_CLUST + 3];
		fat->RootFirstCluster = temp_long.Long;
	}
	
	if((fat->fs_type != FS_FAT16) && (fat->fs_type != FS_FAT32)) return MR_UNSUPPORTED_FS;
	
	// Location of the Root Directory
	fat->RootFirstSector = fat->FirstDataSector - fat->RootDirSectors + fat->fs_partition_offset;
	
	// FAT1 and FAT2 start sector
	fat->Fat1StartSector = fat->fs_partition_offset + BPB_RsvdSecCnt;
	fat->Fat2StartSector = fat->Fat1StartSector + fat->FATSz;
	
	// This is used to calculate free space and find the number of free valid clusters
	// inside the FAT table. This number represents the invalid clusters at the end of the FAT table.
	// So this clusters are not valid because that would translate to sectors outside the valid range.
	// That is because the number of valid clusters are not divisible exactly by the size of a sector.
	// To find the number of extra clusters, first find the total clusters that the FAT table can hold and then subtract
	// the number of valid clusters fat->CountofClusters - 2 clusters reserved for the system.
	//fat->FATmirageClusters = (fat->FATSz * (fat->BPB_BytsPerSec / fat->FATDataSize)) - fat->CountofClusters - 2;
	
	fat->entries_per_sector = fat->BPB_BytsPerSec / 32;
	return MR_OK;
}


/*______________________________________________________________________________________________
	Return volume free space in bytes
_______________________________________________________________________________________________*/
uint64_t FAT_volumeFreeSpace(void){
	uint64_t free_clusters = FAT_tableFindFree(FAT_TASK_TABLE_COUNT_FREE);
	return  free_clusters * fat->BPB_SecPerClus * fat->BPB_BytsPerSec;
}


/*______________________________________________________________________________________________
	Return volume capacity in bytes
_______________________________________________________________________________________________*/
uint64_t FAT_volumeCapacity(void){
	return (fat->BPB_TotSec32 - fat->FirstDataSector) * fat->BPB_BytsPerSec;
}


/*______________________________________________________________________________________________
	Return volume capacity in KiB
_______________________________________________________________________________________________*/
float FAT_volumeCapacityKB(void){
	return FAT_volumeCapacity() / 1024;
}


/*______________________________________________________________________________________________
	Return volume capacity in MiB
_______________________________________________________________________________________________*/
float FAT_volumeCapacityMB(void){
	return FAT_volumeCapacityKB() / 1024;
}


/*______________________________________________________________________________________________
	Return volume capacity in GiB
_______________________________________________________________________________________________*/
float FAT_volumeCapacityGB(void){
	return FAT_volumeCapacityMB() / 1024;
}


/*______________________________________________________________________________________________
	Returns the label and serial number of a volume
	
	label		Pointer to the buffer to store the volume label. 
				If the volume has no label, a null-string will be returned.
				The buffer array must be of type char and 12 bytes in size: 11 for
				volume label and 1 for the null.
						
	vol_sn		Pointer to a uint32_t variable to store the volume serial number.
				Pass 0 if not needed.
						
	Example:
				char label[12];
				uint32_t vol_sn = 0;
				FAT_getLabel(label, &vol_sn);
_______________________________________________________________________________________________*/
FRESULT FAT_getLabel(char* label, uint32_t* vol_sn){
	uint8_t null_pos = 0;
	lng buf;
	
	// Read first sector of root
	FRESULT return_code = sd_read_single_block(fat->RootFirstSector, SD_Buffer);
	if(return_code)	return return_code;
	
	// Extract label
	for(uint8_t i = 11; i-- > 0; ){
		// Trim white spaces at the end
		if((SD_Buffer[i] != ' ') || null_pos){
			label[i] = SD_Buffer[i];
			if(null_pos == 0) null_pos = i;
		}
	}
	
	label[null_pos + 1] = 0; // add null
	
	// Extract volume serial number
	// Read first sector of boot record
	return_code = sd_read_single_block(fat->fs_partition_offset, SD_Buffer);
	if(return_code)	return return_code;
	
	#if FAT_SUPPORT_FAT32 == 1
		if(fat->fs_type == FS_FAT32){			
			buf.Int[0] = SD_Buffer[FAT32_BS_VOL_ID];
			buf.Int[1] = SD_Buffer[FAT32_BS_VOL_ID + 1];
			buf.Int[2] = SD_Buffer[FAT32_BS_VOL_ID + 2];
			buf.Int[3] = SD_Buffer[FAT32_BS_VOL_ID + 3];
		}else 
	#endif
	
	if(fat->fs_type == FS_FAT16){
		buf.Int[0] = SD_Buffer[FAT16_BS_VOL_ID];
		buf.Int[1] = SD_Buffer[FAT16_BS_VOL_ID + 1];
		buf.Int[2] = SD_Buffer[FAT16_BS_VOL_ID + 2];
		buf.Int[3] = SD_Buffer[FAT16_BS_VOL_ID + 3];
	}

	*vol_sn = buf.Long;
	return FR_OK;
}



/*______________________________________________________________________________________________
	Create time and date fields in FAT format with dummy values that could be
	substituted with values from a RTC
_______________________________________________________________________________________________*/
uint8_t FAT_createTimeMilli(void){
	return 1000 / 10 * 2 - 1; // dummy 1000ms
}

uint16_t FAT_createTime(void){
	uint8_t hh = 1;
	uint8_t mm = 0;
	uint8_t ss = 0;
	
	uint16_t time = (hh << 11) | (mm << 5) | (ss / 2);
	return time;
}

uint16_t FAT_createDate(void){
	uint8_t YY = 2022 - 1980; // do not modify or remove 1980
	uint8_t MM = 1;
	uint8_t DD = 1;
	
	uint16_t date = (YY	<< 9) | (MM << 5) | DD;
	return date;
}



/*______________________________________________________________________________________________
	Create a subdirectory at the specified path. Name of the subdirectory is the name
	after the last slash '/' in path.
_______________________________________________________________________________________________*/
FRESULT FAT_makeDir(const char* path){
	return _FAT_dirRegister(path, FAT_TASK_MKDIR);
}



/*______________________________________________________________________________________________
	Open a directory using the given path
_______________________________________________________________________________________________*/
FRESULT FAT_openDir(DIR* dir_p, const char* path){
	FRESULT res;
	dir_p->dir_active_item = dir_p->dir_entry_offset = 0;
	dir_p->dir_start_cluster = fat->RootFirstCluster;
	dir_p->dir_open = false;
	
	_FAT_moveWindow(dir_p, fat->RootFirstCluster); // start directory search from root
	
	res = _FAT_followPath(dir_p, path, FAT_TASK_OPEN_DIR);
	if(res != FR_OK) return res;
	
	// Set window to the beginning of directory for subsequent functions
	_FAT_moveWindow(dir_p, dir_p->dir_start_cluster);
	
	dir_p->dir_open = true;
	return res;
}



/*______________________________________________________________________________________________
	Open a directory with the given index inside the active directory.
	FAT_openDir() must be used before running this function.
_______________________________________________________________________________________________*/
FRESULT FAT_openDirByIndex(DIR* dir_p, FILE* finfo_p, uint16_t idx){
	FRESULT res;
	
	dir_p->dir_open = false;
	dir_p->dir_open_by_idx = true;
	dir_p->dir_active_item = dir_p->dir_entry_offset = 0;
	_FAT_moveWindow(dir_p, dir_p->dir_start_cluster);
	
	res = FAT_findByIndex(dir_p, finfo_p, idx); // find starting cluster of dir
	if(res != FR_OK) return res;
	
	dir_p->dir_open_by_idx = false;
	dir_p->dir_active_item = dir_p->dir_entry_offset = 0;
	
	if(!(finfo_p->file_attrib & FAT_FILE_ATTR_DIRECTORY)){ // check if it is a directory
		return FR_NOT_A_DIRECTORY;
	}
	
	// Set window to the beginning of directory for subsequent functions
	_FAT_moveWindow(dir_p, dir_p->dir_start_cluster);
	
	dir_p->dir_open = true;
	return res;
}



/*______________________________________________________________________________________________
	Go to parent directory of active directory. If the active directory is Root, then
	the function will return FR_ROOT_DIR and active directory will remain Root.
_______________________________________________________________________________________________*/
FRESULT FAT_dirBack(DIR* dir_p){
	FRESULT res;
	lng buf;
	
	// Check if this is the root directory
	if(dir_p->dir_start_cluster == fat->RootFirstCluster) return FR_ROOT_DIR;
	dir_p->dir_open = false;
	
	_FAT_moveWindow(dir_p, dir_p->dir_start_cluster); // start from beginning of directory
	
	// Read first sector
	res = sd_read_single_block(dir_p->dir_start_sector, SD_Buffer);
	if(res) return FR_DEVICE_ERR;
	
	// Get cluster of parent
	buf.Int[0] = SD_Buffer[FAT_DIR_FIRST_CLUS_LOW + 32];
	buf.Int[1] = SD_Buffer[FAT_DIR_FIRST_CLUS_LOW + 32 + 1];
	
	#if FAT_SUPPORT_FAT32 == 1
		if(fat->fs_type == FS_FAT32){
			buf.Int[2] = SD_Buffer[FAT_DIR_FIRST_CLUS_HIGH + 32];
			buf.Int[3] = SD_Buffer[FAT_DIR_FIRST_CLUS_HIGH + 32 + 1];
			
			if(buf.Long == 0) buf.Long = fat->RootFirstCluster;
		}
	#endif
	
	dir_p->dir_start_cluster = buf.Long;
	
	// Set window to parent directory
	_FAT_moveWindow(dir_p, dir_p->dir_start_cluster);
	dir_p->dir_active_item = dir_p->dir_entry_offset = 0;
	
	dir_p->dir_open = true;
	return FR_OK;
}


/*______________________________________________________________________________________________
	Get file info of the item in the directory with a specific index position.
	Index must start from 1. If index is greater than the number of items inside the
	directory, then FR_NOT_FOUND will be returned.
_______________________________________________________________________________________________*/
FRESULT FAT_findByIndex(DIR* dir_p, FILE* finfo_p, uint16_t idx){
	FRESULT res;
	
	dir_p->find_by_index = idx;
	dir_p->dir_active_item = 0;
	dir_p->dir_entry_offset = 0;
	
	_FAT_moveWindow(dir_p, dir_p->dir_start_cluster); // start from beginning of directory
	res = FAT_findNext(dir_p, finfo_p);
	
	dir_p->find_by_index = 0;
	return res;
}


/*______________________________________________________________________________________________
	Get file info of the first or next item in the directory that was opened previously
_______________________________________________________________________________________________*/
FRESULT FAT_findNext(DIR* dir_p, FILE* finfo_p){
	uint16_t idx;
	uint8_t response_code;
	bool has_long_name = false;
	lng buff_long;
	
	// The object signature is stored by bufferModBy and if another object modifies the main buffer
	// then the sector is re-loaded
	if((dir_p->dir_entry_offset == 0) || (bufferModBy != dir_p)){
		response_code = sd_read_single_block(dir_p->dir_start_sector + dir_p->dir_active_sector, SD_Buffer);
		if(response_code) return FR_DEVICE_ERR;
		bufferModBy = dir_p;
	}
	
	BEGIN:;
		
	// Parse each entry in a sector
	for(uint8_t e = dir_p->dir_entry_offset; e < fat->entries_per_sector; e++){
		idx = FAT_DIR_NAME + (e * 32);
			
		// If first char of the entry is 0x00 there are no more entries inside directory
		if(SD_Buffer[idx] == FAT_DIR_FREE_SLOT)	return FR_NOT_FOUND;
			
		// The file was deleted, so skip it
		if(SD_Buffer[idx] == FAT_FILE_DELETED) continue;
			
		// Skip (.) and (..) directories after saving parent directory's cluster
		if(SD_Buffer[idx] == '.')	continue;
			
		// File Attribute
		finfo_p->file_attrib = SD_Buffer[FAT_DIR_ATTR + (e * 32)];
			
		// Skip Volume ID
		if(finfo_p->file_attrib == FAT_FILE_ATTR_VOLUME_ID) continue;
			
		// Long name entry
		if(finfo_p->file_attrib == FAT_FILE_ATTR_LONG_NAME){
			_FAT_parse_long_names(FAT_filename, e);
			has_long_name = true;
				
			// Skip to the next entry since this is a long name entry and doesn't have any attributes
			continue;
				
		}else{
				
			// Parse the short file name only if there is no LFN
			// or a search for SFN was requested in task
			if(has_long_name == false){
				_FAT_getSFN(e, 0, finfo_p->file_attrib);
			}
			
			has_long_name = false;
		} // end get file name
		
		
		// File size
		buff_long.Int[0] = SD_Buffer[FAT_DIR_FILE_SIZE + (e * 32)];
		buff_long.Int[1] = SD_Buffer[FAT_DIR_FILE_SIZE + (e * 32) + 1];
		buff_long.Int[2] = SD_Buffer[FAT_DIR_FILE_SIZE + (e * 32) + 2];
		buff_long.Int[3] = SD_Buffer[FAT_DIR_FILE_SIZE + (e * 32) + 3];
		finfo_p->file_size = buff_long.Long;
		
		// Get write date
		buff_long.Int[0] = SD_Buffer[FAT_DIR_WRITE_DATE + (e * 32)];
		buff_long.Int[1] = SD_Buffer[FAT_DIR_WRITE_DATE + (e * 32) + 1];
		finfo_p->file_write_date = buff_long.Long;
		
		// Get write time
		buff_long.Int[0] = SD_Buffer[FAT_DIR_WRITE_TIME + (e * 32)];
		buff_long.Int[1] = SD_Buffer[FAT_DIR_WRITE_TIME + (e * 32) + 1];
		finfo_p->file_write_time = buff_long.Long;
		
		// Start cluster
		buff_long.Int[0] = SD_Buffer[FAT_DIR_FIRST_CLUS_LOW + (e * 32)];
		buff_long.Int[1] = SD_Buffer[FAT_DIR_FIRST_CLUS_LOW + (e * 32) + 1];
		
		#if FAT_SUPPORT_FAT32 == 1
			if(fat->fs_type == FS_FAT32){
				buff_long.Int[2] = SD_Buffer[FAT_DIR_FIRST_CLUS_HIGH + (e * 32)];
				buff_long.Int[3] = SD_Buffer[FAT_DIR_FIRST_CLUS_HIGH + (e * 32) + 1];
			}
		#endif
		
		if(dir_p->dir_open_by_idx == true) dir_p->dir_start_cluster = buff_long.Long;
		else finfo_p->file_start_cluster = buff_long.Long;
			
		// Keep track of the index position of the item inside this directory
		dir_p->dir_active_item++;
		dir_p->dir_entry_offset = e + 1;
		
		if(dir_p->find_by_index == 0){
			return FR_OK;
		}else if(dir_p->dir_active_item == dir_p->find_by_index){
			return FR_OK;
		}
	}
		
	// Find next item and increment it's index
	dir_p->dir_active_sector++;
	if(_FAT_getFileNextSector(dir_p) == FR_OK){
		dir_p->dir_entry_offset = 0;
		if(dir_p->dir_active_sector) dir_p->dir_active_sector--;
		bufferModBy = dir_p;
		goto BEGIN;
	}
	
	return FR_NOT_FOUND;
}



/*______________________________________________________________________________________________
	Return the total number of files and folders inside the active directory
_______________________________________________________________________________________________*/
uint16_t FAT_dirCountItems(DIR* dir_p){
	dir_p->dir_nr_of_items = 0;
	_FAT_moveWindow(dir_p, dir_p->dir_start_cluster);
	_FAT_followPath(dir_p, "", FAT_TASK_COUNT_ITEMS); // count items
	return dir_p->dir_nr_of_items;
}



/*______________________________________________________________________________________________
	Private: Used to create a subdirectory or file
_______________________________________________________________________________________________*/
static FRESULT _FAT_dirRegister(const char* path, uint8_t task){
	DIR dir_obj;
	const char* lfn_buf;
	char f_sfn[11 + 1]; // short file name length + null
	f_sfn[11] = 0;
	uint16_t entry_start = 0;
	uint16_t entry_cluster = 0;
	uint16_t entry_sector_offset = 0;
	uint32_t entry_start_sector = 0;
	uint16_t dir_nr_of_entries = 0;
	uint16_t idx = 0;
	uint16_t e = 0;
	uint16_t time = 0, date = 0;
	CLSTSIZE_t file_cluster_available;
	SECTSIZE_t file_first_sector = 0;
	uint8_t response_code;
	uint8_t path_length = 0;
	uint8_t dir_entries_necessary;
	uint8_t empty_entries = 0;
	uint8_t lfn_ordinal;
	uint8_t lfn_last_entry_chars = 0;
	uint8_t lfn_checksum = 0;
	uint8_t lfn_null_pos = 0;
	bool stop = false;
	bool needs_lfn = false;
	
	// Number of entries per sector given a 32 bytes entry
	// For a 512 bytes per sector: 512 / 32 = 16
	uint8_t entries_per_sector = fat->entries_per_sector;
	
	// Set starting window at the beginning of root for file search
	dir_obj.dir_start_cluster = fat->RootFirstCluster;
	_FAT_moveWindow(&dir_obj, 0);
	
	// Find if the directory exists and if not get the cluster
	// of the directory to create the subdirectory or file in
	response_code = _FAT_followPath(&dir_obj, path, 0);
	if(response_code != FR_NOT_FOUND) return FR_EXIST; // name collision, a file with this name already exists
	
	// Check the length of the file path including filename and extension
	while(*path){
		path++;
		path_length++;
		if(path_length > FS_MAX_PATH) return FR_PATH_LENGTH_EXCEEDED;
	}
	
	// Get the filename extracted from path by the _FAT_followPath()
	path = dir_obj.ptr_path_buff;
	
	// Check if the path was found
	while(*path){
		// The path to the filename was not found otherwise the _FAT_followPath would have removed the slash
		if(IsSeparator(*path)) return FR_NO_PATH;
		path++;
	}
	
	// Set string pointer to were the filename begins
	path = lfn_buf = dir_obj.ptr_path_buff;
	
	// Set starting window at the beginning of the directory that was found
	// to create the file in
	_FAT_moveWindow(&dir_obj, dir_obj.dir_start_cluster);

	// Generate the SFN and check if it needs LFN
	response_code = _FAT_createSFN(&dir_obj, path, f_sfn, &needs_lfn);
	if(response_code != FR_NOT_FOUND) return FR_EXIST;
	dir_obj.dir_active_sector = 0;
	
	if(dir_obj.filename_length == 0) return FR_DENIED; // no filename
	
	// Calculate how many entries are necessary for the LFN
	// given that one entry can hold 13 chars
	dir_entries_necessary = dir_obj.filename_length / FAT_LFN_MAX_CHARS;
	if(needs_lfn && (lfn_last_entry_chars = MATH_remainder(dir_obj.filename_length, FAT_LFN_MAX_CHARS, dir_entries_necessary))){
		dir_entries_necessary++; // +1 because there is a remainder
	}
	dir_entries_necessary++; // +1 for SFN entry
	
	// Set the ordinal number of the last entry
	lfn_ordinal = (dir_entries_necessary - 1) | FAT_LAST_LONG_ENTRY_MASK; // subtract 1 for SFN
	
	// Calculate checksum for LFN fragments based on the SFN
	lfn_checksum = ChkSum((uint8_t*)f_sfn);
	
	// Set starting window at the beginning of the directory to create the file in
	_FAT_moveWindow(&dir_obj, dir_obj.dir_start_cluster);
	
	
	// Read the sectors in a cluster (or root directory) of the directory to create the file/subdirectory in
	// and find the necessary contiguous free entries
	while((response_code = _FAT_getFileNextSector(&dir_obj)) == FR_OK){
		// Parse each entry in a sector
		for(e = 0; e < entries_per_sector; e++){
			idx = FAT_DIR_NAME + (e * 32);
			dir_nr_of_entries++;
			
			// Find a free entry in the directory to create in
			if((SD_Buffer[idx] == FAT_DIR_FREE_SLOT) || (SD_Buffer[idx] == FAT_FILE_DELETED)){
				empty_entries++;
				
				// Mark the start of the first free entry
				if(entry_start_sector == 0){
					entry_start = e;
					entry_start_sector = dir_obj.dir_start_sector;
					entry_sector_offset = dir_obj.dir_active_sector - 1;
					entry_cluster = dir_obj.dir_active_cluster;
				}
				
				if(empty_entries == dir_entries_necessary){
					stop = true;
					break;
				}
			}else{
				empty_entries = entry_start_sector = 0;
			}
		} // end of entry loop
		
		if(stop) break;
	}
	
	
	
	// Expand directory if not enough free entries and not root directory
	if(empty_entries != dir_entries_necessary && dir_obj.dir_start_cluster > 1 && dir_nr_of_entries < 65535){
		// Find a free cluster in the FAT table
		file_cluster_available = FAT_tableFindFree(FAT_TASK_TABLE_FIND_FREE);
		if(file_cluster_available < 2) return FR_NO_SPACE; // no free space found
		
		// Update FAT table to point to the new allocated cluster
		// dir_obj.dir_start_cluster must point to file_cluster_available
		response_code = _FAT_tableReadSet(dir_obj.dir_start_cluster, file_cluster_available, FAT_TASK_TABLE_SET);
		if(response_code == 0) return FR_DEVICE_ERR;
		
		// Allocate the free cluster by marking it with EOC (End Of Chain)
		response_code = _FAT_tableReadSet(file_cluster_available, fat->EOC, FAT_TASK_TABLE_SET);
		if(response_code == 0) return FR_DEVICE_ERR;
		
		// Clear the cluster with 0
		_FAT_clearCluster(file_cluster_available, 0);

		empty_entries = dir_entries_necessary;
	}
	
	
	// There are enough contiguous free entries
	if(empty_entries == dir_entries_necessary){
		// Reset the active sector to where the first empty entry is
		dir_obj.dir_active_cluster = entry_cluster;
		dir_obj.dir_start_sector = entry_start_sector;
		dir_obj.dir_active_sector = entry_sector_offset;
		
		// A file can only have a cluster allocated when it's size is not 0
		if(task == FAT_TASK_MKDIR){
			// Find a free cluster in the FAT table for the new file
			// Allocate the free cluster by marking it with EOC (End Of Chain)
			response_code = _FAT_allocateCluster(&file_cluster_available, fat->EOC);
			if(response_code != FR_OK) return response_code;
			
			// Clear the cluster with 0
			_FAT_clearCluster(file_cluster_available, 0);
			
			file_first_sector = _FAT_clusterToSector(file_cluster_available);
			if(file_first_sector == 0) return FR_DENIED;
		}else{
			file_cluster_available = 0;
		}
		

		// Load first sector containing the first empty entry
		if(_FAT_getFileNextSector(&dir_obj) != FR_OK) return FR_DEVICE_ERR;
		
		// Parse sectors containing empty entries
		while(empty_entries){
			
			// Start from the beginning of the next sector
			if(entry_start == entries_per_sector){ // number of 32-bytes long entries in one sector
				// Load next sector
				if(_FAT_getFileNextSector(&dir_obj) != FR_OK) return FR_DEVICE_ERR;
				entry_start = 0;
			}
			
			// Fill the entry with 0
			_FAT_fillBufferArray(entry_start * 32, 32, 0);
			
			// Set LFN
			if(needs_lfn && empty_entries > 1){
				// Move pointer to the beginning of the filename
				path = lfn_buf;
				
				SD_Buffer[entry_start * 32] = lfn_ordinal; // LFN fragment ordinal
				lfn_ordinal = empty_entries - 2;
				
				if(dir_obj.filename_length > FAT_LFN_MAX_CHARS){
					path += dir_obj.filename_length - ((dir_entries_necessary - empty_entries) * FAT_LFN_MAX_CHARS) - lfn_last_entry_chars;
				}
				
				// Long File Name fragment
				_FAT_stringLFN((entry_start * 32) + FAT_LONG_DIR_NAME, 10, &path, 2); // Characters 1-5 (5 chars)
				_FAT_stringLFN((entry_start * 32) + FAT_LONG_DIR_NAME2, 12, &path, 2); // Characters 6-11 (6 chars)
				_FAT_stringLFN((entry_start * 32) + FAT_LONG_DIR_NAME3, 4, &path, 2); // Characters 12-13 (2 chars)
				
				// Set the null at the end of the LFN string if it doesn't fill all 13 chars
				if(lfn_last_entry_chars && lfn_null_pos == 0){
					if(lfn_last_entry_chars < 5) lfn_null_pos = FAT_LONG_DIR_NAME + (lfn_last_entry_chars * 2);
					else if(lfn_last_entry_chars < 11) lfn_null_pos = FAT_LONG_DIR_NAME2 + ((lfn_last_entry_chars - 5) * 2);
					else if(lfn_last_entry_chars < 13) lfn_null_pos = FAT_LONG_DIR_NAME3 + ((lfn_last_entry_chars - 5 - 6) * 2);
					SD_Buffer[(entry_start * 32) + lfn_null_pos] = 0;
					SD_Buffer[(entry_start * 32) + lfn_null_pos + 1] = 0;
				}

				SD_Buffer[(entry_start * 32) + FAT_LONG_DIR_ATTR] = FAT_FILE_ATTR_LONG_NAME; // LFN fragment attribute
				SD_Buffer[(entry_start * 32) + FAT_LONG_DIR_CHECKSUM] = lfn_checksum; // LFN fragment checksum
				
			}else if(empty_entries == 1){ // Set SFN entry
				idx = entry_start * 32;
				
				 // File attributes
				if(task == FAT_TASK_MKDIR){
					SD_Buffer[idx + FAT_DIR_ATTR] = FAT_FILE_ATTR_DIRECTORY; // if task is to create directory
				}else{
					SD_Buffer[idx + FAT_DIR_ATTR] = FAT_FILE_ATTR_ARCHIVE; // if task is to create a file
				}
				
				// The code size is less placing the bytes using bit shifting than using a function
				SD_Buffer[idx + FAT_DIR_CREAT_TIME_MILLIS] = FAT_createTimeMilli(); // Creation time: milliseconds (0-199ms), optional
				time = FAT_createTime();
				SD_Buffer[idx + FAT_DIR_CREAT_TIME] = time; // Creation time: HH:MM:SS - optional
				SD_Buffer[idx + FAT_DIR_CREAT_TIME + 1] = time >> 8;
				SD_Buffer[idx + FAT_DIR_WRITE_TIME] = time; // Write time: HH:MM:SS
				SD_Buffer[idx + FAT_DIR_WRITE_TIME + 1] = time >> 8;
				date = FAT_createDate();
				SD_Buffer[idx + FAT_DIR_CREAT_DATE] = date; // Creation date: Year/Month/Day - optional
				SD_Buffer[idx + FAT_DIR_CREAT_DATE + 1] = date >> 8;
				SD_Buffer[idx + FAT_DIR_WRITE_DATE] = date; // Write date: Year/Month/Day
				SD_Buffer[idx + FAT_DIR_WRITE_DATE + 1] = date >> 8;
				//SD_Buffer[idx + FAT_DIR_LAST_ACC_DATE] = 0; // Last access date - optional
				//SD_Buffer[idx + FAT_DIR_LAST_ACC_DATE + 1] = 0;
				SD_Buffer[idx + FAT_DIR_FIRST_CLUS_LOW] = file_cluster_available; // First cluster low
				SD_Buffer[idx + FAT_DIR_FIRST_CLUS_LOW + 1] = file_cluster_available >> 8;

				#if FAT_SUPPORT_FAT32 == 1
					if(fat->fs_type == FS_FAT32){
						SD_Buffer[idx + FAT_DIR_FIRST_CLUS_HIGH] = file_cluster_available >> 16; // First cluster high (0 for FAT16)
						SD_Buffer[idx + FAT_DIR_FIRST_CLUS_HIGH + 1] = file_cluster_available >> 24;
					}
				#endif
				
				// Making this reserved field equal to 24 instead of 0 makes Windows to show
				// the SFN in lowercase. This is desired since it would be the original filename
				// because if it contains uppercase characters it would have a LFN associated.
				if(needs_lfn == false) SD_Buffer[idx + FAT_DIR_NT_RES] = 24;
				
				for(uint8_t i = 0; i < 11; i++){
					SD_Buffer[(entry_start * 32) + i] = f_sfn[i]; // SFN
				}
			}
			
			// Write the sector only if this is the last entry or the sector is to be changed in the next loop
			if((entry_start == (entries_per_sector) - 1) || (empty_entries == 1)){
				sd_write_single_block(dir_obj.dir_start_sector + dir_obj.dir_active_sector - 1, SD_Buffer);
				if(SD_ResponseToken != 0x05) return FR_DEVICE_ERR;
			}
			
			empty_entries--;
			entry_start++;
		}
		
		
		// Add (.) and (..) entries
		if(task == FAT_TASK_MKDIR){
			_FAT_fillBufferArray(0, fat->BPB_BytsPerSec, 0); // Fill the array buffer with 0
			
			// The dot entry is a directory that points to itself
			SD_Buffer[FAT_DIR_NAME] = '.';
			_FAT_fillBufferArray(1, 11 - 1, ' '); // Fill name field after dot with spaces
			SD_Buffer[FAT_DIR_ATTR] = FAT_FILE_ATTR_DIRECTORY;
			SD_Buffer[FAT_DIR_WRITE_TIME] = time; // Write time: HH:MM:SS
			SD_Buffer[FAT_DIR_WRITE_TIME + 1] = time >> 8;
			SD_Buffer[FAT_DIR_WRITE_DATE] = date; // Write date: Year/Month/Day
			SD_Buffer[FAT_DIR_WRITE_DATE + 1] = date >> 8;
			SD_Buffer[FAT_DIR_FIRST_CLUS_LOW] = file_cluster_available; // First cluster low
			SD_Buffer[FAT_DIR_FIRST_CLUS_LOW + 1] = file_cluster_available >> 8;
			
			#if FAT_SUPPORT_FAT32 == 1
				if(fat->fs_type == FS_FAT32){
					SD_Buffer[FAT_DIR_FIRST_CLUS_HIGH] = file_cluster_available >> 16; // First cluster high (0 for FAT16)
					SD_Buffer[FAT_DIR_FIRST_CLUS_HIGH + 1] = file_cluster_available >> 24;
				}
			#endif
			
			// The dot dot entry points to the starting cluster of the parent of this directory 
			// (which is 0 if this directory's parent is the root directory)
			SD_Buffer[32 + FAT_DIR_NAME] = '.';
			SD_Buffer[32 + FAT_DIR_NAME + 1] = '.';
			_FAT_fillBufferArray(32 + 2, 11 - 2, ' '); // Fill name field after dot with spaces
			SD_Buffer[32 + FAT_DIR_ATTR] = FAT_FILE_ATTR_DIRECTORY;
			SD_Buffer[32 + FAT_DIR_WRITE_TIME] = time; // Write time: HH:MM:SS
			SD_Buffer[32 + FAT_DIR_WRITE_TIME + 1] = time >> 8;
			SD_Buffer[32 + FAT_DIR_WRITE_DATE] = date; // Write date: Year/Month/Day
			SD_Buffer[32 + FAT_DIR_WRITE_DATE + 1] = date >> 8;
			
			#if FAT_SUPPORT_FAT32 == 1
				if(fat->fs_type == FS_FAT32){
					if(dir_obj.dir_start_cluster == fat->RootFirstCluster){
						dir_obj.dir_start_cluster = 0;
					}
				
					SD_Buffer[32 + FAT_DIR_FIRST_CLUS_HIGH] = dir_obj.dir_start_cluster >> 16; // First cluster high (0 for FAT16)
					SD_Buffer[32 + FAT_DIR_FIRST_CLUS_HIGH + 1] = dir_obj.dir_start_cluster >> 24;
				}
			#endif
			
			SD_Buffer[32 + FAT_DIR_FIRST_CLUS_LOW] = dir_obj.dir_start_cluster; // First cluster low
			SD_Buffer[32 + FAT_DIR_FIRST_CLUS_LOW + 1] = dir_obj.dir_start_cluster >> 8;
		
			sd_write_single_block(file_first_sector, SD_Buffer);
			if(SD_ResponseToken != 0x05) return FR_DEVICE_ERR;
		}
		
	}else{
		return FR_DENIED;
	}
	
	return FR_OK;
}



/*______________________________________________________________________________________________
	Create a file at the specified path. Name of the file is the name after 
	the last slash '/' in path.
_______________________________________________________________________________________________*/
FRESULT FAT_makeFile(const char* path){
	return _FAT_dirRegister(path, FAT_TASK_MKFILE);
}


/*______________________________________________________________________________________________
	Wrapper function of fwrite() used to convert a float number 
	into a string and write it to a file
	
	decimals	number of digits after the dot
_______________________________________________________________________________________________*/
FRESULT FAT_fwriteFloat(FILE* fp, float float_nr, uint8_t decimals){
	char string_integer[MAX_NR_OF_DIGITS + 1];
	char string_float[MAX_NR_OF_DIGITS + 1];
	
	STRING_ftoa(float_nr, string_integer, string_float, 0, decimals);
	
	uint8_t res;
	if(float_nr < 0) FAT_fwriteString(fp, "-");
	res = FAT_fwriteString(fp, string_integer);
	res |= FAT_fwriteString(fp, ".");
	res |= FAT_fwriteString(fp, string_float);
	
	if(res) return FR_DEVICE_ERR;
	return FR_OK;
}



/*______________________________________________________________________________________________
	Wrapper function of fwrite() used to convert a number into a string and write it to a file
_______________________________________________________________________________________________*/
FRESULT FAT_fwriteInt(FILE* fp, INT_SIZE nr){
	char string[MAX_NR_OF_DIGITS];
	
	STRING_itoa(nr, string, 0);
	return FAT_fwriteString(fp, string);
}



/*______________________________________________________________________________________________
	Wrapper function of fwrite() used to write a string
_______________________________________________________________________________________________*/
FRESULT FAT_fwriteString(FILE* fp, const char* string){
	uint16_t bw;
	uint16_t btw = strlen(string);
	return FAT_fwrite(fp, string, btw, &bw);
}



/*______________________________________________________________________________________________
	Write data to the file at the file offset pointed by read/write pointer.
	The write pointer advances with each byte written.
	CAUTION: running other functions will overwrite the common data buffer causing the loss of
	unsaved data. Use fsync() before using any other function including fseek().
	
	fp			Pointer to the file object structure
	buff		Pointer to the data to be written
	btw			Number of bytes to write
	bw			Pointer to the variable to return number of bytes written
_______________________________________________________________________________________________*/
FRESULT FAT_fwrite(FILE* fp, const void* buff, uint16_t btw, uint16_t* bw){
	FRESULT res;
	CLSTSIZE_t file_cluster_available = 0;
	uint16_t i = 0;
	const uint8_t *wbuff = (const uint8_t*)buff;
	*bw = 0;
	
	if(fp->file_open != true) return FR_DENIED;
	
	// Allocate and set start cluster if is not set
	if(fp->file_start_cluster == 0){
		res = _FAT_allocateCluster(&file_cluster_available, fat->EOC);
		if(res)	return res;
		
		fp->file_active_cluster = fp->file_start_cluster = file_cluster_available;
		fp->file_start_sector = _FAT_clusterToSector(file_cluster_available);
		fp->file_active_sector = 0;
		_FAT_clearCluster(file_cluster_available, 1); // clear only first sector
		
		res = _FAT_updateFileInfo(fp, FAT_TASK_SET_START_CLUSTER);
		if(res)	return res;
	}

	
	// Load the sector to continue writing to, if the fseek() or fsync() was used
	if(fp->w_sec_changed == true){
		fp->w_sec_changed = false;
		
		res = sd_read_single_block(fp->file_start_sector + fp->file_active_sector, SD_Buffer);
		if(res)	return FR_DEVICE_ERR;
	}
	
	
	// The case when the file pointer is set at the end of a cluster using fseek()
	if(fp->eof){
		res = _FAT_allocateCluster(&file_cluster_available, fp->file_active_cluster);
		if(res)	return res;
		fp->eof = false;
		
		fp->file_active_cluster = file_cluster_available;
		fp->file_start_sector = _FAT_clusterToSector(file_cluster_available);
	}
	
	
	// Cache data in the main buffer
	for(i = 0; i < btw; i++){
		// If the buffer is full, write it to storage device and go to next sector
		if(fp->buffer_idx > SD_BUFFER_SIZE - 1){
			fp->buffer_idx = 0;
			// Updating file size in fsync() while batch writting sectors is slow and unnecessary.
			// The file size in updated only when fsync() is not used here.
			fp->file_update_size = false;
			
			res = FAT_fsync(fp);
			if(res) return res;
			fp->file_active_sector++;
			
			fp->w_sec_changed = false; // set by fsync()
			fp->file_update_size = true;
			
			// Last sector of the cluster
			if(fp->file_active_sector >= fat->BPB_SecPerClus){
				// Buffer fp->file_active_cluster that _FAT_nextFileCluster() will modify
				file_cluster_available = fp->file_active_cluster;
				
				// Find and set next cluster of the file
				_FAT_nextFileCluster(fp);
				
				// Allocate a new cluster if this is the last one
				if(fp->eof){
					res = _FAT_allocateCluster(&file_cluster_available, file_cluster_available);
					if(res)	return res;
					fp->eof = false;
					
					fp->file_active_cluster = file_cluster_available;
					fp->file_start_sector = _FAT_clusterToSector(file_cluster_available);
				}
			}
			
			if(fp->fptr < fp->file_size){
				// Load the next sector if the file pointer is not at the end of file
				// to preserve existing data
				res = sd_read_single_block(fp->file_start_sector + fp->file_active_sector, SD_Buffer);
				if(res)	return FR_DEVICE_ERR;
			}else{
				// Fill the buffer with 0 to clear it for the next sector
				_FAT_fillBufferArray(0, SD_BUFFER_SIZE, 0);
			}
		}
		
		// fp->buffer_idx is set by fseek()
		SD_Buffer[fp->buffer_idx++] = *wbuff++;
		fp->fptr++;
	}
	
	*bw = i;
	return FR_OK;
}



/*______________________________________________________________________________________________
	Truncates the file size to the current file read/write pointer
_______________________________________________________________________________________________*/
FRESULT FAT_ftruncate(FILE* fp){
	CLSTSIZE_t next_clst;
	
	if((fp->fptr >= fp->file_size) || (fp->file_open != true)) return FR_DENIED; // if fptr is on the eof
	
	// When set file size to zero, remove entire cluster chain
	if(fp->fptr == 0){
		_FAT_removeChain(fp->file_start_cluster);
		fp->file_start_cluster = 0;
		_FAT_updateFileInfo(fp, FAT_TASK_SET_START_CLUSTER);
		_FAT_freset(fp);
	}else{
		// When truncate a part of the file, remove remaining clusters
		// fseek() set the file_active_cluster so we need the next cluster to start removing the chain
		next_clst = _FAT_tableReadSet(fp->file_active_cluster, 0, FAT_TASK_TABLE_GET_NEXT);
		if(next_clst == 0) return FR_DEVICE_ERR;
		
		// Mark current cluster as EOF
		if(_FAT_tableReadSet(fp->file_active_cluster, fat->EOC, FAT_TASK_TABLE_SET) == 0) return FR_DEVICE_ERR;
		
		// Remove rest of cluster chain
		_FAT_removeChain(next_clst);
	}
	
	fp->file_size = fp->fptr;
	_FAT_updateFileInfo(fp, FAT_TASK_SET_FILESIZE);
	
	return FR_OK;
}


/*______________________________________________________________________________________________
	Flush cached data of the writing file
_______________________________________________________________________________________________*/
FRESULT FAT_fsync(FILE* fp){
	FRESULT res;
	
	// If fseek() was used before and fwrite() return an error and didn't changed fp->w_sec_changed
	// to false, this prevents overwriting existing data
	if((fp->file_open != true) || (fp->w_sec_changed == true)) return FR_DENIED;
	
	// Write to file
	sd_write_single_block(fp->file_start_sector + fp->file_active_sector, SD_Buffer);
	if(SD_ResponseToken != 0x05) return FR_DEVICE_ERR;
	
	// Update file size
	if(fp->file_update_size == true && fp->fptr > fp->file_size){
		res = _FAT_updateFileInfo(fp, FAT_TASK_SET_FILESIZE);
		if(res) return res;
	}
	
	// Flag for the write function to load active sector in memory
	// since between fsync() and fwrite(), other functions could have
	// modified the buffer
	fp->w_sec_changed = true;
	
	return FR_OK;
}



/*______________________________________________________________________________________________
	Open a file using it's name. The search will be made inside the active directory.
_______________________________________________________________________________________________*/
FRESULT FAT_fopen(DIR* dir_p, FILE* file_p, char *file_name){
	FRESULT res;
	file_p->file_open = false;
	if(dir_p->dir_open == false) return FR_DENIED;
	
	// Move pointer to the beginning of active directory
	_FAT_moveWindow(dir_p, dir_p->dir_start_cluster);
	
	// Find the file and get the starting cluster
	res = _FAT_followPath(dir_p, file_name, FAT_TASK_FIND_FILE);
	if(res != FR_OK) return res;
	
	// Get file info
	res = FAT_findByIndex(dir_p, file_p, dir_p->dir_active_item);
	if(res) return res;
	
	// Save entry location as a handle for changing file size
	file_p->entry_start_sector = dir_p->dir_start_sector + dir_p->dir_active_sector;
	file_p->entry_offset = dir_p->dir_entry_offset - 1;
	
	file_p->w_sec_changed = true;
	file_p->file_update_size = true;
	
	_FAT_freset(file_p);
	return FR_OK;
}



/*______________________________________________________________________________________________
	Open a file by index. The search will be made inside the active directory.
_______________________________________________________________________________________________*/
FRESULT FAT_fopenByIndex(DIR* dir_p, FILE* file_p, uint16_t idx){
	FRESULT res;
	file_p->file_open = false;
	if(dir_p->dir_open == false) return FR_DENIED;
	
	// Get file info
	res = FAT_findByIndex(dir_p, file_p, idx);
	
	_FAT_freset(file_p);
	return res;
}



/*______________________________________________________________________________________________
	Private: used by file open functions
_______________________________________________________________________________________________*/
static void _FAT_freset(FILE* file_p){
	file_p->buffer_idx = 0;
	file_p->eof = false;
	file_p->file_open = true;
	file_p->fptr = 0;
	file_p->file_err = 0;
	file_p->file_active_cluster = file_p->file_start_cluster;
	file_p->file_start_sector = _FAT_clusterToSector(file_p->file_active_cluster);
	file_p->file_active_sector = 0;
}



/*______________________________________________________________________________________________
	Read data from a file. Each time the function will return a pointer to the main buffer 
	array containing a block of data that must be used before running other functions that 
	might overwrite the main buffer. The main buffer is used to preserve RAM. The file must
	be opened using the appropriate function before it can be read.
_______________________________________________________________________________________________*/
uint8_t* FAT_fread(FILE* file_p){
	FRESULT res;
	uint16_t idx;
	
	// End of a cluster
	if(file_p->file_active_sector >= fat->BPB_SecPerClus){
		// Find next cluster of the file
		_FAT_nextFileCluster(file_p);
	}
	
	if(file_p->eof)	return 0;
	
	// Read next sector
	res = sd_read_single_block(file_p->file_start_sector + file_p->file_active_sector, SD_Buffer);
	if(res){
		file_p->file_err = FR_DEVICE_ERR;
		return 0;
	}
	
	file_p->file_active_sector++;
	idx = file_p->buffer_idx;
	file_p->buffer_idx = 0;
	return &SD_Buffer[idx];
}


/*______________________________________________________________________________________________
	Return the file pointer
_______________________________________________________________________________________________*/
FSIZE_t FAT_getFptr(FILE* fp){
	return fp->fptr;
}



/*______________________________________________________________________________________________
	Move the file pointer to end of file. Wrapper of fseek().
_______________________________________________________________________________________________*/
void FAT_fseekEnd(FILE* fp){
	FAT_fseek(fp, fp->file_size);
}


/*______________________________________________________________________________________________
	Move the file pointer x number of bytes. fptr must not be greater 
	than the file size in bytes.
_______________________________________________________________________________________________*/
void FAT_fseek(FILE* fp, FSIZE_t fptr){	
	if(fptr > fp->file_size) return;
	
	// Calculate the number of sectors to skip
	SECTSIZE_t skip_sectors = fptr / fat->BPB_BytsPerSec;
	
	// Offset byte inside the buffer. Must not use the recalculated skip_clusters.
	fp->buffer_idx = fptr - (skip_sectors * fat->BPB_BytsPerSec);
	
	// Calculate how many clusters to skip
	CLSTSIZE_t skip_clusters = skip_sectors / fat->BPB_SecPerClus;
	
	// Recalculate sectors to skip based on the number of clusters
	if(skip_clusters){
		skip_sectors = skip_sectors - (fat->BPB_SecPerClus * skip_clusters);
	}
	
	// Since we don't keep track of how many clusters have been red
	// count the clusters from beginning of the file
	fp->file_active_cluster = fp->file_start_cluster;
	
	CLSTSIZE_t file_cluster_available;
	
	// Skip the necessary number of clusters
	while(skip_clusters){
		file_cluster_available = fp->file_active_cluster;
		_FAT_nextFileCluster(fp);
		
		if(fp->eof){
			// Restore active cluster so the write function can allocate a new cluster
			fp->file_active_cluster = file_cluster_available;
			break;
		}
		
		skip_clusters--;
	}
	
	fp->w_sec_changed = true; // flag for write function to load the sector
	fp->file_active_sector = skip_sectors; // set sector
	
	// Set file pointer
	fp->fptr = fptr;
	
	fp->eof = 0;
	fp->file_err = 0;
}



/*______________________________________________________________________________________________
	Used to check for the End Of File
_______________________________________________________________________________________________*/
bool FAT_feof(FILE* fp){
	return ((fp->eof) || (fp->fptr >= fp->file_size));
}


/*______________________________________________________________________________________________
	Used to check if an error occurs during file read
_______________________________________________________________________________________________*/
uint8_t FAT_ferror(FILE* fp){
	return fp->file_err;
}


/*______________________________________________________________________________________________
	Clear the error flag
_______________________________________________________________________________________________*/
void FAT_fclear_error(FILE* fp){
	fp->file_err = 0;
}



/*______________________________________________________________________________________________
	The filename is available immediately only after a function that provides the 
	file info. To preserve memory only a single filename array is used so when multiple
	files are active or functions that open a path are used those will modify the 
	filename buffer.
_______________________________________________________________________________________________*/
char* FAT_getFilename(void){
	return FAT_filename;
}



/*______________________________________________________________________________________________
	Return the index of the active item inside the opened directory
_______________________________________________________________________________________________*/
uint16_t FAT_getItemIndex(DIR* dir_p){
	return dir_p->dir_active_item;
}



/*______________________________________________________________________________________________
	Return the file size in bytes
_______________________________________________________________________________________________*/
FSIZE_t FAT_getFileSize(FILE* finfo_p){
	return finfo_p->file_size;
}



uint16_t FAT_getWriteYear(FILE* finfo_p){
	return 1980 + (finfo_p->file_write_date >> 9);
}


uint8_t FAT_getWriteMonth(FILE* finfo_p){
	return (finfo_p->file_write_date >> 5) & 0x0F;
}


uint8_t FAT_getWriteDay(FILE* finfo_p){
	return (finfo_p->file_write_date) & 0x1F;
}


uint8_t FAT_getWriteHour(FILE* finfo_p){
	return (finfo_p->file_write_time >> 11) & 0x1F;
}


uint8_t FAT_getWriteMinute(FILE* finfo_p){
	return (finfo_p->file_write_time >> 5) & 0x3F;
}


uint8_t FAT_getWriteSecond(FILE* finfo_p){
	// << 1 is similar to * 2
	// because the seconds number indicates the binary number of two-second
	// periods (0-29), representing seconds 0 to 58.
	return ((finfo_p->file_write_time) & 0x1F) << 1;
}


bool FAT_attrIsFolder(FILE* finfo_p){
	return finfo_p->file_attrib & FAT_FILE_ATTR_DIRECTORY;
}

bool FAT_attrIsFile(FILE* finfo_p){
	return (!(finfo_p->file_attrib & FAT_FILE_ATTR_DIRECTORY));
}

bool FAT_attrIsHidden(FILE* finfo_p){
	return (finfo_p->file_attrib & FAT_FILE_ATTR_HIDDEN);
}

bool FAT_attrIsSystem(FILE* finfo_p){
	return (finfo_p->file_attrib & FAT_FILE_ATTR_SYSTEM);
}

bool FAT_attrIsReadOnly(FILE* finfo_p){
	return (finfo_p->file_attrib & FAT_FILE_ATTR_READ_ONLY);
}

bool FAT_attrIsArchive(FILE* finfo_p){
	return (finfo_p->file_attrib & FAT_FILE_ATTR_ARCHIVE);
}



/*______________________________________________________________________________________________
	Private: Remove a cluster chain
	
	cluster			Cluster to remove a chain from
	prev_cluster	Previous cluster of <cluster> (0 if entire chain)
_______________________________________________________________________________________________*/
static void _FAT_removeChain(CLSTSIZE_t start_cluster){
	uint16_t timeout = 65535;
	
	// Remove cluster chain starting from start_cluster
	while(timeout && start_cluster && start_cluster != fat->EOC){
		start_cluster = _FAT_tableReadSet(start_cluster, 0, FAT_TASK_TABLE_READ_SET);
		timeout--;
	}
}



/*______________________________________________________________________________________________
	Private: Returns next file cluster or EOF. Used by the file read/write functions.
_______________________________________________________________________________________________*/
static uint8_t _FAT_nextFileCluster(FILE* file_p){
	if(file_p->file_active_cluster < 2) return 1; // cluster allocation must start from 2
	
	// Find next cluster of the file
	file_p->file_active_cluster = _FAT_tableReadSet(file_p->file_active_cluster, 0, FAT_TASK_TABLE_GET_NEXT);
	file_p->file_active_sector = 0;
	
	if(file_p->file_active_cluster == fat->EOC){
		file_p->eof = true;
		return 2;
	}else if(file_p->file_active_cluster == 0){
		file_p->file_err = FR_DEVICE_ERR;
		return 3;
	}
	
	file_p->file_start_sector = _FAT_clusterToSector(file_p->file_active_cluster);
	return 0;
}



/*______________________________________________________________________________________________
	Private: Update file directory entry
_______________________________________________________________________________________________*/
static FRESULT _FAT_updateFileInfo(FILE* fp, uint8_t task){
	FRESULT res;
	uint16_t idx = 0;
	
	res = sd_read_single_block(fp->entry_start_sector, SD_Buffer);
	if(res)	return FR_DEVICE_ERR;

	if(task == FAT_TASK_SET_FILESIZE){
		idx = (fp->entry_offset * 32) + FAT_DIR_FILE_SIZE;
		SD_Buffer[idx++] = fp->fptr;
		SD_Buffer[idx++] = fp->fptr >> 8;
		SD_Buffer[idx++] = fp->fptr >> 16;
		SD_Buffer[idx] = fp->fptr >> 24;
		
		fp->file_size = fp->fptr;
	}else if(task == FAT_TASK_SET_START_CLUSTER){
		idx = (fp->entry_offset * 32) + FAT_DIR_FIRST_CLUS_LOW;
		SD_Buffer[idx++] = fp->file_start_cluster; // First cluster low
		SD_Buffer[idx] = fp->file_start_cluster >> 8;

		#if FAT_SUPPORT_FAT32 == 1
			if(fat->fs_type == FS_FAT32){
				idx = (fp->entry_offset * 32) + FAT_DIR_FIRST_CLUS_HIGH;
				SD_Buffer[idx++] = fp->file_start_cluster >> 16; // First cluster high (0 for FAT16)
				SD_Buffer[idx] = fp->file_start_cluster >> 24;
			}
		#endif
	}

	sd_write_single_block(fp->entry_start_sector, SD_Buffer);
	if(SD_ResponseToken != 0x05) return FR_DEVICE_ERR;
	
	return FR_OK;
}



/*______________________________________________________________________________________________
	Private: Allocate a new cluster or expand a file
_______________________________________________________________________________________________*/
static FRESULT _FAT_allocateCluster(CLSTSIZE_t* new_cluster, CLSTSIZE_t fat_points_to){
	FRESULT response_code = 0;
	
	// Find a free cluster in the FAT table for the new file
	*new_cluster = FAT_tableFindFree(FAT_TASK_TABLE_FIND_FREE);
	if(*new_cluster < 2) return FR_NO_SPACE; // no free space found

	if(fat_points_to != fat->EOC){
		// Update FAT table to point to the new allocated cluster
		// dir_obj.dir_start_cluster must point to file_cluster_available
		response_code = _FAT_tableReadSet(fat_points_to, *new_cluster, FAT_TASK_TABLE_SET);
		if(response_code == 0) return FR_DEVICE_ERR;
		fat_points_to = fat->EOC;
	}
	
	// Allocate the free cluster by marking it with EOC (End Of Chain)
	response_code = _FAT_tableReadSet(*new_cluster, fat_points_to, FAT_TASK_TABLE_SET);
	if(response_code == 0) return FR_DEVICE_ERR;
	
	return FR_OK;
}




/*______________________________________________________________________________________________
	Private: Use the file start cluster to move the reading window to the first	sector 
	and cluster of file. This must be done after functions that move the window. 
_______________________________________________________________________________________________*/
static void _FAT_moveWindow(DIR* dir_p, CLSTSIZE_t start_cluster){
	if(start_cluster < 2) dir_p->dir_start_sector = fat->RootFirstSector; // start from root on FAT16
	else dir_p->dir_start_sector = _FAT_clusterToSector(start_cluster);
	dir_p->dir_active_cluster = start_cluster;
	dir_p->dir_active_sector = 0;
}



/*______________________________________________________________________________________________
	Private: Fill each sector in a cluster with 0
_______________________________________________________________________________________________*/
static FRESULT _FAT_clearCluster(CLSTSIZE_t cluster, bool first_sector){
	uint8_t response_code = FR_OK;
	
	_FAT_fillBufferArray(0, SD_BUFFER_SIZE, 0);
	cluster = _FAT_clusterToSector(cluster);
	
	for(uint16_t i = 0; i < fat->BPB_SecPerClus; i++){
		response_code = sd_write_single_block(cluster + i, SD_Buffer);
		if(SD_ResponseToken != 0x05) return FR_DEVICE_ERR;
		
		if(first_sector) break; // clear only first sector
	}
	
	return response_code;
}



/*______________________________________________________________________________________________
	Private: Find a free cluster and returns it's number inside the FAT table.
	Also depending on task, can parse the FAT table and count all free clusters.
_______________________________________________________________________________________________*/
static CLSTSIZE_t FAT_tableFindFree(uint8_t task){
	CLSTSIZE_t free_clusters = 0;
	CLSTSIZE_t cluster_nr = 0;
	uint16_t sector_range = fat->BPB_BytsPerSec;
	uint8_t return_code;
	lng buf;
	
	// Read each sector in the FAT table
	for(uint16_t s = 0; s < fat->FATSz; s++){
		return_code = sd_read_single_block(fat->Fat1StartSector + s, SD_Buffer);
		if(return_code)	return 0;
		
		// If this is the last FAT sector skip the invalid clusters at the end
		//if(s == fat->FATSz - 1) sector_range = fat->BPB_BytsPerSec - (fat->FATmirageClusters * fat->FATDataSize);
		
		// Parse each entry in the FAT table sector
		for(uint16_t i = 0; i < sector_range; ){
			#if FAT_SUPPORT_FAT32 == 1
				if(fat->fs_type == FS_FAT32){
					buf.Int[0] = SD_Buffer[i++];
					buf.Int[1] = SD_Buffer[i++];
					buf.Int[2] = SD_Buffer[i++];
					buf.Int[3] = SD_Buffer[i++];
					
					buf.Long &= 0x0FFFFFFF; // exclude the 4 MSB
				}else 
			#endif
			
			if(fat->fs_type == FS_FAT16){
				buf.Int[0] = SD_Buffer[i++];
				buf.Int[1] = SD_Buffer[i++];
			}
			
			if(buf.Long == 0){
				free_clusters++;
				if(task == FAT_TASK_TABLE_FIND_FREE) return cluster_nr;
			}
			
			cluster_nr++;
			
			// Skip the invalid clusters at the end
			if(cluster_nr > fat->CountofClusters + 2) break;
		}
	}
	
	return free_clusters;
}


//-----------------------------------------------------------------------------
// ChkSum()
// Returns an unsigned byte checksum computed on an unsigned byte
// array. The array must be 11 bytes long and is assumed to contain
// a name stored in the format of a MS-DOS directory entry.
// Passed: pFcbName Pointer to an unsigned byte array assumed to be
// 11 bytes long.
// Returns: Sum An 8-bit unsigned checksum of the array pointed
// to by pFcbName.
//------------------------------------------------------------------------------
static unsigned char ChkSum(unsigned char *pFcbName){
	unsigned char Sum = 0;

	for(short FcbNameLen = 11; FcbNameLen != 0; FcbNameLen--){
		// NOTE: The operation is an unsigned char rotate right
		Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *pFcbName++;
	}
	
	return Sum;
}



/*______________________________________________________________________________________________
	Private: Used to place LFN fragments in the main buffer array. The source string
	pointer is affected by the function intentionally. If the string ends before length,
	the remaining spaces are filled with 0xFF conforming with the FAT standard. 
	The null is not added here.
_______________________________________________________________________________________________*/
static void _FAT_stringLFN(uint16_t start_idx, uint16_t length, const char** data, uint8_t data_size){
	for(uint16_t i = 0; i < length; i += data_size){
		if(**data){
			SD_Buffer[start_idx + i] = **data;
			(*data)++;
		}else{
			SD_Buffer[start_idx + i] = 0xFF;
			SD_Buffer[start_idx + i + 1] = 0xFF;
		}
	}
}



/*______________________________________________________________________________________________
	Private: Fills the main buffer array with the specified value starting from a
	given index until the specified length
_______________________________________________________________________________________________*/
static void _FAT_fillBufferArray(uint16_t start_idx, uint16_t length, uint8_t value){
	for(uint16_t i = 0; i < length; i++){
		SD_Buffer[start_idx + i] = value;
	}
}



/*______________________________________________________________________________________________
	Private: Extracts the Short File Name into the FAT_filename array
_______________________________________________________________________________________________*/
static void _FAT_getSFN(uint8_t entry_pos, uint8_t task, uint8_t file_attrib){
	uint8_t short_name;
	uint8_t short_name_idx = 0;
	
	for(uint8_t j = 0; j < 11; j++){
		short_name = SD_Buffer[FAT_DIR_NAME + (entry_pos * 32) + j];
		
		// Convert the short name to lowercase
		if(IsUpper(short_name)) short_name += 32;
		
		if(j == 8 && task != FAT_TASK_SEARCH_SFN){
			FAT_filename[short_name_idx] = '.';
			short_name_idx++;
		}

		if((short_name != ' ') || (task == FAT_TASK_SEARCH_SFN)){
			FAT_filename[short_name_idx] = short_name;
			short_name_idx++;
		}
	}
	
	// Remove dot extension if directory
	if(short_name_idx && (file_attrib & FAT_FILE_ATTR_DIRECTORY)) short_name_idx--;
	
	// Add null
	FAT_filename[short_name_idx] = 0;
}



/*______________________________________________________________________________________________
	Private: Used by other functions to find if a file or path exists and when creating
	a file it clips only the name from the end of the path in dir_p->ptr_path_buff.
	When a file is found it saves the starting cluster.
	If the file is known to exist but the function returns not found, run chkdsk to verify
	for LFN errors.
_______________________________________________________________________________________________*/
static FRESULT _FAT_followPath(DIR* dir_p, const char* path, uint8_t task){
	char path_buff = 0;
	char name_buff = 0;
	uint16_t idx;
	uint8_t file_attrib;
	uint8_t j = 0;
	bool has_long_name = false;
	bool path_match = false;
	bool is_separator = false;
	lng buf;
	
	// Save the path start
	dir_p->ptr_path_buff = path;
	dir_p->dir_active_sector = 0;
	dir_p->dir_active_item = 0;
		
	while(_FAT_getFileNextSector(dir_p) == FR_OK){
		
		// Parse each entry in a sector
		for(uint8_t e = 0; e < fat->entries_per_sector; e++){
			idx = FAT_DIR_NAME + (e * 32);
			
			// If first char of the entry is 0x00 there are no more entries inside directory
			if(SD_Buffer[idx] == FAT_DIR_FREE_SLOT)	return FR_NOT_FOUND;
			
			// The file was deleted, so skip it
			if(SD_Buffer[idx] == FAT_FILE_DELETED) continue;
			
			// Skip (.) and (..) directories
			if(SD_Buffer[idx++] == '.')	continue;
			
			// File Attribute
			file_attrib = SD_Buffer[FAT_DIR_ATTR + (e * 32)];
			
			// Skip Volume ID
			if(file_attrib == FAT_FILE_ATTR_VOLUME_ID) continue;
			
			// Long name entry
			if(file_attrib == FAT_FILE_ATTR_LONG_NAME){
				if(task != FAT_TASK_SEARCH_SFN && task != FAT_TASK_COUNT_ITEMS){
					_FAT_parse_long_names(FAT_filename, e);
					has_long_name = true;
				}
				
				// Skip to the next entry since this is a long name entry and doesn't have any attributes
				continue;
				
			}else{
				
				// Parse the short file name only if there is no LFN
				// or a search for SFN was requested in task
				if((has_long_name == false) || (task == FAT_TASK_SEARCH_SFN)){
					_FAT_getSFN(e, task, file_attrib);
				}
				
			} // end get file name
			
			
			// Count the number of files and folders inside the active directory
			if(task == FAT_TASK_COUNT_ITEMS){				
				dir_p->dir_nr_of_items++;
				continue;
			}
			
			
			// File index inside directory
			dir_p->dir_active_item++;
			
			
			// Find file/folder
			j = 0;
			
			if(IsSeparator(*path)){
				path++;
				dir_p->ptr_path_buff++; // skip the first slash
				
				// Root directory
				if(*path == 0){
					dir_p->dir_start_cluster = fat->RootFirstCluster;
					return FR_OK;
				}
			}
			
			// Load the remaining path to search
			path = dir_p->ptr_path_buff;
			
			// Case insensitive search
			while(*path && FAT_filename[j]){
				// Convert the path to lowercase
				path_buff = IsUpper(*path) ? *path + 32 : *path;
				
				// Convert the filename to lowercase
				name_buff = IsUpper(FAT_filename[j]) ? FAT_filename[j] + 32 : FAT_filename[j];
				
				if(path_buff == name_buff){
					path_match = true;
				}else{
					path_match = false;
					break;
				}
				
				j++;
				path++;
				
				// Go to next folder path
				if(IsSeparator(*path)){
					is_separator = true;
					path++;
					break;
				}
			}
			
			has_long_name = false;
			
			// If there are characters left to be compared in the filename or path
			// but the string to check is over and all characters until here have been matched
			if(((FAT_filename[j]) || (*path)) && is_separator == false && path_match == true) path_match = false;
			is_separator = false;
			
			
			if(path_match){
				// Save the first cluster of the file but not when checking the SFN numeric-tail
				if(task != FAT_TASK_SEARCH_SFN && task != FAT_TASK_FIND_FILE){
					buf.Int[0] = SD_Buffer[FAT_DIR_FIRST_CLUS_LOW + (e * 32)];
					buf.Int[1] = SD_Buffer[FAT_DIR_FIRST_CLUS_LOW + (e * 32) + 1];
					#if FAT_SUPPORT_FAT32 == 1
						if(fat->fs_type == FS_FAT32){
							buf.Int[2] = SD_Buffer[FAT_DIR_FIRST_CLUS_HIGH + (e * 32)];
							buf.Int[3] = SD_Buffer[FAT_DIR_FIRST_CLUS_HIGH + (e * 32) + 1];
						}
					#endif
					dir_p->dir_start_cluster = buf.Long;
				}
				
				// If this is the last dir in path means the file was found
				if(*path == 0){
					if(task == FAT_TASK_OPEN_DIR && !(file_attrib & FAT_FILE_ATTR_DIRECTORY)){
						return FR_NOT_A_DIRECTORY;
					}
					
					return FR_OK;	
				}
				
				// Skip the last matched directories in path
				// Also provides the extracted filename from the path to the functions that needs it
				dir_p->ptr_path_buff = path;
				
				// Go to this directory sector on the next loop
				dir_p->dir_active_cluster = dir_p->dir_start_cluster;
				dir_p->dir_start_sector = _FAT_clusterToSector(dir_p->dir_active_cluster);
				dir_p->dir_active_sector = 0;
				break;
			}
			
		} // end of entry loop
	} // end of sectors loop
	
	return FR_NOT_FOUND;
}




/*______________________________________________________________________________________________
	Private: Used to create SFN (Short File Name) entry. It formats the filename and adds
	a numeric tail if necessary. It sets needs_lfn true or false if a LFN in necessary.
_______________________________________________________________________________________________*/
static FRESULT _FAT_createSFN(DIR* dir_p, const char* source, char dest[], bool* needs_lfn){
	uint8_t j = 0, i;
	uint8_t dots = 0;
	uint8_t tail_length = 1;
	uint8_t name_length = 0;
	bool needs_tail = false;
	char cbuf;
	const char* extension_start_p = 0;
	const char* path_start_p = source;
	FRESULT res = FR_OK;
	
	
	// Count the filename length and get the first 8 valid characters
	while(*source && name_length < 255){
		if(*source == '.'){
			extension_start_p = source; // mark the start of the extension, last one will be kept
			dots++;
		}
		
		if(j < 8){
			// The following filter is not implemented "*+,./:;<=>?[\]|
			if(!((*source == ' ') || (*source == '/') || (*source == '\\') ||  (*source == '.') || (*source < 0x20))){
				// Convert to uppercase
				if(IsLower(*source)){
					cbuf = *source - 32;
				}else{
					cbuf = *source;
					
					// If there are characters not allowed in SFN but allowed in LFN or there are
					// uppercase characters then a LFN (Long File Name) must be created
					if(IsUpper(*source)) *needs_lfn = true; // has uppercase
				}
				
				dest[j] = cbuf;
				j++;
			}else{
				// Has special characters. Ignore the dot now since it could indicate extension.
				if(*source != '.') *needs_lfn = true;
			}
		}
		
		name_length++;
		source++;
	}
	
	dir_p->filename_length = name_length;
	
	source = extension_start_p; // start from the extension if any
	if(extension_start_p && ((extension_start_p - path_start_p) < 8)) j = extension_start_p - path_start_p;
	if(dots > 1) *needs_lfn = true;
	if(dots < 2 && j == 8) needs_tail = *needs_lfn = true;
	
	// Fill the array with spaces
	while(j < 11){
		dest[j] = ' ';
		j++;
	}
	
	// Get extension
	j = 0;
	while(*source && dots){
		source++; // skip the dot first
		
		if(j < 3){
			if(*source == ' '){
				*needs_lfn = true; // extension includes space
				continue;
			}
			
			// Convert to uppercase
			cbuf = IsLower(*source) ? *source - 32 : *source;
			dest[8 + j] = cbuf;
		}
		
		j++;
	}
	
	// Extension in greater than 3 chars
	if(j > 4) *needs_lfn = true;
	
	// The short name is only the basis-name without the numeric tail
	if(needs_tail == false){
		return _FAT_followPath(dir_p, dest, FAT_TASK_SEARCH_SFN);
	}
	
	// Reuse variable to mark the start index for numeric-tail
	j = 6;
	
	// Reusing variables
	//dots = numeric tail
	
	// Numeric-tail generation
	for(i = 0; i < 255; i++){
		dots = i;
		
		if(i == 10){
			j--; // make room for 2 digit tail
			tail_length = 2;
		}else if(i == 100){
			j--; // make room for 3 digit tail
			tail_length = 3;
		}
		
		//if(j < 1) break;
		dest[j] = '~';
		
		while(tail_length){
			dest[j + tail_length] = (dots % 10) + '0';
			tail_length--;
			dots /= 10;
		}
		
		tail_length = 7 - j; // restore numeric tail length
		// Set starting window at the beginning of the directory
		_FAT_moveWindow(dir_p, dir_p->dir_start_cluster);
		res = _FAT_followPath(dir_p, dest, FAT_TASK_SEARCH_SFN);
		if(res == FR_NOT_FOUND) break; // if no name collision
	}
	
	if((i == 255) || (j == 0)) return FR_DENIED; // too many attempts to create a file without name collision
	return res;
}



/*______________________________________________________________________________________________
	Private: Read the next sector in the main buffer array starting from dir_p->dir_start_sector.
	Then it gets the next cluster (if any) using dir_p->dir_active_cluster.
	dir_p->dir_active_sector is incremented each time the function runs and it represents
	the active sector incremented from the dir_p->dir_start_sector.

	Before running the function the following must be set accordingly:
		dir_p->dir_start_cluster	- the starting cluster of a file
		dir_p->dir_active_cluster	- can be the starting cluster of a file
		dir_p->dir_start_sector		- the starting cluster converted to a sector
		dir_p->dir_active_sector	- to start from the beginning of a file set this to 0
_______________________________________________________________________________________________*/
static FRESULT _FAT_getFileNextSector(DIR* dir_p){
	bool root = false;
	uint8_t response_code;
	uint16_t sectors_per_dir;
	
	// Determine if dir is root
	if(dir_p->dir_start_cluster < 2){
		sectors_per_dir = fat->RootDirSectors; // root
		root = true;
	}else{
		sectors_per_dir = fat->BPB_SecPerClus;
	}
		
	// If this is the last sector in the cluster
	// check the FAT table for the next cluster. If root return EOC (End Of Chain).
	if(dir_p->dir_active_sector >= sectors_per_dir){
		if(root) return FR_EOF; // FAT16 only. FAT32 can have many clusters.
		dir_p->dir_active_cluster = _FAT_tableReadSet(dir_p->dir_active_cluster, 0, FAT_TASK_TABLE_GET_NEXT);
		if(dir_p->dir_active_cluster == fat->EOC) return FR_EOF;
		
		dir_p->dir_start_sector = _FAT_clusterToSector(dir_p->dir_active_cluster);
		dir_p->dir_active_sector = 0;
	}
	
	response_code = sd_read_single_block(dir_p->dir_start_sector + dir_p->dir_active_sector, SD_Buffer);
	if(response_code) return FR_DEVICE_ERR;
		
	dir_p->dir_active_sector++;
	return FR_OK;
}


/*______________________________________________________________________________________________
	Calculate and return the remainder using the formula:
		Remainder = Dividend - (Divisor * Quotient)
				
	Example 1: What is the remainder when 3723 is divided by 23?
	Given: Dividend = 3723 and Divisor = 23
	Quotient = Dividend / Divisor
	Remainder = 20
_______________________________________________________________________________________________*/
static uint16_t MATH_remainder(uint16_t divident, uint16_t divisor, uint16_t quotient){
	return divident - (divisor * quotient);
}



/*______________________________________________________________________________________________
	Private: Convert a cluster to sector number
_______________________________________________________________________________________________*/
static SECTSIZE_t _FAT_clusterToSector(CLSTSIZE_t cluster){
	return ((cluster - 2) * (SECTSIZE_t)fat->BPB_SecPerClus) + fat->FirstDataSector + fat->fs_partition_offset;
}



/*______________________________________________________________________________________________
	Private: Read the FAT table and get the next cluster using current one. Depending on task
	it can also set a cluster to the specified value.
_______________________________________________________________________________________________*/
static CLSTSIZE_t _FAT_tableReadSet(CLSTSIZE_t cluster, CLSTSIZE_t new_value, uint8_t task){
	// Offset inside the FAT table
	// Take the cluster number and double it. That's the same as bit-shifting the entire value one place to the left.
	uint32_t FATOffset = cluster << fat->fs_type; // 1 or 2; for FAT16 or FAT32 because cluster * 2 is cluster << 1
	// ThisFATSecNum
	uint16_t ThisFATSecNum = FATOffset / fat->BPB_BytsPerSec;
	// ThisFATEntOffset
	uint16_t ThisFATEntOffset = FATOffset - (ThisFATSecNum * fat->BPB_BytsPerSec);
	
	// Read FAT and get the next cluster
	uint8_t return_code = sd_read_single_block(fat->Fat1StartSector + ThisFATSecNum, SD_Buffer);
	if(return_code) return 0;
	
	// Read cluster value
	if((task == FAT_TASK_TABLE_READ_SET) || (task == FAT_TASK_TABLE_GET_NEXT)){
		cluster = SD_Buffer[ThisFATEntOffset];
		cluster |= SD_Buffer[ThisFATEntOffset + 1] << 8;
		
		#if FAT_SUPPORT_FAT32 == 1
			if(fat->fs_type == FS_FAT32){
				cluster |= (CLSTSIZE_t) SD_Buffer[ThisFATEntOffset + 2] << 16;
				cluster |= (CLSTSIZE_t) SD_Buffer[ThisFATEntOffset + 3] << 24;
				cluster = cluster & 0x0FFFFFFF;
			}
		#endif
	}
	
	// Set cluster to the given value
	if((task == FAT_TASK_TABLE_READ_SET) || (task == FAT_TASK_TABLE_SET)){
		#if FAT_SUPPORT_FAT32 == 1
			// First read existing value in order to preserve the first 4 MSB
			if(fat->fs_type == FS_FAT32){
				CLSTSIZE_t clst_buf = SD_Buffer[ThisFATEntOffset];
				clst_buf |= SD_Buffer[ThisFATEntOffset + 1] << 8;
				clst_buf |= (CLSTSIZE_t) SD_Buffer[ThisFATEntOffset + 2] << 16;
				clst_buf |= (CLSTSIZE_t) SD_Buffer[ThisFATEntOffset + 3] << 24;
				clst_buf &= 0xF0000000;
				new_value |= clst_buf; // add the 4 MSB to the existing value
			}
		#endif
		
		// Write new cluster value to main buffer
		SD_Buffer[ThisFATEntOffset] = new_value;
		SD_Buffer[ThisFATEntOffset + 1] = new_value >> 8;
		
		#if FAT_SUPPORT_FAT32 == 1
			if(fat->fs_type == FS_FAT32){
				SD_Buffer[ThisFATEntOffset + 2] = new_value >> 16;
				SD_Buffer[ThisFATEntOffset + 3] = new_value >> 24;
			}
		#endif
		
		sd_write_single_block(fat->Fat1StartSector + ThisFATSecNum, SD_Buffer);
		if(SD_ResponseToken != 0x05) return 0;
	}
	
	return cluster;
}



/*______________________________________________________________________________________________
	Private: Extract LFN
_______________________________________________________________________________________________*/
static void _FAT_parse_long_names(char buff[], uint16_t sector_offset){
	// The long names are placed starting from the tail.
	// Calculate the array offset position where to place the fragment
	// One long name entry can hold 13 Unicode characters (16-bit)
	uint8_t long_dir_order = SD_Buffer[FAT_LONG_DIR_ORDER + (sector_offset * 32)];
	
	long_dir_order = long_dir_order & (~FAT_LAST_LONG_ENTRY_MASK);
	uint8_t array_offset = (long_dir_order - 1) * FAT_LFN_MAX_CHARS;
	
	array_offset = _FAT_parse_long_names_(buff, FAT_LONG_DIR_NAME, sector_offset, array_offset, 10);
	array_offset = _FAT_parse_long_names_(buff, FAT_LONG_DIR_NAME2, sector_offset, array_offset, 12);
	array_offset = _FAT_parse_long_names_(buff, FAT_LONG_DIR_NAME3, sector_offset, array_offset, 4);
	
	if(set_null){
		set_null = false;
		null_is_set = true;
		buff[array_offset] = 0;
	}
	
	if(long_dir_order == 1)	null_is_set = false;
}


static uint8_t _FAT_parse_long_names_(char buff[], uint8_t start_idx, uint16_t sector_offset, uint8_t buff_idx, uint8_t len){
	uint8_t character;
	
	for(uint8_t i = 0; i < len; i += 2){
		character = SD_Buffer[start_idx + i + (sector_offset * 32)];

		if(character){
			if((character == 0xFF) || (buff_idx > FAT_MAX_FILENAME_LENGTH - 1)) break;			
			buff[buff_idx] = character;
			if(null_is_set == false) set_null = true; // set null only on greatest ordinal
			buff_idx++;
		}
	}
	
	return buff_idx;
}



/*
void FAT16_getDirectoryName(char buff[], uint8_t length){
	// Save current folder
	dir->dir_start_cluster_buff = dir->dir_start_cluster;
	uint16_t current_cluster = dir->dir_start_cluster;
	uint32_t dir_current_sector = dir->dir_start_sector;
	uint16_t dir_sectors = dir->dir_sectors;
	
	// Go to parent folder
	uint8_t return_code = FAT16_dirBack();
	if(return_code == FR_ROOT_DIR){
		if(length > 1){
			buff[0] = '/';
			buff[1] = 0;
		}
		
		return;
	}
	
	// Search for the folder with the specified cluster (dir_start_cluster_buff) and get the folder index
	FAT16_parseDir("", 0, TASK_OPEN_DIR_BY_CLUSTER);
	
	// Save dir name
	uint8_t ix = 0;
	
	// Subtract 1 from length for the null in case user forgets to account for it
	while(file_name[ix] && ix < length - 1){
		buff[ix] = file_name[ix];
		ix++;
	}
	
	 // Add null
	buff[ix] = 0;
	
	// Restore folder
	dir->dir_start_cluster = current_cluster;
	dir->dir_start_sector = dir_current_sector;
	dir->dir_sectors = dir_sectors;
	dir->dir_nr_of_folders = 0;
	dir->dir_nr_of_files = 0;
	
	// Go back to the initial folder
	FAT16_parseDir("", dir->dir_idx, TASK_OPEN_DIR_BY_INDEX);
}*/

#endif /* FAT_H_ */