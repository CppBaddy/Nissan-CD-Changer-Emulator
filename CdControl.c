/*
 * CdControl.c
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

#include "HuUart.h"
#include "CdControl.h"
#include "PlayerControl.h"
#include "Timeout.h"
#include <util/delay.h>

enum AsciiControl
{
	STX = 0x02,
	ETX = 0x03,
	EOT = 0x04,
	ACK = 0x06,
	DLE = 0x10
};

void RcvCommand();
void ReportActivated();
void SendPlayInfo();
void SendParameters();
void SendParameters2();
void StartPlayback();
void StopPlayback();
void NextCD();
void PrevCD();
void NextTrack();
void PrevTrack();

//void ModeNormal();
//void ModeLoopAll();
//void ModeLoopOne();
//void ModeAllRandom();

void FastForwardPressed();
void FastForwardReleased();
void FastReversePressed();
void FastReverseReleased();

void TxtDataPacket(const uint8_t* data, uint8_t size);
void BinDataPacket(const uint8_t* data, uint8_t size);

void SendAck();
inline void SendAck()
{
	Hu_PutByte(ACK);
}

void SendEOT();
inline void SendEOT()
{
	Hu_PutByte(EOT);
}

void SendSTX();
inline void SendSTX()
{
	Hu_PutByte(STX);
}

void SendETX();
inline void SendETX()
{
	Hu_PutByte(ETX);
}

void SendDLE();
inline void SendDLE()
{
	Hu_PutByte(DLE);
}

void ResetReceiveCommand();

enum eCdState
{
	BootState, //first state after powered on
	WaitForCommandState,

	TxPlayInfoState,
	ReportActivatedStatus,
	ReportParameters

};

static uint8_t gState = BootState;

static uint8_t gInitialized = false;

enum eRcvState
{
	RcvIdleState, RcvText, RcvCRC, WaitForAckState, WaitForEOTState
};

static uint8_t gRcvState = RcvIdleState;
static uint8_t gCrc = 0; //TODO clear on new receive

static uint8_t command;

//static bool gInitialized = false;
static bool gPlayInfoFlag = false;
static bool gReqIdle = false;

void HeadUnit_Update()
{
	switch (gState)
	{
	case BootState:
		if(Hu_RxReady())
		{
			Hu_ClrRTS();
			resetTimeout();

			gState = WaitForCommandState;
		}
		else if(isTimedOut())
		{
			gReqIdle = !gReqIdle;

			if(gReqIdle)
			{
				Hu_ClrRTS();
				resetTimeout1ms();
			}
			else
			{
				Hu_SetRTS();
				resetTimeout10ms();
			}
		}
		break;

	case WaitForCommandState:
		if (Hu_RxReady())
		{
			RcvCommand();
		}
		else if(isTimedOut())
		{
			ResetReceiveCommand();
		}
		else if (gInitialized && RcvIdleState == gRcvState)
		{
			if(gPlayInfoFlag != gPPS)
			{
				gState = TxPlayInfoState;
				gPlayInfoFlag = gPPS;
			}
		}
		break;

	case ReportActivatedStatus:
		ReportActivated();
		break;

	case ReportParameters:
		break;

	case TxPlayInfoState:
		SendPlayInfo();
		break;

	default:
		break;
	}
}

void ResetReceiveCommand()
{
	resetTimeout();
	gRcvState = RcvIdleState;
	gState = WaitForCommandState;
}

void RcvCommand()
{
	uint8_t b = Hu_GetByte();

	switch (gRcvState)
	{
	case RcvIdleState:
		if (STX == b)
		{
			gRcvState = RcvText;
			gCrc = 0;
			command = 0;
			resetTimeout();
		}
		else
		{
			ResetReceiveCommand();
		}
		break;
	case RcvText:
		gCrc ^= b;

		if (ETX == b)
		{
			gRcvState = RcvCRC;
		}
		else
		{
			command <<= 4;
			command |= (b - 0x30) & 0x0f;
		}

		resetTimeout();
		break;
	case RcvCRC:
		if (gCrc == b)
		{
			gRcvState = WaitForAckState;

			resetTimeout();

			switch (command)
			{
			case 0: //Activate Changer
				SendAck();
				gRcvState = WaitForEOTState;
				break;
			case 1: //send parameters
				SendParameters();
				gInitialized = true;
				break;
			case 2: //TODO 'r' [0x72]
				SendParameters2();
				break;
			case 0x60: // Play
				gPlay.loopMode = All;
				StartPlayback();
				Player_PlayTrack();
				break;
			case 0x62: // Stop
				StopPlayback();
				Player_Pause();
				break;
			case 0x63: // Next Track
				NextTrack();
				Player_PlayNext();
				break;
			case 0x64: // Prev Track
				PrevTrack();
				Player_PlayPrev();
				break;
			case 0x65: // FF pressed
				FastForwardPressed();
				Player_NextFolder();
				break;
			case 0x66: // FF released
				FastForwardReleased();
				StartPlayback();
				break;
			case 0x67: // FR pressed
				FastReversePressed();
				Player_PrevFolder();
				break;
			case 0x68: // FR released
				FastReverseReleased();
				StartPlayback();
				break;
			case 0x6a: // Next CD
				NextCD();
				Player_InitializeSd();
				StartPlayback();
				break;
			case 0x6b: // Prev CD
				PrevCD();
				Player_InitializeUsb();
				StartPlayback();
				break;
			case 0x6c:
				gPlay.loopMode = None;
				StartPlayback();
				break;
			case 0x6d:
				gPlay.loopMode = One;
				StartPlayback();
				break;
			case 0x6e:
				gPlay.loopMode = RandomAll;
				StartPlayback();
				break;
			case 0x6f:
				gPlay.loopMode = All;
				StartPlayback();
				break;
			default:
				ResetReceiveCommand();
				break;
			}
		}
		else
		{
			ResetReceiveCommand();
		}
		break;
	case WaitForEOTState:
		if (EOT == b)
		{
			resetTimeout();

			if (0 == command)
			{
				gRcvState = RcvIdleState;
				gState = ReportActivatedStatus;
			}
		}
		else
		{
			ResetReceiveCommand();
		}
		break;
	case WaitForAckState:
		if (ACK == b)
		{
			resetTimeout();
			SendEOT();
		}

		ResetReceiveCommand();
		break;
	default:
		ResetReceiveCommand();
		break;
	}
}

enum eReqStates
{
	ReqIdleState, ReqWaitForAck, ReqWaitForAck2
};

void ReportActivated()
{
	static uint8_t state = ReqIdleState;

	switch (state)
	{
	case ReqIdleState:
		Hu_SetRTS();
		resetTimeout();
		state = ReqWaitForAck;
		break;
	case ReqWaitForAck:
		if (Hu_RxReady())
		{
			uint8_t b = Hu_GetByte();
			if (ACK == b)
			{
				resetTimeout();
				state = ReqWaitForAck2;
				TxtDataPacket((uint8_t*) "0", 1);
			}
		}
		break;
	case ReqWaitForAck2:
		if (Hu_RxReady())
		{
			uint8_t b = Hu_GetByte();
			if (ACK == b)
			{
				resetTimeout();
				state = ReqIdleState;
				gState = WaitForCommandState;
				Hu_ClrRTS();
				SendEOT();
			}
		}
		break;
	default:
		break;
	}

	if(isTimedOut())
	{
		resetTimeout();
		state = ReqIdleState;
		gState = WaitForCommandState;
		Hu_ClrRTS();
	}
}


static uint8_t playInfo[9]	= { 0x35, 0x30, 0x31, 0x30, 0x31, 0x30, 0x30, 0x30, 0x30 };
//const uint8_t playInfo[]	= { "501010000" };

void SendPlayInfo()
{
	static uint8_t state = ReqIdleState;

	switch (state)
	{
	case ReqIdleState:
		playInfo[1] = 0x30 + gPlay.device / 10;
		playInfo[2] = 0x30 + gPlay.device % 10;

		playInfo[3] = 0x30 + gPlay.folder / 10;
		playInfo[4] = 0x30 + gPlay.folder % 10;

		uint8_t msd = gPlay.fileNum / 100;
		playInfo[5] = 0x30 + msd / 10;
		playInfo[6] = 0x30 + msd % 10;

		uint8_t lsd = gPlay.fileNum % 100;
		playInfo[7] = 0x30 + lsd / 10;
		playInfo[8] = 0x30 + lsd % 10;

		Hu_SetRTS();
		resetTimeout();
		state = ReqWaitForAck;
		break;
	case ReqWaitForAck:
		if (Hu_RxReady())
		{
			uint8_t b = Hu_GetByte();
			if (ACK == b)
			{
				resetTimeout();
				TxtDataPacket(playInfo, sizeof(playInfo));
				state = ReqWaitForAck2;
			}
		}
		else if(isTimedOut())
		{
			Hu_ClrRTS();
			resetTimeout();
			state = ReqIdleState;
			gState = WaitForCommandState;
		}
		break;
	case ReqWaitForAck2:
		if (Hu_RxReady())
		{
			uint8_t b = Hu_GetByte();
			if (ACK == b)
			{
				Hu_ClrRTS();
				SendEOT();
			}

			Hu_ClrRTS();
			resetTimeout();
			state = ReqIdleState;
			gState = WaitForCommandState;
		}
		else if(isTimedOut())
		{
			Hu_ClrRTS();
			resetTimeout();
			state = ReqIdleState;
			gState = WaitForCommandState;
		}
		break;
	default:
		break;
	}
}

static const uint8_t paramsInit[] 	= { 0x31, 0x00, 0x00, 0x20, 0x00, 0x00 };

static const uint8_t params[] 		= { 0x31, 0x3f, 0x80, 0x20, 0x01, 0x00 };

//static const uint8_t params2b[] 	= { 0x32, 0x3f, 0x80, 0x20, 0x01, 0x00 };

//static const uint8_t todo[] 		= { 0x32, 0x38, 0x80 };

//static const uint8_t todo2[] 		= { 0x32, 0x3f, 0x80 }; //CDs scan done

//Status report 0x33
static uint8_t cdStatusBuf[3]  = { 0x33, 0x22, 0x01 };

//static const uint8_t stopped[] 			= { 0x33, 0x20, 0x01 }; //last byte is play mode

//static const uint8_t playAll[] 			= { 0x33, 0x22, 0x01 }; //0x60
//static const uint8_t playNormal[] 		= { 0x33, 0x22, 0x05 }; //0x6c
//static const uint8_t playLoopOne[]   		= { 0x33, 0x22, 0x09 }; //0x6d
//static const uint8_t playAllRandom[] 		= { 0x33, 0x22, 0x0d }; //0x6e

//static const uint8_t nexttrack[] 			= { 0x33, 0x24, 0x01 };
//static const uint8_t prevtrack[] 			= { 0x33, 0x25, 0x01 };

//static const uint8_t fastforwardPress[] 	= { 0x33, 0x26, 0x01 };
//static const uint8_t fastforwardRelease[] = { 0x33, 0x22, 0x01 }; //playing

//static const uint8_t fastreversePress[] 	= { 0x33, 0x27, 0x01 };
//static const uint8_t fastreverseRelease[] = { 0x33, 0x22, 0x01 }; //playing

static const uint8_t changingcd[] 			= { 0x33, 0x2a, 0x00 }; // 1st
//{ 0x33, 0x25, 0x01 }; //2nd
//{ 0x33, 0x24, 0x01 }; //3rd
//{ 0x33, 0x22, 0x01 }; //4th

void SendParameters()
{
	BinDataPacket(params, sizeof(params));
}

void SendParameters2()
{
	BinDataPacket(paramsInit, sizeof(paramsInit));
}

void StopPlayback()
{
	cdStatusBuf[1] = 0x20;
	BinDataPacket(cdStatusBuf, sizeof(cdStatusBuf));
}

void StartPlayback()
{
	cdStatusBuf[1] = 0x22;
	cdStatusBuf[2] = gPlay.loopMode;
	BinDataPacket(cdStatusBuf, sizeof(cdStatusBuf));
}

void NextCD()
{
	BinDataPacket(changingcd, sizeof(changingcd));
}

void PrevCD()
{
	BinDataPacket(changingcd, sizeof(changingcd));
}

void FastForwardPressed()
{
	cdStatusBuf[1] = 0x26;
	BinDataPacket(cdStatusBuf, sizeof(cdStatusBuf));
}

void FastForwardReleased()
{
	StartPlayback();
}

void FastReversePressed()
{
	cdStatusBuf[1] = 0x27;
	BinDataPacket(cdStatusBuf, sizeof(cdStatusBuf));
}

void FastReverseReleased()
{
	StartPlayback();
}

void NextTrack()
{
	cdStatusBuf[1] = 0x24;
	BinDataPacket(cdStatusBuf, sizeof(cdStatusBuf));
}

void PrevTrack()
{
	cdStatusBuf[1] = 0x25;
	BinDataPacket(cdStatusBuf, sizeof(cdStatusBuf));
}

void TxtDataPacket(const uint8_t* data, uint8_t size)
{
	SendSTX();

	uint8_t crc = 0;
	for (uint8_t i = 0; i < size; ++i)
	{
		Hu_PutByte(data[i]);

		crc ^= data[i];
	}

	SendETX();
	crc ^= ETX;

	Hu_PutByte(crc);
}

void BinDataPacket(const uint8_t* data, uint8_t size)
{
	SendDLE();
	SendSTX();

	uint8_t crc = 0;
	for (uint8_t i = 0; i < size; ++i)
	{
		Hu_PutByte(data[i]);

		crc ^= data[i];
	}

	SendDLE();
	crc ^= DLE;

	SendETX();
	crc ^= ETX;

	Hu_PutByte(crc);
}
