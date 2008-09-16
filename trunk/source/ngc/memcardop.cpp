/****************************************************************************
 * Snes9x 1.51 Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * crunchy2 May-June 2007
 * Tantric September 2008
 *
 * memcardop.cpp
 *
 * Memory Card routines
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vba.h"
#include "video.h"
#include "menudraw.h"
#include "menu.h"
#include "preferences.h"
#include "memcardop.h"
#include "fileop.h"

#define VERIFBUFFERSIZE 65536
static u8 SysArea[CARD_WORKAREA] ATTRIBUTE_ALIGN (32);
extern unsigned char savebuffer[];
unsigned char verifbuffer[VERIFBUFFERSIZE] ATTRIBUTE_ALIGN (32);
card_dir CardDir;
card_file CardFile;
card_stat CardStatus;

/****************************************************************************
 * CardFileExists
 *
 * Wrapper to search through the files on the card.
 * Returns TRUE if found.
 ***************************************************************************/
int
CardFileExists (char *filename, int slot)
{
	int CardError;

	CardError = CARD_FindFirst (slot, &CardDir, TRUE);
	while (CardError != CARD_ERROR_NOFILE)
	{
		CardError = CARD_FindNext (&CardDir);

		if (strcmp ((char *) CardDir.filename, filename) == 0)
			return 1;
	}

	return 0;
}

/****************************************************************************
 * TestCard
 *
 * Checks to see if a card is in the card slot specified
 ***************************************************************************/
bool TestCard(int slot, bool silent)
{
	// Memory Cards do not work in Wii mode - disable
	#ifdef HW_RVL
	return false;
	#endif

	/*** Initialize Card System ***/
	memset (SysArea, 0, CARD_WORKAREA);
	CARD_Init ("SNES", "00");

	/*** Try to mount the card ***/
	if (MountCard(slot, silent) == 0)
	{
		// Mount successful!
		if(!silent)
		{
			if (slot == CARD_SLOTA)
				WaitPrompt((char*) "Mounted Slot A Memory Card!");
			else
				WaitPrompt((char*) "Mounted Slot B Memory Card!");
		}
		CARD_Unmount (slot);
		return true;
	}
	else
	{
		if(!silent)
		{
			if (slot == CARD_SLOTA)
				WaitPrompt((char*) "Unable to Mount Slot A Memory Card!");
			else
				WaitPrompt((char*) "Unable to Mount Slot B Memory Card!");
		}

		return false;
	}
}

/****************************************************************************
 * MountCard
 *
 * Mounts the memory card in the given slot.
 * Returns the result of the last attempted CARD_Mount command.
 ***************************************************************************/
int MountCard(int cslot, bool silent)
{
	int ret = -1;
	int tries = 0;

	// Mount the card
	while ( tries < 10 && ret != 0)
	{
		EXI_ProbeReset ();
		ret = CARD_Mount (cslot, SysArea, NULL);
		VIDEO_WaitVSync ();
		tries++;
	}
	return ret;
}


/****************************************************************************
 * Verify Memory Card file against buffer
 ***************************************************************************/
int
VerifyMCFile (unsigned char *buf, int slot, char *filename, int datasize)
{
	int CardError;
	unsigned int blocks;
	unsigned int SectorSize;
	char msg[80];
    int bytesleft = 0;
    int bytesread = 0;

	/*** Initialize Card System ***/
	memset (SysArea, 0, CARD_WORKAREA);
	CARD_Init ("SNES", "00");

	/*** Try to mount the card ***/
	CardError = MountCard(slot, NOTSILENT);

	if (CardError == 0)
	{
		/*** Get Sector Size ***/
		CARD_GetSectorSize (slot, &SectorSize);

		if (!CardFileExists (filename, slot))
		{
            CARD_Unmount (slot);
		    WaitPrompt((char*) "Unable to open file for verify!");
			return 0;
		}

		memset (&CardFile, 0, sizeof (CardFile));
		CardError = CARD_Open (slot, filename, &CardFile);

		blocks = CardFile.len;

		if (blocks < SectorSize)
			blocks = SectorSize;

		if (blocks % SectorSize)
			blocks += SectorSize;

        if (blocks > (unsigned int)datasize)
            blocks = datasize;

		memset (verifbuffer, 0, VERIFBUFFERSIZE);
		bytesleft = blocks;
		bytesread = 0;
		while (bytesleft > 0)
		{
			CARD_Read (&CardFile, verifbuffer, SectorSize, bytesread);
			if ( memcmp (buf + bytesread, verifbuffer, (unsigned int)bytesleft < SectorSize ? bytesleft : SectorSize) )
            {
                CARD_Close (&CardFile);
                CARD_Unmount (slot);
                WaitPrompt((char*) "File did not verify!");
                return 0;
            }

			bytesleft -= SectorSize;
			bytesread += SectorSize;

            sprintf (msg, "Verified %d of %d bytes", bytesread, blocks);
            ShowProgress (msg, bytesread, blocks);
		}
		CARD_Close (&CardFile);
		CARD_Unmount (slot);

		return 1;
	}
	else
		if (slot == CARD_SLOTA)
			WaitPrompt((char*) "Unable to Mount Slot A Memory Card!");
		else
			WaitPrompt((char*) "Unable to Mount Slot B Memory Card!");

	return 0;
}

/****************************************************************************
 * Load savebuffer from Memory Card file
 ***************************************************************************/
int
LoadBufferFromMC (unsigned char *buf, int slot, char *filename, bool silent)
{
	int CardError;
	unsigned int blocks;
	unsigned int SectorSize;
    int bytesleft = 0;
    int bytesread = 0;

	/*** Initialize Card System ***/
	memset (SysArea, 0, CARD_WORKAREA);
	CARD_Init ("SNES", "00");

	/*** Try to mount the card ***/
	CardError = MountCard(slot, NOTSILENT);

	if (CardError == 0)
	{
		/*** Get Sector Size ***/
		CARD_GetSectorSize (slot, &SectorSize);

		if (!CardFileExists (filename, slot))
		{
			if (!silent)
				WaitPrompt((char*) "Unable to open file");
			return 0;
		}

		memset (&CardFile, 0, sizeof (CardFile));
		CardError = CARD_Open (slot, filename, &CardFile);

		blocks = CardFile.len;

		if (blocks < SectorSize)
			blocks = SectorSize;

		if (blocks % SectorSize)
			blocks += SectorSize;

		memset (buf, 0, 0x22000);

		bytesleft = blocks;
		bytesread = 0;
		while (bytesleft > 0)
		{
			CARD_Read (&CardFile, buf + bytesread, SectorSize, bytesread);
			bytesleft -= SectorSize;
			bytesread += SectorSize;
		}
		CARD_Close (&CardFile);
		CARD_Unmount (slot);
	}
	else
		if (slot == CARD_SLOTA)
			WaitPrompt((char*) "Unable to Mount Slot A Memory Card!");
		else
			WaitPrompt((char*) "Unable to Mount Slot B Memory Card!");

	return bytesread;
}


/****************************************************************************
 * Write savebuffer to Memory Card file
 ***************************************************************************/
int
SaveBufferToMC (unsigned char *buf, int slot, char *filename, int datasize, bool silent)
{
	int CardError;
	unsigned int blocks;
	unsigned int SectorSize;
	char msg[80];

	/*** Initialize Card System ***/
	memset (SysArea, 0, CARD_WORKAREA);
	CARD_Init ("SNES", "00");

	/*** Try to mount the card ***/
	CardError = MountCard(slot, NOTSILENT);

	if (CardError == 0)
	{
		/*** Get Sector Size ***/
		CARD_GetSectorSize (slot, &SectorSize);

		if (datasize)
		{
			/*** Calculate number of blocks required ***/
			blocks = (datasize / SectorSize) * SectorSize;
			if (datasize % SectorSize)
				blocks += SectorSize;

			/*** Does this file exist ? ***/
			if (CardFileExists (filename, slot))
			{
				/*** Try to open the file ***/
				CardError = CARD_Open (slot, filename, &CardFile);
				if (CardError)
				{
                    CARD_Unmount (slot);
					WaitPrompt((char*) "Unable to open card file!");
					return 0;
				}

				if ( (s32)blocks > CardFile.len )  /*** new data is longer ***/
				{
                    CARD_Close (&CardFile);

                    /*** Try to create temp file to check available space ***/
                    CardError = CARD_Create (slot, "TEMPFILESNES9XGX201", blocks, &CardFile);
                    if (CardError)
                    {
                        CARD_Unmount (slot);
                        WaitPrompt((char*) "Not enough space to update file!");
                        return 0;
                    }

                    /*** Delete the temporary file ***/
                    CARD_Close (&CardFile);
                    CardError = CARD_Delete(slot, "TEMPFILESNES9XGX201");
                    if (CardError)
                    {
                        CARD_Unmount (slot);
                        WaitPrompt((char*) "Unable to delete temporary file!");
                        return 0;
                    }

                    /*** Delete the existing shorter file ***/
                    CardError = CARD_Delete(slot, filename);
                    if (CardError)
                    {
                        CARD_Unmount (slot);
                        WaitPrompt((char*) "Unable to delete existing file!");
                        return 0;
                    }

                    /*** Create new, longer file ***/
                    CardError = CARD_Create (slot, filename, blocks, &CardFile);
                    if (CardError)
                    {
                        CARD_Unmount (slot);
                        WaitPrompt((char*) "Unable to create updated card file!");
                        return 0;
                    }
				}
			}
			else  /*** no file existed, create new one ***/
			{
                /*** Create new file ***/
                CardError = CARD_Create (slot, filename, blocks, &CardFile);
                if (CardError)
                {
                    CARD_Unmount (slot);
                    if ( CardError == CARD_ERROR_INSSPACE )
                        WaitPrompt((char*) "Not enough space to create file!");
                    else
                        WaitPrompt((char*) "Unable to create card file!");
                    return 0;
                }
            }

			/*** Now, have an open file handle, ready to send out the data ***/
			CARD_GetStatus (slot, CardFile.filenum, &CardStatus);
			CardStatus.icon_addr = 0x0;
			CardStatus.icon_fmt = 2;
			CardStatus.icon_speed = 1;
			CardStatus.comment_addr = 2048;
			CARD_SetStatus (slot, CardFile.filenum, &CardStatus);

			int byteswritten = 0;
			int bytesleft = blocks;
			while (bytesleft > 0)
			{
				CardError =
					CARD_Write (&CardFile, buf + byteswritten,
                                SectorSize, byteswritten);
				bytesleft -= SectorSize;
				byteswritten += SectorSize;

                sprintf (msg, "Wrote %d of %d bytes", byteswritten, blocks);
                ShowProgress (msg, byteswritten, blocks);
			}

			CARD_Close (&CardFile);
			CARD_Unmount (slot);

			if ( GCSettings.VerifySaves )
			{
			    /*** Verify the written file, but only up to the length we wrote
			         because the file could be longer due to past writes    ***/
                if ( VerifyMCFile (buf, slot, filename, byteswritten) )
                    return byteswritten;
                else
                    return 0;
            }
            else
                return byteswritten;
		}
		else
			if ( !silent )
				WaitPrompt((char*) "This game does not appear to use SRAM");
	}
	else
		if (slot == CARD_SLOTA)
			WaitPrompt((char*) "Unable to Mount Slot A Memory Card!");
		else
			WaitPrompt((char*) "Unable to Mount Slot B Memory Card!");

	return 0;

}
