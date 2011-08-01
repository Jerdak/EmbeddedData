// EmbedData.cpp : Defines the entry point for the console application.
//




#ifdef LINUX
	#include <windows.h>
	#include <scrnsave.h> 
	#include <stdio.h>

	int get_file_size(HANDLE file){
	  DWORD sizeHigh;
	  DWORD sizeLow = ::GetFileSize(file, &sizeHigh);
	  if(sizeLow == 0xFFFFFFFF)
		if(::GetLastError() != NO_ERROR)
		  return false;
	  return (((int)sizeHigh) << 32) + sizeLow;

	}
	int main(int argc, char*argv[]){
		HANDLE file=CreateFile("EmbedData2.exe", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		int size = get_file_size(file);
		if(!file){
			printf("File not already open.\n");
			return 0;
		} else {
			printf("File opened self.  Size: %d\n",size);
		}
		printf("Hello world.\n");
		return 0;
	}
#else
	#include <windows.h>
	#include <scrnsave.h> 

	#include <stdio.h>
	#include "oglManager.h"
	///Load data from an embedded resource
	bool read_resource_data();

	LONG WINAPI ScreenSaverProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		static UINT uTimer;
		static POINT InitCursorPos;
		
		static HDC hDC;
		static HGLRC hRC;
		switch (msg)
		{
			case WM_CREATE:
				if(read_resource_data()){
					normalize_vertices();
					sort_by_height(-1);
				}
				Init(hWnd,hDC,hRC);
				uTimer = SetTimer(hWnd, 1, 10, NULL); 
				break;
			case WM_DESTROY:
				if(uTimer)KillTimer(hWnd, uTimer); 
				Kill(hWnd,hDC,hRC);
				PostQuitMessage(0);
				break;
			case WM_TIMER:
				Update(hDC);
				break;
			case WM_PAINT:
				Update(hDC);
				break;
			case WM_ERASEBKGND:
				break;
		}
		return DefScreenSaverProc(hWnd, msg, wParam, lParam);
	}

	// Register sub class dialogs, which we don't have.
	BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
	{
	   return TRUE;
	}
	BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return FALSE;
	}

	bool read_resource_data(){
		HMODULE hLibrary;
		HRSRC hResource;
		HGLOBAL hResourceLoaded; 
		LPBYTE lpBuffer;

		hLibrary = GetModuleHandle(NULL);

		if (NULL != hLibrary)
		{
			hResource = FindResource(hLibrary, MAKEINTRESOURCE(100), RT_RCDATA);
			if (NULL != hResource)
			{ 
				///Load vertex data
				hResourceLoaded = LoadResource(hLibrary, hResource);
				if (NULL != hResourceLoaded)        
				{
					lpBuffer = (LPBYTE) LockResource(hResourceLoaded);            
					if (NULL != lpBuffer)            
					{        
						int size = ((float*)lpBuffer)[0];
						int ct = 0;
						printf("Load %d vertices from resource.\n",size);
						for(int i = 1; i < size; i+=3){
							//printf("v[%d]: %f %f %f\n",ct++,((float*)lpBuffer)[i],((float*)lpBuffer)[i+1],((float*)lpBuffer)[i+2]);
							add_vertex(((float*)lpBuffer)[i],((float*)lpBuffer)[i+1],((float*)lpBuffer)[i+2]);
						}
						if(lpBuffer){delete [] lpBuffer; lpBuffer = NULL;}
					} else {
						DWORD err = GetLastError();
						printf("  - Warning[%d], couldn't load buffer\n",err);
						return false;
					}
				}   else {
					DWORD err = GetLastError();
					printf("  - Warning[%d], couldn't load resource handle\n",err);
					return false;
				} 
			} else {
				DWORD err = GetLastError();
				printf("  - Warning[%d], couldn't load current module.\n",err);
				return false;
			}
			hResource = FindResource(hLibrary, MAKEINTRESOURCE(101), RT_RCDATA);
			
			///Load color data
			if (NULL != hResource)
			{ 
				hResourceLoaded = LoadResource(hLibrary, hResource);
				if (NULL != hResourceLoaded)        
				{
					lpBuffer = (LPBYTE) LockResource(hResourceLoaded);            
					if (NULL != lpBuffer)            
					{        
						int size = ((float*)lpBuffer)[0];
						int ct = 0;
						printf("Load %d colors from resource.\n",size);
						for(int i = 1; i < size; i+=3){
							if(i< 20)printf("c[%d]: %f %f %f\n",ct++,((float*)lpBuffer)[i],((float*)lpBuffer)[i+1],((float*)lpBuffer)[i+2]);
							add_color(i/3,((float*)lpBuffer)[i],((float*)lpBuffer)[i+1],((float*)lpBuffer)[i+2]);
						}
						if(lpBuffer){delete [] lpBuffer; lpBuffer = NULL;}
					} else {
						DWORD err = GetLastError();
						printf("  - Warning[%d], couldn't load buffer\n",err);
						return false;
					}
				}   else {
					DWORD err = GetLastError();
					printf("  - Warning[%d], couldn't load resource handle\n",err);
					return false;
				} 
			} else {
				DWORD err = GetLastError();
				printf("  - Warning[%d], couldn't load current module.\n",err);
				return false;
			}
			FreeLibrary(hLibrary);
		} else {
			DWORD err = GetLastError();
			printf("  - Warning[%d], couldn't load current module.\n",err);
			return false;
		}
		return true;
	}
#endif
