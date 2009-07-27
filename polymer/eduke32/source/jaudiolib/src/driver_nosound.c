/*
 Copyright (C) 2009 Jonathon Fowler <jf@jonof.id.au>
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 
 See the GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
 */

/**
 * Stub driver for no output
 */

#include "inttypes.h"

int32_t NoSoundDrv_GetError(void)
{
	return 0;
}

const char *NoSoundDrv_ErrorString( int32_t ErrorNumber )
{
	return "No sound, Ok.";
}

int32_t NoSoundDrv_Init(int32_t mixrate, int32_t numchannels, int32_t samplebits, void * initdata)
{
	return 0;
}

void NoSoundDrv_Shutdown(void)
{
}

int32_t NoSoundDrv_BeginPlayback(char *BufferStart, int32_t BufferSize,
						int32_t NumDivisions, void ( *CallBackFunc )( void ) )
{
	return 0;
}

void NoSoundDrv_StopPlayback(void)
{
}

void NoSoundDrv_Lock(void)
{
}

void NoSoundDrv_Unlock(void)
{
}
