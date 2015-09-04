#define NUM 5

#define WIN32_LEAN_AND_MEAN
#define STRICT

//----------------------------------------------------------------------------------------------

#include <ddraw.h>
#include <mmsystem.h>
#include <stdio.h>
#include <math.h>

#include "misc.h"

//----------------------------------------------------------------------------------------------

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

//----------------------------------------------------------------------------------------------

IDirectDraw        *g_pDD     = NULL;
IDirectDrawSurface *g_pddsPri = NULL;
IDirectDrawSurface *g_pddsBck = NULL;

char                g_szAppName[] = "MMX";
bool                g_bActive     = true;
bool				g_bMMX		  = false;
DWORD               g_dwLastTick;

DWORD			   *g_pWhiteBlob;

POINT				g_coords[NUM];
POINT				g_napr[NUM];

//----------------------------------------------------------------------------------------------

int  APIENTRY    WinMain( HINSTANCE hInst, HINSTANCE hPInst, LPSTR lpCmdLine, int nCmdShow );
LRESULT WINAPI   WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK CfgDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

HRESULT CrtWnd( HINSTANCE hInst, int nCmdShow, HWND *phWnd );
HRESULT InitDD( HWND hWnd );
void    FreeDD( HWND hWnd );
HRESULT RestoreSurfaces();
HRESULT ClrSurface( IDirectDrawSurface *pdds );
HRESULT DisplayFrame();

void GenerateBlob();
void GerenerateCoords();

//----------------------------------------------------------------------------------------------

void GerenerateCoords()
{
	srand( timeGetTime() ); 

	for( int i = 0; i < NUM; i++ )
	{
		g_coords[i].x = rand() % 200 + 20;
		g_coords[i].y = rand() % 140 + 20;

		g_napr[i].x   = 5 - rand() % 11;
		g_napr[i].y   = 5 - rand() % 11;
	}
}

//----------------------------------------------------------------------------------------------

void GenerateBlob()
{
	for( int x = 0; x < 300; x++ )
	{
		for( int y = 0; y < 300; y++ )
		{
		  	double range = sqrt((x-150)*(x-150) + (y-150)*(y-150));
			double intense = 1 / exp((range/50)*(range/50));
			BYTE tmp = BYTE(255 * intense);
			g_pWhiteBlob[y*300+x] = tmp | tmp<<8 | tmp<<16;
		}
	}
}

//----------------------------------------------------------------------------------------------

HRESULT DisplayFrame()
{

	HRESULT hr;
    HDC     hDC;

	DDSURFACEDESC ddsd;

	ZeroMemory( &ddsd, sizeof( ddsd ) );
	ddsd.dwSize = sizeof( ddsd );       

	static char szMMXstateON[] = "Using MMX technology";
	static char szMMXstateOFF[] = "Using standard commands";
	static char szCurrFPS[] = "                           ";
	static FLOAT fFPS = 0;
    static FLOAT fLastTime = 0.0f;
    static DWORD dwFrames  = 0L;
    FLOAT fTime = (float)(timeGetTime() * 0.001);
    ++dwFrames;

    // Update the scene stats once per second
    if( fTime - fLastTime > 1.0f )
    {
        fFPS    = dwFrames / (fTime - fLastTime);
        fLastTime = fTime;
        dwFrames  = 0L;

		sprintf( szCurrFPS, "fps: %.1f", fFPS );
	}

    if( g_pddsBck->GetDC( &hDC ) != DDERR_SURFACELOST )
    {
//------------------------------------------------------------------------------------------------

		SetBkColor( hDC, RGB( 0, 0, 0 ) );
		SetTextColor( hDC, RGB( 0xff, 0xff, 0 ) );
        TextOut( hDC, 10, 30, szCurrFPS, lstrlen( szCurrFPS ) );
		
		if( g_bMMX )
		{
			TextOut( hDC, 10, 10, szMMXstateON, lstrlen( szMMXstateON ) );
		}
		else
		{
			TextOut( hDC, 10, 10, szMMXstateOFF, lstrlen( szMMXstateOFF ) );
		}

//------------------------------------------------------------------------------------------------

        if( g_pddsBck->ReleaseDC( hDC ) == DDERR_SURFACELOST )
        {
            if( FAILED( hr = RestoreSurfaces() ) )
            {
                return hr;

            } // if( FAILED( hr = RestoreSurfaces() ) )

        } // if( g_pddsBck->ReleaseDC( hDC ) == DDERRSURFACELOST )

    } // if( g_pddsBck->GetDC( &hDC ) != DDERR_SURFACELOST )
    else
    {
        if( FAILED( hr = RestoreSurfaces() ) )
        {
            return hr;

        } // if( FAILED( hr = RestoreSurfaces() ) )

    } // if( g_pddsBck->GetDC( &hDC ) == DDERR_SURFACELOST )

    if( g_pddsBck->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL ) != DDERR_SURFACELOST )
    {
        g_pScr   = (DWORD *)ddsd.lpSurface;
        g_lPitch = ddsd.lPitch;

//------------------------------------------------------------------------------------------------

		for( int i = 0; i < NUM; i++ )
		{
			if( g_bMMX )
			{
				PutBlobMMX( g_coords[i].x, g_coords[i].y, g_pWhiteBlob );
			}
			else
			{
				PutBlob( g_coords[i].x, g_coords[i].y, g_pWhiteBlob );
			}
		}

//------------------------------------------------------------------------------------------------

		if( g_pddsBck->Unlock( NULL ) == DDERR_SURFACELOST )
        {
            if( FAILED( hr = RestoreSurfaces() ) )
            {
                return hr;

            } // if( FAILED( hr = RestoreSurfaces() ) )

        } // if( g_pddsBck->Unlock( NULL ) == DDERR_SURFACELOST )

	} // if( g_pddsBck->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL ) != DDERR_SURFACELOST )
    else
    {
        if( FAILED( hr = RestoreSurfaces() ) )
        {
            return hr;

        } // if( FAILED( hr = RestoreSurfaces() ) )

    } // if( g_pddsBck->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL ) == DDERR_SURFACELOST )

    if( g_pddsPri->Flip( NULL, DDFLIP_WAIT ) == DDERR_SURFACELOST )
    {
        if( FAILED( hr = RestoreSurfaces() ) )
        {
            return hr;

        } // if( FAILED( hr = RestoreSurfaces() ) )

    } //  if( g_pddsPri->Flip( NULL, DDFLIP_WAIT ) == DDERR_SURFACELOST )

	ClrSurface( g_pddsBck );

	for( int i = 0; i < NUM; i++ )
	{
		g_coords[i].x += g_napr[i].x;
		g_coords[i].y += g_napr[i].y;

		if( ( g_coords[i].x > 330 )||( g_coords[i].x < 10 ) )
		{
			g_napr[i].x = - g_napr[i].x;
		}

		if( ( g_coords[i].y > 170 )||( g_coords[i].y < 10 ) )
		{
			g_napr[i].y = - g_napr[i].y;
		}
	}

	return S_OK;

} // DisplayFrame

//----------------------------------------------------------------------------------------------

HRESULT ClrSurface( IDirectDrawSurface *pdds )
{
    DDBLTFX ddbltfx;

    ZeroMemory( &ddbltfx, sizeof( ddbltfx ) );

    ddbltfx.dwSize      = sizeof( ddbltfx );
    ddbltfx.dwFillColor = 0;

    return pdds->Blt( NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx );

} // ClrSurface

//----------------------------------------------------------------------------------------------

HRESULT RestoreSurfaces()
{
    HRESULT hr;

    if( FAILED( hr = g_pddsPri->Restore() ) )
    {
        return hr;

    } // if( FAILED( hr = g_pddsPri->Restore() ) )

    if( FAILED( hr = g_pddsBck->Restore() ) )
    {
        return hr;

    } // if( FAILED( hr = g_pddsBck->Restore() ) )

	if( FAILED( hr = ClrSurface( g_pddsPri ) ) )
	{
		return hr;
	}

	return ClrSurface( g_pddsBck );

} // RestoreSurfaces

//----------------------------------------------------------------------------------------------

LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
    case WM_SIZE:

        if( ( wParam == SIZE_MAXHIDE )||( wParam == SIZE_MINIMIZED ) )
        {
            g_bActive = FALSE;

        } // if( (SIZE_MAXHIDE == wParam )||( SIZE_MINIMIZED == wParam ) )
        else
        {
            g_bActive = TRUE;

        } // if( !( (SIZE_MAXHIDE == wParam )||( SIZE_MINIMIZED == wParam ) ) )

        break;

    case WM_KEYDOWN:

		switch( wParam )                               
		{                                              
		case VK_ESCAPE:                                
			PostMessage(hWnd, WM_CLOSE, 0, 0);         
		break;                                     
		} 
        return 0;

    case WM_SETCURSOR:

        SetCursor( NULL );
        return TRUE;

    case WM_DESTROY:

		delete[] g_pWhiteBlob;
        FreeDD( hWnd );
        PostQuitMessage( 0 );
        return 0;

    } // switch( msg )

    return DefWindowProc( hWnd, msg, wParam, lParam );

} // WndProc

//----------------------------------------------------------------------------------------------

void FreeDD( HWND hWnd )
{
    if( g_pDD )
    {
        SAFE_RELEASE( g_pddsBck );
        SAFE_RELEASE( g_pddsPri );

        g_pDD->RestoreDisplayMode();
        g_pDD->SetCooperativeLevel( hWnd, DDSCL_NORMAL );
        g_pDD->Release();

        g_pDD = NULL;

    } // if( g_pDD )

} // FreeDD

//----------------------------------------------------------------------------------------------

HRESULT InitDD( HWND hWnd )
{
    HRESULT       hr;
    DDSURFACEDESC ddsd;
    DDSCAPS       ddscaps;

    if( FAILED( hr = DirectDrawCreate( NULL, &g_pDD, NULL ) ) )
    {
        return hr;

    } // if( FAILED( hr = DirectDrawCreate( NULL, &g_pDD, NULL ) ) )

    if( FAILED( hr = g_pDD->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN ) ) )
    {
        return hr;

    } // if( FAILED( hr = g_pDD->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN ) ) )

    if( FAILED( hr = g_pDD->SetDisplayMode( 640, 480, 32 ) ) )
    {
        return hr;

    } // if( FAILED( hr = g_pDD->SetDisplayMode( 640, 480, 8 ) ) )

    ZeroMemory( &ddsd, sizeof( ddsd ) );

    ddsd.dwSize            = sizeof( ddsd );
    ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;

    if( FAILED( hr = g_pDD->CreateSurface( &ddsd, &g_pddsPri, NULL ) ) )
    {
        return hr;

    } // if( FAILED( hr = g_pDD->CreateSurface( &ddsd, &g_pddsPri, NULL ) ) )

    ZeroMemory( &ddscaps, sizeof( ddscaps ) );
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

    if( FAILED( hr = g_pddsPri->GetAttachedSurface( &ddscaps, &g_pddsBck ) ) )
    {
        return hr;

    } // if( FAILED( hr = g_pddsPri->GetAttachedSurface( &ddscaps, &g_pddsBck ) ) )

    if( FAILED( hr = ClrSurface( g_pddsBck ) ) )
    {
        return hr;

    } // if( FAILED( hr = ClrSurface( g_pddsBck ) ) )

	return ClrSurface( g_pddsPri );

} // InitDD

//----------------------------------------------------------------------------------------------

HRESULT CrtWnd( HINSTANCE hInst, int nCmdShow, HWND *phWnd )
{
    HWND     hWnd;
    WNDCLASS wc;

    wc.lpszClassName = g_szAppName;
    wc.lpfnWndProc   = WndProc;
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = ( HBRUSH )GetStockObject( BLACK_BRUSH );
    wc.lpszMenuName  = NULL;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;

    if( !RegisterClass( &wc ) )
    {
        return E_FAIL;

    } // if( !RegisterClass( &wc ) )

    hWnd = CreateWindowEx(

        0,
        g_szAppName,
        g_szAppName,
        WS_POPUP,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInst,
        NULL

    ); // CreateWindowEx

    if( !hWnd )
    {
        return E_FAIL;

    } // if( !hWnd )

    ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

    *phWnd = hWnd;

    return S_OK;

} // CrtWnd

//----------------------------------------------------------------------------------------------

int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPInst, LPSTR lpCmdLine, int nCmdShow )
{
	MSG     msg;
    HWND    hWnd;
    HRESULT hr;

	if( ( *lpCmdLine == 'm' )||( *lpCmdLine == 'M' ) )
    {
		if( !TestMMX() )
		{
			MessageBox( 
				NULL, 
				"Ваш процессор не поддерживает MMX",
				"Ошибка",
				MB_OK | MB_ICONERROR
			);
			return FALSE;
		}
		g_bMMX = true;
	}

    g_dwLastTick = timeGetTime();

    if( FAILED( CrtWnd( hInst, nCmdShow, &hWnd ) ) )
    {
        return FALSE;

    } // if( FAILED( CrtWnd( hInst, nCmdShow, &hWnd ) ) )

    if( FAILED( hr = InitDD( hWnd ) ) )
    {
        FreeDD( hWnd );

        if( hr != DDERR_INVALIDMODE )
        {
            MessageBox(

                hWnd,
                "DirectDraw init failed.",
                g_szAppName,
                MB_ICONERROR | MB_OK

            ); // MessageBox

        } // if( hr != DDERR_INVALIDMODE )

        return FALSE;

    } // if( FAILED( InitDD( hWnd ) ) )

	g_pWhiteBlob = new DWORD[300 * 300];
	GenerateBlob();
	GerenerateCoords();

    while( TRUE )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
            if( !GetMessage(&msg, NULL, 0, 0 ) )
            {
                return (int)msg.wParam;

            } // if( !GetMessage(&msg, NULL, 0, 0 ) )

            TranslateMessage( &msg );
            DispatchMessage( &msg );

        } // if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        else
        {
            if( g_bActive )
            {
                if( FAILED( DisplayFrame() ) )
                {
                    FreeDD( hWnd );

                    MessageBox(

                        NULL,
                        "Displaying the next frame failed.",
                        g_szAppName,
                        MB_ICONERROR | MB_OK

                    ); // MessageBox

                    return FALSE;

                } // if( FAILED( DisplayFrame() ) )

            } // if( g_bActive )
            else
            {
                WaitMessage();

                g_dwLastTick = timeGetTime();

            } // if( !g_bActive )

        } // if( !PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )

    } // while( TRUE )

} // WinMain

//----------------------------------------------------------------------------------------------
