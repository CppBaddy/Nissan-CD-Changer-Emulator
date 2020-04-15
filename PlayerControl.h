/*
 * PlayerControl.h
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


#ifndef PLAYERCONTROL_H_
#define PLAYERCONTROL_H_

#include <stdint.h>
#include <stdbool.h>

void Player_LoadState();

void Player_Update();

void Player_InitializeUsb();
void Player_InitializeSd();

void Player_PlayTrack();

void Player_Play();
void Player_Pause();

void Player_PlayNext();
void Player_PlayPrev();

void Player_NextFolder();
void Player_PrevFolder();

void PlayNormal();
void PlayLoopAll();
void PlayLoopOne();
void PlayAllRandom();

void RequestCurrTrack();


enum eMode
{
	All 	= 1,
	None	= 5,
	One		= 9,
	RandomAll = 0x0d
};

enum
{
	Randomizator = 63,  //odd number
	MaxFileNum   = 2999,
	MaxFolderNum = 99,
	FolderSize   = 16,
	FolderSizeMax = 32
};

struct PlayerState
{
	//Non persistent model
	uint8_t device;		// USB or SD or AUX
	uint8_t deviceMask;
	uint8_t loopMode; 		//All, None, One, Random All

	//Persistent model

	//presentation
	uint8_t folder;			//virtual folder #

	uint8_t folderSize;		//files per virtual folder
	uint8_t maxFolders;		//max folders

	union
	{
		uint16_t fileNum;	//global file number on media (USB/SD)
		uint8_t file[2];
	};

	union
	{
		uint16_t totalFiles;
		uint8_t total[2];
	};

	//uint8_t crc8;
};


extern struct PlayerState gPlay;


#endif /* PLAYERCONTROL_H_ */
