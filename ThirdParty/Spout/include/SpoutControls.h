/*

					SpoutControls.h

		Copyright (c) 2015-2016, Lynn Jarvis. All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, 
		are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
		EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
		OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
		IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
		INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
		PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
		INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
		LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
// ====================================================================================
//		Revisions :
//
//		18.08.15	- Cleanup for 1.002 release
//
#pragma once

#ifndef __SpoutControls__
#define __SpoutControls__

#include <Shlobj.h> // to get the program folder path
#include <Knownfolders.h>
#include <comutil.h> // for _bstr_t (used in the string conversion)
#include <direct.h> // for directories
#include <io.h> // for file existence check

#include "spoutSDK.h"

#pragma comment(lib, "comsuppw") // for _bstr_t

// Warnings disabled in this project : C4250

// ISF to freeframe parameters
struct filecontrol {
	string name;
	string desc;    // To link with descriptons "TYPE"
					// "event"
					// "bool"
					// "long"
					// "text"
					// "float"
					// "point2D"
					// "color"
					// "image"
	int    fftype;  // freeframe parameter type - 0 - 6, 10, 100
	float  def;     // default float value
	float  value;   // user float value
	float  min;		// Minimum
	float  max;		// Maximum
	string text;	// text value
	// TODO deftext
};



struct control {
    string name;
    int type; // 0-checkbox, 1-button, 10-float, 100-string
    float value;
    string text;
};

class SPOUT_DLLEXP SpoutControls {

	public:

	SpoutControls();
    ~SpoutControls();

	// The controller
	bool FindControls  (string &mapname);
	bool CreateControls(string mapname, vector<control> controls);
	bool SetControls   (vector<control> controls);
	bool Cleanup();
	void CloseMap();

	bool FindControlFile (string &filepath);
	bool CopyControlFile (string &filepath, string &destpath);

	// The sender being controlled
	bool CreateControl(string name, string type);
	bool CreateControl(string name, string type, float value);
	bool CreateControl(string name, string type, string text);
	bool CreateControl(string name, string type, float minimum, float maximum, float value);

	bool OpenControls (string mapname);
	bool GetControls  (vector<control> &controls);
	bool CheckControls(vector<control> &controls);
	bool CloseControls();
	bool OpenSpoutController(string CommandLine = "");

	// Utility for other apps
	bool WritePathToRegistry   (const char *filepath, const char *subkey, const char *valuename);
	bool ReadPathFromRegistry  (char *filepath, const char *subkey, const char *valuename);
	bool WriteDwordToRegistry  (DWORD dwValue, const char *subkey, const char *valuename);
	bool ReadDwordFromRegistry (DWORD *pValue, const char *subkey, const char *valuename);
	bool RemovePathFromRegistry(char *filepath, const char *subkey);

protected :

	string m_sharedMemoryName; // Memory map name
	HANDLE m_hSharedMemory; // Memory map handle
	LPTSTR m_pBuffer; // Shared memory pointer
	HANDLE m_hAccessMutex; // Map access mutex
	HANDLE m_hSlot;
	DWORD m_dwSize; // Size of the memory map
	char m_ControlFilePath[MAX_PATH];

	vector<filecontrol> filecontrols; // Vector of control information used to create the sender control file

	bool UpdateControls(vector<control> controls);
	bool WriteControls(void *pBuffer, vector<control> controls);
	bool ReadControls (void *pBuffer, vector<control> &controls);

	bool CreateMail(string slotname, HANDLE &hSlot);
	bool WriteMail (string SlotName, string SlotMessage);
	bool ReadMail  (string SlotName, HANDLE hSlot, string &SlotMessage);
	bool CheckMail (string SlotName, HANDLE hSlot);

	bool CreateFileControl(string name, string type, float minimum, float maximum, float value, string text);
	bool CreateControlFile(const char *filepath);

};

#endif
