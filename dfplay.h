/*
 * dfplay.h
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

enum eCommand
{
    //Commands
  ePlayNext      = 1,
  ePlayPrev      = 2,
  ePlayTrackNo   = 3, // 1...2999 located in the root folder of the device
  eVolumeUp      = 4,
  eVolumeDown    = 5,
  eSetVolume     = 6, // 0...30
  eSetEq         = 7, // 0:normal, 1:pop, 2:rock, 3:jazz, 4:bass
  eRepeatTrackNo = 8, // single repeat playback
  eSelectSource  = 9, // 0:usb, 1:sd

  eEnterStandby  = 0x0a,
  eWakeUp		 = 0x0b, //? says normal working
  eReset         = 0x0c,

  ePlayback      = 0x0d,
  ePause         = 0x0e,

  eSetFileFolder = 0x0f, // folder number 01...99

  eAudioAmp      = 0x10, // DH=1:amp on, DL:set gain 0...31

  eLoopMode      = 0x11, // 1:start playback, 0:stop playback

  ePlayMP3       = 0x12, // play folder "MP3"
  eAnnounce      = 0x13, // interrupt music playback and play announcement

  ePlaybackTrack = 0x14, // track No 1...3000 in folders 01..15

  eStopAnnounce  = 0x15, //Stop announcement and play interrupted music

  eStopPlayback  = 0x16, //Stop playback

  eLoopFolder    = 0x17, //Specify repeat playback of a folder
  eRandomPlay    = 0x18, //Random playback, always starts from file 1
  eLoopFile      = 0x19, //Specify playback of current track

  eDacMute       = 0x1a, //On/Off DAC
  
  //Notifications
  eDeviceInserted  = 0x3a, // param: Usb = 1, Sd = 2
  eDeviceRemoved   = 0x3b, // param: Usb = 1, Sd = 2

  eUsbPlayFinished 	= 0x3c, // param: track #
  eSdPlayFinished  	= 0x3d, // param: track #
  eNandPlayFinished = 0x3e, // NA

  //Queries
  eInitialize    = 0x3f, //parame Usb = 1, Sd = 2, PC = 4, Usb+Sd = 3
  eError         = 0x40,
  eFeedback      = 0x41,
  
  eStatus        = 0x42,
  eVolume        = 0x43,
  eEqualizer     = 0x44,
  
  eUSBTotalFiles = 0x47,
  eSDTotalFiles  = 0x48,
  
  eKeepOn		= 0x4a,

  eUSBCurrFile  = 0x4b,
  eSDCurrFile   = 0x4c,
  
  eFolderTotalFiles = 0x4e,
  eTotalFolders  = 0x4f
};

enum eSource
{
	UsbDrive	= 1,
	SdCard		= 2
};

