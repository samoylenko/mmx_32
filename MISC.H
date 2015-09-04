extern "C" void PutBlob( int x, int y, DWORD *blob );
extern "C" void PutBlobMMX( int x, int y, DWORD *blob );
extern "C" bool	TestMMX();

//----------------------------------------------------------------------------------------------

extern "C" DWORD *g_pScr;
extern "C" long   g_lPitch;
