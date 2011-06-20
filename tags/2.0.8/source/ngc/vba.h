/****************************************************************************
 * Visual Boy Advance GX
 *
 * Tantric September 2008
 *
 * vba.h
 *
 * This file controls overall program flow. Most things start and end here!
 ***************************************************************************/
#ifndef _VBAGX_H_
#define _VBAGX_H_

#include <unistd.h>

#include "FreeTypeGX.h"
#include "filelist.h"

#define APPNAME 		"Visual Boy Advance GX"
#define APPVERSION 		"2.0.8"
#define APPFOLDER 		"vbagx"
#define PREF_FILE_NAME 	"settings.xml"
#define PAL_FILE_NAME 	"palettes.xml"

#define NOTSILENT 0
#define SILENT 1

const char pathPrefix[9][8] =
{ "", "sd:/", "usb:/", "dvd:/", "smb:/", "mca:/", "mcb:/", "carda:/", "cardb:/" };

enum {
	DEVICE_AUTO,
	DEVICE_SD,
	DEVICE_USB,
	DEVICE_DVD,
	DEVICE_SMB,
	DEVICE_MC_SLOTA,
	DEVICE_MC_SLOTB,
	DEVICE_SD_SLOTA,
	DEVICE_SD_SLOTB
};

enum {
	FILE_SRAM,
	FILE_SNAPSHOT,
	FILE_ROM
};

struct SGCSettings{
    int		AutoLoad;
    int		AutoSave;
    int		LoadMethod; // For ROMS: Auto, SD, DVD, USB, Network (SMB)
	int		SaveMethod; // For SRAM, Freeze, Prefs: Auto, SD, Memory Card Slot A, Memory Card Slot B, USB, SMB
	char	LoadFolder[MAXPATHLEN]; // Path to game files
	char	SaveFolder[MAXPATHLEN]; // Path to save files
	char	CheatFolder[MAXPATHLEN]; // Path to cheat files
	int		VerifySaves;

	char	smbip[16];
	char	smbuser[20];
	char	smbpwd[20];
	char	smbshare[20];

    float	ZoomLevel; // zoom amount
    int		videomode; // 0 - automatic, 1 - NTSC (480i), 2 - Progressive (480p), 3 - PAL (50Hz), 4 - PAL (60Hz)
    int		scaling; // 0 - default, 1 - partial stretch, 2 - stretch to fit, 3 - widescreen correction
	int		render;		// 0 - original, 1 - filtered, 2 - unfiltered
	int		xshift;		// video output shift
	int		yshift;
	int     colorize; // colorize Mono Gameboy games

	int		WiiControls; // Match Wii Game
	int		WiimoteOrientation;
	int		ExitAction;
	int		MusicVolume;
	int		SFXVolume;
	int		Rumble;
};

void ExitApp();
void ShutdownWii();
extern struct SGCSettings GCSettings;
extern int ScreenshotRequested;
extern int ConfigRequested;
extern int ShutdownRequested;
extern int ExitRequested;
extern char appPath[];
extern FreeTypeGX *fontSystem[];

#endif