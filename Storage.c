/*
 * Storage.c
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

#include "avr/eeprom.h"

#include "PlayerControl.h"


uint8_t EEMEM usbState[sizeof(struct PlayerState)] = {
		1,	//device USB
		1,	//device mask USB
		1,	//loop mode All
		1,	//folder
		16, //folder size
		MaxFolderNum, //max folders
		1, 0, //file number
		0xb7, 0xb //2999 total files
};


uint8_t EEMEM sdState[sizeof(struct PlayerState)] = {
		2,	//device SD
		2,	//device mask SD
		1,	//loop mode All
		1,	//folder
		16, //folder size
		MaxFolderNum, //max folders
		1, 0, //file number
		0xb7, 0xb //2999 total files
};



// device, device mask and loopMode do not persist
#define PERSISTENT_SIZE		sizeof(struct PlayerState)
//#define PERSISTENT_SIZE		(sizeof(struct PlayerState) - 3)

void LoadUsbState()
{
	eeprom_read_block(&gPlay, usbState, PERSISTENT_SIZE);
}

void LoadSdState()
{
	eeprom_read_block(&gPlay, sdState, PERSISTENT_SIZE);
}


void StoreUsbState()
{
	eeprom_update_block(&gPlay, usbState, PERSISTENT_SIZE);
}

void StoreSdState()
{
	eeprom_update_block(&gPlay, sdState, PERSISTENT_SIZE);
}




