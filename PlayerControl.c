/*
 * PlayerControl.c
 *
 * Copyright (c) 2018 Yulay Rakhmangulov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include "PlayerControl.h"
#include "MpUart.h"
#include "dfplay.h"
#include "Storage.h"
#include "Timeout.h"

static void SendCmd(const uint8_t* cmd, uint8_t size);
static void HandleMessage();
static void Reset();
static void SaveState();
static void SelectActiveSource();
static void RequestTotalFiles();

//TODO set watchdog timer to get initialized
// send reset command if > 5 sec

static void UpdateFolder()
{
//	gPlay.folder = (uint8_t)(gPlay.fileNum >> 4) + 1;
	gPlay.folder = (gPlay.fileNum / gPlay.folderSize) + 1;
}

struct PlayerState gPlay =
{
	.device     = UsbDrive,
	.deviceMask = UsbDrive,
	.loopMode   = All,
	.folder	    = 1,
	.folderSize = FolderSize,
	.maxFolders = MaxFolderNum,
	.fileNum    = 1,
	.totalFiles = MaxFileNum
};


enum ePlayerState
{
	WaitForMessage,
	ReadingVer,
	ReadingLen,
	ReadingCmd,
	ReadingAck,
	ReadingParH,
	ReadingParL,
	ReadingCrcH,
	ReadingCrcL,
	WaitForETX
};


bool requestFlag = false;
bool playFlag = false;
bool totalFilesFlag = false;
bool initUsbFlag = false;

static uint8_t lastCommand = 0;

static uint8_t state = WaitForMessage;
static uint16_t crc = 0;
static uint8_t rcvBuff[6];
static uint8_t idx = 0;

static uint8_t requested = 0;

static void Reset()
{
	state = WaitForMessage;
	crc = 0;
	idx = 0;
}

void Player_Update()
{
	if(Mp_RxReady())
	{
		uint8_t b = Mp_GetByte();

		switch(state)
		{
		case WaitForMessage:
			if(0x7e == b)
			{
				++state;
				crc = 0;
			}
			break;
		case ReadingVer:
			if(0xff == b)
			{
				++state;
				crc += b;
			}
			else
			{
				Reset();
			}
			break;
		case ReadingLen:
			if(0x06 == b)
			{
				++state;
				crc += b;
				idx = 0;
			}
			else
			{
				Reset();
			}
			break;
		case ReadingCmd:
		case ReadingAck:
		case ReadingParH:
		case ReadingParL:
			++state;
			rcvBuff[idx] = b;
			++idx;
			crc += b;
			break;
		case ReadingCrcH:
		{
			crc = ~crc;
			++crc;

			uint8_t* p = (uint8_t*)&crc;

			if(b == p[1])
			{
				++state;
			}
			else
			{
				Reset();
			}
			break;
		}
		case ReadingCrcL:
		{
			uint8_t* p = (uint8_t*)&crc;

			if(b == p[0])
			{
				++state;
			}
			else
			{
				Reset();
			}
			break;
		}
		case WaitForETX:
			if(0xef == b)
			{
				HandleMessage();
			}

			Reset();
			break;
		default:
			Reset();
			break;
		}
	}
	else if(200 == gDelayFlag)
	{
		if(initUsbFlag)
		{
			initUsbFlag = false;
			Player_InitializeUsb();
		}
		else if(totalFilesFlag)
		{
			totalFilesFlag = false;
			RequestTotalFiles();
		}
		else if(requestFlag)
		{
			requestFlag = false;
			RequestCurrTrack();
		}
		else if(playFlag)
		{
			playFlag = false;
			Player_PlayNext();
		}

		if(initUsbFlag || totalFilesFlag || requestFlag || playFlag)
		{
			gDelayFlag = (gTime + 10) % 100 + 1;
		}
		else
		{
			gDelayFlag = 201;
		}
	}
}


void Player_LoadState()
{
	switch(gPlay.device)
	{
	case UsbDrive:
		LoadUsbState();
		break;
	case SdCard:
		LoadSdState();
		break;
	default:
		break;
	}
}

void SaveState()
{
	switch(gPlay.device)
	{
	case UsbDrive:
		StoreUsbState();
		break;
	case SdCard:
		StoreSdState();
		break;
	default:
		break;
	}
}

void SelectActiveSource()
{
	if(gPlay.deviceMask & UsbDrive)
	{
		gPlay.device = UsbDrive;
	}
	else if(gPlay.deviceMask & SdCard)
	{
		gPlay.device = SdCard;
	}

	Player_LoadState();
}

/*inline*/ void SchedulePlay()
{
	playFlag = true;
	gDelayFlag = (gTime + 10) % 100 + 1;
}

/*inline*/ void ScheduleRequestCurrTrack()
{
	requestFlag = true;
	gDelayFlag = (gTime + 20) % 100 + 1;
}

/*inline*/ void ScheduleTotalFiles()
{
	totalFilesFlag = true;
	gDelayFlag = (gTime + 80) % 200 + 1;
}

//inline void ScheduleInitUsb()
//{
//	initUsbFlag = true;
//	gDelayFlag = (gTime + 50) % 100 + 1;
//}

static void HandleMessage()
{
	switch(rcvBuff[0])
	{
	case eDeviceInserted:
		gPlay.device = rcvBuff[3]; //select it as active
		gPlay.deviceMask |= gPlay.device;

		Player_LoadState();

		ScheduleTotalFiles();
		break;

	case eDeviceRemoved:
		gPlay.deviceMask &= ~(rcvBuff[3]);

		if(0 == (gPlay.deviceMask & gPlay.device))
		{
			SelectActiveSource();
		}
		break;

	case eUsbPlayFinished:
		gPlay.device = UsbDrive;

		SchedulePlay();
		break;

	case eSdPlayFinished:
		gPlay.device = SdCard;

		SchedulePlay();
		break;

	case eInitialize:
		if(rcvBuff[3])
		{
			gPlay.deviceMask = rcvBuff[3];

			SelectActiveSource();

			ScheduleTotalFiles();
		}
		break;

	case eUSBCurrFile:
	case eSDCurrFile:
		if(rcvBuff[2] || rcvBuff[3]) //if file num > 0
		{
			gPlay.file[0] = rcvBuff[3];
			gPlay.file[1] = rcvBuff[2];

			UpdateFolder();

			if(requested) //this gets requested after play cmd
			{
                requested = 0;
			    SaveState();
			}
			else //track ended notification for mh2024
			{
		        SchedulePlay();
			}
		}
		else
		{
			ScheduleRequestCurrTrack();
		}
		break;

	case eError:
		break;

	case eFeedback:
	{
		switch(lastCommand)
		{
		case eStatus:
			gPlay.device = rcvBuff[2];

			if(rcvBuff[3] == 0) //not playing
			{
				SchedulePlay();
			}
			break;

		case ePlayNext:
		case ePlayPrev:
		case ePlayTrackNo:
		case ePlayback:
		case eInitialize:
			ScheduleRequestCurrTrack();
			break;

		default:
			break;
		}

		break;
	}

	case eUSBTotalFiles:
	case eSDTotalFiles:
		if(rcvBuff[2] && rcvBuff[3])
		{
			gPlay.total[0] = rcvBuff[3];
			gPlay.total[1] = rcvBuff[2];

			if(gPlay.totalFiles > (FolderSize * MaxFolderNum))
			{
				gPlay.folderSize = FolderSizeMax;
			}
			else
			{
				gPlay.folderSize = FolderSize;
			}

			gPlay.maxFolders = (gPlay.totalFiles / gPlay.folderSize) + 1;

			SaveState();
		}
		else
		{
			ScheduleTotalFiles();
		}
		break;

	default:
		break;
	}
}


inline static void SendSTX() 		{ Mp_PutByte(0x7e); }
inline static void SendVersion() 	{ Mp_PutByte(0xff); }
inline static void SendETX() 		{ Mp_PutByte(0xef); }

void SendCmd(const uint8_t* cmd, uint8_t size)
{
    SendSTX();
    SendVersion();

    uint16_t crc = 0xff; //start with packet version

    for(uint8_t i=0; i<size; ++i)
    {
        Mp_PutByte(cmd[i]);
        crc += cmd[i];
    }

    crc = ~crc;
    ++crc;

    uint8_t* p = (uint8_t*)&crc;

    Mp_PutByte(p[1]);
    Mp_PutByte(p[0]);

    SendETX();

    if(cmd[1]) //feedback requested
    {
        lastCommand = cmd[1];
    }
}

// fileNum is global file number on media (usb, microSD) 1...2999
// represented as:
// 		folder - 01 to 99
// 		file   - 001 to 256

//static const uint8_t reset[]	    = { 6, eReset,			1, 0, 0 };

//Playback control
static const uint8_t playNext[] 	= { 6, ePlayNext, 		1, 0, 0 };
static const uint8_t playPrev[] 	= { 6, ePlayPrev, 		1, 0, 0 };

static uint8_t setTrack[] 	        = { 6, ePlayTrackNo, 	1, 0, 0 };

static uint8_t setVolume[]          = { 6, eSetVolume,      1, 0, 28 };
static uint8_t setEqualizer[]       = { 6, eSetEq,          1, 0, 0 };

static const uint8_t play[] 		= { 6, ePlayback, 		1, 0, 0 };
static const uint8_t pause[] 		= { 6, ePause, 			1, 0, 0 };

//static const uint8_t selectUsb[] 	= { 6, eSelectSource, 	1, 0, 0 };
//static const uint8_t selectSd[] 	= { 6, eSelectSource, 	1, 0, 1 };

//static uint8_t setFileFolder[] 			= { 6, eSetFileFolder, 	1, 0, 0 };

//Playback mode
static const uint8_t disableLoop[] 	= { 6, eLoopMode, 		1, 0, 1 };
static const uint8_t enableLoop[] 	= { 6, eLoopMode, 		1, 0, 0 };
static const uint8_t enableRandom[]   = { 6, eRandomPlay, 	1, 0, 0 };

//static uint8_t loopFolder[] 		= { 6, eLoopFolder, 	1, 0, 0 };

static const uint8_t loopFile[] 	= { 6, eLoopFile, 		1, 0, 0 };

static uint8_t queryInitialize[] 	= { 6, eInitialize, 	1, 0, 1 };

static const uint8_t queryPlayState[] 		= { 6, eStatus, 1, 0, 0 };

static const uint8_t queryTotalUsbFiles[] 	= { 6, eUSBTotalFiles, 1, 0, 0 };
static const uint8_t queryTotalSdFiles[] 	= { 6, eSDTotalFiles,  1, 0, 0 };

static const uint8_t queryCurrUsbFile[] = { 6, eUSBCurrFile, 1, 0, 0 };
static const uint8_t queryCurrSdFile[] 	= { 6, eSDCurrFile,	 1, 0, 0 };


void Player_Initialize()
{
    SendCmd(setVolume, sizeof(setVolume));
    SendCmd(setEqualizer, sizeof(setEqualizer));

    Player_PlayNext();
}

void Player_InitializeUsb()
{
	gPlay.device = 1;
	queryInitialize[4] = 1;
	SendCmd(queryInitialize, sizeof(queryInitialize));
}

void Player_InitializeSd()
{
	gPlay.device = 2;
	queryInitialize[4] = 2;
	SendCmd(queryInitialize, sizeof(queryInitialize));
}

void Player_Play()
{
	SendCmd(play, sizeof(play));
}

void Player_Pause()
{
	SendCmd(pause, sizeof(pause));
}

void Player_PlayNext()
{
	uint16_t n = gPlay.fileNum - 1;

	switch(gPlay.loopMode)
	{
	case None:
	case One:
	case All:
		++n;
		break;

	case RandomAll:
		n += Randomizator; //odd number

		if(gPlay.total[0] & 1)
		{
			--n; //if total number is odd, make jump even
		}
		break;

	default:
		break;
	}

	gPlay.fileNum = (n % gPlay.totalFiles) + 1;

	UpdateFolder();

	SendCmd(playNext, sizeof(playNext));
}

void Player_PlayPrev() //in current folder
{
	uint16_t n = gPlay.fileNum - 1;

	switch(gPlay.loopMode)
	{
	case None:
	case One:
	case All:
		--n;
		break;

	case RandomAll:
		n -= Randomizator; //odd number

		if(gPlay.total[0] & 1) //if total number is odd, make jump even
		{
			++n;
		}
		break;

	default:
		break;
	}

	gPlay.fileNum = (n % gPlay.totalFiles) + 1;

	UpdateFolder();

	SendCmd(playPrev, sizeof(playPrev));
}

void Player_PlayTrack()
{
	setTrack[3] = gPlay.file[1];
	setTrack[4] = gPlay.file[0];
	SendCmd(setTrack, sizeof(setTrack));
}

void CalcFileNum()
{
	//16 files per folder
	gPlay.fileNum = (gPlay.folder - 1) * gPlay.folderSize + 1;

	if(gPlay.totalFiles < gPlay.fileNum) //1...2999
	{
		gPlay.fileNum = 1;
		gPlay.folder = 1;
	}
}

void Player_NextFolder()
{
	++gPlay.folder;

	if(gPlay.folder > gPlay.maxFolders) //1...99
	{
		gPlay.folder = 1;
	}

	CalcFileNum();

	Player_PlayTrack();
}

void Player_PrevFolder()
{
	--gPlay.folder;

	if(0 == gPlay.folder || gPlay.folder > gPlay.maxFolders)
	{
		gPlay.folder = gPlay.maxFolders; //1...99
	}

	CalcFileNum();

	Player_PlayTrack();
}

void PlayNormal()
{
	SendCmd(disableLoop, sizeof(disableLoop));
}

void PlayLoopAll()
{
	SendCmd(enableLoop, sizeof(enableLoop));
}

void PlayLoopOne()
{
	SendCmd(loopFile, sizeof(loopFile));
}

void PlayAllRandom()
{
	SendCmd(enableRandom, sizeof(enableRandom));
}

void RequestCurrDevice()
{
	SendCmd(queryPlayState, sizeof(queryPlayState));
}

void RequestCurrTrack()
{
	switch(gPlay.device)
	{
	case UsbDrive:
        requested = 1;
		SendCmd(queryCurrUsbFile, sizeof(queryCurrSdFile));
		break;
	case SdCard:
	    requested = 1;
		SendCmd(queryCurrSdFile, sizeof(queryCurrSdFile));
		break;
	default:
		break;
	}
}

void RequestTotalFiles()
{
	switch(gPlay.device)
	{
	case UsbDrive:
		SendCmd(queryTotalUsbFiles, sizeof(queryTotalUsbFiles));
		break;
	case SdCard:
		SendCmd(queryTotalSdFiles, sizeof(queryTotalSdFiles));
		break;
	default:
		break;
	}
}
