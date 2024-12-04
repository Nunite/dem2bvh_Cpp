/*
	OpenDemo structs, errors, loading function and examples

	Copyright (c) 2010, eDark & Vee
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name eDark nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.
		* The source code may not be used in any commercial applications.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL eDark BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once

#include "allocator.h"
#include "filereader.h"
#include "vector.h"

#include <vector>
#include <list>
#include <cstring>
#include <string>

using namespace std;

#define DEMOPROTOCOL_MIN		5		// minimal supported demoversion
#define NETWORKPROTOCOL_MIN		48		// minimal supported networkversion

namespace OpenDemo
{
    enum ErrorCode
    {
        OD_SUCCESS              = 0, // No error.
        OD_ERROR_NO_FILE        = 1, // No file found/loaded.
        OD_ERROR_NO_DEMO        = 2, // File is not a demo.
        OD_ERROR_LOADING        = 3, // Loading failed.
        OD_ERROR_OLD_PROTOCOL   = 4, // Old protocol.
        OD_ERROR_CORRUPT_FILE   = 5, // File is corrupted.
		OD_ERROR_NO_FRAME		= 6,
		OD_ERROR_NO_MACRO		= 7,
    };

    enum MacroID
    {
        MACROID_LOADDATA        = 0,
        MACROID_GAMEDATA        = 1,
        MACROID_UNUSED          = 2,
        MACROID_CLIENTCOMMAND   = 3,
        MACROID_CLIENTDATA      = 4,
        MACROID_ENDOFSEGMENT    = 5,
        MACROID_EVENT           = 6,
        MACROID_WEAPONANIM      = 7,
        MACROID_PLAYSOUND       = 8,
        MACROID_DEMOBUFFER      = 9
    };

	enum ButtonFlag
	{
		BF_NULL			= 0x0,
		BF_JUMP			= 0x1,
		BF_DUCK			= 0x2,
		BF_FORWARD		= 0x4,
		BF_BACK			= 0x8,
		BF_MOVELEFT		= 0x10,
		BF_MOVERIGHT	= 0x20,
		BF_JUMP_SCROLL	= 0x40,
		BF_DUCK_SCROLL	= 0x80,
		BF_SCROLLS		= 0xC0,
	};

	struct PlayerData
	{
		vec3_t position;
		vec3_t velocity;
		vec3_t orientation;
	};

	struct CameraData
	{
		vec3_t position;
		vec3_t orientation;
	};

	struct Settings
	{
		float sv_gravity;
		float sv_stopspeed;
		float sv_maxspeed;
		float sv_maxspectatorspeed;
		float sv_accelerate;
		float sv_airaccelerate;
		float sv_wateraccelerate;
		float sv_friction;
		float edgefriction;
		float sv_waterfriction;
		float entgravity;
		float sv_bounce;
		float sv_stepsize;
		float sv_maxvelocity;
		float sv_zmax;
		float sv_wateramp;
		int mp_footsteps;
	};

	#pragma pack(1)

	struct DemoHeader
	{
		char magic[8];		// 'HLDEMO'
		uint32 demo_protocol;
		uint32 network_protocol;
		char map_name[260];
		char game_dll[260];
		uint32 map_crc;
		uint32 directory_offset;
	};

	struct DemoDirectoryEntry
	{
		uint32 number;
		char title[64];
		uint32 flags;
		int32 play;
		float time;
		uint32 frames;
		uint32 offset;	// points to the DemoSegment
		uint32 length;	// length of the DemoSegment at offset
	};

	struct DemoMacroHeader
	{
		uint8 type;
		float time;
		uint32 frame;
	};
	#pragma pack()

	struct DemoMacro
	{
		DemoMacroHeader* header;
		DemoMacro* nextMacro;
	};

	#pragma pack(push, 1)

	struct DemoMacro_GameData
	{
		DemoMacroHeader header;
		char skip0[4];
		CameraData camera;
		char skip1[36];
		float framelength;
		float servertime;
		char skip2[12];
		uint32 groundflag;
		char skip3[4];
		vec3_t playervelocity;
		vec3_t playerposition;
		char skip4[104];
		short screen_width;
		short screen_height;
		short screen_depth;
		char skip5[12];
		short engine_framelength;
		char skip6[12];
		float forwardmove;
		float sidemove;
		char skip7[28];
		Settings settings;
		char sky_name[40];
		vec3_t playerposition_2;
		char skip8[56];
		uint32 chunklength;		// size of following DemoServerMessage
	};

	struct DemoMacro_ClientCommand
	{
		DemoMacroHeader header;
		char cmd[64];
	};

	struct DemoMacro_ClientData
	{
		DemoMacroHeader header;
		vec3_t playerposition;
		vec3_t playerorientation;
		char data[8];
	};

	struct DemoMacro_LastInSegment
	{
		DemoMacroHeader header;
	};

	struct DemoMacro_Event
	{
		DemoMacroHeader header;
		uint32 uk_i1;
		uint32 uk_i2;
		float uk_f;
		char data[72];
	};

	struct DemoMacro_WeaponAnim
	{
		DemoMacroHeader header;
		uint32 uk_i1;
		uint32 uk_i2;
	};

	struct DemoMacro_PlaySound
	{
		DemoMacroHeader header;
		uint32 uk_i1;
		uint32 sound_name_length;
	};

	struct DemoMacro_PlaySound_2
	{
		float uk_f1;
		float uk_f2;
		uint32 uk_i2;
		uint32 uk_i3;
	};

	struct DemoMacro_DemoBuffer
	{
		DemoMacroHeader header;
		uint32 chunklength;
	};

	#pragma pack(pop)

	struct DemoSegment
	{
		const DemoDirectoryEntry *dir_entry;
		DemoMacro *firstMacro;
        Allocator<DemoMacro> macroAllocator;

		// pointer to the first macro of a frame for easier access
		vector<DemoMacro*> frames;
		// ButtonFlags of frame
		vector<uint32> buttonFlag;
	};

    struct ErrorInfo
    {
        ErrorCode code;
        size_t offset;
        uint8 macroId;
    };

	class Demo
	{
	public:

		Demo();
		~Demo();

		size_t GetNumberOfFrames() {return m_segments[1].frames.size();}

		void GetDemoProtocolVer(uint32* demoProtocolVer) {*demoProtocolVer = m_header->demo_protocol;}
		void NetworkProtocolVer(uint32* networkProtocolVer) {*networkProtocolVer = m_header->network_protocol;}
		void GetMapName(char* mapName) { memcpy(mapName, m_header->map_name, 260); }
		void GetGameDll(char* gameDll) { memcpy(gameDll, m_header->game_dll, 260); }
		void GetMapCrc(uint32* mapCrc) {*mapCrc = m_header->map_crc;}

		bool GetFrameTime(size_t frame, float* frameTime);
		bool GetCameraPosition(size_t frame, vec3_t* cameraPos);
		bool GetCameraOrientation(size_t frame, vec3_t* cameraRot);
		bool GetCameraData(size_t frame, CameraData* camera);
		bool GetFrameLength(size_t frame, float* frameLength);
		bool GetServerTime(size_t frame, float* serverTime);
		bool GetGroundFlag(size_t frame, uint32* groundFlag);
		bool GetPlayerVelocity(size_t frame, vec3_t* playerVel);
		bool GetPlayerPosition(size_t frame, vec3_t* playerPos);
		bool GetPlayerOrientation(size_t frame, vec3_t* playerRot);
		bool GetPlayerData(size_t frame, PlayerData* playerData);
		bool GetScreenResolution(size_t frame, short* screen_width, short* screen_height, short* screen_depth);
		bool GetEngineFrameLength(size_t frame, short* engineFrameLength);
		bool GetForwardmove(size_t frame, float* forwardmove);
		bool GetSidemove(size_t frame, float* sidemove);
		bool GetSettings(size_t frame, Settings* settings);
		bool GetSkyName(size_t frame, char* sky_name); // [40]

		bool GetNumberOfCommands(size_t frame, size_t* numCommands);
		bool GetCommands(size_t frame, char** commands, size_t numCommands); // [64]
		bool GetButtonFlags(size_t frame, uint32* flags);

		bool Open(wchar_t *wcFileName);
		ErrorCode GetLastError() const;

		// unsave functions with direct access to demo buffer
		// only use these if you know what you re doing

		void _GetSegments(vector<DemoSegment>** segments) {*segments = &m_segments;}

	protected:

		ErrorInfo m_errorInfo;

		FileReader m_f;
		wstring m_wsFileName;
		long long m_fileSize;
        
		DemoHeader* m_header;
		vector<DemoSegment> m_segments;
		DemoMacro* macro;

		bool GetFirstMacroOfFrame(size_t frame, DemoMacro** macro);
		bool GetMacroOfFrame(size_t frame, DemoMacro** macro, MacroID macroId);
		bool GetMacroOfFrame(size_t frame, DemoMacro** macro, MacroID macroId1, MacroID macroId2);

        void SetErrorInfo (const DemoMacroHeader* header);
		void SetLastError(ErrorCode error_code);
	};

    inline ErrorCode Demo::GetLastError () const
    {
        return m_errorInfo.code;
    }

    inline void Demo::SetLastError (ErrorCode error_code)
    {
        m_errorInfo.code = error_code;
    }
}
