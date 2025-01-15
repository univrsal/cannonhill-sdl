#if _WIN32
#include "SDL_Main.h"
#endif

#include "panzer.h"

#include <chrono>
#include <thread>

#include "mixer.hpp"
#include "audio.hpp"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

LPDDSURFACEDESC2 lpDDSBack{};
SDL_Cursor *lpDDSCursor = NULL;

LPDDSURFACEDESC2 lpDDSSchrift1;
LPDDSURFACEDESC2 lpDDSSchrift2;
LPDDSURFACEDESC2 lpDDSSchrift3;
LPDDSURFACEDESC2 lpDDSScape;
LPDDSURFACEDESC2 lpDDSHimmel;
LPDDSURFACEDESC2 lpDDSHimmel2;
LPDDSURFACEDESC2 lpDDSHimmel3;
LPDDSURFACEDESC2 lpDDSSchrift;
LPDDSURFACEDESC2 lpDDSPanzer;
LPDDSURFACEDESC2 lpDDSMunition;
LPDDSURFACEDESC2 lpDDSPanzSave;
LPDDSURFACEDESC2 lpDDSSonstiges;
LPDDSURFACEDESC2 lpDDSMBack;
LPDDSURFACEDESC2 lpDDSMSBack;
LPDDSURFACEDESC2 lpDDSMPunkt;
LPDDSURFACEDESC2 lpDDSMGBack;
LPDDSURFACEDESC2 lpDDSMKeyboard;
LPDDSURFACEDESC2 lpDDSCredits;
LPDDSURFACEDESC2 lpDDSTitel;

SDL_PixelFormat ddpf;

audio::manager* audio_manager{};

BOOL bActive = false;
int Spielzustand = SZNICHTS;			 // in welchem Zustand ist das Spiel?
SHORT CursorTyp;						 // Welcher Cursortyp?
BOOL Button0down;						 // linke Maustaste gedr�ckt gehalten
BOOL Button1down;						 // rechte Maustaste gedr�ckt gehalten
short ButtonPush;						 // Knopf gedr�ckt(1) oder losgelassen(-1) oder gedr�ckt(0) gehalten
short MouseSoundBuffer;					 // Auf welchem Kanal kommt der Mousesound
Uint32 Zeitgenau;						 // Ticks seit start in ms
SHORT Bild, LastBild;					 // Anzahl der Bilder in der Sekunde
RECT rcRectdes, rcRectsrc;				 // St�ndig ben�tigte Variable zum Blitten
CHAR StdString[1024];					 // Standard string
SHORT Soundzustand;						 //-1 = keine Soundkarte;0=kein Sound;1=Sound an
double pi = 3.1415926535;				 // pi, was sonst
double g = 9.81;						 // g = 40;		//Erdanziehungskraft
double t = 0.08;						 // Wieviel Zeit vergeht in einem Berechnungsdurchgang (mu� noch an die Framerate angepasst werden)
BOOL BereichAktiv[BERANZAHL][BERANZAHL]; // Bereiche zur ermittelung der aktiven Pixel
bool Testmodus = false;					 // Schaltet den Testmodus an und aus
DWORD Tansparent;						 // In DWORD umgerechnete Tansparente Farbe
short Wetter;							 // 0=keinWetter,1=Regen,2=Schnee
short LastFX;							 // Der letzte FX-Pixel in der FX-Liste
short AktMenue;							 // In welchem Menue
bool Wetterschalter;					 // Wetter an/aus
bool Windschalter;						 // Wind	 an/aus
short AktRunde;							 // In welcher Runde
short MaxRunde;							 // Anzahl der Runden
long CreditsZaehler;					 // Zur Zeitabh�ngigen Steuerung
short Himmel;							 // Welcher Himmel?

// Bereiche
//							links,		oben,		rechts,				unten
RECT rcGesamt = {0, 0, MAXX, MAXY};

ZWEID MousePosition;		  //     Aktuelle Mauskoordinaten
ZWEID PanzSavPos[MAXSPIELER]; // Postionen der Panzer in der PanzSavSurface
ZWEIDGENAU Wind;
ZWEIDGENAU AddBeschleunigung(ZWEIDGENAU Add, ZWEIDGENAU Neu); // Addiert einen Beschleunigungsvektor zu einem anderen
RGBSTRUCT rgbStruct;										  // Hier werden die Farben eines Pixels zwischengespeichert
FXLISTE FXListe[MAXFX];
PHYSIK Objekt, HimmelPixel[MAXHIMMELPIXEL];
PIXEL Pixel[MAXX][MAXY];
PANZER Panzer[MAXSPIELER];
MUNITION Munition[MUNANZAHL];
MUNLISTE MunListe[MAXMUN];
ZWEID Physik(bool touch, short InfoNr); // Ver�ndert die in Objekt gespeicherten Wert (Bei !touch keine Ber�hrung mit anderen Pixel)
ART Art[ARTANZAHL];
BMP Bmp[BILDANZ];
WAV Wav[WAVANZ];
Menueseite Menue[MENANZ];
BALLON Ballon; // Eigenschaften vom Ballon
CREDITSLISTE CreditsListe[MAXCREDITS];

// Bilder


static char Cursorbmp[] = PATH_PREFIX "img/Cursor.BMP";
static char Schrift2[] = PATH_PREFIX "img/schrift2.bmp";
static char Schrift1[] = PATH_PREFIX "img/schrift1.bmp";
static char Schrift3[] = PATH_PREFIX "img/schrift3.bmp";
static char Panzerbmp[] = PATH_PREFIX "img/Panzer.bmp";
static char Munitionbmp[] = PATH_PREFIX "img/Munition.bmp";
static char Himmelbmp[] = PATH_PREFIX "img/himmel.bmp";
static char Himmel2bmp[] = PATH_PREFIX "img/himmel2.bmp";
static char Himmel3bmp[] = PATH_PREFIX "img/himmel3.bmp";
static char Sonstigesbmp[] = PATH_PREFIX "img/sonstiges.bmp";
static char MBackbmp[] = PATH_PREFIX "img/mback.bmp";
static char MSBackbmp[] = PATH_PREFIX "img/msback.bmp";
static char MPunktbmp[] = PATH_PREFIX "img/mpunkt.bmp";
static char MGBackbmp[] = PATH_PREFIX "img/mgback.bmp";
static char MKeyboardbmp[] = PATH_PREFIX "img/Keyboard.bmp";
static char Creditsbmp[] = PATH_PREFIX "img/credits.bmp";
static char Titelbmp[] = PATH_PREFIX "img/titel.bmp";

float ScaleFactor = 2.0f;

/*
 * finiObjects
 *
 * finished with all objects we use; release them
 */
static void finiObjects(void)
{
	lpDDSBack.destroy();
	lpDDSSchrift1.destroy();
	lpDDSSchrift2.destroy();
	lpDDSSchrift3.destroy();
	lpDDSScape.destroy();
	lpDDSHimmel.destroy();
	lpDDSHimmel2.destroy();
	lpDDSHimmel3.destroy();
	lpDDSSchrift.destroy();
	lpDDSPanzer.destroy();
	lpDDSMunition.destroy();
	lpDDSPanzSave.destroy();
	lpDDSSonstiges.destroy();
	lpDDSMBack.destroy();
	lpDDSMSBack.destroy();
	lpDDSMPunkt.destroy();
	lpDDSMGBack.destroy();
	lpDDSMKeyboard.destroy();
	lpDDSCredits.destroy();
	lpDDSTitel.destroy();
	delete audio_manager;
	audio::close();
	SDL_FreeCursor(lpDDSCursor);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
} /* finiObjects */

void InitDDraw()
{
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (!renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		return;
	}

	// Create the back buffer
	lpDDSBack.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, MAXX, MAXY);

	// Create the texture for the landscape
	auto *surf = SDL_CreateRGBSurface(0, MAXX, MAXY, 32, 0, 0, 0, 0);

	SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, 255, 0, 255));
	lpDDSScape.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, surf->w, surf->h);
	SDL_UpdateTexture(lpDDSScape.texture, NULL, surf->pixels, surf->pitch);
	SDL_FreeSurface(surf);

	if (!lpDDSScape.texture)
	{
		return;
	}

	lpDDSPanzSave.texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, MAXSPIELER * PANZERBREITE, PANZERHOEHE + 10);
	SDL_SetRenderTarget(renderer, lpDDSPanzSave.texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, NULL);

	// Load icon
	surf = SDL_LoadBMP("img/icon.bmp");
	SDL_SetWindowIcon(window, surf);
	SDL_FreeSurface(surf);


	// In diese Surface sollen die Cursor geladen werden
	surf = DDLoadBitmap(Cursorbmp, 0, 0, 255, 0, 255);
	auto* fmt = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
	auto* tmp = SDL_ConvertSurface(surf, fmt, 0);
	auto* cursor = ScaleSurface(tmp, ScaleFactor);
	lpDDSCursor = SDL_CreateColorCursor(cursor, 0, 0);
	SDL_FreeSurface(surf);
	SDL_FreeSurface(tmp);
	SDL_FreeSurface(cursor);

	// use cursor
	SDL_SetCursor(lpDDSCursor);

	// In diese Surface sollen die Schrift1 geladen werden
	lpDDSSchrift1 = DDLoadTexture(renderer, Schrift1);

	// In diese Surface sollen die Schrift2 geladen werden
	lpDDSSchrift2 = DDLoadTexture(renderer, Schrift2, 0, 0, 255, 0, 255, true);

	// In diese Surface sollen die Schrift3 geladen werden
	lpDDSSchrift3 = DDLoadTexture(renderer, Schrift3);

	// In diese Surface sollen die Panzer geladen werden
	lpDDSPanzer = DDLoadTexture(renderer, Panzerbmp, 0, 0, 255, 0, 255, true);

	// In diese Surface soll die Munition geladen werden
	lpDDSMunition = DDLoadTexture(renderer, Munitionbmp, 0, 0, 255, 0, 255, true);

	// Da kommt der Hintergrund rein
	lpDDSHimmel = DDLoadTexture(renderer, Himmelbmp);

	// Da kommt der Hintergrund rein
	lpDDSHimmel2 = DDLoadTexture(renderer, Himmel2bmp);

	// Da kommt der Hintergrund rein
	lpDDSHimmel3 = DDLoadTexture(renderer, Himmel3bmp);

	// und hier die Sonstigen sachen
	lpDDSSonstiges = DDLoadTexture(renderer, Sonstigesbmp, 0, 0, 255, 0, 255, true);

	// hier der Menuehintergrund
	lpDDSMBack = DDLoadTexture(renderer, MBackbmp);

	// hier der Shophintergrund
	lpDDSMSBack = DDLoadTexture(renderer, MSBackbmp);

	// hier der Gewonnenhintergrund
	lpDDSMGBack = DDLoadTexture(renderer, MGBackbmp);

	// hier die Menuepunkte
	lpDDSMPunkt = DDLoadTexture(renderer, MPunktbmp);

	// hier das Keyboardbild
	lpDDSMKeyboard = DDLoadTexture(renderer, MKeyboardbmp);

	// hier die Credits
	lpDDSCredits = DDLoadTexture(renderer, Creditsbmp, 0, 0, 255, 0, 255, true);
	lpDDSCredits.texture = SDL_CreateTextureFromSurface(renderer, lpDDSCredits.surface);

	// hier das Titelbild
	lpDDSTitel = DDLoadTexture(renderer, Titelbmp);
}

void InitDSound()
{
	// Check if sdl audio is initialized
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		return;
	}

	if (!audio::init())
	{
		audio::close();
		Soundzustand = 0;
		return;
	}
	audio_manager = new audio::manager();
	audio_manager->init();

	Soundzustand = 1; // Sound anschalten
}

void LoadSound(short Sound)
{
	// Datei �ffnen
	// TODO
	// DateiHandle = mmioOpen(Wav[Sound].Dateiname, NULL, MMIO_READ | MMIO_ALLOCBUF);

	// lpds->CreateSoundBuffer(&dsbdesc, &lpdsbWav[Sound], NULL);

	// // Die Standardlautst�rke festlegen
	// lpdsbWav[Sound]->SetVolume((long)(-10000 + 100 * Wav[Sound].Volume));
}

short PlaySound(short Sound, short Volume, short Pan, short SoundBuffer, bool Loop, short Frequenz)
{
	double z;
	short i;
	DWORD lpdwStatus;
	// TODO
	return 0;

	// if ((Sound == 0) || (Soundzustand <= 0))
	// 	return -1;
	// if (SoundBuffer == -1)
	// {
	// 	// Freien Buffer finden
	// 	for (i = 0; i < MAXCHANNELS; i++)
	// 	{
	// 		lpdsbWavPlay[i]->GetStatus(&lpdwStatus);
	// 		if ((lpdwStatus & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING)
	// 			break;
	// 	}
	// 	if (i == MAXCHANNELS)
	// 		return -1;
	// 	else
	// 		SoundBuffer = i;

	// 	// Soundbuffer freigeben (ist das n�tig??)
	// 	lpdsbWavPlay[SoundBuffer]->Release();

	// 	// SoundBuffer duplizieren
	// 	lpds->DuplicateSoundBuffer(lpdsbWav[Sound], &lpdsbWavPlay[SoundBuffer]);
	// }

	// z = -10000 + 100 * Wav[Sound].Volume;
	// lpdsbWavPlay[SoundBuffer]->SetVolume((long)(-10000 + (10000 + z) * Volume / 100));

	// if (Pan >= MAXX)
	// 	Pan = MAXX;
	// if (Pan < 0)
	// 	Pan = 0;
	// z = 10000 * pow((double(Pan) - double(MAXX) / 2) / (double(MAXX) / 2), 3);
	// lpdsbWavPlay[SoundBuffer]->SetPan(long(z));

	// if (Frequenz != -1)
	// 	lpdsbWavPlay[SoundBuffer]->SetFrequency(Frequenz);

	// lpdsbWavPlay[SoundBuffer]->GetStatus(&lpdwStatus);
	// if ((lpdwStatus & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING)
	// {
	// 	if (Loop)
	// 		lpdsbWavPlay[SoundBuffer]->Play(NULL, NULL, DSBPLAY_LOOPING);
	// 	else
	// 		lpdsbWavPlay[SoundBuffer]->Play(NULL, NULL, NULL);
	// }
	// return SoundBuffer;
}

void StopSound(audio::file f)
{
	if (Soundzustand <= 0 || f == audio::NO_SOUND)
		return;
	audio_manager->stop(f);
}

void StopAllSound()
{
	audio::manager::stop_channel(audio::id::SFX);
	audio::manager::stop_channel(audio::id::MUSIC);
}

void SetAcquire()
{
	// if (!MouseInit)
	// 	return;
	// if (bActive)
	// {
	// 	g_pMouse->Acquire();
	// 	if (g_pKey)
	// 		g_pKey->Acquire();
	// }
	// else
	// {
	// 	g_pMouse->Unacquire();
	// 	if (g_pKey)
	// 		g_pKey->Unacquire();
	// }
}

void SaveGame()
{
	short i;

	GetMenue();

	std::ofstream ofs("save.dat", std::ios::binary);
	if (!ofs.good())
		return;

	ofs.write((char *)&Windschalter, sizeof(Windschalter));
	ofs.write((char *)&Wetterschalter, sizeof(Wetterschalter));

	for (i = 0; i < MAXSPIELER; i++)
	{
		ofs.write((char *)&Panzer[i].Aktiv, sizeof(Panzer[i].Aktiv));
		ofs.write((char *)&Panzer[i].Computer, sizeof(Panzer[i].Computer));
	}
	ofs.write((char *)&MaxRunde, sizeof(MaxRunde));
	ofs.write((char *)&Soundzustand, sizeof(Soundzustand));
}

bool LoadGame()
{
	short i;
	short tmp;

	std::ifstream ifs("save.dat", std::ios::binary);
	if (!ifs.good())
	{
		PutMenue();
		return (false);
	}

	ifs.read((char *)&Windschalter, sizeof(Windschalter));
	ifs.read((char *)&Wetterschalter, sizeof(Wetterschalter));

	for (i = 0; i < MAXSPIELER; i++)
	{
		ifs.read((char *)&Panzer[i].Aktiv, sizeof(Panzer[i].Aktiv));
		ifs.read((char *)&Panzer[i].Computer, sizeof(Panzer[i].Computer));
	}
	ifs.read((char *)&MaxRunde, sizeof(MaxRunde));

	ifs.read((char *)&tmp, sizeof(Soundzustand));

	// Sound �berpr�fen
	if ((Soundzustand != -1) && (tmp != -1))
		Soundzustand = tmp;

	PutMenue();

	return (true);
}

void GetMenue()
{
	int i;

	// Infos vom Men� auswerten

	// Spielereinstellungen
	for (i = 1; i <= 8; i++)
	{
		if (i % 2 != 0)
			continue;
		if (Menue[MENSPIELER].getVersion(i) != 2)
			Panzer[i / 2 - 1].Aktiv = true;
		else
			Panzer[i / 2 - 1].Aktiv = false;
		if (Menue[MENSPIELER].getVersion(i) == 1)
			Panzer[i / 2 - 1].Computer = true;
		else
			Panzer[i / 2 - 1].Computer = false;
	}

	// Wind
	if (Menue[MENOPTION].getVersion(2) == 0)
		Windschalter = true;
	else
		Windschalter = false;

	// Wetter
	if (Menue[MENOPTION].getVersion(4) == 0)
		Wetterschalter = true;
	else
		Wetterschalter = false;

	// Maximale Rundenanzahl
	MaxRunde = Menue[MENOPTION].getZiffer(6);

	// Sound
	if (Menue[MENOPTION].getVersion(10) == 0)
		Soundzustand = 1;
	else if (Soundzustand != -1)
		Soundzustand = 0;
}

void PutMenue()
{
	int i;

	// Spielereinstellungen
	for (i = 1; i <= 8; i++)
	{
		if (i % 2 != 0)
			continue;
		if (Panzer[i / 2 - 1].Aktiv == false)
			Menue[MENSPIELER].putVersion(i, 2);
		else if (Panzer[i / 2 - 1].Computer == true)
			Menue[MENSPIELER].putVersion(i, 1);
		else
			Menue[MENSPIELER].putVersion(i, 0);
	}

	// Wind
	if (Windschalter == false)
		Menue[MENOPTION].putVersion(2, 1);
	else
		Menue[MENOPTION].putVersion(2, 0);

	// Wetter
	if (Wetterschalter == false)
		Menue[MENOPTION].putVersion(4, 1);
	else
		Menue[MENOPTION].putVersion(4, 0);

	// Maximale Rundenanzahl
	Menue[MENOPTION].putZiffer(6, MaxRunde);

	// Sound
	if (Soundzustand <= 0)
		Menue[MENOPTION].putVersion(10, 1);
	else
		Menue[MENOPTION].putVersion(10, 0);
}

void PutGMenue()
{
	int i, gewinner;
	long tmp, max;

	// Gewinner ermitteln
	max = 0;
	gewinner = 0;
	for (i = 0; i < MAXSPIELER; i++)
	{
		tmp = Panzer[i].RGewonnen * 1000000;
		tmp += Panzer[i].Abschuesse * 100000;
		tmp += Panzer[i].Treffer;
		if (tmp > max)
		{
			max = tmp;
			gewinner = i;
		}
	}

	// Mit Werten belegen
	Menue[AktMenue].putBild(0, MROTER + gewinner);

	for (i = 0; i < MAXSPIELER; i++)
	{
		Menue[AktMenue].putZiffer(1 + i * 3, Panzer[i].Treffer);
		Menue[AktMenue].putZiffer(2 + i * 3, Panzer[i].Abschuesse);
		Menue[AktMenue].putZiffer(3 + i * 3, Panzer[i].RGewonnen);
	}
}

void Blitten2(SDL_Texture *lpDDSVon, SDL_Texture *lpDDSNach, bool Transp)
{
	SDL_Rect src, dest;

	src.x = rcRectsrc.left;
	src.y = rcRectsrc.top;
	src.w = rcRectsrc.right - rcRectsrc.left;
	src.h = rcRectsrc.bottom - rcRectsrc.top;

	dest.x = rcRectdes.left;
	dest.y = rcRectdes.top;
	dest.w = rcRectdes.right - rcRectdes.left;
	dest.h = rcRectdes.bottom - rcRectdes.top;
	SDL_SetRenderTarget(renderer, lpDDSNach);
	if (Transp)
        SDL_SetTextureBlendMode(lpDDSVon, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(renderer, lpDDSVon, &src, &dest);
	SDL_SetRenderTarget(renderer, NULL);
}

void InitStructs(short zustand)
{
	short i, j;

	if (zustand == 0)
	{
		// transparent = ARGB(0, 0, 0, 0);
		Tansparent = 0;
		Bild = 1;
		LastBild = 25;
		MouseSoundBuffer = -1;

		// BILD
		// Standardbildinitialisierung
		for (i = 0; i < BILDANZ; i++)
		{
			Bmp[i].Surface = {};
			Bmp[i].rcSrc.left = 0;
			Bmp[i].rcSrc.right = 0;
			Bmp[i].rcSrc.top = 0;
			Bmp[i].rcSrc.bottom = 0;
			Bmp[i].Breite = 0;
			Bmp[i].Hoehe = 0;
		}
		// Cursor
		// for (i = CUKREUZ; i <= CUKREUZ; i++)
		// {
		// 	Bmp[i].Surface = lpDDSCursor;
		// 	Bmp[i].rcSrc.left = (i - CUKREUZ) * 18;
		// 	Bmp[i].rcSrc.top = 0;
		// 	Bmp[i].rcSrc.right = Bmp[i].rcSrc.left + 18;
		// 	Bmp[i].rcSrc.bottom = 18;
		// 	Bmp[i].Breite = 18;
		// 	Bmp[i].Hoehe = 18;
		// }

		for (i = PANZERNULL; i <= PANZERDREI; i++)
		{
			Bmp[i].Surface = lpDDSPanzer;
			Bmp[i].Breite = 20;
			Bmp[i].Hoehe = 10;
			Bmp[i].rcSrc.left = (i - PANZERNULL) * 20;
			Bmp[i].rcSrc.top = 0;
			Bmp[i].rcSrc.right = Bmp[i].rcSrc.left + Bmp[i].Breite;
			Bmp[i].rcSrc.bottom = Bmp[i].rcSrc.top + Bmp[i].Hoehe;
		}
		for (i = MUNNULL; i <= MUNZWOELF; i++)
		{
			Bmp[i].Surface = lpDDSMunition;
			Bmp[i].Breite = 10;
			Bmp[i].Hoehe = 10;
			Bmp[i].rcSrc.left = (i - MUNNULL) * 10;
			Bmp[i].rcSrc.top = 0;
			Bmp[i].rcSrc.right = Bmp[i].rcSrc.left + Bmp[i].Breite;
			Bmp[i].rcSrc.bottom = Bmp[i].rcSrc.top + Bmp[i].Hoehe;
		}

		Bmp[POWER].Surface = lpDDSPanzer;
		Bmp[POWER].Breite = 17;
		Bmp[POWER].Hoehe = 3;
		Bmp[POWER].rcSrc.left = 80;
		Bmp[POWER].rcSrc.top = 0;
		Bmp[POWER].rcSrc.right = Bmp[POWER].rcSrc.left + Bmp[POWER].Breite;
		Bmp[POWER].rcSrc.bottom = Bmp[POWER].rcSrc.top + Bmp[POWER].Hoehe;

		Bmp[SCHILD].Surface = lpDDSPanzer;
		Bmp[SCHILD].Breite = 30;
		Bmp[SCHILD].Hoehe = 15;
		Bmp[SCHILD].rcSrc.left = 97;
		Bmp[SCHILD].rcSrc.top = 0;
		Bmp[SCHILD].rcSrc.right = Bmp[SCHILD].rcSrc.left + Bmp[SCHILD].Breite;
		Bmp[SCHILD].rcSrc.bottom = Bmp[SCHILD].rcSrc.top + Bmp[SCHILD].Hoehe;

		Bmp[MUNBOX].Surface = lpDDSPanzer;
		Bmp[MUNBOX].Breite = 20;
		Bmp[MUNBOX].Hoehe = 29;
		Bmp[MUNBOX].rcSrc.left = 130;
		Bmp[MUNBOX].rcSrc.top = 0;
		Bmp[MUNBOX].rcSrc.right = Bmp[MUNBOX].rcSrc.left + Bmp[MUNBOX].Breite;
		Bmp[MUNBOX].rcSrc.bottom = Bmp[MUNBOX].rcSrc.top + Bmp[MUNBOX].Hoehe;

		Bmp[ENERGIE].Surface = lpDDSPanzer;
		Bmp[ENERGIE].Breite = 3;
		Bmp[ENERGIE].Hoehe = 13;
		Bmp[ENERGIE].rcSrc.left = 127;
		Bmp[ENERGIE].rcSrc.top = 0;
		Bmp[ENERGIE].rcSrc.right = Bmp[ENERGIE].rcSrc.left + Bmp[ENERGIE].Breite;
		Bmp[ENERGIE].rcSrc.bottom = Bmp[ENERGIE].rcSrc.top + Bmp[ENERGIE].Hoehe;

		Bmp[TEXFELS].Surface = lpDDSSonstiges;
		Bmp[TEXFELS].Breite = 184;
		Bmp[TEXFELS].Hoehe = 162;
		Bmp[TEXFELS].rcSrc.left = 0;
		Bmp[TEXFELS].rcSrc.top = 0;
		Bmp[TEXFELS].rcSrc.right = Bmp[TEXFELS].rcSrc.left + Bmp[TEXFELS].Breite;
		Bmp[TEXFELS].rcSrc.bottom = Bmp[TEXFELS].rcSrc.top + Bmp[TEXFELS].Hoehe;

		Bmp[WINDHOSE].Surface = lpDDSSonstiges;
		Bmp[WINDHOSE].Breite = 36;
		Bmp[WINDHOSE].Hoehe = 8;
		Bmp[WINDHOSE].rcSrc.left = 184;
		Bmp[WINDHOSE].rcSrc.top = 0;
		Bmp[WINDHOSE].rcSrc.right = Bmp[WINDHOSE].rcSrc.left + Bmp[WINDHOSE].Breite;
		Bmp[WINDHOSE].rcSrc.bottom = Bmp[WINDHOSE].rcSrc.top + Bmp[WINDHOSE].Hoehe;

		Bmp[BALLONBILD].Surface = lpDDSSonstiges;
		Bmp[BALLONBILD].Breite = 32;
		Bmp[BALLONBILD].Hoehe = 55;
		Bmp[BALLONBILD].rcSrc.left = 184;
		Bmp[BALLONBILD].rcSrc.top = 84;
		Bmp[BALLONBILD].rcSrc.right = Bmp[BALLONBILD].rcSrc.left + Bmp[BALLONBILD].Breite;
		Bmp[BALLONBILD].rcSrc.bottom = Bmp[BALLONBILD].rcSrc.top + Bmp[BALLONBILD].Hoehe;

		Bmp[GEWEHR].Surface = lpDDSMunition;
		Bmp[GEWEHR].Breite = 10;
		Bmp[GEWEHR].Hoehe = 10;
		Bmp[GEWEHR].rcSrc.left = (MUNFUENF - MUNNULL) * 10;
		Bmp[GEWEHR].rcSrc.top = 8 * 10;
		Bmp[GEWEHR].rcSrc.right = Bmp[GEWEHR].rcSrc.left + Bmp[GEWEHR].Breite;
		Bmp[GEWEHR].rcSrc.bottom = Bmp[GEWEHR].rcSrc.top + Bmp[GEWEHR].Hoehe;

		Bmp[FASS].Surface = lpDDSMunition;
		Bmp[FASS].Breite = 10;
		Bmp[FASS].Hoehe = 10;
		Bmp[FASS].rcSrc.left = (MUNSECHS - MUNNULL) * 10;
		Bmp[FASS].rcSrc.top = 8 * 10;
		Bmp[FASS].rcSrc.right = Bmp[FASS].rcSrc.left + Bmp[FASS].Breite;
		Bmp[FASS].rcSrc.bottom = Bmp[FASS].rcSrc.top + Bmp[FASS].Hoehe;

		Bmp[MAGNET].Surface = lpDDSPanzer;
		Bmp[MAGNET].Breite = 9;
		Bmp[MAGNET].Hoehe = 12;
		Bmp[MAGNET].rcSrc.left = 80;
		Bmp[MAGNET].rcSrc.top = 3;
		Bmp[MAGNET].rcSrc.right = Bmp[MAGNET].rcSrc.left + Bmp[MAGNET].Breite;
		Bmp[MAGNET].rcSrc.bottom = Bmp[MAGNET].rcSrc.top + Bmp[MAGNET].Hoehe;

		Bmp[MBACK].Surface = lpDDSMBack;
		Bmp[MBACK].Breite = 640;
		Bmp[MBACK].Hoehe = 480;
		Bmp[MBACK].rcSrc.left = 0;
		Bmp[MBACK].rcSrc.top = 0;
		Bmp[MBACK].rcSrc.right = Bmp[MBACK].rcSrc.left + Bmp[MBACK].Breite;
		Bmp[MBACK].rcSrc.bottom = Bmp[MBACK].rcSrc.top + Bmp[MBACK].Hoehe;

		Bmp[MOPTION].Surface = lpDDSMPunkt;
		Bmp[MOPTION].Breite = 158;
		Bmp[MOPTION].Hoehe = 44;
		Bmp[MOPTION].rcSrc.left = 0;
		Bmp[MOPTION].rcSrc.top = 0;
		Bmp[MOPTION].rcSrc.right = Bmp[MOPTION].rcSrc.left + Bmp[MOPTION].Breite;
		Bmp[MOPTION].rcSrc.bottom = Bmp[MOPTION].rcSrc.top + Bmp[MOPTION].Hoehe;

		Bmp[MZURUECK].Surface = lpDDSMPunkt;
		Bmp[MZURUECK].Breite = 123;
		Bmp[MZURUECK].Hoehe = 44;
		Bmp[MZURUECK].rcSrc.left = 0;
		Bmp[MZURUECK].rcSrc.top = 88;
		Bmp[MZURUECK].rcSrc.right = Bmp[MZURUECK].rcSrc.left + Bmp[MZURUECK].Breite;
		Bmp[MZURUECK].rcSrc.bottom = Bmp[MZURUECK].rcSrc.top + Bmp[MZURUECK].Hoehe;

		Bmp[MSTARTEN].Surface = lpDDSMPunkt;
		Bmp[MSTARTEN].Breite = 219;
		Bmp[MSTARTEN].Hoehe = 44;
		Bmp[MSTARTEN].rcSrc.left = 0;
		Bmp[MSTARTEN].rcSrc.top = 176;
		Bmp[MSTARTEN].rcSrc.right = Bmp[MSTARTEN].rcSrc.left + Bmp[MSTARTEN].Breite;
		Bmp[MSTARTEN].rcSrc.bottom = Bmp[MSTARTEN].rcSrc.top + Bmp[MSTARTEN].Hoehe;

		Bmp[MHAUPTMENUE].Surface = lpDDSMPunkt;
		Bmp[MHAUPTMENUE].Breite = 255;
		Bmp[MHAUPTMENUE].Hoehe = 44;
		Bmp[MHAUPTMENUE].rcSrc.left = 0;
		Bmp[MHAUPTMENUE].rcSrc.top = 264;
		Bmp[MHAUPTMENUE].rcSrc.right = Bmp[MHAUPTMENUE].rcSrc.left + Bmp[MHAUPTMENUE].Breite;
		Bmp[MHAUPTMENUE].rcSrc.bottom = Bmp[MHAUPTMENUE].rcSrc.top + Bmp[MHAUPTMENUE].Hoehe;

		Bmp[MOPTIONENG].Surface = lpDDSMPunkt;
		Bmp[MOPTIONENG].Breite = 207;
		Bmp[MOPTIONENG].Hoehe = 44;
		Bmp[MOPTIONENG].rcSrc.left = 0;
		Bmp[MOPTIONENG].rcSrc.top = 308;
		Bmp[MOPTIONENG].rcSrc.right = Bmp[MOPTIONENG].rcSrc.left + Bmp[MOPTIONENG].Breite;
		Bmp[MOPTIONENG].rcSrc.bottom = Bmp[MOPTIONENG].rcSrc.top + Bmp[MOPTIONENG].Hoehe;

		Bmp[MWIND].Surface = lpDDSMPunkt;
		Bmp[MWIND].Breite = 93;
		Bmp[MWIND].Hoehe = 44;
		Bmp[MWIND].rcSrc.left = 0;
		Bmp[MWIND].rcSrc.top = 352;
		Bmp[MWIND].rcSrc.right = Bmp[MWIND].rcSrc.left + Bmp[MWIND].Breite;
		Bmp[MWIND].rcSrc.bottom = Bmp[MWIND].rcSrc.top + Bmp[MWIND].Hoehe;

		Bmp[MAN].Surface = lpDDSMPunkt;
		Bmp[MAN].Breite = 68;
		Bmp[MAN].Hoehe = 44;
		Bmp[MAN].rcSrc.left = 0;
		Bmp[MAN].rcSrc.top = 396;
		Bmp[MAN].rcSrc.right = Bmp[MAN].rcSrc.left + Bmp[MAN].Breite;
		Bmp[MAN].rcSrc.bottom = Bmp[MAN].rcSrc.top + Bmp[MAN].Hoehe;

		Bmp[MWETTER].Surface = lpDDSMPunkt;
		Bmp[MWETTER].Breite = 121;
		Bmp[MWETTER].Hoehe = 44;
		Bmp[MWETTER].rcSrc.left = 0;
		Bmp[MWETTER].rcSrc.top = 572;
		Bmp[MWETTER].rcSrc.right = Bmp[MWETTER].rcSrc.left + Bmp[MWETTER].Breite;
		Bmp[MWETTER].rcSrc.bottom = Bmp[MWETTER].rcSrc.top + Bmp[MWETTER].Hoehe;

		Bmp[MSPIELER].Surface = lpDDSMPunkt;
		Bmp[MSPIELER].Breite = 123;
		Bmp[MSPIELER].Hoehe = 44;
		Bmp[MSPIELER].rcSrc.left = 0;
		Bmp[MSPIELER].rcSrc.top = 616;
		Bmp[MSPIELER].rcSrc.right = Bmp[MSPIELER].rcSrc.left + Bmp[MSPIELER].Breite;
		Bmp[MSPIELER].rcSrc.bottom = Bmp[MSPIELER].rcSrc.top + Bmp[MSPIELER].Hoehe;

		Bmp[MSPIELERG].Surface = lpDDSMPunkt;
		Bmp[MSPIELERG].Breite = 202;
		Bmp[MSPIELERG].Hoehe = 44;
		Bmp[MSPIELERG].rcSrc.left = 0;
		Bmp[MSPIELERG].rcSrc.top = 704;
		Bmp[MSPIELERG].rcSrc.right = Bmp[MSPIELERG].rcSrc.left + Bmp[MSPIELERG].Breite;
		Bmp[MSPIELERG].rcSrc.bottom = Bmp[MSPIELERG].rcSrc.top + Bmp[MSPIELERG].Hoehe;

		Bmp[MMENSCH].Surface = lpDDSMPunkt;
		Bmp[MMENSCH].Breite = 178;
		Bmp[MMENSCH].Hoehe = 44;
		Bmp[MMENSCH].rcSrc.left = 0;
		Bmp[MMENSCH].rcSrc.top = 748;
		Bmp[MMENSCH].rcSrc.right = Bmp[MMENSCH].rcSrc.left + Bmp[MMENSCH].Breite;
		Bmp[MMENSCH].rcSrc.bottom = Bmp[MMENSCH].rcSrc.top + Bmp[MMENSCH].Hoehe;

		Bmp[MROTER].Surface = lpDDSMPunkt;
		Bmp[MROTER].Breite = 229;
		Bmp[MROTER].Hoehe = 44;
		Bmp[MROTER].rcSrc.left = 0;
		Bmp[MROTER].rcSrc.top = 1012;
		Bmp[MROTER].rcSrc.right = Bmp[MROTER].rcSrc.left + Bmp[MROTER].Breite;
		Bmp[MROTER].rcSrc.bottom = Bmp[MROTER].rcSrc.top + Bmp[MROTER].Hoehe;

		Bmp[MGRUENER].Surface = lpDDSMPunkt;
		Bmp[MGRUENER].Breite = 258;
		Bmp[MGRUENER].Hoehe = 44;
		Bmp[MGRUENER].rcSrc.left = 0;
		Bmp[MGRUENER].rcSrc.top = 1056;
		Bmp[MGRUENER].rcSrc.right = Bmp[MGRUENER].rcSrc.left + Bmp[MGRUENER].Breite;
		Bmp[MGRUENER].rcSrc.bottom = Bmp[MGRUENER].rcSrc.top + Bmp[MGRUENER].Hoehe;

		Bmp[MBLAUER].Surface = lpDDSMPunkt;
		Bmp[MBLAUER].Breite = 251;
		Bmp[MBLAUER].Hoehe = 44;
		Bmp[MBLAUER].rcSrc.left = 0;
		Bmp[MBLAUER].rcSrc.top = 1100;
		Bmp[MBLAUER].rcSrc.right = Bmp[MBLAUER].rcSrc.left + Bmp[MBLAUER].Breite;
		Bmp[MBLAUER].rcSrc.bottom = Bmp[MBLAUER].rcSrc.top + Bmp[MBLAUER].Hoehe;

		Bmp[MGELBER].Surface = lpDDSMPunkt;
		Bmp[MGELBER].Breite = 250;
		Bmp[MGELBER].Hoehe = 44;
		Bmp[MGELBER].rcSrc.left = 0;
		Bmp[MGELBER].rcSrc.top = 1144;
		Bmp[MGELBER].rcSrc.right = Bmp[MGELBER].rcSrc.left + Bmp[MGELBER].Breite;
		Bmp[MGELBER].rcSrc.bottom = Bmp[MGELBER].rcSrc.top + Bmp[MGELBER].Hoehe;

		Bmp[MSBACK].Surface = lpDDSMSBack;
		Bmp[MSBACK].Breite = 640;
		Bmp[MSBACK].Hoehe = 480;
		Bmp[MSBACK].rcSrc.left = 0;
		Bmp[MSBACK].rcSrc.top = 0;
		Bmp[MSBACK].rcSrc.right = Bmp[MSBACK].rcSrc.left + Bmp[MSBACK].Breite;
		Bmp[MSBACK].rcSrc.bottom = Bmp[MSBACK].rcSrc.top + Bmp[MSBACK].Hoehe;

		Bmp[MWEITER].Surface = lpDDSMPunkt;
		Bmp[MWEITER].Breite = 114;
		Bmp[MWEITER].Hoehe = 44;
		Bmp[MWEITER].rcSrc.left = 0;
		Bmp[MWEITER].rcSrc.top = 1188;
		Bmp[MWEITER].rcSrc.right = Bmp[MWEITER].rcSrc.left + Bmp[MWEITER].Breite;
		Bmp[MWEITER].rcSrc.bottom = Bmp[MWEITER].rcSrc.top + Bmp[MWEITER].Hoehe;

		Bmp[MKAUFEN].Surface = lpDDSMPunkt;
		Bmp[MKAUFEN].Breite = 123;
		Bmp[MKAUFEN].Hoehe = 44;
		Bmp[MKAUFEN].rcSrc.left = 0;
		Bmp[MKAUFEN].rcSrc.top = 1276;
		Bmp[MKAUFEN].rcSrc.right = Bmp[MKAUFEN].rcSrc.left + Bmp[MKAUFEN].Breite;
		Bmp[MKAUFEN].rcSrc.bottom = Bmp[MKAUFEN].rcSrc.top + Bmp[MKAUFEN].Hoehe;

		Bmp[MPFEILH].Surface = lpDDSSonstiges;
		Bmp[MPFEILH].Breite = 15;
		Bmp[MPFEILH].Hoehe = 14;
		Bmp[MPFEILH].rcSrc.left = 184;
		Bmp[MPFEILH].rcSrc.top = 56;
		Bmp[MPFEILH].rcSrc.right = Bmp[MPFEILH].rcSrc.left + Bmp[MPFEILH].Breite;
		Bmp[MPFEILH].rcSrc.bottom = Bmp[MPFEILH].rcSrc.top + Bmp[MPFEILH].Hoehe;

		Bmp[MPFEILR].Surface = lpDDSSonstiges;
		Bmp[MPFEILR].Breite = 15;
		Bmp[MPFEILR].Hoehe = 14;
		Bmp[MPFEILR].rcSrc.left = 199;
		Bmp[MPFEILR].rcSrc.top = 56;
		Bmp[MPFEILR].rcSrc.right = Bmp[MPFEILR].rcSrc.left + Bmp[MPFEILR].Breite;
		Bmp[MPFEILR].rcSrc.bottom = Bmp[MPFEILR].rcSrc.top + Bmp[MPFEILR].Hoehe;

		Bmp[MOBACK].Surface = lpDDSSonstiges;
		Bmp[MOBACK].Breite = 300;
		Bmp[MOBACK].Hoehe = 195;
		Bmp[MOBACK].rcSrc.left = 0;
		Bmp[MOBACK].rcSrc.top = 162;
		Bmp[MOBACK].rcSrc.right = Bmp[MOBACK].rcSrc.left + Bmp[MOBACK].Breite;
		Bmp[MOBACK].rcSrc.bottom = Bmp[MOBACK].rcSrc.top + Bmp[MOBACK].Hoehe;

		Bmp[MRUNDE].Surface = lpDDSMPunkt;
		Bmp[MRUNDE].Breite = 113;
		Bmp[MRUNDE].Hoehe = 44;
		Bmp[MRUNDE].rcSrc.left = 0;
		Bmp[MRUNDE].rcSrc.top = 1364;
		Bmp[MRUNDE].rcSrc.right = Bmp[MRUNDE].rcSrc.left + Bmp[MRUNDE].Breite;
		Bmp[MRUNDE].rcSrc.bottom = Bmp[MRUNDE].rcSrc.top + Bmp[MRUNDE].Hoehe;

		Bmp[MRUNDEN].Surface = lpDDSMPunkt;
		Bmp[MRUNDEN].Breite = 135;
		Bmp[MRUNDEN].Hoehe = 44;
		Bmp[MRUNDEN].rcSrc.left = 0;
		Bmp[MRUNDEN].rcSrc.top = 1408;
		Bmp[MRUNDEN].rcSrc.right = Bmp[MRUNDEN].rcSrc.left + Bmp[MRUNDEN].Breite;
		Bmp[MRUNDEN].rcSrc.bottom = Bmp[MRUNDEN].rcSrc.top + Bmp[MRUNDEN].Hoehe;

		Bmp[MHATDIESE].Surface = lpDDSMPunkt;
		Bmp[MHATDIESE].Breite = 163;
		Bmp[MHATDIESE].Hoehe = 44;
		Bmp[MHATDIESE].rcSrc.left = 0;
		Bmp[MHATDIESE].rcSrc.top = 1452;
		Bmp[MHATDIESE].rcSrc.right = Bmp[MHATDIESE].rcSrc.left + Bmp[MHATDIESE].Breite;
		Bmp[MHATDIESE].rcSrc.bottom = Bmp[MHATDIESE].rcSrc.top + Bmp[MHATDIESE].Hoehe;

		Bmp[MGEWONNEN].Surface = lpDDSMPunkt;
		Bmp[MGEWONNEN].Breite = 182;
		Bmp[MGEWONNEN].Hoehe = 44;
		Bmp[MGEWONNEN].rcSrc.left = 0;
		Bmp[MGEWONNEN].rcSrc.top = 1496;
		Bmp[MGEWONNEN].rcSrc.right = Bmp[MGEWONNEN].rcSrc.left + Bmp[MGEWONNEN].Breite;
		Bmp[MGEWONNEN].rcSrc.bottom = Bmp[MGEWONNEN].rcSrc.top + Bmp[MGEWONNEN].Hoehe;

		Bmp[MGBACK].Surface = lpDDSMGBack;
		Bmp[MGBACK].Breite = 640;
		Bmp[MGBACK].Hoehe = 480;
		Bmp[MGBACK].rcSrc.left = 0;
		Bmp[MGBACK].rcSrc.top = 0;
		Bmp[MGBACK].rcSrc.right = Bmp[MGBACK].rcSrc.left + Bmp[MGBACK].Breite;
		Bmp[MGBACK].rcSrc.bottom = Bmp[MGBACK].rcSrc.top + Bmp[MGBACK].Hoehe;

		Bmp[MKEYBOARD].Surface = lpDDSMKeyboard;
		Bmp[MKEYBOARD].Breite = 524;
		Bmp[MKEYBOARD].Hoehe = 128;
		Bmp[MKEYBOARD].rcSrc.left = 0;
		Bmp[MKEYBOARD].rcSrc.top = 0;
		Bmp[MKEYBOARD].rcSrc.right = Bmp[MKEYBOARD].rcSrc.left + Bmp[MKEYBOARD].Breite;
		Bmp[MKEYBOARD].rcSrc.bottom = Bmp[MKEYBOARD].rcSrc.top + Bmp[MKEYBOARD].Hoehe;

		Bmp[MTASTEN].Surface = lpDDSMPunkt;
		Bmp[MTASTEN].Breite = 125;
		Bmp[MTASTEN].Hoehe = 44;
		Bmp[MTASTEN].rcSrc.left = 0;
		Bmp[MTASTEN].rcSrc.top = 1540;
		Bmp[MTASTEN].rcSrc.right = Bmp[MTASTEN].rcSrc.left + Bmp[MTASTEN].Breite;
		Bmp[MTASTEN].rcSrc.bottom = Bmp[MTASTEN].rcSrc.top + Bmp[MTASTEN].Hoehe;

		Bmp[MTASTENG].Surface = lpDDSMPunkt;
		Bmp[MTASTENG].Breite = 164;
		Bmp[MTASTENG].Hoehe = 44;
		Bmp[MTASTENG].rcSrc.left = 0;
		Bmp[MTASTENG].rcSrc.top = 1628;
		Bmp[MTASTENG].rcSrc.right = Bmp[MTASTENG].rcSrc.left + Bmp[MTASTENG].Breite;
		Bmp[MTASTENG].rcSrc.bottom = Bmp[MTASTENG].rcSrc.top + Bmp[MTASTENG].Hoehe;

		Bmp[MBEENDEN].Surface = lpDDSMPunkt;
		Bmp[MBEENDEN].Breite = 150;
		Bmp[MBEENDEN].Hoehe = 44;
		Bmp[MBEENDEN].rcSrc.left = 0;
		Bmp[MBEENDEN].rcSrc.top = 1672;
		Bmp[MBEENDEN].rcSrc.right = Bmp[MBEENDEN].rcSrc.left + Bmp[MBEENDEN].Breite;
		Bmp[MBEENDEN].rcSrc.bottom = Bmp[MBEENDEN].rcSrc.top + Bmp[MBEENDEN].Hoehe;

		Bmp[MLADESPIEL].Surface = lpDDSMPunkt;
		Bmp[MLADESPIEL].Breite = 220;
		Bmp[MLADESPIEL].Hoehe = 44;
		Bmp[MLADESPIEL].rcSrc.left = 0;
		Bmp[MLADESPIEL].rcSrc.top = 1760;
		Bmp[MLADESPIEL].rcSrc.right = Bmp[MLADESPIEL].rcSrc.left + Bmp[MLADESPIEL].Breite;
		Bmp[MLADESPIEL].rcSrc.bottom = Bmp[MLADESPIEL].rcSrc.top + Bmp[MLADESPIEL].Hoehe;

		Bmp[MSOUND].Surface = lpDDSMPunkt;
		Bmp[MSOUND].Breite = 115;
		Bmp[MSOUND].Hoehe = 44;
		Bmp[MSOUND].rcSrc.left = 0;
		Bmp[MSOUND].rcSrc.top = 1804;
		Bmp[MSOUND].rcSrc.right = Bmp[MSOUND].rcSrc.left + Bmp[MSOUND].Breite;
		Bmp[MSOUND].rcSrc.bottom = Bmp[MSOUND].rcSrc.top + Bmp[MSOUND].Hoehe;

		Bmp[MCREDITS].Surface = lpDDSMPunkt;
		Bmp[MCREDITS].Breite = 130;
		Bmp[MCREDITS].Hoehe = 44;
		Bmp[MCREDITS].rcSrc.left = 0;
		Bmp[MCREDITS].rcSrc.top = 1892;
		Bmp[MCREDITS].rcSrc.right = Bmp[MCREDITS].rcSrc.left + Bmp[MCREDITS].Breite;
		Bmp[MCREDITS].rcSrc.bottom = Bmp[MCREDITS].rcSrc.top + Bmp[MCREDITS].Hoehe;

		Bmp[MBBUNKER].Surface = lpDDSMPunkt;
		Bmp[MBBUNKER].Breite = 189;
		Bmp[MBBUNKER].Hoehe = 98;
		Bmp[MBBUNKER].rcSrc.left = 0;
		Bmp[MBBUNKER].rcSrc.top = 1988;
		Bmp[MBBUNKER].rcSrc.right = Bmp[MBBUNKER].rcSrc.left + Bmp[MBBUNKER].Breite;
		Bmp[MBBUNKER].rcSrc.bottom = Bmp[MBBUNKER].rcSrc.top + Bmp[MBBUNKER].Hoehe;

		Bmp[MABUNKER].Surface = lpDDSMPunkt;
		Bmp[MABUNKER].Breite = 181;
		Bmp[MABUNKER].Hoehe = 115;
		Bmp[MABUNKER].rcSrc.left = 0;
		Bmp[MABUNKER].rcSrc.top = 2084;
		Bmp[MABUNKER].rcSrc.right = Bmp[MABUNKER].rcSrc.left + Bmp[MABUNKER].Breite;
		Bmp[MABUNKER].rcSrc.bottom = Bmp[MABUNKER].rcSrc.top + Bmp[MABUNKER].Hoehe;

		Bmp[CRPROGRAMMIERUNG].Surface = lpDDSCredits;
		Bmp[CRPROGRAMMIERUNG].Breite = 340;
		Bmp[CRPROGRAMMIERUNG].Hoehe = 48;
		Bmp[CRPROGRAMMIERUNG].rcSrc.left = 0;
		Bmp[CRPROGRAMMIERUNG].rcSrc.top = 0;
		Bmp[CRPROGRAMMIERUNG].rcSrc.right = Bmp[CRPROGRAMMIERUNG].rcSrc.left + Bmp[CRPROGRAMMIERUNG].Breite;
		Bmp[CRPROGRAMMIERUNG].rcSrc.bottom = Bmp[CRPROGRAMMIERUNG].rcSrc.top + Bmp[CRPROGRAMMIERUNG].Hoehe;

		Bmp[CRDIRKPLATE].Surface = lpDDSCredits;
		Bmp[CRDIRKPLATE].Breite = 242;
		Bmp[CRDIRKPLATE].Hoehe = 48;
		Bmp[CRDIRKPLATE].rcSrc.left = 0;
		Bmp[CRDIRKPLATE].rcSrc.top = 48;
		Bmp[CRDIRKPLATE].rcSrc.right = Bmp[CRDIRKPLATE].rcSrc.left + Bmp[CRDIRKPLATE].Breite;
		Bmp[CRDIRKPLATE].rcSrc.bottom = Bmp[CRDIRKPLATE].rcSrc.top + Bmp[CRDIRKPLATE].Hoehe;

		Bmp[CRBETATESTER].Surface = lpDDSCredits;
		Bmp[CRBETATESTER].Breite = 242;
		Bmp[CRBETATESTER].Hoehe = 48;
		Bmp[CRBETATESTER].rcSrc.left = 0;
		Bmp[CRBETATESTER].rcSrc.top = 96;
		Bmp[CRBETATESTER].rcSrc.right = Bmp[CRBETATESTER].rcSrc.left + Bmp[CRBETATESTER].Breite;
		Bmp[CRBETATESTER].rcSrc.bottom = Bmp[CRBETATESTER].rcSrc.top + Bmp[CRBETATESTER].Hoehe;

		Bmp[CRMATTHIASBUCHETICS].Surface = lpDDSCredits;
		Bmp[CRMATTHIASBUCHETICS].Breite = 437;
		Bmp[CRMATTHIASBUCHETICS].Hoehe = 48;
		Bmp[CRMATTHIASBUCHETICS].rcSrc.left = 0;
		Bmp[CRMATTHIASBUCHETICS].rcSrc.top = 144;
		Bmp[CRMATTHIASBUCHETICS].rcSrc.right = Bmp[CRMATTHIASBUCHETICS].rcSrc.left + Bmp[CRMATTHIASBUCHETICS].Breite;
		Bmp[CRMATTHIASBUCHETICS].rcSrc.bottom = Bmp[CRMATTHIASBUCHETICS].rcSrc.top + Bmp[CRMATTHIASBUCHETICS].Hoehe;

		Bmp[CRMARCEBERHARDT].Surface = lpDDSCredits;
		Bmp[CRMARCEBERHARDT].Breite = 340;
		Bmp[CRMARCEBERHARDT].Hoehe = 48;
		Bmp[CRMARCEBERHARDT].rcSrc.left = 0;
		Bmp[CRMARCEBERHARDT].rcSrc.top = 192;
		Bmp[CRMARCEBERHARDT].rcSrc.right = Bmp[CRMARCEBERHARDT].rcSrc.left + Bmp[CRMARCEBERHARDT].Breite;
		Bmp[CRMARCEBERHARDT].rcSrc.bottom = Bmp[CRMARCEBERHARDT].rcSrc.top + Bmp[CRMARCEBERHARDT].Hoehe;

		Bmp[CRSOUND].Surface = lpDDSCredits;
		Bmp[CRSOUND].Breite = 124;
		Bmp[CRSOUND].Hoehe = 48;
		Bmp[CRSOUND].rcSrc.left = 0;
		Bmp[CRSOUND].rcSrc.top = 240;
		Bmp[CRSOUND].rcSrc.right = Bmp[CRSOUND].rcSrc.left + Bmp[CRSOUND].Breite;
		Bmp[CRSOUND].rcSrc.bottom = Bmp[CRSOUND].rcSrc.top + Bmp[CRSOUND].Hoehe;

		Bmp[CRMUSIK].Surface = lpDDSCredits;
		Bmp[CRMUSIK].Breite = 127;
		Bmp[CRMUSIK].Hoehe = 48;
		Bmp[CRMUSIK].rcSrc.left = 0;
		Bmp[CRMUSIK].rcSrc.top = 288;
		Bmp[CRMUSIK].rcSrc.right = Bmp[CRMUSIK].rcSrc.left + Bmp[CRMUSIK].Breite;
		Bmp[CRMUSIK].rcSrc.bottom = Bmp[CRMUSIK].rcSrc.top + Bmp[CRMUSIK].Hoehe;

		Bmp[CRHEIKOPLATE].Surface = lpDDSCredits;
		Bmp[CRHEIKOPLATE].Breite = 270;
		Bmp[CRHEIKOPLATE].Hoehe = 48;
		Bmp[CRHEIKOPLATE].rcSrc.left = 0;
		Bmp[CRHEIKOPLATE].rcSrc.top = 336;
		Bmp[CRHEIKOPLATE].rcSrc.right = Bmp[CRHEIKOPLATE].rcSrc.left + Bmp[CRHEIKOPLATE].Breite;
		Bmp[CRHEIKOPLATE].rcSrc.bottom = Bmp[CRHEIKOPLATE].rcSrc.top + Bmp[CRHEIKOPLATE].Hoehe;

		Bmp[CRGRAFIK].Surface = lpDDSCredits;
		Bmp[CRGRAFIK].Breite = 150;
		Bmp[CRGRAFIK].Hoehe = 48;
		Bmp[CRGRAFIK].rcSrc.left = 0;
		Bmp[CRGRAFIK].rcSrc.top = 384;
		Bmp[CRGRAFIK].rcSrc.right = Bmp[CRGRAFIK].rcSrc.left + Bmp[CRGRAFIK].Breite;
		Bmp[CRGRAFIK].rcSrc.bottom = Bmp[CRGRAFIK].rcSrc.top + Bmp[CRGRAFIK].Hoehe;

		Bmp[CRDPSOFTWARE].Surface = lpDDSCredits;
		Bmp[CRDPSOFTWARE].Breite = 291;
		Bmp[CRDPSOFTWARE].Hoehe = 99;
		Bmp[CRDPSOFTWARE].rcSrc.left = 0;
		Bmp[CRDPSOFTWARE].rcSrc.top = 437;
		Bmp[CRDPSOFTWARE].rcSrc.right = Bmp[CRDPSOFTWARE].rcSrc.left + Bmp[CRDPSOFTWARE].Breite;
		Bmp[CRDPSOFTWARE].rcSrc.bottom = Bmp[CRDPSOFTWARE].rcSrc.top + Bmp[CRDPSOFTWARE].Hoehe;

		// Sounds
		for (i = 0; i < WAVANZ; i++)
		{
			Wav[i].Dateiname = "sounds/klick.wav";
			Wav[i].Volume = 100;
		}
		Wav[WAVKLICK].Dateiname = "sounds/klick.wav";
		Wav[WAVKLICK].Volume = 100;

		Wav[WAVRAKETE].Dateiname = "sounds/rakete.wav";
		Wav[WAVRAKETE].Volume = 95;

		Wav[WAVMOUSEEINS].Dateiname = "sounds/mouse1.wav";
		Wav[WAVMOUSEEINS].Volume = 100;

		Wav[WAVEXPLOSIONG].Dateiname = "sounds/explosiong.wav";
		Wav[WAVEXPLOSIONG].Volume = 100;

		Wav[WAVGEWEHR].Dateiname = "sounds/gewehr.wav";
		Wav[WAVGEWEHR].Volume = 100;

		Wav[WAVQUERSCHUSS].Dateiname = "sounds/querschuss.wav";
		Wav[WAVQUERSCHUSS].Volume = 100;

		Wav[WAVGRANATE].Dateiname = "sounds/granate.wav";
		Wav[WAVGRANATE].Volume = 100;

		Wav[WAVSPLITTER].Dateiname = "sounds/splitter.wav";
		Wav[WAVSPLITTER].Volume = 100;

		Wav[WAVMAGNETSCHILD].Dateiname = "sounds/magnetschild.wav";
		Wav[WAVMAGNETSCHILD].Volume = 100;

		Wav[WAVSTEIN].Dateiname = "sounds/stein.wav";
		Wav[WAVSTEIN].Volume = 100;

		Wav[WAVSCHILD].Dateiname = "sounds/schild.wav";
		Wav[WAVSCHILD].Volume = 100;

		Wav[WAVKNISTERN].Dateiname = "sounds/knistern.wav";
		Wav[WAVKNISTERN].Volume = 100;

		Wav[WAVLASER].Dateiname = "sounds/laser.wav";
		Wav[WAVLASER].Volume = 100;

		Wav[WAVKASSE].Dateiname = "sounds/kasse.wav";
		Wav[WAVKASSE].Volume = 100;

		Wav[WAVSAEURE].Dateiname = "sounds/saeure.wav";
		Wav[WAVSAEURE].Volume = 100;

		Wav[WAVPANZER].Dateiname = "sounds/panzer.wav";
		Wav[WAVPANZER].Volume = 100;

		Wav[WAVFALLSCHIRM].Dateiname = "sounds/fallschirm.wav";
		Wav[WAVFALLSCHIRM].Volume = 100;

		Wav[WAVFALLSCHIRMZWEI].Dateiname = "sounds/fallschirm2.wav";
		Wav[WAVFALLSCHIRMZWEI].Volume = 100;

		Wav[WAVYIPPEE].Dateiname = "sounds/yippee.wav";
		Wav[WAVYIPPEE].Volume = 100;

		Wav[WAVAPPLAUS].Dateiname = "sounds/applaus.wav";
		Wav[WAVAPPLAUS].Volume = 100;

		Wav[WAVCREDITS].Dateiname = "sounds/credits.wav";
		Wav[WAVCREDITS].Volume = 90;

		Wav[WAVCREDITSMUSIK].Dateiname = "sounds/creditsmusik.wav";
		Wav[WAVCREDITSMUSIK].Volume = 100;

		Wav[WAVTITELMUSIK].Dateiname = "sounds/titelmusik.wav";
		Wav[WAVTITELMUSIK].Volume = 100;

		for (i = 0; i < WAVANZ; i++)
			LoadSound(i);

		// Gesteinsarten (und sonstige physikalische Objekte
		for (i = 0; i < ARTANZAHL; i++)
		{
			Art[i].Farbe.r = 255;
			Art[i].Farbe.g = 0;
			Art[i].Farbe.b = 255;
			Art[i].m = 0;
			Art[i].Reibung = 0;
		}
		Art[ARTNICHTS].Farbe.r = 255;
		Art[ARTNICHTS].Farbe.g = 0;
		Art[ARTNICHTS].Farbe.b = 255;
		Art[ARTNICHTS].m = 0;
		Art[ARTNICHTS].Reibung = 0.3;

		Art[ARTGRASS].Farbe.r = 0;
		Art[ARTGRASS].Farbe.g = 150;
		Art[ARTGRASS].Farbe.b = 0;
		Art[ARTGRASS].m = 50;
		Art[ARTGRASS].Reibung = 2.0;

		Art[ARTWASSER].Farbe.r = 100;
		Art[ARTWASSER].Farbe.g = 100;
		Art[ARTWASSER].Farbe.b = 255;
		Art[ARTWASSER].m = 100;
		Art[ARTWASSER].Reibung = 0;

		Art[ARTSTEIN].Farbe.r = 150;
		Art[ARTSTEIN].Farbe.g = 150;
		Art[ARTSTEIN].Farbe.b = 150;
		Art[ARTSTEIN].m = 200;
		Art[ARTSTEIN].Reibung = 3.0;

		Art[ARTSCHNEE].Farbe.r = 255;
		Art[ARTSCHNEE].Farbe.g = 255;
		Art[ARTSCHNEE].Farbe.b = 255;
		Art[ARTSCHNEE].m = 20;
		Art[ARTSCHNEE].Reibung = 1.0;

		Art[ARTERDE].Farbe.r = 140;
		Art[ARTERDE].Farbe.g = 111;
		Art[ARTERDE].Farbe.b = 50;
		Art[ARTERDE].m = 80;
		Art[ARTERDE].Reibung = 1.5;

		Art[ARTPANZER].Farbe.r = 255;
		Art[ARTPANZER].Farbe.g = 0;
		Art[ARTPANZER].Farbe.b = 0;
		Art[ARTPANZER].m = 500;
		Art[ARTPANZER].Reibung = 2.0;

		Art[ARTMUNITION].Farbe.r = 255;
		Art[ARTMUNITION].Farbe.g = 0;
		Art[ARTMUNITION].Farbe.b = 0;
		Art[ARTMUNITION].m = 100;
		Art[ARTMUNITION].Reibung = 1.0;

		Art[ARTSAEURE].Farbe.r = 0;
		Art[ARTSAEURE].Farbe.g = 255;
		Art[ARTSAEURE].Farbe.b = 0;
		Art[ARTSAEURE].m = 100;
		Art[ARTSAEURE].Reibung = 0;

		// Munition
		for (i = 0; i < MUNANZAHL; i++)
		{
			Munition[i].Bild = 0;
			Munition[i].Explosion = EXPNICHTS;
			Munition[i].Smoke[0] = SMOKENICHTS;
			Munition[i].Smoke[1] = SMOKENICHTS;
			Munition[i].MaxDamage = 0;
			Munition[i].Dauer = -1;
			Munition[i].Preis = 0;
			Munition[i].FlugSound = audio::NO_SOUND;
			Munition[i].AbschussSound = audio::NO_SOUND;
		};
		Munition[MUNRAKETE].Bild = MUNNULL;
		Munition[MUNRAKETE].Explosion = EXPEXPLOSION;
		Munition[MUNRAKETE].Smoke[0] = SMOKERAUCH;
		Munition[MUNRAKETE].Smoke[1] = SMOKEFEUER;
		Munition[MUNRAKETE].MaxDamage = 300;
		Munition[MUNRAKETE].Preis = 70;
		Munition[MUNRAKETE].FlugSound = audio::ROCKET;

		Munition[MUNSTEIN].Bild = MUNZWEI;
		Munition[MUNSTEIN].Explosion = EXPKRATER;
		Munition[MUNSTEIN].MaxDamage = 200;
		Munition[MUNSTEIN].Preis = 0;

		Munition[MUNSCHILD].Bild = MUNEINS;
		Munition[MUNSCHILD].Dauer = 500;
		Munition[MUNSCHILD].Preis = 200;
		Munition[MUNSCHILD].AbschussSound = audio::SHIELD;

		Munition[MUNGRANATE].Bild = MUNDREI;
		Munition[MUNGRANATE].Explosion = EXPGRANATE;
		Munition[MUNGRANATE].Smoke[0] = SMOKEFEUER;
		Munition[MUNGRANATE].MaxDamage = 300;
		Munition[MUNGRANATE].Dauer = 100;
		Munition[MUNGRANATE].Preis = 80;
		Munition[MUNGRANATE].FlugSound = audio::CRACKLING;

		Munition[MUNLASER].Bild = MUNVIER;
		Munition[MUNLASER].MaxDamage = 500;
		Munition[MUNLASER].Dauer = 10;
		Munition[MUNLASER].Preis = 250;
		Munition[MUNLASER].AbschussSound = audio::LASER;

		Munition[MUNGEWEHR].Bild = MUNFUENF;
		Munition[MUNGEWEHR].Explosion = EXPSPLITTER;
		Munition[MUNGEWEHR].MaxDamage = 300;
		Munition[MUNGEWEHR].Preis = 150;
		Munition[MUNGEWEHR].AbschussSound = audio::RIFLE;

		Munition[MUNFASS].Bild = MUNSECHS;
		Munition[MUNFASS].Explosion = EXPFASS;
		Munition[MUNFASS].MaxDamage = 100;
		Munition[MUNFASS].Dauer = 200;
		Munition[MUNFASS].Preis = 130;

		Munition[MUNMEDI].Bild = MUNSIEBEN;
		Munition[MUNMEDI].Preis = 500;

		Munition[MUNMAGRAKETE].Bild = MUNACHT;
		Munition[MUNMAGRAKETE].Explosion = EXPEXPLOSION;
		Munition[MUNMAGRAKETE].Smoke[0] = SMOKERAUCH;
		Munition[MUNMAGRAKETE].Smoke[1] = SMOKEFEUER;
		Munition[MUNMAGRAKETE].MaxDamage = 300;
		Munition[MUNMAGRAKETE].Preis = 155;
		Munition[MUNMAGRAKETE].FlugSound = audio::ROCKET;

		Munition[MUNMAGNET].Bild = MUNNEUN;
		Munition[MUNMAGNET].Dauer = 500;
		Munition[MUNMAGNET].Preis = 180;
		Munition[MUNMAGNET].AbschussSound = audio::MAGNET_SHIELD;

		Munition[MUNMEGA].Bild = MUNZEHN;
		Munition[MUNMEGA].Explosion = EXPMEGA;
		Munition[MUNMEGA].MaxDamage = 0;
		Munition[MUNMEGA].Dauer = 300;
		Munition[MUNMEGA].Preis = 850;

		Munition[MUNSCHNEEBALL].Bild = MUNELF;
		Munition[MUNSCHNEEBALL].Explosion = EXPSCHNEEBALL;
		Munition[MUNSCHNEEBALL].MaxDamage = 0;
		Munition[MUNSCHNEEBALL].Dauer = 300;
		Munition[MUNSCHNEEBALL].Preis = 25;

		Munition[MUNSPLITTER].Bild = MUNZWOELF;
		Munition[MUNSPLITTER].Explosion = EXPSPLITTER;
		Munition[MUNSPLITTER].MaxDamage = 100;
		Munition[MUNSPLITTER].Smoke[0] = SMOKERAUCH;

		// Menues einstellen
		AktMenue = MENMAIN;
		Menue[MENMAIN].setzeBackground(MBACK);
		Menue[MENMAIN].setzeMenuepunkt(190, 40, MHAUPTMENUE, false, -1, 0, -1, -1);
		Menue[MENMAIN].setzeMenuepunkt(80, 120, MSTARTEN, true, -3, 0, -1, -1);
		Menue[MENMAIN].setzeMenuepunkt(80, 170, MOPTION, true, MENOPTION, 0, -1, -1);
		Menue[MENMAIN].setzeMenuepunkt(80, 220, MSPIELER, true, MENSPIELER, 0, -1, -1);
		Menue[MENMAIN].setzeMenuepunkt(80, 270, MCREDITS, true, -4, 0, -1, -1);
#ifndef __EMSCRIPTEN__ // Emscripten does not support exiting the game
		Menue[MENMAIN].setzeMenuepunkt(80, 350, MBEENDEN, true, -2, 0, -1, -1);
#endif
		Menue[MENMAIN].setzeMenuepunkt(350, 250, MBBUNKER, false, -1, 0, -1, -1);

		Menue[MENOPTION].setzeBackground(MBACK);
		Menue[MENOPTION].setzeMenuepunkt(210, 40, MOPTIONENG, false, -1, 0, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(80, 120, MWIND, false, -1, 0, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(250, 120, MAN, true, -1, 2, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(80, 170, MWETTER, false, -1, 0, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(250, 170, MAN, true, -1, 2, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(80, 220, MRUNDEN, false, -1, 0, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(250, 225, -1, false, -1, 0, 0, 1);
		Menue[MENOPTION].setzeMenuepunkt(288, 226, MPFEILH, true, -1, 0, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(288, 241, MPFEILR, true, -1, 0, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(80, 270, MSOUND, false, -1, 0, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(250, 270, MAN, true, -1, 2, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(80, 350, MZURUECK, true, MENMAIN, 0, -1, -1);
		Menue[MENOPTION].setzeMenuepunkt(400, 200, MABUNKER, false, -1, 0, -1, -1);

		Menue[MENSPIELER].setzeBackground(MBACK);
		Menue[MENSPIELER].setzeMenuepunkt(210, 40, MSPIELERG, false, -1, 0, -1, -1);
		Menue[MENSPIELER].setzeMenuepunkt(80, 120, MROTER, false, -1, 0, -1, -1);
		Menue[MENSPIELER].setzeMenuepunkt(350, 120, MMENSCH, true, -1, 3, -1, -1);
		Menue[MENSPIELER].setzeMenuepunkt(80, 170, MGRUENER, false, -1, 0, -1, -1);
		Menue[MENSPIELER].setzeMenuepunkt(350, 170, MMENSCH, true, -1, 3, -1, -1);
		Menue[MENSPIELER].setzeMenuepunkt(80, 220, MBLAUER, false, -1, 0, -1, -1);
		Menue[MENSPIELER].setzeMenuepunkt(350, 220, MMENSCH, true, -1, 3, -1, -1);
		Menue[MENSPIELER].setzeMenuepunkt(80, 270, MGELBER, false, -1, 0, -1, -1);
		Menue[MENSPIELER].setzeMenuepunkt(350, 270, MMENSCH, true, -1, 3, -1, -1);
		Menue[MENSPIELER].setzeMenuepunkt(80, 350, MZURUECK, true, MENMAIN, 0, -1, -1);
		Menue[MENSPIELER].setzeMenuepunkt(410, 350, MTASTEN, true, MENTASTEN, 0, -1, -1);

		Menue[MENTASTEN].setzeBackground(MBACK);
		Menue[MENTASTEN].setzeMenuepunkt(230, 40, MTASTENG, false, -1, 0, -1, -1);
		Menue[MENTASTEN].setzeMenuepunkt(60, 150, MKEYBOARD, false, -1, 0, -1, -1);
		Menue[MENTASTEN].setzeMenuepunkt(80, 350, MZURUECK, true, MENSPIELER, 0, -1, -1);

		Menue[MENSHOP].setzeBackground(MSBACK);
		Menue[MENSHOP].setzeMenuepunkt(200, 10, MROTER, false, -1, 0, -1, -1);
		for (i = 0; i < MUNANZAHL - 2; i++)
		{

			if (i < 6)
			{
				Menue[MENSHOP].setzeMenuepunkt(37, 124 + i * 39, -1, false, -1, 0, 0, 3);
				if (MUNNULL + i == Munition[MUNGEWEHR].Bild)
					Menue[MENSHOP].setzeMenuepunkt(103, 128 + i * 39, GEWEHR, false, -1, 0, -1, -1);
				else
					Menue[MENSHOP].setzeMenuepunkt(103, 128 + i * 39, MUNNULL + i, false, -1, 0, -1, -1);
				Menue[MENSHOP].setzeMenuepunkt(136, 124 + i * 39, -1, false, -1, 0, Munition[i + 1].Preis, 3);
				Menue[MENSHOP].setzeMenuepunkt(245, 124 + i * 39, -1, false, -1, 0, 0, 3);
				Menue[MENSHOP].setzeMenuepunkt(283, 120 + i * 39, MPFEILH, true, -1, 0, -1, -1);
				Menue[MENSHOP].setzeMenuepunkt(283, 135 + i * 39, MPFEILR, true, -1, 0, -1, -1);
			}
			else
			{
				Menue[MENSHOP].setzeMenuepunkt(37 + 320, 124 + i * 39 - 234, -1, false, -1, 0, 0, 3);
				if (MUNNULL + i == Munition[MUNFASS].Bild)
					Menue[MENSHOP].setzeMenuepunkt(103 + 320, 128 + i * 39 - 234, FASS, false, -1, 0, -1, -1);
				else
					Menue[MENSHOP].setzeMenuepunkt(103 + 320, 128 + i * 39 - 234, MUNNULL + i, false, -1, 0, -1, -1);
				Menue[MENSHOP].setzeMenuepunkt(136 + 320, 124 + i * 39 - 234, -1, false, -1, 0, Munition[i + 1].Preis, 3);
				Menue[MENSHOP].setzeMenuepunkt(245 + 320, 124 + i * 39 - 234, -1, false, -1, 0, 0, 3);
				Menue[MENSHOP].setzeMenuepunkt(283 + 320, 120 + i * 39 - 234, MPFEILH, true, -1, 0, -1, -1);
				Menue[MENSHOP].setzeMenuepunkt(283 + 320, 135 + i * 39 - 234, MPFEILR, true, -1, 0, -1, -1);
			}
		}
		Menue[MENSHOP].setzeMenuepunkt(136, 379, -1, false, -1, 0, 0, 3);
		Menue[MENSHOP].setzeMenuepunkt(482, 379, -1, false, -1, 0, 0, 3);
		Menue[MENSHOP].setzeMenuepunkt(120, 420, MKAUFEN, true, -1, 0, -1, -1);
		Menue[MENSHOP].setzeMenuepunkt(450, 420, MWEITER, true, -1, 0, -1, -1);

		Menue[MENGGEWONNEN].setzeBackground(MGBACK);
		Menue[MENGGEWONNEN].setzeMenuepunkt(200, 22, MROTER, false, -1, 0, -1, -1);
		for (i = 0; i < MAXSPIELER; i++)
		{
			Menue[MENGGEWONNEN].setzeMenuepunkt(224, 202 + i * 44, -1, false, -1, 0, 0, 3);
			Menue[MENGGEWONNEN].setzeMenuepunkt(367, 202 + i * 44, -1, false, -1, 0, 0, 3);
			Menue[MENGGEWONNEN].setzeMenuepunkt(528, 202 + i * 44, -1, false, -1, 0, 0, 3);
		}
		Menue[MENGGEWONNEN].setzeMenuepunkt(450, 420, MWEITER, true, MENMAIN, 0, -1, -1);

		Menue[MENSTART].setzeMenuepunkt(180, 220, MOBACK, false, -1, 0, -1, -1);
		Menue[MENSTART].setzeMenuepunkt(250, 250, MRUNDE, false, -1, 0, -1, -1);
		Menue[MENSTART].setzeMenuepunkt(380, 255, -1, false, -1, 0, 0, 1);
		Menue[MENSTART].setzeMenuepunkt(270, 330, MWEITER, true, -1, 0, -1, -1);

		Menue[MENRGEWONNEN].setzeMenuepunkt(180, 220, MOBACK, false, -1, 0, -1, -1);
		Menue[MENRGEWONNEN].setzeMenuepunkt(210, 227, MROTER, false, -1, 0, -1, -1);
		Menue[MENRGEWONNEN].setzeMenuepunkt(190, 275, MHATDIESE, false, -1, 0, -1, -1);
		Menue[MENRGEWONNEN].setzeMenuepunkt(355, 275, MRUNDE, false, -1, 0, -1, -1);
		Menue[MENRGEWONNEN].setzeMenuepunkt(250, 314, MGEWONNEN, false, -1, 0, -1, -1);
		Menue[MENRGEWONNEN].setzeMenuepunkt(270, 360, MWEITER, true, -1, 0, -1, -1);

		Menue[MENTASTENO].setzeMenuepunkt(60, 150, MKEYBOARD, false, -1, 0, -1, -1);

		Menue[MENBEENDEN].setzeMenuepunkt(180, 220, MOBACK, false, -1, 0, -1, -1);
		Menue[MENBEENDEN].setzeMenuepunkt(230, 250, MWEITER, true, -1, 0, -1, -1);
		Menue[MENBEENDEN].setzeMenuepunkt(230, 300, MTASTEN, true, -1, 0, -1, -1);
		Menue[MENBEENDEN].setzeMenuepunkt(230, 350, MBEENDEN, true, -1, 0, -1, -1);

		// Credits
		for (i = 0; i < MAXCREDITS; i++)
		{
			CreditsListe[i].Bild = -1;
			CreditsListe[i].Zeit = -1;
			CreditsListe[i].Ueberschrift = false;
		}

		CreditsListe[0].Bild = CRPROGRAMMIERUNG;
		CreditsListe[0].Zeit = 50;
		CreditsListe[0].Ueberschrift = true;

		CreditsListe[1].Bild = CRDIRKPLATE;
		CreditsListe[1].Zeit = 100;
		CreditsListe[1].Ueberschrift = false;

		CreditsListe[2].Bild = -3;
		CreditsListe[2].Zeit = 110;

		CreditsListe[3].Bild = -2;
		CreditsListe[3].Zeit = 200;

		CreditsListe[4].Bild = CRGRAFIK;
		CreditsListe[4].Zeit = 400;
		CreditsListe[4].Ueberschrift = true;

		CreditsListe[5].Bild = CRDIRKPLATE;
		CreditsListe[5].Zeit = 450;
		CreditsListe[5].Ueberschrift = false;

		CreditsListe[6].Bild = -3;
		CreditsListe[6].Zeit = 460;

		CreditsListe[7].Bild = -2;
		CreditsListe[7].Zeit = 550;

		CreditsListe[8].Bild = CRSOUND;
		CreditsListe[8].Zeit = 650;
		CreditsListe[8].Ueberschrift = true;

		CreditsListe[9].Bild = CRDIRKPLATE;
		CreditsListe[9].Zeit = 700;
		CreditsListe[9].Ueberschrift = false;

		CreditsListe[10].Bild = -3;
		CreditsListe[10].Zeit = 710;

		CreditsListe[11].Bild = -2;
		CreditsListe[11].Zeit = 800;

		CreditsListe[12].Bild = CRMUSIK;
		CreditsListe[12].Zeit = 900;
		CreditsListe[12].Ueberschrift = true;

		CreditsListe[13].Bild = CRHEIKOPLATE;
		CreditsListe[13].Zeit = 950;
		CreditsListe[13].Ueberschrift = false;

		CreditsListe[14].Bild = -3;
		CreditsListe[14].Zeit = 1060;

		CreditsListe[15].Bild = -2;
		CreditsListe[15].Zeit = 1080;

		CreditsListe[16].Bild = CRBETATESTER;
		CreditsListe[16].Zeit = 1150;
		CreditsListe[16].Ueberschrift = true;

		CreditsListe[17].Bild = CRMATTHIASBUCHETICS;
		CreditsListe[17].Zeit = 1200;
		CreditsListe[17].Ueberschrift = false;

		CreditsListe[18].Bild = -3;
		CreditsListe[18].Zeit = 1310;

		CreditsListe[19].Bild = CRMARCEBERHARDT;
		CreditsListe[19].Zeit = 1400;
		CreditsListe[19].Ueberschrift = false;

		CreditsListe[20].Bild = -3;
		CreditsListe[20].Zeit = 1510;

		CreditsListe[21].Bild = -2;
		CreditsListe[21].Zeit = 1600;

		CreditsListe[22].Bild = CRDPSOFTWARE;
		CreditsListe[22].Zeit = 1700;
		CreditsListe[22].Ueberschrift = false;

		CreditsListe[23].Bild = -3;
		CreditsListe[23].Zeit = 1800;

		CreditsListe[24].Bild = -2; // Am Schluss noch 50 Pause
		CreditsListe[24].Zeit = 1950;

		Zeitgenau = SDL_GetTicks();

		AktRunde = 0;
		MaxRunde = 3;
		Windschalter = false;
		Wetterschalter = false;
		// Panzer (die notwenigen Standardeinstellungen f�rs Men� schon hier festlegen
		for (i = 0; i < MAXSPIELER; i++)
		{
			Panzer[i].Aktiv = false;
			Panzer[i].Computer = false;
		}
		Panzer[0].Aktiv = true;
		Panzer[0].Computer = true;
		Panzer[1].Aktiv = false;
		Panzer[1].Computer = false;
		Panzer[2].Aktiv = true;
		Panzer[2].Computer = false;
		Panzer[3].Aktiv = false;
		Panzer[3].Computer = false;
	}
	if ((zustand == 1) || (zustand == 2))
	{
		// Pixel
		for (i = 0; i < MAXX; i++)
			for (j = 0; j < MAXY; j++)
			{
				Pixel[i][j].Aktiv = false;
				Pixel[i][j].p.Art = ARTNICHTS;
				Pixel[i][j].p.Relx = 0.5;
				Pixel[i][j].p.Rely = 0.5;
				Pixel[i][j].CheckNr = 0;
				Pixel[i][j].p.v.x = 0;
				Pixel[i][j].p.v.y = 0;
				Pixel[i][j].Farbe = Tansparent;
				Pixel[i][j].p.Beschleunigung.x = 0;
				Pixel[i][j].p.Beschleunigung.y = 0;
				Pixel[i][j].p.Lifetime = -1;
			}

		// HimmelPixel
		for (i = 0; i < MAXHIMMELPIXEL; i++)
		{
			HimmelPixel[i].Art = ARTNICHTS;
			HimmelPixel[i].Relx = 0.5;
			HimmelPixel[i].Rely = 0.5;
			HimmelPixel[i].v.x = 0;
			HimmelPixel[i].v.y = 0;
			HimmelPixel[i].Beschleunigung.x = 0;
			HimmelPixel[i].Beschleunigung.y = 0;
			HimmelPixel[i].Pos.x = 0;
			HimmelPixel[i].Pos.y = 0;
		}

		// MunListe
		for (i = 0; i < MAXMUN; i++)
		{
			MunListe[i].Ausserhalb = false;
			MunListe[i].p.Lifetime = -1;
			MunListe[i].Besitzer = -1;
			MunListe[i].SoundBuffer = audio::NO_SOUND;
			MunListe[i].p.Art = MUNSTEIN;
			MunListe[i].p.Relx = 0.5;
			MunListe[i].p.Rely = 0.5;
			MunListe[i].p.v.x = 0;
			MunListe[i].p.v.y = 0;
			MunListe[i].p.Pos.x = 1;
			MunListe[i].p.Pos.y = 1;
		}

		// Panzer
		for (i = 0; i < MAXSPIELER; i++)
		{
			Panzer[i].Aktiv = false;
			Panzer[i].p.Art = ARTPANZER;
			Panzer[i].p.Relx = 0.5;
			Panzer[i].p.Rely = 0.5;
			Panzer[i].p.v.x = 0;
			Panzer[i].p.v.y = 0;
			Panzer[i].p.Pos.x = 10;
			Panzer[i].p.Pos.y = 10;
			Panzer[i].Bild = 0;
			Panzer[i].Version = 0;
			Panzer[i].KanonePos.x = 0;
			Panzer[i].KanonePos.y = -KANONENLAENGE;
			Panzer[i].SchussAktiv = false;
			Panzer[i].Munition = MUNSTEIN;

			if (zustand == 1) // Lager nur bei erster Runde initialisieren
			{
				for (j = 0; j <= MUNANZAHL; j++)
					Panzer[i].Lager[i] = 0;
				Panzer[i].Lager[MUNSTEIN] = -1;
				Panzer[i].Lager[MUNRAKETE] = 4;
				Panzer[i].Lager[MUNSCHILD] = 1;
				Panzer[i].Lager[MUNGRANATE] = 2;
				Panzer[i].Lager[MUNGEWEHR] = 0;
				Panzer[i].Lager[MUNFASS] = 2;
				Panzer[i].Lager[MUNLASER] = 0;
				Panzer[i].Lager[MUNMEDI] = 0;
				Panzer[i].Lager[MUNMAGRAKETE] = 0;
				Panzer[i].Lager[MUNMAGNET] = 0;
				Panzer[i].Lager[MUNMEGA] = 0;
				Panzer[i].Lager[MUNSCHNEEBALL] = 2;
				Panzer[i].Konto = 0;
				Panzer[i].RGewonnen = 0;
				Panzer[i].Treffer = 0;
				Panzer[i].Abschuesse = 0;
			}

			Panzer[i].ButtonDownUp = false;
			Panzer[i].ButtonDownDown = false;
			Panzer[i].ButtonDownFire = false;
			Panzer[i].SchussEnergie = 0;
			Panzer[i].LebensEnergie = MAXLEBENSENERGIE;
			Panzer[i].Farbe = RGB2DWORD(0, 0, 0);
			Panzer[i].Schild = 0;
			Panzer[i].Magnet = 0;
			Panzer[i].Computer = false;
			Panzer[i].Ziel = -1;
			Panzer[i].WunschMunition = MUNSTEIN;
			Panzer[i].WunschRichtung.x = 0;
			Panzer[i].WunschRichtung.y = -1;
			for (j = 0; j < MAXSPIELER; j++)
			{
				Panzer[i].Comp[j].LastSchussEnergie = 0;
				Panzer[i].Comp[j].Genauigkeit = MAXSCHUSSENERGIE / 2;
				Panzer[i].Comp[j].Zuweit = false;
				Panzer[i].Comp[j].Zaehler = 0;
				Panzer[i].Comp[j].Richtung = Panzer[i].KanonePos;
			}
			Panzer[i].KeyLeft = SDLK_UNKNOWN;
			Panzer[i].KeyRight = SDLK_UNKNOWN;
			Panzer[i].KeyUp = SDLK_UNKNOWN;
			Panzer[i].KeyDown = SDLK_UNKNOWN;
			Panzer[i].KeyFire = SDLK_UNKNOWN;
		}
		Panzer[0].Aktiv = false;
		Panzer[0].Bild = PANZERNULL;
		Panzer[0].Farbe = RGB2DWORD(255, 0, 0);
		Panzer[0].Computer = false;
		Panzer[0].KeyLeft = SDLK_d;
		Panzer[0].KeyRight = SDLK_f;
		Panzer[0].KeyUp = SDLK_w;
		Panzer[0].KeyDown = SDLK_s;
		Panzer[0].KeyFire = SDLK_LSHIFT;

		Panzer[1].Aktiv = false;
		Panzer[1].Bild = PANZEREINS;
		Panzer[1].Farbe = RGB2DWORD(0, 255, 0);
		Panzer[1].Computer = false;
		Panzer[1].KeyLeft = SDLK_COMMA;
		Panzer[1].KeyRight = SDLK_PERIOD;
		Panzer[1].KeyUp = SDLK_j;
		Panzer[1].KeyDown = SDLK_m;
		Panzer[1].KeyFire = SDLK_SPACE;

		Panzer[2].Aktiv = false;
		Panzer[2].Bild = PANZERZWEI;
		Panzer[2].Farbe = RGB2DWORD(100, 100, 255);
		Panzer[2].Computer = false;
		Panzer[2].KeyLeft = SDLK_LEFT;
		Panzer[2].KeyRight = SDLK_RIGHT;
		Panzer[2].KeyUp = SDLK_UP;
		Panzer[2].KeyDown = SDLK_DOWN;
		Panzer[2].KeyFire = SDLK_RETURN;

		Panzer[3].Aktiv = false;
		Panzer[3].Bild = PANZERDREI;
		Panzer[3].Farbe = RGB2DWORD(255, 255, 0);
		Panzer[3].Computer = false;
		Panzer[3].KeyLeft = SDLK_KP_4;
		Panzer[3].KeyRight = SDLK_KP_6;
		Panzer[3].KeyUp = SDLK_KP_8;
		Panzer[3].KeyDown = SDLK_KP_5;
		Panzer[3].KeyFire = SDLK_KP_ENTER;

		for (i = 0; i < MAXSPIELER; i++)
		{
			PanzSavPos[i].x = (i * PANZERBREITE) + 10;
			PanzSavPos[i].y = 10 + Bmp[Panzer[i].Bild].Hoehe;
		}

		// Aktivbereiche
		for (i = 0; i < BERANZAHL; i++)
			for (j = 0; j < BERANZAHL; j++)
				BereichAktiv[i][j] = false;

		// FX Pixel
		for (i = 0; i < MAXFX; i++)
		{
			FXListe[i].Pos.x = -1;
			FXListe[i].Pos.y = -1;
			FXListe[i].Lifetime = -1;
			FXListe[i].Rel.x = 0.5;
			FXListe[i].Rel.y = 0.5;
			FXListe[i].Bewegung.x = 0;
			FXListe[i].Bewegung.y = 0;
		}
		LastFX = 0;

		// Ballon
		Ballon.Aktiv = false;
		Ballon.Pos.x = -1;
		Ballon.Pos.y = -1;
		Ballon.Munition = -1;
		Ballon.Anzahl = 0;
		Ballon.Rel.x = 0.5;
		Ballon.Rel.y = 0.5;
	}
}

void CheckMouse(SDL_Event *event)
{
	static Uint32 LastMouseSound = 0;
	static Uint32 SoundID = 0;
	Sint32 xDiff{}, yDiff{}; // Die Differenz zur vorherigen Position ((F�r Scrollen)
	ZWEID Pos;
	float Entf;

	if (event->type == SDL_MOUSEMOTION)
	{
		xDiff = event->motion.xrel;
		yDiff = event->motion.yrel;

		MousePosition.x = event->motion.x / ScaleFactor;
		MousePosition.y = event->motion.y / ScaleFactor;
	}
	// Mausbewegung
	if (MousePosition.x < 0)
		MousePosition.x = 0;
	if (MousePosition.x > MAXX - Bmp[CursorTyp].Breite)
		MousePosition.x = MAXX - Bmp[CursorTyp].Breite;

	if (MousePosition.y < 0)
		MousePosition.y = 0;
	if (MousePosition.y > MAXY - Bmp[CursorTyp].Hoehe)
		MousePosition.y = MAXY - Bmp[CursorTyp].Hoehe;

	Entf = sqrt(xDiff * xDiff + yDiff * yDiff);
	if (Entf > 2 && SDL_GetTicks() - LastMouseSound > 10) {
		LastMouseSound = SDL_GetTicks();
		audio_manager->play(static_cast<audio::file>(audio::MOUSE1 + SoundID++), audio::id::SFX, 10);
		SoundID %= 4;
	}

	CursorTyp = CUKREUZ;

	if (event->type == SDL_MOUSEBUTTONUP)
	{
		if (event->button.button == SDL_BUTTON_LEFT)
		{
			if (Button0down)
			{
				ButtonPush = -1;
				Button0down = false;
			}
		}
		if (event->button.button == SDL_BUTTON_RIGHT)
		{
			if (Button1down)
			{
				ButtonPush = -1;
				Button1down = false;
			}
		}
	}
	if (event->type == SDL_MOUSEBUTTONDOWN)
	{
		if (event->button.button == SDL_BUTTON_LEFT)
		{
			ButtonPush = 1;
			Button0down = true;
		}
		if (event->button.button == SDL_BUTTON_RIGHT)
		{
			ButtonPush = 1;
			Button1down = true;
		}
	}

	/*dims.lX, dims.lY, dims.lZ,
					 (dims.rgbButtons[0] & 0x80) ? '0' : ' ',
					 (dims.rgbButtons[1] & 0x80) ? '1' : ' ',
					 (dims.rgbButtons[2] & 0x80) ? '2' : ' ',
					 (dims.rgbButtons[3] & 0x80) ? '3' : ' ');*/

	if (Testmodus)
	{
		Pos.x = MousePosition.x + Bmp[CUKREUZ].Breite / 2;
		Pos.y = MousePosition.y + Bmp[CUKREUZ].Hoehe / 2;
		sprintf(StdString, "%f", (float)Pixel[Pos.x][Pos.y].p.v.x);
		DrawString(StdString, 20, 10, 1, lpDDSSchrift.texture);
		sprintf(StdString, "%f", (float)Pixel[Pos.x][Pos.y].p.v.y);
		DrawString(StdString, 20, 30, 1, lpDDSSchrift.texture);
		sprintf(StdString, "%f", (float)Pixel[Pos.x][Pos.y].p.Relx);
		DrawString(StdString, 20, 50, 1, lpDDSSchrift.texture);
		sprintf(StdString, "%f", (float)Pixel[Pos.x][Pos.y].p.Rely);
		DrawString(StdString, 20, 70, 1, lpDDSSchrift.texture);
		sprintf(StdString, "%f", (float)Pixel[Pos.x][Pos.y].p.Art);
		DrawString(StdString, 20, 90, 1, lpDDSSchrift.texture);
	}
}

short CheckKey(SDL_Event *event)
{
#define KEYDOWN(__key) (event->type == SDL_KEYDOWN && event->key.keysym.sym == __key)
#define KEYUP(__key) (event->type == SDL_KEYUP && event->key.keysym.sym == __key)

	short i, j;

	if (Spielzustand == SZSPIEL)
	{
		if (KEYDOWN(SDLK_ESCAPE))
		{
			AktMenue = MENBEENDEN;
			return (1);
		}
		if (KEYDOWN(SDLK_F1))
		{
			AktMenue = MENTASTENO;
			return (1);
		}
		/*if (KEYDOWN(SDLK_F11))
		{
			NeuesSpiel();
			return(1);
		}*/
		for (i = 0; i < MAXSPIELER; i++)
		{
			if ((Panzer[i].Aktiv) && !(Panzer[i].Computer))
			{
				if (KEYDOWN(Panzer[i].KeyLeft))
				{
					MoveKanone(i, -1);
				}
				if (KEYDOWN(Panzer[i].KeyRight))
				{
					MoveKanone(i, +1);
				}
				if (KEYDOWN(Panzer[i].KeyUp))
				{
					if (!Panzer[i].ButtonDownUp)
					{
						Panzer[i].ButtonDownUp = true;
						ChangeMunition(i, -1);
					}
				}
				else
					Panzer[i].ButtonDownUp = false;
				if (KEYDOWN(Panzer[i].KeyDown))
				{
					if (!Panzer[i].ButtonDownDown)
					{
						Panzer[i].ButtonDownDown = true;
						ChangeMunition(i, +1);
					}
				}
				else
					Panzer[i].ButtonDownDown = false;
				if ((KEYDOWN(Panzer[i].KeyFire)) && !(Panzer[i].SchussAktiv))
				{
					if ((Panzer[i].Munition == MUNSCHILD) ||
						(Panzer[i].Munition == MUNGEWEHR) ||
						(Panzer[i].Munition == MUNLASER) ||
						(Panzer[i].Munition == MUNMEDI))
					{
						if (!Panzer[i].ButtonDownFire)
						{
							Panzer[i].ButtonDownFire = true;
							Panzer[i].SchussEnergie = MAXSCHUSSENERGIE;
							Abschuss(i);
						}
					}
					else if (Panzer[i].SchussEnergie < MAXSCHUSSENERGIE)
					{
						Panzer[i].SchussEnergie += 1;
						ZeichnePanzer(i, 1);
					}
				}
				else if (KEYUP(Panzer[i].KeyFire))
				{
					Panzer[i].ButtonDownFire = false;
					if ((Panzer[i].SchussEnergie != 0) &&
						(Panzer[i].SchussAktiv == false))
						Abschuss(i);
				}
			}
		}

		// Cheats
		/*if (KEYDOWN(buffer, DIK_T))
		{
			if (Testmodus == true)
			{
				rcRectdes.left		= 0;
				rcRectdes.top		= 0;
				rcRectdes.right		= MAXX;
				rcRectdes.bottom	= MAXY;
				ddbltfx.dwFillColor = Tansparent;
				lpDDSScape->Blt(&rcRectdes, NULL,NULL,DDBLT_COLORFILL, &ddbltfx);
				Testmodus = false;
			}
			else Testmodus = true;
		}*/
		if ((KEYDOWN(SDLK_LCTRL)) && (KEYDOWN(SDLK_PRIOR)) &&
			(KEYDOWN(SDLK_SPACE)))
		{
			g = 9.81;
			return (1);
		}
		if ((KEYDOWN(SDLK_LCTRL)) && (KEYDOWN(SDLK_PRIOR)))
		{
			g++;
			return (1);
		}
		if ((KEYDOWN(SDLK_LCTRL)) && (KEYDOWN(SDLK_SPACE)))
		{
			g--;
			if (g < 0)
				g = 0;
			return (1);
		}
		if ((KEYDOWN(SDLK_LCTRL)) && (KEYDOWN(SDLK_HOME)))
		{
			for (i = 0; i < MAXSPIELER; i++)
				for (j = 1; j < MUNANZAHL; j++)
				{
					if (j != MUNSPLITTER)
						Panzer[i].Lager[j] = -1;
				}
			return (1);
		}
	}
	else if (Spielzustand == SZCREDITS)
	{
		if (KEYDOWN(SDLK_ESCAPE))
		{
			Spielzustand = SZMENUE;
			AktMenue = MENMAIN;
			StopAllSound();
			return (1);
		}
	}
	else if (Spielzustand == SZTITEL)
	{
		if ((KEYDOWN(SDLK_ESCAPE)) || (KEYDOWN(SDLK_SPACE)) ||
			(KEYDOWN(SDLK_RETURN)) || (KEYDOWN(SDLK_KP_ENTER)))
		{
			Spielzustand = SZMENUE;
			AktMenue = MENMAIN;
			StopAllSound();
			return (1);
		}
	}
	return (1);
}

void Computer()
{
	short i, tmp;
	short WunschSchussEnergie;
	short TestZaehler;
	ZWEID erg;
	ZWEID Ziel;
	ZWEID d;
	double alpha;

	for (i = 0; i < MAXSPIELER; i++)
	{
		if (!(Panzer[i].Aktiv) || !(Panzer[i].Computer))
			continue;

		if (!Panzer[Panzer[i].Ziel].Aktiv)
			Panzer[i].Ziel = -1;

		if ((Panzer[i].Ziel != -1) &&
			!(Panzer[i].Comp[Panzer[i].Ziel].Zuweit) &&
			(Panzer[i].Comp[Panzer[i].Ziel].LastSchussEnergie >= MAXSCHUSSENERGIE) &&
			!(Panzer[i].SchussAktiv))
		{
			Panzer[i].Comp[Panzer[i].Ziel].Zaehler = MAXGEGNERSCHUSS; // Damit wird sp�ter ein neuer Winkel gew�hlt
			Panzer[i].Comp[Panzer[i].Ziel].Zuweit = true;
		}

		if (Panzer[i].WunschMunition == MUNGEWEHR)
		{
			Ziel.x = Panzer[Panzer[i].Ziel].p.Pos.x;
			Ziel.y = Panzer[Panzer[i].Ziel].p.Pos.y;
			erg = LineIntersect(Panzer[i].p.Pos.x, Panzer[i].p.Pos.y - 5,
								Ziel.x, Ziel.y - 5, true);

			if ((erg.x != -1) && (Ziel.y < Panzer[i].p.Pos.y)) // Luftlinie vorhanden
			{
				d.x = ROUND(Ziel.x - Panzer[i].p.Pos.x);
				d.y = ROUND(Ziel.y - Panzer[i].p.Pos.y);
				alpha = atan(double(d.y) / double(d.x));
				Panzer[i].WunschRichtung.x = ROUND(KANONENLAENGE * cos(alpha));
				if (d.x < 0)
					Panzer[i].WunschRichtung.x *= -1;
				Panzer[i].WunschRichtung.y = ROUND(KANONENLAENGE * sin(alpha));
				if (d.x < 0)
					Panzer[i].WunschRichtung.y *= -1;

				if ((Panzer[i].WunschRichtung.x * Panzer[i].WunschRichtung.x +
					 Panzer[i].WunschRichtung.y * Panzer[i].WunschRichtung.y) <= 376)
					Panzer[i].WunschRichtung.y -= 1;

				if (Panzer[i].WunschRichtung.y >= -2)
				{
					Panzer[i].WunschRichtung.y = -1;
					Panzer[i].WunschMunition = MUNSTEIN;
				}
			}
			else
				Panzer[i].WunschMunition = MUNSTEIN;
		}
		else if (Panzer[i].WunschMunition == MUNLASER)
		{
			Ziel.x = Panzer[Panzer[i].Ziel].p.Pos.x;
			Ziel.y = Panzer[Panzer[i].Ziel].p.Pos.y;

			if (Ziel.y < Panzer[i].p.Pos.y) // Ziel in Reichweite
			{
				d.x = ROUND(Ziel.x - Panzer[i].p.Pos.x);
				d.y = ROUND(Ziel.y - Panzer[i].p.Pos.y);
				alpha = atan(double(d.y) / double(d.x));
				Panzer[i].WunschRichtung.x = ROUND(KANONENLAENGE * cos(alpha));
				if (d.x < 0)
					Panzer[i].WunschRichtung.x *= -1;
				Panzer[i].WunschRichtung.y = ROUND(KANONENLAENGE * sin(alpha));
				if (d.x < 0)
					Panzer[i].WunschRichtung.y *= -1;

				if ((Panzer[i].WunschRichtung.x * Panzer[i].WunschRichtung.x +
					 Panzer[i].WunschRichtung.y * Panzer[i].WunschRichtung.y) <= 376)
					Panzer[i].WunschRichtung.y -= 1;

				if (Panzer[i].WunschRichtung.y >= -2)
				{
					Panzer[i].WunschRichtung.y = -1;
					Panzer[i].WunschMunition = MUNSTEIN;
				}
			}
			else
				Panzer[i].WunschMunition = MUNSTEIN;
		}
		else
			Panzer[i].WunschRichtung.y = -1;

		// Noch kein Ziel gew�hlt
		if (Panzer[i].Ziel == -1)
		{
			TestZaehler = 0; // Damit er bei einem Spieler nicht stehenbleibt
			while (1)
			{
				Panzer[i].Ziel = rand() % MAXSPIELER;
				if ((Panzer[i].Ziel != i) && (Panzer[Panzer[i].Ziel].Aktiv))
					break;
				TestZaehler++;
				if (TestZaehler == 100)
				{
					break;
				}
			}
			if (Panzer[Panzer[i].Ziel].p.Pos.x > Panzer[i].p.Pos.x)
				Panzer[i].WunschRichtung.x = KANONENLAENGE / 2;
			else
				Panzer[i].WunschRichtung.x = -KANONENLAENGE / 2;
		}

		// Richtung scheint nicht zum Erfolg zu f�hren
		if (Panzer[i].Comp[Panzer[i].Ziel].Zaehler >= MAXGEGNERSCHUSS)
		{
			Panzer[i].Comp[Panzer[i].Ziel].Zaehler = 0;
			Panzer[i].Comp[Panzer[i].Ziel].Genauigkeit = MAXSCHUSSENERGIE / 2;
			Panzer[i].WunschRichtung.x = Panzer[i].Comp[Panzer[i].Ziel].Richtung.x;
			Panzer[i].WunschRichtung.y = Panzer[i].Comp[Panzer[i].Ziel].Richtung.y;
			tmp = 0;
			TestZaehler = 0;
			if (Panzer[i].WunschRichtung.x >= 0)
			{
				while (1)
				{
					tmp = rand() % (KANONENLAENGE / 2);
					if ((tmp > 2) && (tmp <= KANONENLAENGE))
						break;
					TestZaehler++;
					if (TestZaehler == 100)
					{
						SDL_TriggerBreakpoint();
						break;
					}
				}
			}
			else
			{
				while (1)
				{
					tmp = -rand() % (KANONENLAENGE / 2);
					if ((tmp < -2) && (tmp >= -KANONENLAENGE))
						break;
					TestZaehler++;
					if (TestZaehler == 100)
					{
						SDL_TriggerBreakpoint();
						break;
					}
				}
			}
			Panzer[i].WunschRichtung.x = tmp;
		}

		if ((Panzer[i].Lager[Panzer[i].WunschMunition] == 0) ||
			(Panzer[i].Comp[Panzer[i].Ziel].Zaehler % 5 == 0) ||
			((Panzer[i].Schild != 0) && (Panzer[i].WunschMunition == MUNSCHILD)) ||
			((Panzer[i].Magnet != 0) && (Panzer[i].WunschMunition == MUNMAGNET))) // Munition w�hlen
		{
			Panzer[i].Comp[Panzer[i].Ziel].Zaehler++; // Damit im n�chsten Durchgang hier nicht wieder rein (eigentlic falsch)
			TestZaehler = 0;
			while (1)
			{
				Panzer[i].WunschMunition = rand() % MUNANZAHL;
				if ((Panzer[i].WunschMunition == MUNMEDI) &&
					(Panzer[i].LebensEnergie >= MAXLEBENSENERGIE / 2))
					continue;
				if ((Panzer[i].Lager[Panzer[i].WunschMunition] != 0))
					break;
				TestZaehler++;
				if (TestZaehler == 100)
				{
					SDL_TriggerBreakpoint();
					break;
				}
			}
		}
		// Munition w�hlen
		if (Panzer[i].Munition != Panzer[i].WunschMunition)
		{
			ChangeMunition(i, +1);
			continue;
		}

		// Kanone ausrichten
		if (Panzer[i].KanonePos.x > Panzer[i].WunschRichtung.x)
		{
			MoveKanone(i, -1);
			continue;
		}
		else if (Panzer[i].KanonePos.x < Panzer[i].WunschRichtung.x)
		{
			MoveKanone(i, +1);
			continue;
		}
		if ((Panzer[i].KanonePos.y > Panzer[i].WunschRichtung.y + 1) &&
			(Panzer[i].WunschRichtung.y != -1))
		{
			if (Panzer[i].KanonePos.x < 0)
				MoveKanone(i, +1);
			else
				MoveKanone(i, -1);
			continue;
		}
		else if ((Panzer[i].KanonePos.y < Panzer[i].WunschRichtung.y - 1) &&
				 (Panzer[i].WunschRichtung.y != -1))
		{
			if (Panzer[i].KanonePos.x < 0)
				MoveKanone(i, -1);
			else
				MoveKanone(i, +1);
			continue;
		}

		// Feuern
		if (!Panzer[i].SchussAktiv)
		{
			// Sonderf�lle
			if ((Panzer[i].Munition == MUNSCHILD) || (Panzer[i].Munition == MUNMAGNET) ||
				(Panzer[i].Munition == MUNMEDI))
			{
				Panzer[i].SchussEnergie = MAXSCHUSSENERGIE;
				Abschuss(i);
				Panzer[i].WunschMunition = MUNSTEIN;
				continue;
			}
			if (Panzer[i].Munition == MUNGEWEHR)

			{
				Panzer[i].SchussEnergie = MAXSCHUSSENERGIE;
				Abschuss(i);
				Panzer[i].Comp[Panzer[i].Ziel].Zaehler++;
				Panzer[i].Comp[Panzer[i].Ziel].Richtung.x = Panzer[i].WunschRichtung.x;
				Panzer[i].Comp[Panzer[i].Ziel].Richtung.y = Panzer[i].WunschRichtung.y;
				Panzer[i].WunschRichtung.y = -1;
				continue;
			}
			if (Panzer[i].Munition == MUNLASER)
			{
				Panzer[i].SchussEnergie = MAXSCHUSSENERGIE;
				Abschuss(i);
				Panzer[i].Comp[Panzer[i].Ziel].Richtung.x = Panzer[i].WunschRichtung.x;
				Panzer[i].Comp[Panzer[i].Ziel].Richtung.y = Panzer[i].WunschRichtung.y;
				Panzer[i].WunschRichtung.y = -1;
				Panzer[i].WunschMunition = MUNSTEIN;
				Panzer[i].Comp[Panzer[i].Ziel].Zaehler = MAXGEGNERSCHUSS;
				continue;
			}

			// normale F�lle
			if (Panzer[i].Comp[Panzer[i].Ziel].Zuweit)
				WunschSchussEnergie = Panzer[i].Comp[Panzer[i].Ziel].LastSchussEnergie -
									  Panzer[i].Comp[Panzer[i].Ziel].Genauigkeit;
			else
				WunschSchussEnergie = Panzer[i].Comp[Panzer[i].Ziel].LastSchussEnergie +
									  Panzer[i].Comp[Panzer[i].Ziel].Genauigkeit;
			if (WunschSchussEnergie < 10)
				WunschSchussEnergie = 10; // Mindestschussenergie sicherstellen
			if ((Panzer[i].Munition == MUNMAGRAKETE) && (WunschSchussEnergie < 30))
				WunschSchussEnergie = 30;
			if (WunschSchussEnergie > MAXSCHUSSENERGIE)
				WunschSchussEnergie = MAXSCHUSSENERGIE;
			if ((Panzer[i].SchussEnergie >= MAXSCHUSSENERGIE) ||
				(Panzer[i].SchussEnergie >= WunschSchussEnergie))
			{
				Abschuss(i);
				Panzer[i].Comp[Panzer[i].Ziel].Zaehler++;
				Panzer[i].Comp[Panzer[i].Ziel].Richtung = Panzer[i].WunschRichtung;
			}
			else
			{
				Panzer[i].SchussEnergie += 1;
				ZeichnePanzer(i, 1);
			}
			continue;
		}
	}
}

void CalcSchussErgebnis(short x, short Besitzer)
{
	bool Zuweittmp;

	if (Panzer[Besitzer].Computer == false)
		return;

	if (Panzer[Besitzer].p.Pos.x > Panzer[Panzer[Besitzer].Ziel].p.Pos.x)
	{
		if (x < Panzer[Panzer[Besitzer].Ziel].p.Pos.x)
			Zuweittmp = true;
		else
			Zuweittmp = false;
	}
	else
	{
		if (x > Panzer[Panzer[Besitzer].Ziel].p.Pos.x)
			Zuweittmp = true;
		else
			Zuweittmp = false;
	}
	if (Zuweittmp != Panzer[Besitzer].Comp[Panzer[Besitzer].Ziel].Zuweit)
		Panzer[Besitzer].Comp[Panzer[Besitzer].Ziel].Genauigkeit /= 2;
	if (Panzer[Besitzer].Comp[Panzer[Besitzer].Ziel].Genauigkeit == 0)
		Panzer[Besitzer].Comp[Panzer[Besitzer].Ziel].Genauigkeit = 1;
	Panzer[Besitzer].Comp[Panzer[Besitzer].Ziel].Zuweit = Zuweittmp;
	if ((Panzer[Besitzer].Comp[Panzer[Besitzer].Ziel].Zaehler >= MAXGEGNERSCHUSS) ||
		!(Panzer[Panzer[Besitzer].Ziel].Aktiv))
		Panzer[Besitzer].Ziel = -1;
}

inline DWORD RGB2DWORD(BYTE r, BYTE g, BYTE b)
{
	DWORD Erg = 0;
	Erg = (r << 16) | (g << 8) | b;
	return Erg;
}

inline void DWORD2RGB(DWORD color)
{
	rgbStruct.r = (BYTE)(color >> 16) & 0xFF;
	rgbStruct.g = (BYTE)(color >> 8) & 0xFF;
	rgbStruct.b = (BYTE)color & 0xFF;
}

void PutPixel(short x, short y, DWORD color, LPDDSURFACEDESC2 *ddsdtmp)
{
	if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
		return;

	int *pixels = ddsdtmp->lpSurface;
	int pitch = ddsdtmp->lPitch / 4;
	SDL_assert(pixels != nullptr);

	// convert rgb to rgba
	if (color != 0)
		color = (color << 8) | 0xFF;
	pixels[y * pitch + x] = color;
}


DWORD GetPixel2(short x, short y, LPDDSURFACEDESC2 *ddsdtmp)
{
	SDL_assert(ddsdtmp->surface != nullptr);

	int bpp = ddsdtmp->surface->format->BytesPerPixel;
	int pitch = ddsdtmp->surface->pitch;

	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)ddsdtmp->surface->pixels + y * pitch + x * bpp;

	switch (bpp) {
		case 1:
			return *p;
		break;

		case 2:
			return *(Uint16 *)p;
		break;

		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
		break;

		case 4:
			return *(Uint32 *)p;
		break;

		default:
			return 0; /* shouldn't happen, but avoids warnings */
	}

}

DWORD GetPixel(short x, short y, LPDDSURFACEDESC2 *ddsdtmp)
{
	int color;

	if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
		return 0;

	SDL_assert(ddsdtmp->surface != nullptr);

	int *pixels = (int*)ddsdtmp->surface->pixels;
	color = pixels[y * ddsdtmp->surface->w + x];

	// remove alpha channel from rgba value
	color = color >> 8;
	return color;
}

inline double Absdouble(double Zahl)
{
	return (Zahl >= 0 ? Zahl : -Zahl);
}

void Aktivate(short x, short y, bool Aktiv) // De-,Aktiviert einen Pixel
{
	bool check;
	short x1, y1;

	if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
		return;

	if (Aktiv)
	{
		Pixel[x][y].Aktiv = true;
		BereichAktiv[(short)(x / (MAXX / BERANZAHL))][(short)(y / (MAXY / BERANZAHL))] = true;
	}
	else
	{
		Pixel[x][y].Aktiv = false;
		Pixel[x][y].p.v.x = 0;
		Pixel[x][y].p.v.y = 0;
		Pixel[x][y].p.Relx = 0.5;
		Pixel[x][y].p.Rely = 0.5;
		Pixel[x][y].p.Beschleunigung.x = 0;
		Pixel[x][y].p.Beschleunigung.y = 0;
		check = false;
		// Alle Pixel im Bereich abchecken, ob noch eins Aktiv
		for (x1 = (short)(x / (MAXX / BERANZAHL)) * (MAXX / BERANZAHL); x1 < ((short)(x / (MAXX / BERANZAHL)) + 1) * (MAXX / BERANZAHL); x1++)
			for (y1 = (short)(y / (MAXY / BERANZAHL)) * (MAXY / BERANZAHL); y1 < ((short)(y / (MAXY / BERANZAHL)) + 1) * (MAXY / BERANZAHL); y1++)
				if (Pixel[x1][y1].Aktiv)
					check = true;
		if (!check)
			BereichAktiv[(short)(x / (MAXX / BERANZAHL))][(short)(y / (MAXY / BERANZAHL))] = false;
	}
}

ZWEID Physik(bool touch, short InfoNr)
{
	short xneu, yneu, i;
	double HangWinkel, vAbsolut, NormalAbsolut, Reibungsfaktor;
	ZWEIDGENAU Div, Add, Normal, Reibung;
	ZWEID erg;

	// Erdanziehung
	Add.x = 0;
	Add.y = g;

	if (touch)
	{
		// Hangwinkel
		HangWinkel = Steigung(Objekt.Pos.x, Objekt.Pos.y);
		if (HangWinkel == -1)
			HangWinkel = 0;

		// Neue Beschleunigung durch Hang
		Normal.x = g * cos(HangWinkel) * sin(HangWinkel);
		Normal.y = -(g * cos(HangWinkel) * cos(HangWinkel));

		// Beschleunigung dazuaddieren
		Add = AddBeschleunigung(Add, Normal);

		// Neue Beschleunigung durch Reibung
		vAbsolut = sqrt(Objekt.v.x * Objekt.v.x + Objekt.v.y * Objekt.v.y);
		NormalAbsolut = sqrt(Normal.x * Normal.x + Normal.y * Normal.y);
		Reibungsfaktor = sqrt(Art[Objekt.Art].Reibung *
							  Art[Pixel[Objekt.Pos.x][Objekt.Pos.y + 1].p.Art].Reibung);

		if ((vAbsolut == 0) || (HangWinkel == 0))
		{
			Reibung.x = 0;
			Reibung.y = 0;
		}
		else
		{
			Reibung.x = -Reibungsfaktor * NormalAbsolut * Objekt.v.x / vAbsolut;
			Reibung.y = -Reibungsfaktor * NormalAbsolut * Objekt.v.y / vAbsolut;
		}

		// Beschleunigung dazuaddieren
		Add = AddBeschleunigung(Add, Reibung);
	}
	// Zus�tzliche Beschleunigung
	Add = AddBeschleunigung(Add, Objekt.Beschleunigung);

	// Wind
	if ((Pixel[Objekt.Pos.x][Objekt.Pos.y + 1].p.Art == ARTNICHTS) &&
		(Wind.x != 0))
	{
		Div.x = (double)Wind.x * 10 - Objekt.v.x;
		Div.y = 0;
		if (Wind.x > 0)
		{
			if (Div.x < 0)
				Div.x = 0;
			if (Div.x > Wind.x)
				Div.x = Wind.x;
		}
		if (Wind.x < 0)
		{
			if (Div.x > 0)
				Div.x = 0;
			if (Div.x < Wind.x)
				Div.x = Wind.x;
		}
		Add = AddBeschleunigung(Add, Div);
	}

	// Endgeschwindigkeit
	Div.x = Objekt.v.x + 0.5 * Add.x * t;
	Div.y = Objekt.v.y + 0.5 * Add.y * t;

	// Neue Positions ausrechnen
	xneu = Objekt.Pos.x;
	yneu = Objekt.Pos.y;
	Objekt.Relx += Div.x * t;
	Objekt.Rely += Div.y * t;
	if ((Objekt.Relx < 0) || (Objekt.Relx >= 1))
	{
		xneu = (short)(Objekt.Pos.x + Objekt.Relx);
		Objekt.Relx -= xneu - Objekt.Pos.x;
	}
	if ((Objekt.Rely < 0) || (Objekt.Rely >= 1))
	{
		// Haftung (haupts�chlich, um den bescheuerten Fehler zu beseitigen)
		if ((Objekt.Rely < 0) && (Div.y < 0) && (Div.y > -0.5) &&
			(Pixel[Objekt.Pos.x][Objekt.Pos.y + 1].p.Art != ARTNICHTS))
		{
			Div.y = 0;
			Objekt.Rely = 0.5;
		}
		else
		{
			yneu = (short)(Objekt.Pos.y + Objekt.Rely);
			Objekt.Rely -= yneu - Objekt.Pos.y;
		}
	}

	// Au�erhalb des Bildschirms
	if ((xneu >= MAXX) || (xneu < 0))
	{
		erg.x = -1;
		erg.y = -1;
		// Objekt.Kontakt = true; //Nur zum testen
		if (InfoNr < 0) // Bei normalen Pixeln
		{
			if (Pixel[Objekt.Pos.x][Objekt.Pos.y + 1].p.Art == ARTNICHTS)
				DeletePixel(Objekt.Pos.x, Objekt.Pos.y);
		}
		else // Bei Munition
		{
			Panzer[MunListe[InfoNr].Besitzer].SchussAktiv = false;
			CalcSchussErgebnis(xneu, MunListe[InfoNr].Besitzer);
			for (i = 0; i < MAXMUN; i++)
			{
				if ((MunListe[i].Besitzer == MunListe[InfoNr].Besitzer) && (i != InfoNr))
				{
					Panzer[MunListe[InfoNr].Besitzer].SchussAktiv = true;
					break;
				}
			}
			if (!Panzer[MunListe[InfoNr].Besitzer].SchussAktiv)
			{
				Panzer[MunListe[InfoNr].Besitzer].SchussEnergie = 0;
				ZeichnePanzer(MunListe[InfoNr].Besitzer, 1);
			}
			MunListe[InfoNr].Besitzer = -1;
			StopSound(MunListe[InfoNr].SoundBuffer);
			MunListe[InfoNr].SoundBuffer = audio::NO_SOUND;
		}
		return (erg);
	}
	else if (yneu >= MAXY)
	{
		xneu = Objekt.Pos.x;
		yneu = Objekt.Pos.y;
		Div.x = 0;
		Div.y = 0;
		Aktivate(xneu, yneu, false);
		if (InfoNr >= 0)
		{
			Panzer[MunListe[InfoNr].Besitzer].SchussAktiv = false;
			CalcSchussErgebnis(xneu, MunListe[InfoNr].Besitzer);
			MunListe[InfoNr].Besitzer = -1;
			StopSound(MunListe[InfoNr].SoundBuffer);
			MunListe[InfoNr].SoundBuffer = audio::NO_SOUND;
			Panzer[MunListe[InfoNr].Besitzer].SchussEnergie = 0;
			ZeichnePanzer(MunListe[InfoNr].Besitzer, 1);
		}
		erg.x = -1;
		erg.y = -1;
		return (erg);
	}
	else if (yneu < 0)
	{
		if (InfoNr < 0)
		{
			for (i = 0; i < MAXHIMMELPIXEL; i++)
			{
				if (HimmelPixel[i].Art != ARTNICHTS)
					continue;
				HimmelPixel[i].Art = Objekt.Art;
				HimmelPixel[i].v.x = Div.x;
				HimmelPixel[i].v.y = Div.y;
				HimmelPixel[i].Pos.x = Objekt.Pos.x;
				HimmelPixel[i].Pos.y = Objekt.Pos.y + HIMMELHOEHE - 1;
				HimmelPixel[i].Lifetime = Objekt.Lifetime;
				break;
			}
			Objekt.Kontakt = true; // Nur zum testen
			DeletePixel(Objekt.Pos.x, Objekt.Pos.y);
			erg.x = -1;
			erg.y = -1;
			return (erg);
		}
		else if ((InfoNr >= 0) && (MunListe[InfoNr].Ausserhalb == false))
		{
			MunListe[InfoNr].Ausserhalb = true;
			MunListe[InfoNr].p.v.x = Div.x;
			MunListe[InfoNr].p.v.y = Div.y;
			MunListe[InfoNr].p.Pos.y = -1;
			erg.x = xneu;
			erg.y = yneu;
			return (erg);
		}
	}

	// Wenn Pixel schon belegt...
	if ((Pixel[xneu][yneu].p.Art != ARTNICHTS) && ((xneu != Objekt.Pos.x) || (yneu != Objekt.Pos.y)) && (touch))
	{
		erg = LineIntersect(Objekt.Pos.x, Objekt.Pos.y, xneu, yneu, false);
		if ((xneu > erg.x) && (Pixel[erg.x + 1][erg.y].p.Art != ARTNICHTS) &&
			(Div.x > Div.y))
		{
			Objekt.Relx = (double)0.9;
			// Impuls
			Pixel[erg.x + 1][erg.y].p.v.x += IMPULSABGABE * Div.x *
											 Art[Objekt.Art].m /
											 Art[Pixel[erg.x + 1][erg.y].p.Art].m;
			Pixel[erg.x + 1][erg.y].p.v.y += IMPULSABGABE * Div.y *
											 Art[Objekt.Art].m /
											 Art[Pixel[erg.x + 1][erg.y].p.Art].m;
			Aktivate(erg.x + 1, erg.y, true);

			Div.x = -(1 - IMPULSABGABE) * Div.x * 0.6;
			Div.y = (1 - IMPULSABGABE) * Div.y * 0.6;
		}
		if ((xneu < erg.x) && (Pixel[erg.x - 1][erg.y].p.Art != ARTNICHTS) &&
			(Div.x < Div.y))
		{
			Objekt.Relx = (double)0.1;
			// Impuls
			Pixel[erg.x - 1][erg.y].p.v.x += IMPULSABGABE * Div.x *
											 Art[Objekt.Art].m /
											 Art[Pixel[erg.x - 1][erg.y].p.Art].m;
			Pixel[erg.x - 1][erg.y].p.v.y += IMPULSABGABE * Div.y *
											 Art[Objekt.Art].m /
											 Art[Pixel[erg.x - 1][erg.y].p.Art].m;
			Aktivate(erg.x - 1, erg.y, true);

			Div.x = -(1 - IMPULSABGABE) * Div.x * 0.6;
			Div.y = (1 - IMPULSABGABE) * Div.y * 0.6;
		}
		if ((yneu > erg.y) && (Pixel[erg.x][erg.y + 1].p.Art != ARTNICHTS) &&
			(Div.y > Div.x))
		{
			Objekt.Rely = (double)0.9;
			// Impuls
			Pixel[erg.x][erg.y + 1].p.v.x += IMPULSABGABE * Div.x *
											 Art[Objekt.Art].m /
											 Art[Pixel[erg.x][erg.y + 1].p.Art].m;
			Pixel[erg.x][erg.y + 1].p.v.y += IMPULSABGABE * Div.y *
											 Art[Objekt.Art].m /
											 Art[Pixel[erg.x][erg.y + 1].p.Art].m;
			Aktivate(erg.x, erg.y + 1, true);

			Div.x = (1 - IMPULSABGABE) * Div.x * 0.6;
			Div.y = -(1 - IMPULSABGABE) * Div.y * 0.6;
		}
		if ((yneu < erg.y) && (Pixel[erg.x][erg.y - 1].p.Art != ARTNICHTS) &&
			(Div.y < Div.x))
		{
			Objekt.Rely = (double)0.1;
			// Impuls
			Pixel[erg.x][erg.y - 1].p.v.x += IMPULSABGABE * Div.x *
											 Art[Objekt.Art].m /
											 Art[Pixel[erg.x][erg.y - 1].p.Art].m;
			Pixel[erg.x][erg.y - 1].p.v.y += IMPULSABGABE * Div.y *
											 Art[Objekt.Art].m /
											 Art[Pixel[erg.x][erg.y - 1].p.Art].m;
			Aktivate(erg.x, erg.y - 1, true);

			Div.x = (1 - IMPULSABGABE) * Div.x * 0.6;
			Div.y = -(1 - IMPULSABGABE) * Div.y * 0.6;
		}
		if ((Objekt.Pos.x == erg.x) && (Objekt.Pos.y == erg.y) &&
			(Pixel[erg.x - 1][erg.y].p.Art != ARTNICHTS) &&
			(Pixel[erg.x + 1][erg.y].p.Art != ARTNICHTS) &&
			(Pixel[erg.x][erg.y - 1].p.Art != ARTNICHTS) &&
			(Pixel[erg.x][erg.y + 1].p.Art != ARTNICHTS))
		{
			Div.x = 0;
			Div.y = 0;
		}

		xneu = erg.x;
		yneu = erg.y;

		Objekt.Kontakt = true;
	}
	// Neue Geschwindigkeit eintragen
	Objekt.v.x = Div.x;
	Objekt.v.y = Div.y;

	erg.x = xneu;
	erg.y = yneu;

	return (erg);
}

void CheckMunListe()
{
	short i, j, k, tmp, Treffer;
	ZWEID erg;

	for (i = 0; i < MAXMUN; i++)
	{
		if (MunListe[i].Besitzer == -1)
			continue;

		if (Bild % (LastBild / 30 + 1) == 0)
			MunListe[i].p.Lifetime--;

		Objekt.Art = ARTMUNITION;
		Objekt.Pos.x = MunListe[i].p.Pos.x;
		Objekt.Pos.y = MunListe[i].p.Pos.y;
		Objekt.Relx = MunListe[i].p.Relx;
		Objekt.Rely = MunListe[i].p.Rely;
		Objekt.v.x = MunListe[i].p.v.x;
		Objekt.v.y = MunListe[i].p.v.y;
		Objekt.Kontakt = false;
		Objekt.Lifetime = MunListe[i].p.Lifetime;
		Objekt.Beschleunigung.x = MunListe[i].p.Beschleunigung.x;
		Objekt.Beschleunigung.y = MunListe[i].p.Beschleunigung.y;

		// Magnetrakete
		if (MunListe[i].p.Art == MUNMAGRAKETE)
			MakeMagRakete(); // benutzt das 'objekt'

		if (MunListe[i].Ausserhalb == true)
			erg = Physik(false, i);
		else
			erg = Physik(true, i);
		if ((erg.x == -1) && (erg.y == -1))
			continue;

		MunListe[i].p.Relx = Objekt.Relx;
		MunListe[i].p.Rely = Objekt.Rely;
		MunListe[i].p.Pos.x = erg.x;
		MunListe[i].p.Pos.y = erg.y;
		MunListe[i].p.Beschleunigung.x = 0;
		MunListe[i].p.Beschleunigung.y = 0;

		// Wenn in den sichtbaren Bereich (bei Munition ausserhalb)
		if ((erg.y >= 0) && (MunListe[i].Ausserhalb == true))
			MunListe[i].Ausserhalb = false;

		// Reichweite f�r den Computer ermitteln
		CalcSchussErgebnis(erg.x, MunListe[i].Besitzer);

		// Ber�hrung mit Panzer oder Munition?
		Treffer = CheckKollision(erg.x, erg.y, false);
		if (Treffer >= MAXSPIELER)
			if (MunListe[i].Besitzer == MunListe[Treffer - MAXSPIELER].Besitzer)
				Treffer = -1;
		// Explosion?
		if (((Treffer != -1) && (!MunListe[i].Ausserhalb)) ||
			(MunListe[i].p.Lifetime == 0) || Objekt.Kontakt)
		{
			if ((erg.x >= 0) && (erg.x < MAXX - 1))
				Explosion(i);

			if (Treffer >= MAXSPIELER) // Treffer mit anderer Munition
			{
				k = Treffer - MAXSPIELER;
				if ((MunListe[k].p.Pos.x >= 0) && (MunListe[k].p.Pos.x < MAXX - 1))
					Explosion(k);
			}

			Panzer[MunListe[i].Besitzer].SchussAktiv = false;

			for (j = 0; j < MAXMUN; j++)
			{
				if ((MunListe[j].Besitzer == MunListe[i].Besitzer) && (i != j))
				{
					Panzer[MunListe[i].Besitzer].SchussAktiv = true;
					break;
				}
			}
			if (!Panzer[MunListe[i].Besitzer].SchussAktiv)
			{
				Panzer[MunListe[i].Besitzer].SchussEnergie = 0;
				ZeichnePanzer(MunListe[i].Besitzer, 1);
			}
			tmp = MunListe[i].Besitzer;
			MunListe[i].Besitzer = -1;
			StopSound(MunListe[i].SoundBuffer);
			MunListe[i].SoundBuffer = audio::NO_SOUND;

			if (Treffer >= MAXSPIELER) // Treffer mit anderer Munition
			{
				k = Treffer - MAXSPIELER;
				Panzer[MunListe[k].Besitzer].SchussAktiv = false;
				CalcSchussErgebnis(MunListe[k].p.Pos.x, MunListe[k].Besitzer);
				for (j = 0; j < MAXMUN; j++)
				{
					if ((MunListe[j].Besitzer == MunListe[k].Besitzer) && (j != k))
					{
						Panzer[MunListe[k].Besitzer].SchussAktiv = true;
						break;
					}
				}
				if (!Panzer[MunListe[k].Besitzer].SchussAktiv)
				{
					Panzer[MunListe[k].Besitzer].SchussEnergie = 0;
					ZeichnePanzer(MunListe[k].Besitzer, 1);
				}
				MunListe[k].Besitzer = -1;
				StopSound(MunListe[k].SoundBuffer);
				MunListe[k].SoundBuffer = audio::NO_SOUND;
			}
			if (Treffer == -2)
				BallonAbschuss(tmp);
		}

		MunListe[i].p.v.x = Objekt.v.x;
		MunListe[i].p.v.y = Objekt.v.y;

		// FlugSound abspielen
		if ((MunListe[i].Besitzer != -1) && (Munition[MunListe[i].p.Art].FlugSound != 0))
		{
			if (Bild % (LastBild / 10 + 1) == 0) // nur 10 mal in der sek aktualisieren
				MunListe[i].SoundBuffer = Munition[MunListe[i].p.Art].FlugSound;
				audio_manager->play(Munition[MunListe[i].p.Art].FlugSound,
					audio::id::SFX, 70, false);
		}

		// Smoke
		if (Bild % (LastBild / 50 + 1) == 0) // unabh�nig von der Framerate
		{
			for (k = 0; k < 2; k++)
			{
				if ((Munition[MunListe[i].p.Art].Smoke[k] != SMOKENICHTS) &&
					(MunListe[i].Ausserhalb == false))
				{
					switch (Munition[MunListe[i].p.Art].Smoke[k])
					{
					case SMOKERAUCH:
						tmp = rand() % 50;
						MakeFXPixel(MunListe[i].p.Pos.x - 2 + rand() % 4,
									MunListe[i].p.Pos.y - 2 + rand() % 4,
									RGB2DWORD(100 + tmp, 100 + tmp, 100 + tmp),
									20 + rand() % 10, (double)Wind.x / 5, -(double)(rand() % 5) / 10);
						break;
					case SMOKEFEUER:
						tmp = rand() % 20;
						MakeFXPixel(MunListe[i].p.Pos.x - 2 + rand() % 4,
									MunListe[i].p.Pos.y - 2 + rand() % 4,
									RGB2DWORD(235 + tmp, 235, 10),
									5, 0, 0);
						break;
					}
				}
			}
		}
	}
}

void CheckPanzer()
{
	short i, j, tmp;
	ZWEID erg;

	for (i = 0; i < MAXSPIELER; i++)
	{
		if (!Panzer[i].Aktiv)
			continue;

		// Panzer bewegen

		Objekt.Art = Panzer[i].p.Art;
		Objekt.Pos.x = Panzer[i].p.Pos.x;
		Objekt.Pos.y = Panzer[i].p.Pos.y;
		Objekt.Relx = Panzer[i].p.Relx;
		Objekt.Rely = Panzer[i].p.Rely;
		Objekt.v.x = Panzer[i].p.v.x;
		Objekt.v.y = Panzer[i].p.v.y;
		Objekt.Lifetime = Panzer[i].p.Lifetime;
		Objekt.Beschleunigung.x = 0;
		Objekt.Beschleunigung.y = 0;

		erg = Physik(true, -2);
		if ((erg.x == -1) && (erg.y == -1))
			continue;

		Panzer[i].p.Relx = Objekt.Relx;
		Panzer[i].p.Rely = Objekt.Rely;
		Panzer[i].p.v.x = Objekt.v.x;
		Panzer[i].p.v.y = Objekt.v.y;
		Panzer[i].p.Pos.x = erg.x;
		Panzer[i].p.Pos.y = erg.y;

		// Magnet?
		if (Panzer[i].Magnet > 0)
			MakeMagnet(i);

		// Abflackeffekte
		if (Bild % (LastBild / 20 + 1) == 0) // unabh�ngig von der Framerate
		{
			switch (Panzer[i].Version)
			{
			case 1:
				tmp = rand() % 50; // Rauch
				MakeFXPixel(Panzer[i].p.Pos.x - 10 + rand() % 20,
							Panzer[i].p.Pos.y - 1,
							RGB2DWORD(100 + tmp, 100 + tmp, 100 + tmp),
							20 + rand() % 10, (double)Wind.x / 5, -0.5 - (double)(rand() % 10) / 10);
				break;
			case 2:
				for (j = 0; j < 2; j++) // Rauch
				{
					tmp = rand() % 50;
					MakeFXPixel(Panzer[i].p.Pos.x - 10 + rand() % 20,
								Panzer[i].p.Pos.y - 1,
								RGB2DWORD(100 + tmp, 100 + tmp, 100 + tmp),
								30 + rand() % 10, (double)Wind.x / 5, -0.5 - (double)(rand() % 10) / 10);
				}
				for (j = 0; j < 3; j++) // Feuer
				{
					tmp = rand() % 20;
					MakeFXPixel(Panzer[i].p.Pos.x - 10 + rand() % 20,
								Panzer[i].p.Pos.y - 1,
								RGB2DWORD(235 + tmp, 235, 10),
								15 + rand() % 10, 0, -0.5 - (double)(rand() % 10) / 10);
					break;
				}
			}
		}
	}
}

void CheckHimmelPixel()
{
	short i;
	ZWEID erg;

	for (i = 0; i < MAXHIMMELPIXEL; i++)
	{
		if (HimmelPixel[i].Art == ARTNICHTS)
			continue;

		Objekt.Art = HimmelPixel[i].Art;
		Objekt.Pos.x = HimmelPixel[i].Pos.x;
		Objekt.Pos.y = HimmelPixel[i].Pos.y;
		Objekt.Relx = HimmelPixel[i].Relx;
		Objekt.Rely = HimmelPixel[i].Rely;
		Objekt.v.x = HimmelPixel[i].v.x;
		Objekt.v.y = HimmelPixel[i].v.y;
		Objekt.Lifetime = HimmelPixel[i].Lifetime;

		erg = Physik(false, -1);
		// Wenn au�erhalb vom berechenbaren Bereich
		if ((erg.x == -1) && (erg.y == -1))
		{
			HimmelPixel[i].Art = ARTNICHTS;
			continue;
		}

		HimmelPixel[i].Relx = Objekt.Relx;
		HimmelPixel[i].Rely = Objekt.Rely;
		HimmelPixel[i].v.x = Objekt.v.x;
		HimmelPixel[i].v.y = Objekt.v.y;
		HimmelPixel[i].Pos.x = erg.x;
		HimmelPixel[i].Pos.y = erg.y;

		// Wenn in den sichtbaren Bereich
		if (erg.y >= HIMMELHOEHE)
		{
			MakePixel(erg.x, erg.y - HIMMELHOEHE + 1, HimmelPixel[i].Art,
					  (short)HimmelPixel[i].v.x, (short)HimmelPixel[i].v.y, true,
					  HimmelPixel[i].Lifetime);
			HimmelPixel[i].Art = ARTNICHTS;
		}
	}
}

void CheckFXPixel()
{
	short i;
	ZWEID neu;
	for (i = LastFX; i >= 0; i--)
	{
		if ((FXListe[i].Pos.x == -1) || (FXListe[i].Pos.y == -1))
		{
			if (i == LastFX)
				LastFX--;
			continue;
		}
		if (FXListe[i].Lifetime > 0)
		{
			if ((FXListe[i].Bewegung.x != 0) || (FXListe[i].Bewegung.y != 0))
			{
				FXListe[i].Rel.x += FXListe[i].Bewegung.x;
				FXListe[i].Rel.y += FXListe[i].Bewegung.y;
				neu.x = FXListe[i].Pos.x + (short)FXListe[i].Rel.x;
				neu.y = FXListe[i].Pos.y + (short)FXListe[i].Rel.y;
				FXListe[i].Rel.x = FXListe[i].Rel.x - (short)FXListe[i].Rel.x;
				FXListe[i].Rel.y = FXListe[i].Rel.y - (short)FXListe[i].Rel.y;
				if ((neu.x != FXListe[i].Pos.x) || (neu.y != FXListe[i].Pos.y))
				{
					// Im Bildschirm?
					if ((neu.x >= MAXX) || (neu.x < 0) || (neu.y >= MAXY) || (neu.y < 0))
					{
						// Wenn nicht, Pixel l�schen
						PutPixel(FXListe[i].Pos.x, FXListe[i].Pos.y, Tansparent, &lpDDSScape);
						if (Pixel[FXListe[i].Pos.x][FXListe[i].Pos.y].p.Art != ARTNICHTS)
						{
							MovePixel(FXListe[i].Pos.x, FXListe[i].Pos.y, FXListe[i].Pos.x, FXListe[i].Pos.y);
						}
						FXListe[i].Pos.x = -1;
						FXListe[i].Pos.y = -1;
						if (i == LastFX)
							LastFX--;
					}
					else
					{
						// Alten Pixel l�schen
						PutPixel(FXListe[i].Pos.x, FXListe[i].Pos.y, Tansparent, &lpDDSScape);
						if (Pixel[FXListe[i].Pos.x][FXListe[i].Pos.y].p.Art != ARTNICHTS)
						{
							MovePixel(FXListe[i].Pos.x, FXListe[i].Pos.y, FXListe[i].Pos.x, FXListe[i].Pos.y);
						}
						FXListe[i].Pos.x = neu.x;
						FXListe[i].Pos.y = neu.y;
					}
				}
			}
			if (i <= LastFX)
				PutPixel(FXListe[i].Pos.x, FXListe[i].Pos.y,
						 FXListe[i].Farbe, &lpDDSScape); // Pixel in die Surface malen
		}
		else
		{
			PutPixel(FXListe[i].Pos.x, FXListe[i].Pos.y, Tansparent, &lpDDSScape);
			if (Pixel[FXListe[i].Pos.x][FXListe[i].Pos.y].p.Art != ARTNICHTS)
			{
				MovePixel(FXListe[i].Pos.x, FXListe[i].Pos.y, FXListe[i].Pos.x, FXListe[i].Pos.y);
			}
			FXListe[i].Pos.x = -1;
			FXListe[i].Pos.y = -1;
			if (i == LastFX)
				LastFX--;
		}
		FXListe[i].Lifetime -= 1;
	}
}

void MakeFXPixel(short x, short y, DWORD Farbe, short Lifetime, double Bewx, double Bewy)
{
	int j;

	// Auserhalb des Bildschirms?
	if ((x >= MAXX) || (x < 0) ||
		(y >= MAXY) || (y < 0))
		return;

	for (j = 0; j <= LastFX; j++)
	{
		if ((FXListe[j].Pos.x != -1) && (FXListe[j].Pos.y != -1))
			continue;

		FXListe[j].Pos.x = x;
		FXListe[j].Pos.y = y;
		FXListe[j].Farbe = Farbe;
		FXListe[j].Lifetime = Lifetime;
		FXListe[j].Rel.x = 0.5;
		FXListe[j].Rel.y = 0.5;
		FXListe[j].Bewegung.x = Bewx;
		FXListe[j].Bewegung.y = Bewy;
		return;
	}
	// Hier kommt er hin, wenn Bereich bis LastFX nicht ausreicht
	LastFX++;
	FXListe[LastFX].Pos.x = x;
	FXListe[LastFX].Pos.y = y;
	FXListe[LastFX].Farbe = Farbe;
	FXListe[LastFX].Lifetime = Lifetime;
	FXListe[LastFX].Rel.x = 0.5;
	FXListe[LastFX].Rel.y = 0.5;
	FXListe[LastFX].Bewegung.x = Bewx;
	FXListe[LastFX].Bewegung.y = Bewy;
}

void CheckBallon()
{
	// Ballon erzeugen?
	if ((!Ballon.Aktiv) && (rand() % BALLONRANDOM == 0))
	{
		Ballon.Aktiv = true;
		Ballon.Pos.x = rand() % (MAXX - Bmp[BALLONBILD].Breite);
		Ballon.Pos.y = -Bmp[BALLONBILD].Hoehe;
		Ballon.Munition = (rand() % (MUNANZAHL - 2)) + 1;
		Ballon.Anzahl = (rand() % 3) + 1;
		Ballon.Rel.x = 0.5;
		Ballon.Rel.y = 0.5;
		audio_manager->play(audio::PARACHUTE, audio::id::SFX, 100, false);
		return;
	}
	if (!Ballon.Aktiv)
		return;
	// Bewegen
	Ballon.Rel.x += (double)Wind.x / 5;
	Ballon.Rel.y += 0.5;
	Ballon.Pos.x = Ballon.Pos.x + (short)Ballon.Rel.x;
	Ballon.Pos.y = Ballon.Pos.y + (short)Ballon.Rel.y;
	Ballon.Rel.x = Ballon.Rel.x - (short)Ballon.Rel.x;
	Ballon.Rel.y = Ballon.Rel.y - (short)Ballon.Rel.y;
	// Auf den Boden aufgeschlagen?
	if ((Pixel[Ballon.Pos.x + Bmp[BALLONBILD].Breite / 2][Ballon.Pos.y + Bmp[BALLONBILD].Hoehe + 1].p.Art != ARTNICHTS) &&
		(Ballon.Pos.x + Bmp[BALLONBILD].Breite / 2 >= 0) && (Ballon.Pos.x + Bmp[BALLONBILD].Breite / 2 < MAXX))
		BallonAbschuss(-1);
	// Au�erhalb des Bildes
	if ((Ballon.Pos.x >= MAXX) || (Ballon.Pos.x + Bmp[BALLONBILD].Breite < 0) ||
		(Ballon.Pos.y >= MAXY))
		Ballon.Aktiv = false;
}

void MoveKanone(short i, short Richtung)
{
	ZWEID NewPos;
	double Div1, Div2; // Unterschied zur richtigen L�nge

	NewPos = Panzer[i].KanonePos;

	if (Richtung >= 0)
		NewPos.x++;
	else
		NewPos.x--;
	if (Panzer[i].KanonePos.x >= 0)
	{
		if (Richtung >= 0)
			NewPos.y++;
		else
			NewPos.y--;
	}
	else
	{
		if (Richtung >= 0)
			NewPos.y--;
		else
			NewPos.y++;
	}
	// Die bessere neue Position ausrechnen
	Div1 = sqrt(NewPos.x * NewPos.x + Panzer[i].KanonePos.y * Panzer[i].KanonePos.y) - KANONENLAENGE;
	Div2 = sqrt(NewPos.y * NewPos.y + Panzer[i].KanonePos.x * Panzer[i].KanonePos.x) - KANONENLAENGE;
	if (Div1 < 0)
		Div1 = -Div1;
	if (Div2 < 0)
		Div2 = -Div2;

	if (Div1 <= Div2)
		Panzer[i].KanonePos.x = NewPos.x;
	else
		Panzer[i].KanonePos.y = NewPos.y;
	if (Panzer[i].KanonePos.y > -2)
		Panzer[i].KanonePos.y = -2;
	ZeichnePanzer(i, 0);
}

void Abschuss(short i)
{
	short j;

	if ((Panzer[i].SchussEnergie != 0) &&
		(Panzer[i].SchussAktiv == false) &&
		((Panzer[i].Lager[Panzer[i].Munition] > 0) ||
		 (Panzer[i].Lager[Panzer[i].Munition] == -1)))
	{
		if (Munition[Panzer[i].Munition].AbschussSound != 0)
			audio_manager->play(Munition[Panzer[i].Munition].AbschussSound,
					  audio::id::SFX, 100, false, false);

		if (Panzer[i].Munition == MUNSCHILD)
		{
			Panzer[i].SchussEnergie = 0;
			if (Panzer[i].Schild != 0)
				return;
			Panzer[i].Schild = Munition[MUNSCHILD].Dauer;
		}
		else if (Panzer[i].Munition == MUNMAGNET)
		{
			Panzer[i].SchussEnergie = 0;
			if (Panzer[i].Magnet != 0)
				return;
			Panzer[i].Magnet = Munition[MUNMAGNET].Dauer;
		}
		else if (Panzer[i].Munition == MUNLASER)
		{
			Panzer[i].SchussEnergie = 0;
			Laser(i);
		}
		else if (Panzer[i].Munition == MUNMEDI)
		{
			Panzer[i].SchussEnergie = 0;
			ChangeLebensEnergie(i, +MAXLEBENSENERGIE, -1);
		}
		else
		{
			for (j = 0; j < MAXMUN; j++)
			{
				if (MunListe[j].Besitzer != -1)
					continue;
				Panzer[i].SchussAktiv = true;
				MunListe[j].p.Art = Panzer[i].Munition;
				MunListe[j].p.Pos.x = Panzer[i].p.Pos.x + Panzer[i].KanonePos.x;
				MunListe[j].p.Pos.y = Panzer[i].p.Pos.y + Panzer[i].KanonePos.y;
				MunListe[j].p.v.x = Panzer[i].KanonePos.x * Panzer[i].SchussEnergie / (MAXSCHUSSENERGIE / 4);
				MunListe[j].p.v.y = Panzer[i].KanonePos.y * Panzer[i].SchussEnergie / (MAXSCHUSSENERGIE / 4);
				MunListe[j].Besitzer = i;
				MunListe[j].SoundBuffer = audio::NO_SOUND;
				MunListe[j].p.Lifetime = Munition[Panzer[i].Munition].Dauer;
				break;
			}
			if (Panzer[i].Ziel != -1)
				Panzer[i].Comp[Panzer[i].Ziel].LastSchussEnergie = Panzer[i].SchussEnergie;
		}
		if (Panzer[i].Lager[Panzer[i].Munition] != -1)
			Panzer[i].Lager[Panzer[i].Munition]--;
	}
	else if ((Panzer[i].Lager[Panzer[i].Munition] <= 0) &&
			 (Panzer[i].Lager[Panzer[i].Munition] != -1))
		Panzer[i].SchussEnergie = 0;
	ZeichnePanzer(i, 1);
}

void ChangeLebensEnergie(short i, short Anzahl, short Verursacher)
{
	bool ChangeVersion = false;

	if (Anzahl == 0)
		return;

	Panzer[i].LebensEnergie += Anzahl;
	if ((Verursacher != -1) && (Verursacher != i))
	{
		Panzer[Verursacher].Konto -= Anzahl;
		Panzer[Verursacher].Treffer -= Anzahl;
	}
	if (Panzer[i].LebensEnergie > MAXLEBENSENERGIE)
		Panzer[i].LebensEnergie = MAXLEBENSENERGIE;

	if (Panzer[i].LebensEnergie < 0)
	{
		if ((Verursacher != -1) && (Verursacher != i))
			Panzer[Verursacher].Abschuesse++;
		Panzer[i].LebensEnergie = 0;
		ZeichnePanzer(i, 2);
		Panzer[i].Aktiv = false;
		PanzerExpl(i);
		return;
	}

	if ((Panzer[i].LebensEnergie < MAXLEBENSENERGIE * 3 / 4) &&
		(Panzer[i].LebensEnergie > MAXLEBENSENERGIE / 4) &&
		(Panzer[i].Version != 1))
	{
		ChangeVersion = true;
		Panzer[i].Version = 1;
	}
	if ((Panzer[i].LebensEnergie > MAXLEBENSENERGIE * 3 / 4) &&
		(Panzer[i].Version != 0))
	{
		ChangeVersion = true;
		Panzer[i].Version = 0;
	}
	if ((Panzer[i].LebensEnergie < MAXLEBENSENERGIE / 4) &&
		(Panzer[i].Version != 2))
	{
		ChangeVersion = true;
		Panzer[i].Version = 2;
	}

	if (ChangeVersion)
		ZeichnePanzer(i, 0);

	/*if ((Anzahl < -1) || (Anzahl > +1))
	{*/
	ZeichnePanzer(i, 1); // muss noch verbessert werden
						 //}
}

void ChangeMunition(short i, short add)
{
	Panzer[i].Munition += add;
	if (Panzer[i].Munition >= MUNANZAHL)
		Panzer[i].Munition = 0;
	else if (Panzer[i].Munition < 0)
		Panzer[i].Munition = MUNANZAHL - 1;
	if (Panzer[i].Lager[Panzer[i].Munition] == 0)
		ChangeMunition(i, add);
	ZeichnePanzer(i, 1);
}

short CheckKollision(short x, short y, bool genau)
{
	short erg, i, xintern, yintern;

	erg = -1;
	for (i = 0; i < MAXSPIELER; i++) // Panzer
	{
		if (!Panzer[i].Aktiv)
			continue;
		// Grobe Einkreisung
		if ((x >= Panzer[i].p.Pos.x - Bmp[Panzer[i].Bild].Breite / 2) &&
			(x < Panzer[i].p.Pos.x + Bmp[Panzer[i].Bild].Breite / 2) &&
			(y >= Panzer[i].p.Pos.y - Bmp[Panzer[i].Bild].Hoehe) &&
			(y < Panzer[i].p.Pos.y))
		{
			if (genau) // F�r genaue Abfrage erst die Surface locken!!
			{
				xintern = Bmp[Panzer[i].Bild].rcSrc.left +
						  x - (Panzer[i].p.Pos.x - Bmp[Panzer[i].Bild].Breite / 2);
				yintern = Bmp[Panzer[i].Bild].rcSrc.top +
						  y - (Panzer[i].p.Pos.y - Bmp[Panzer[i].Bild].Hoehe);
				// Treffer?
				if (GetPixel(xintern, yintern, &lpDDSScape) != Tansparent)
				{
					if (Testmodus)
						PutPixel(xintern, yintern, RGB2DWORD(0, 0, 255), &lpDDSScape);
					erg = i;
					break;
				}
			}
			else
			{
				erg = i;
				break;
			}
		}
	}
	if (erg == -1)
	{
		for (i = 0; i < MAXMUN; i++) // Munition
		{
			if (MunListe[i].Besitzer == -1)
				continue;
			if ((MunListe[i].p.Pos.x == x) && (MunListe[i].p.Pos.y == y))
				continue;
			// Grobe Einkreisung
			if ((x >= MunListe[i].p.Pos.x - Bmp[Munition[MunListe[i].p.Art].Bild].Breite / 2) &&
				(x < MunListe[i].p.Pos.x + Bmp[Munition[MunListe[i].p.Art].Bild].Breite / 2) &&
				(y >= MunListe[i].p.Pos.y - Bmp[Munition[MunListe[i].p.Art].Bild].Hoehe / 2) &&
				(y < MunListe[i].p.Pos.y + Bmp[Munition[MunListe[i].p.Art].Bild].Hoehe / 2))
			{
				erg = i + MAXSPIELER;
				break;
			}
		}
	}
	// Ballontreffer?
	if ((erg == -1) && (Ballon.Aktiv))
	{
		// Grobe Einkreisung
		if ((x >= Ballon.Pos.x) &&
			(x < Ballon.Pos.x + Bmp[BALLONBILD].Breite) &&
			(y >= Ballon.Pos.y) &&
			(y < Ballon.Pos.y + Bmp[BALLONBILD].Hoehe))
		{
			erg = -2;
		}
	}
	return erg;
}

void FindActivePixel()
{
	short x, y, i, j, CheckNr, tmp;
	ZWEID erg;
	short PanzerDamage[MAXSPIELER]; // Liste des angerichteten Schadens
	for (i = 0; i < MAXSPIELER; i++)
		PanzerDamage[i] = 0;

	CheckNr = rand() % 1000; // Checknr damit jeder Pixel nur einmal im Durchgang berechnet wird

	for (i = 0; i < BERANZAHL; i++)
		for (j = 0; j < BERANZAHL; j++)
		{
			if ((!BereichAktiv[i][j]) && (!Testmodus))
				continue;
			for (x = i * (MAXX / BERANZAHL); x < (i + 1) * (MAXX / BERANZAHL); x++)
				for (y = j * (MAXY / BERANZAHL); y < (j + 1) * (MAXY / BERANZAHL); y++)
				{
					if ((!Pixel[x][y].Aktiv) || (Pixel[x][y].CheckNr == CheckNr) ||
						(Pixel[x][y].p.Art == ARTNICHTS))
						continue;

					if (Testmodus)
					{
						if (Pixel[x][y].Aktiv)
							PutPixel(x, y, RGB2DWORD(0, 255, 0), &lpDDSScape);
						else
							PutPixel(x, y, RGB2DWORD(255, 0, 0), &lpDDSScape);
					}

					Pixel[x][y].CheckNr = CheckNr;

					Objekt.Art = Pixel[x][y].p.Art;
					Objekt.Pos.x = x;
					Objekt.Pos.y = y;
					Objekt.Relx = Pixel[x][y].p.Relx;
					Objekt.Rely = Pixel[x][y].p.Rely;
					Objekt.v.x = Pixel[x][y].p.v.x;
					Objekt.v.y = Pixel[x][y].p.v.y;
					Objekt.Beschleunigung.x = Pixel[x][y].p.Beschleunigung.x;
					Objekt.Beschleunigung.y = Pixel[x][y].p.Beschleunigung.y;
					Objekt.Lifetime = Pixel[x][y].p.Lifetime;

					erg = Physik(true, -2);
					if ((erg.x == -1) && (erg.y == -1))
						continue;

					Pixel[x][y].p.Relx = Objekt.Relx;
					Pixel[x][y].p.Rely = Objekt.Rely;

					Pixel[x][y].p.v.x = Objekt.v.x;
					Pixel[x][y].p.v.y = Objekt.v.y;
					Pixel[x][y].p.Beschleunigung.x = 0;
					Pixel[x][y].p.Beschleunigung.y = 0;

					// Im Wasser (und S�ure) versinken
					if (((Pixel[x][y + 1].p.Art == ARTWASSER) || (Pixel[x][y + 1].p.Art == ARTSAEURE)) &&
						!((Pixel[x][y].p.Art == ARTWASSER) || (Pixel[x][y].p.Art == ARTSAEURE)))
					{
						MovePixel(x, y, x, y + 1);
						erg.x = x;
						erg.y = y;
					}

					// Pixel mit beschr�nkter Lebensdauer l�schen
					if (Bild % (LastBild / 10 + 1) == 0) // unabh�nig von der Framerate
					{
						if (Pixel[x][y].p.Lifetime > 0)
							Pixel[x][y].p.Lifetime--;
						if (Pixel[x][y].p.Lifetime == 0)
						{
							DeletePixel(x, y);
							continue;
						}
					}
					// Ber�hrung der S�ure mit Panzer?
					tmp = -1;
					if (Pixel[x][y].p.Art == ARTSAEURE)
						tmp = CheckKollision(x, y, false);
					if ((tmp >= 0) && (tmp < MAXSPIELER))
					{
						PanzerDamage[tmp]++;
						DeletePixel(x, y);
						continue;
					}

					// Pixel bewegen
					if ((sqrt(Pixel[x][y].p.v.x * Pixel[x][y].p.v.x +
							  Pixel[x][y].p.v.y * Pixel[x][y].p.v.y) < 0.05) && // Muss noch verbessert werden
						(erg.x == x) &&
						(erg.y == y) &&
						(Pixel[erg.x][erg.y + 1].p.Art != ARTNICHTS) &&
						(Pixel[x][y].p.Lifetime == -1))
						Aktivate(x, y, false); // Anhalten wenn zu langsam
					else
					{
						if ((erg.x >= MAXX) || (erg.y >= MAXY) ||
							(erg.x < 0) || (erg.y < 0)) // Auserhalb vom Bildschirm?
						{
							DeletePixel(x, y);
						}
						else if ((erg.x != x) || (erg.y != y))
						{
							MovePixel(x, y, erg.x, erg.y);
						}
					}
				}
			if ((Testmodus) && (BereichAktiv[i][j]))
				PutPixel(i * (MAXX / BERANZAHL), j * (MAXY / BERANZAHL), RGB2DWORD(0, 0, 255), &lpDDSScape);
		}
	// Energie abziehen
	for (i = 0; i < MAXSPIELER; i++)
		ChangeLebensEnergie(i, -PanzerDamage[i], -1);
}

ZWEID LineIntersect(short x1, short y1, short x2, short y2, bool gesamt)
{
	short i;
	float x, y;
	short Dx, Dy;
	float Sx, Sy;
	ZWEID erg;
	;
	short Steps;

	Steps = 0;
	erg.x = -1;
	erg.y = -1;

	if ((x1 >= MAXX) || (x1 < 0) || (y1 >= MAXY) || (y1 < 0))
		return erg;
	if ((x2 >= MAXX) || (x2 < 0) || (y2 >= MAXY) || (y2 < 0))
		return erg;

	// Sonderf�lle
	if ((x1 == x2) && (y1 == y2))
	{
		erg.x = x1;
		erg.y = y1;
		return erg;
	}

	Dx = x2 - x1;
	Dy = y2 - y1;
	x = x2;
	y = y2;
	if (abs(Dx) > abs(Dy))
	{
		if (Dx > 0)
			Sx = -1;
		else
			Sx = 1;
		if (Dx == 0)
			Sy = 0;
		else
			Sy = (float)Dy / ((float)(Dx * Sx));
		Steps = abs(Dx);
	}
	else
	{
		if (Dy > 0)
			Sy = -1;
		else
			Sy = 1;
		if (Dy == 0)
			Sx = 0;
		else
			Sx = (float)Dx / ((float)(Dy * Sy));
		Steps = abs(Dy);
	}

	for (i = 0; i < Steps; i++)
	{
		if (!gesamt)
		{
			if (Pixel[(short)(x)][(short)(y)].p.Art == ARTNICHTS)
			{
				erg.x = (short)(x);
				erg.y = (short)(y);
				break;
			}
		}
		if (gesamt) // F�r den Computergegner
		{
			if (Pixel[(short)(x)][(short)(y)].p.Art != ARTNICHTS)
			{
				erg.x = -1;
				erg.y = -1;
				return erg;
			}
		}
		x += Sx;
		y += Sy;
	}
	if (erg.x == -1) // Wenn keinen freien Pixel gefunden, nehme die Anfangspixel
	{
		erg.x = x1;
		erg.y = y1;
	}
	return erg;
}

void MovePixel(short x, short y, short xneu, short yneu)
{
	PIXEL Pixeltmp;

	if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
		return;
	if ((xneu >= MAXX) || (xneu < 0) || (yneu >= MAXY) || (yneu < 0))
		return;

	Pixeltmp.p.Art = Pixel[x][y].p.Art;
	Pixeltmp.p.Relx = Pixel[x][y].p.Relx;
	Pixeltmp.p.Rely = Pixel[x][y].p.Rely;
	Pixeltmp.p.v.x = Pixel[x][y].p.v.x;
	Pixeltmp.p.v.y = Pixel[x][y].p.v.y;
	Pixeltmp.Farbe = Pixel[x][y].Farbe;
	Pixeltmp.CheckNr = Pixel[x][y].CheckNr;
	Pixeltmp.p.Lifetime = Pixel[x][y].p.Lifetime;

	Pixel[x][y].p.Art = Pixel[xneu][yneu].p.Art;
	Pixel[x][y].p.Relx = Pixel[xneu][yneu].p.Relx;
	Pixel[x][y].p.Rely = Pixel[xneu][yneu].p.Rely;
	Pixel[x][y].p.v.x = Pixel[xneu][yneu].p.v.x;
	Pixel[x][y].p.v.y = Pixel[xneu][yneu].p.v.y;
	Pixel[x][y].Farbe = Pixel[xneu][yneu].Farbe;
	Pixel[x][y].CheckNr = Pixel[xneu][yneu].CheckNr;
	Pixel[x][y].p.Lifetime = Pixel[xneu][yneu].p.Lifetime;

	if (!Testmodus)
	{
		PutPixel(x, y, Pixel[x][y].Farbe, &lpDDSScape);
	}
	Aktivate(x, y, false);
	// Alle Pixel ausenherum aktivieren
	if (x - 1 >= 0)
		if (Pixel[x - 1][y].p.Art != ARTNICHTS)
			Aktivate(x - 1, y, true);
	if ((x - 1 >= 0) && (y - 1 >= 0))
		if (Pixel[x - 1][y - 1].p.Art != ARTNICHTS)
			Aktivate(x - 1, y - 1, true);
	if (y - 1 >= 0)
		if (Pixel[x][y - 1].p.Art != ARTNICHTS)
			Aktivate(x, y - 1, true);
	if ((x + 1 < MAXX) && (y - 1 >= 0))
		if (Pixel[x + 1][y - 1].p.Art != ARTNICHTS)
			Aktivate(x + 1, y - 1, true);
	if (x + 1 < MAXX)
		if (Pixel[x + 1][y].p.Art != ARTNICHTS)
			Aktivate(x + 1, y, true);
	if ((x + 1 < MAXX) && (y + 1 < MAXY))
		if (Pixel[x + 1][y + 1].p.Art != ARTNICHTS)
			Aktivate(x + 1, y + 1, true);
	if (y + 1 < MAXY)
		if (Pixel[x][y + 1].p.Art != ARTNICHTS)
			Aktivate(x, y + 1, true);
	if ((x - 1 >= 0) && (y + 1 < MAXY))
		if (Pixel[x - 1][y + 1].p.Art != ARTNICHTS)
			Aktivate(x - 1, y + 1, true);

	Pixel[xneu][yneu].p.Art = Pixeltmp.p.Art;
	Pixel[xneu][yneu].p.Relx = Pixeltmp.p.Relx;
	Pixel[xneu][yneu].p.Rely = Pixeltmp.p.Rely;
	Pixel[xneu][yneu].p.v.x = Pixeltmp.p.v.x;
	Pixel[xneu][yneu].p.v.y = Pixeltmp.p.v.y;
	Pixel[xneu][yneu].Farbe = Pixeltmp.Farbe;
	Pixel[xneu][yneu].CheckNr = Pixeltmp.CheckNr;
	Pixel[xneu][yneu].p.Lifetime = Pixeltmp.p.Lifetime;

	if (!Testmodus)
	{
		PutPixel(xneu, yneu, Pixel[xneu][yneu].Farbe, &lpDDSScape); // Pixel malen
	}

	Aktivate(xneu, yneu, true);
}

void DeletePixel(short x, short y)
{
	if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
		return;

	Pixel[x][y].p.Art = ARTNICHTS;
	Pixel[x][y].p.Relx = 0.5;
	Pixel[x][y].p.Rely = 0.5;
	Pixel[x][y].p.v.x = 0;
	Pixel[x][y].p.v.y = 0;
	Pixel[x][y].Farbe = Tansparent;
	Pixel[x][y].p.Lifetime = -1;
	PutPixel(x, y, Pixel[x][y].Farbe, &lpDDSScape);
	Aktivate(x, y, false);
	// Alle Pixel ausenherum aktivieren
	if (x - 1 >= 0)
		if (Pixel[x - 1][y].p.Art != ARTNICHTS)
			Aktivate(x - 1, y, true);
	if ((x - 1 >= 0) && (y - 1 >= 0))
		if (Pixel[x - 1][y - 1].p.Art != ARTNICHTS)
			Aktivate(x - 1, y - 1, true);
	if (y - 1 >= 0)
		if (Pixel[x][y - 1].p.Art != ARTNICHTS)
			Aktivate(x, y - 1, true);
	if ((x + 1 < MAXX) && (y - 1 >= 0))
		if (Pixel[x + 1][y - 1].p.Art != ARTNICHTS)
			Aktivate(x + 1, y - 1, true);
	if (x + 1 < MAXX)
		if (Pixel[x + 1][y].p.Art != ARTNICHTS)
			Aktivate(x + 1, y, true);
	if ((x + 1 < MAXX) && (y + 1 < MAXY))
		if (Pixel[x + 1][y + 1].p.Art != ARTNICHTS)
			Aktivate(x + 1, y + 1, true);
	if (y + 1 < MAXY)
		if (Pixel[x][y + 1].p.Art != ARTNICHTS)
			Aktivate(x, y + 1, true);
	if ((x - 1 >= 0) && (y + 1 < MAXY))
		if (Pixel[x - 1][y + 1].p.Art != ARTNICHTS)
			Aktivate(x - 1, y + 1, true);
}

void MakePixel(short x, short y, short Typ, short vx, short vy, bool Active, long Lifetime)
{
	short r, g, b;

	if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
		return;

	Pixel[x][y].p.Art = Typ;
	if (Pixel[x][y].p.Art == ARTNICHTS)
		return;
	Pixel[x][y].p.v.x = vx;
	Pixel[x][y].p.v.y = vy;
	Pixel[x][y].p.Lifetime = Lifetime;
	if (Active)
		Aktivate(x, y, true);
	if (Typ != ARTSTEIN)
	{
		r = Art[Typ].Farbe.r - ZFARBE + rand() % (ZFARBE * 2);
		if (r > 255)
			r = 255;
		if (r < 0)
			r = 0;
		g = Art[Typ].Farbe.g - ZFARBE + rand() % (ZFARBE * 2);
		if (g > 255)
			g = 255;
		if (g < 0)
			g = 0;
		b = Art[Typ].Farbe.b - ZFARBE + rand() % (ZFARBE * 2);
		if (b > 255)
			b = 255;
		if (b < 0)
			b = 0;
		Pixel[x][y].Farbe = RGB2DWORD((BYTE)r, (BYTE)g, (BYTE)b);
	}
	else
	{
		auto col = GetPixel(x % Bmp[TEXFELS].Breite,
									 y % Bmp[TEXFELS].Hoehe, &Bmp[TEXFELS].Surface);
		// get rid of alpha channel in argb value
		auto r = col & 0xFF;
		auto g = (col >> 8) & 0xFF;
		auto b = (col >> 16) & 0xFF;
		b -= 75; // TODO: This is a hack

		col = SDL_MapRGB(Bmp[TEXFELS].Surface.surface->format, r, g, b);
		Pixel[x][y].Farbe = col;
	}

	PutPixel(x, y, Pixel[x][y].Farbe, &lpDDSScape); // Pixel in die Surface malen
}

double Steigung(short x, short y)
{
	double xdiv, ydiv, erg;
	short y1, y2, x1, x2;

	if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
		return 0;

	// Sonderf�lle
	if ((x + 1 >= MAXX) || (x - 1 <= 0) || (y + 1 >= MAXY) || (y - 1 <= 0))
		erg = 0;
	else if (Pixel[x][y + 1].p.Art == ARTNICHTS)
		erg = 0.5 * pi;
	else if ((Pixel[x + 1][y + 1].p.Art == ARTNICHTS) || (Pixel[x - 1][y + 1].p.Art == ARTNICHTS))
	{
		for (y1 = y; y1 <= MAXY; y1++)
		{
			if (Pixel[x - 1][y1].p.Art != ARTNICHTS)
				break;
		}
		for (y2 = y; y2 <= MAXY; y2++)
		{
			if (Pixel[x + 1][y2].p.Art != ARTNICHTS)
				break;
		}
		if (y1 >= y2)
		{
			xdiv = -1;
			ydiv = y1 - y;
			erg = pi + atan(ydiv / xdiv);
		}
		else
		{
			xdiv = +1;
			ydiv = y2 - y;
			erg = atan(ydiv / xdiv);
		}
	}
	else
	{
		for (x1 = x - 1; x1 >= 0; x1--)
		{
			if (Pixel[x1][y].p.Art != ARTNICHTS)
			{
				x1 = -1;
				break;
			}
			if (Pixel[x1][y + 1].p.Art == ARTNICHTS)
				break;
		}
		for (x2 = x + 1; x2 < MAXX; x2++)
		{
			if (Pixel[x2][y].p.Art != ARTNICHTS)
			{
				x2 = -1;
				break;
			}
			if (Pixel[x2][y + 1].p.Art == ARTNICHTS)
				break;
		}
		if (x1 == -1)
		{
			if (x2 == -1)
			{
				erg = 0;
			}
			else
			{
				xdiv = x2 - x;
				ydiv = 1;
				erg = atan(ydiv / xdiv);
			}
		}
		else if (x2 == -1)
		{
			xdiv = x1 - x;
			ydiv = 1;
			erg = pi + atan(ydiv / xdiv);
		}
		else if (abs(x1 - x) >= abs(x2 - x))
		{
			xdiv = x2 - x;
			ydiv = 1;
			erg = atan(ydiv / xdiv);
		}
		else
		{
			xdiv = x1 - x;
			ydiv = 1;
			erg = pi + atan(ydiv / xdiv);
		}
	}
	if (erg < 0)
		erg = 0; // Sicherheitsabfrage
	return (erg);
}

void MakeMagRakete()
{
	ZWEIDGENAU EinzelKraft, dif;
	short i;
	double Entf, Kraft;

	for (i = 0; i < MAXSPIELER; i++)
	{
		if (!Panzer[i].Aktiv)
			continue;
		dif.x = Objekt.Pos.x - Panzer[i].p.Pos.x;
		dif.y = Objekt.Pos.y - Panzer[i].p.Pos.y;
		Entf = dif.x * dif.x + dif.y * dif.y;
		Kraft = MAGNETKONSTANTE / Entf;
		EinzelKraft.x = -dif.x * Kraft / sqrt(Entf);
		EinzelKraft.y = -dif.y * Kraft / sqrt(Entf);
		Objekt.Beschleunigung = AddBeschleunigung(Objekt.Beschleunigung, EinzelKraft);
	}
}

void MakeMagnet(short i)
{
	ZWEIDGENAU EinzelKraft, dif;
	short j;
	double Entf, Kraft;

	for (j = 0; j < MAXMUN; j++)
	{
		if ((MunListe[j].Besitzer == -1) ||
			(MunListe[j].p.Art == MUNSTEIN) ||
			(MunListe[j].p.Art == MUNSCHNEEBALL) ||
			(MunListe[j].p.Art == MUNMEGA))
			continue;
		dif.x = MunListe[j].p.Pos.x - Panzer[i].p.Pos.x;
		dif.y = MunListe[j].p.Pos.y - Panzer[i].p.Pos.y;
		Entf = dif.x * dif.x + dif.y * dif.y;
		Kraft = MAGNETKONSTANTE / Entf;
		EinzelKraft.x = dif.x * Kraft / sqrt(Entf);
		EinzelKraft.y = dif.y * Kraft / sqrt(Entf);
		MunListe[j].p.Beschleunigung = AddBeschleunigung(MunListe[j].p.Beschleunigung, EinzelKraft);
	}
}

ZWEIDGENAU AddBeschleunigung(ZWEIDGENAU Add, ZWEIDGENAU Neu)
{
	ZWEIDGENAU Erg;

	Erg.x = Add.x + Neu.x;
	Erg.y = Add.y + Neu.y;

	return (Erg); // test
}

void MakeWetter()
{
	if (Bild % (LastBild / 10 + 1) != 0)
		return; // unabh�nig von der Framerate

	if (Wetter == 0)
		return;
	else if (Wetter == 1)
	{
		MakePixel(rand() % MAXX, 2, ARTWASSER, 0, 0, true, -1);
	}
	else if (Wetter == 2)
	{
		MakePixel(rand() % MAXX, 2, ARTSCHNEE, 0, 0, true, -1);
	}
}

void NeuesSpiel()
{
	int i;

	if (AktRunde == 0)
		InitStructs(1);
	else
		InitStructs(2);

	GetMenue();

	AktMenue = MENSTART;
	AktRunde++;
	Menue[AktMenue].putZiffer(2, AktRunde);

	lpDDSScape.clear();

	lpDDSScape.lock();
	for (int x = 0; x < MAXX; x++)
        for (int y = 0; y < MAXY; y++)
			lpDDSScape.lpSurface[y * (lpDDSScape.lPitch/4) + x] = 0;
	lpDDSScape.unlock();

	// Windbeschleunigung
	if (Windschalter)
		Wind.x = rand() % 7 - 3;
	else
		Wind.x = 0;
	Wind.y = 0;

	// Wetter
	if (Wetterschalter)
		Wetter = rand() % 3;
	else
		Wetter = 0;

	// Himmel
	Himmel = rand() % 3;

	// F�r die Statusanzeige
	rcRectdes.left = 0;
	rcRectdes.top = 0;
	rcRectdes.right = MAXX;
	rcRectdes.bottom = MAXY;
	rcRectsrc.left = Bmp[MLADESPIEL].rcSrc.left;
	rcRectsrc.top = Bmp[MLADESPIEL].rcSrc.top;
	rcRectsrc.right = Bmp[MLADESPIEL].rcSrc.right;
	rcRectsrc.bottom = Bmp[MLADESPIEL].rcSrc.bottom;
	rcRectdes.left = MAXX / 2 - Bmp[MLADESPIEL].Breite / 2;
	rcRectdes.top = MAXY / 2 - Bmp[MLADESPIEL].Hoehe / 2;
	rcRectdes.right = rcRectdes.top + Bmp[MLADESPIEL].Breite;
	rcRectdes.bottom = rcRectdes.top + Bmp[MLADESPIEL].Hoehe;
	rcRectdes.left = 0;
	rcRectdes.top = 0;
	rcRectdes.right = MAXX;
	rcRectdes.bottom = MAXY;
	// Schriftsurface l�schen
	// ddbltfx.dwFillColor = Tansparent;
	// lpDDSSchrift->Blt(&rcRectdes, NULL,NULL,DDBLT_COLORFILL, &ddbltfx);
	// Scapesurface l�schen
	lpDDSScape.clear();
	// PanzSavesurface l�schen
	rcRectdes.left = 0;
	rcRectdes.top = 0;
	rcRectdes.right = MAXSPIELER * PANZERBREITE;
	rcRectdes.bottom = PANZERHOEHE;
	// ddbltfx.dwFillColor = Tansparent;
	lpDDSPanzSave.clear();

	// Landschaft erzeugen
	printf("Generating landscape...\n");
	Compute();
	printf("Landscape generated.\n");

	AktMenue = MENSTART;
	// Panzer setzen
	PutPanzer();

	Spielzustand = SZSPIEL;

	// Schriftsurface l�schen
	/*rcRectdes.left		= 0;
	rcRectdes.top		= 0;
	rcRectdes.right		= MAXX;
	rcRectdes.bottom	= MAXY;
	ddbltfx.dwFillColor = Tansparent;
	lpDDSSchrift->Blt(&rcRectdes, NULL,NULL,DDBLT_COLORFILL, &ddbltfx);
	*/

	for (i = 0; i < MAXSPIELER; i++)
	{
		ZeichnePanzer(i, 1);
		ZeichnePanzer(i, 0);
	}
	ZeichneBmp(0, PANZERHOEHE, WINDHOSE, rcGesamt, (short)Wind.x + 3, lpDDSPanzSave.texture);

}

void InitCredits()
{
	short x, y;

	InitStructs(2); // Hier wird eventuell zu viel gemacht

	lpDDSScape.clear();

	lpDDSScape.lock();
	for (int x = 0; x < MAXX; x++)
        for (int y = 0; y < MAXY; y++)
			lpDDSScape.lpSurface[y * (lpDDSScape.lPitch/4) + x] = 0;
	lpDDSScape.unlock();
	AktMenue = -1;
	Wind.x = 0;
	Wind.y = 0;
	Wetter = 0;
	Spielzustand = SZCREDITS;
	CreditsZaehler = 0;

	// Boden einbauen
	lpDDSScape.lock();
	for (x = 0; x < MAXX; x++)
		for (y = MAXY - 2; y < MAXY; y++)
			MakePixel(x, y, ARTSTEIN, 0, 0, false, -1);
	lpDDSScape.unlock();
	audio_manager->play(audio::CREDITS_MUSIC, audio::id::MUSIC, 255, true, false);
}

void CheckCredits()
{
	short x, y, Relx, Rely, i, Material;
	DWORD Farbe;

	CreditsZaehler++;

	for (i = 0; i < MAXCREDITS; i++)
	{
		if (CreditsListe[i].Bild == -1)
		{
			CreditsZaehler = 0;
			return;
		}
		if (CreditsListe[i].Zeit * 2 == CreditsZaehler)
			break;
		if (CreditsListe[i].Zeit * 2 > CreditsZaehler)
			return;
	}

	if (CreditsListe[i].Bild == -2)
	{
		for (x = 0; x < MAXX; x++)
			for (y = 20; y < 68; y++)
				Aktivate(x, y, true);
		return;
	}
	if (CreditsListe[i].Bild == -3)
	{
		for (x = 0; x < MAXX; x++)
			for (y = MAXY / 2 - 24; y < MAXY / 2 + 24; y++)
				Aktivate(x, y, true);
		return;
	}

	Bild = CreditsListe[i].Bild;
	if (CreditsListe[i].Ueberschrift)
		for (x = 0; x < MAXX; x++)
			for (y = 20; y < 68; y++)
				Aktivate(x, y, false);
	else
		for (x = 0; x < MAXX; x++)
			for (y = MAXY / 2 - 24; y < MAXY / 2 + 24; y++)
				Aktivate(x, y, false);

	if (CreditsListe[i].Ueberschrift)
		y = 20;
	else
		y = MAXY / 2 - Bmp[Bild].Hoehe / 2;
	x = MAXX / 2 - Bmp[Bild].Breite / 2;
	Material = 1 + (rand() % 5);


	lpDDSScape.lock();
	for (Rely = 0; Rely < Bmp[Bild].Hoehe; Rely++) {
		for (Relx = 0; Relx < Bmp[Bild].Breite; Relx++)
		{
			Farbe = GetPixel2(Bmp[Bild].rcSrc.left + Relx, Bmp[Bild].rcSrc.top + Rely, &Bmp[Bild].Surface);
			if (Farbe == 0xffffffff)
				continue;
			if (CreditsListe[i].Ueberschrift)
				MakePixel(x + Relx, y + Rely, Material, 0, 0, false, -1);
			else
				MakePixel(x + Relx, y + Rely, Material, 0, 0, false, -1);
		}
	}
	audio_manager->play(audio::CREDITS, audio::id::SFX, 100, false);
	lpDDSScape.unlock();
	SDL_UnlockTexture(Bmp[Bild].Surface.texture);
}

void Zeige(bool flippen)
{
	int ddrval;

	rcRectsrc.left = 0;
	rcRectsrc.top = 0;
	rcRectsrc.right = MAXX;
	rcRectsrc.bottom = MAXY;
	rcRectdes.left = 0;
	rcRectdes.top = 0;
	rcRectdes.right = MAXX;
	rcRectdes.bottom = MAXY;

	if ((Spielzustand == SZCREDITS) || (Testmodus))
		Himmel = 1;

	// Hintergrund zeichnen
	if (Himmel == 0)
		Blitten2(lpDDSHimmel.texture, lpDDSBack.texture, false);
	else if (Himmel == 1)
		Blitten2(lpDDSHimmel2.texture, lpDDSBack.texture, false);
	else if (Himmel == 2)
		Blitten2(lpDDSHimmel3.texture, lpDDSBack.texture, false);

	Blitten2(lpDDSScape.texture, lpDDSBack.texture, true); // Landschaft zeichnen

	ZeigePanzer(); // Panzer + Schutzschild

	ZeichneMunition(); // Munition zeichnen

	// Ballon malen
	if (Ballon.Aktiv)
	{
		ZeichneBmp(Ballon.Pos.x, Ballon.Pos.y, BALLONBILD, rcGesamt, 0, lpDDSBack.texture);
		if ((Ballon.Munition == MUNGEWEHR) || (Ballon.Munition == MUNFASS))
		{
			ZeichneBmp(Ballon.Pos.x + 11,
					   Ballon.Pos.y + 42,
					   Munition[Ballon.Munition].Bild, rcGesamt, 8, lpDDSBack.texture);
		}
		else
			ZeichneBmp(Ballon.Pos.x + 11,
					   Ballon.Pos.y + 42,
					   Munition[Ballon.Munition].Bild, rcGesamt, 0, lpDDSBack.texture);
		sprintf(StdString, "%d", Ballon.Anzahl);
		DrawString(StdString, Ballon.Pos.x + 10, Ballon.Pos.y + 42, 2, lpDDSBack.texture);
	}

	// Windhose malen
	if ((Spielzustand == SZSPIEL) && (Wind.x != 0))
	{
		rcRectsrc.left = 0;
		rcRectsrc.top = PANZERHOEHE;
		rcRectsrc.right = Bmp[WINDHOSE].Breite;
		rcRectsrc.bottom = PANZERHOEHE + Bmp[WINDHOSE].Hoehe;
		rcRectdes.left = MAXX - Bmp[WINDHOSE].Breite - 5;
		rcRectdes.top = 2;
		rcRectdes.right = MAXX - 5;
		rcRectdes.bottom = 2 + Bmp[WINDHOSE].Hoehe;
		Blitten2(lpDDSPanzSave.texture, lpDDSBack.texture, false);
	}

	/*rcRectsrc.left		= 0;
	rcRectsrc.top		= 0;
	rcRectsrc.right		= MAXX;
	rcRectsrc.bottom	= MAXY;
	rcRectdes.left		= 0;
	rcRectdes.top		= 0;
	rcRectdes.right		= MAXX;
	rcRectdes.bottom	= MAXY;
	Blitten(lpDDSSchrift,lpDDSBack,true); //Schrift zeichnen
*/
	// Flippen
	if (flippen)
	{
			// int result = SDL_RenderCopy(renderer, lpDDSBack.texture, NULL, NULL);
	}
}

void MakeTitel()
{
	rcRectsrc.left = 0;
	rcRectsrc.top = 0;
	rcRectsrc.right = MAXX;
	rcRectsrc.bottom = MAXY;
	rcRectdes.left = 0;
	rcRectdes.top = 0;
	rcRectdes.right = MAXX;
	rcRectdes.bottom = MAXY;
	SDL_RenderCopy(renderer, lpDDSTitel.texture, NULL, NULL);


	// PlaySound(WAVTITELMUSIK, 100, MAXX / 2, -1, true, -1);
}

void ZeigePanzer()
{
	int i;

	for (i = 0; i < MAXSPIELER; i++)
	{
		if (!Panzer[i].Aktiv)
			continue;

		rcRectsrc.left = i * PANZERBREITE;
		rcRectsrc.top = 0;
		rcRectsrc.right = rcRectsrc.left + PANZERBREITE;
		rcRectsrc.bottom = rcRectsrc.top + PANZERHOEHE;
		rcRectdes.left = Panzer[i].p.Pos.x - PANZERBREITE / 2;
		rcRectdes.top = Panzer[i].p.Pos.y - 20;
		rcRectdes.right = rcRectdes.left + PANZERBREITE;
		rcRectdes.bottom = rcRectdes.top + PANZERHOEHE;
		CalcRect(rcGesamt);
		Blitten2(lpDDSPanzSave.texture, lpDDSBack.texture, true); // Panzer zeichnen

		if (Panzer[i].Schild > 0)
		{
			if (Bild % (LastBild / 30 + 1) == 0)
				Panzer[i].Schild--;
			if (Bild % 2 == 0)
				ZeichneBmp(Panzer[i].p.Pos.x - 15, Panzer[i].p.Pos.y - 14, SCHILD, rcGesamt, 0, lpDDSBack.texture);
		}
		if (Panzer[i].Magnet > 0)
		{
			if (Bild % (LastBild / 30 + 1) == 0)
				Panzer[i].Magnet--;
			ZeichneBmp(Panzer[i].p.Pos.x - 5, Panzer[i].p.Pos.y - 13, MAGNET, rcGesamt, 0, lpDDSBack.texture);
		}
	}
}

void ZeichneMunition()
{
	short i, Richtung;
	double Grad, x, y;

	for (i = 0; i < MAXMUN; i++)
	{
		if (MunListe[i].Besitzer == -1)
			continue;
		// Geschoss
		if (MunListe[i].Ausserhalb == false)
		{
			// BildVersion rausfinden
			x = MunListe[i].p.v.x;
			y = MunListe[i].p.v.y;
			if (x == 0)
			{
				if (y >= 0)
					Richtung = 2;
				else
					Richtung = 6;
			}
			else
			{
				Grad = atan(y / x);
				if (x >= 0)
				{
					if (Grad <= -3 * pi / 8)
						Richtung = 6;
					else if (Grad <= -pi / 8)
						Richtung = 7;
					else if (Grad <= +pi / 8)
						Richtung = 0;
					else if (Grad <= +3 * pi / 8)
						Richtung = 1;
					else
						Richtung = 2;
				}
				else
				{
					if (Grad <= -3 * pi / 8)
						Richtung = 2;
					else if (Grad <= -pi / 8)
						Richtung = 3;
					else if (Grad <= +pi / 8)
						Richtung = 4;
					else if (Grad <= +3 * pi / 8)
						Richtung = 5;
					else
						Richtung = 6;
				}
			}
			ZeichneBmp(MunListe[i].p.Pos.x - Bmp[Munition[MunListe[i].p.Art].Bild].Breite / 2,
					   MunListe[i].p.Pos.y - Bmp[Munition[MunListe[i].p.Art].Bild].Hoehe / 2,
					   Munition[MunListe[i].p.Art].Bild, rcGesamt, Richtung, lpDDSBack.texture);
		}
		else
		{
			// Wenn Munition auserhalb des Bildschirm
			ZeichneLinie(MunListe[i].p.Pos.x - 5, 0, MunListe[i].p.Pos.x + 5, 0,
						 Panzer[MunListe[i].Besitzer].Farbe, lpDDSBack);
		}
	}
}

void ZeichnePanzer(short i, short Teil)
{
	if (!Panzer[i].Aktiv)
		return;
	SDL_Rect rc{};

	switch (Teil)
	{
	case 0:
		rcRectdes.left = PANZERBREITE * i;
		rcRectdes.top = 0;
		rcRectdes.right = rcRectdes.left + PANZERBREITE;
		rcRectdes.bottom = PanzSavPos[i].y;

		SDL_SetRenderTarget(renderer, lpDDSPanzSave.texture);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		rc = {rcRectdes.left, rcRectdes.top, rcRectdes.right - rcRectdes.left, rcRectdes.bottom - rcRectdes.top};
		SDL_RenderFillRect(renderer, &rc);
		SDL_SetRenderTarget(renderer, nullptr);

		// lpDDSPanzSave.lock();
		// Kanone
		ZeichneLinie(PanzSavPos[i].x + Bmp[Panzer[i].Bild].Breite / 2,
					 PanzSavPos[i].y,
					 PanzSavPos[i].x + Bmp[Panzer[i].Bild].Breite / 2 + Panzer[i].KanonePos.x,
					 PanzSavPos[i].y + Panzer[i].KanonePos.y,
					 Panzer[i].Farbe, lpDDSPanzSave);
		// lpDDSPanzSave.unlock();

		// Panzer
		ZeichneBmp(PanzSavPos[i].x,
				   PanzSavPos[i].y - Bmp[Panzer[i].Bild].Hoehe,
				   Panzer[i].Bild, rcGesamt, Panzer[i].Version, lpDDSPanzSave.texture);
		lpDDSPanzSave.Blt(&rcRectdes, NULL, NULL);
		break;
	case 1:
		// Munitionsbox
		ZeichneBmp(PanzSavPos[i].x,
				   PanzSavPos[i].y + 2,
				   MUNBOX, rcGesamt, 0, lpDDSPanzSave.texture);
		// SchussEnergieanzeige
		if (Panzer[i].SchussEnergie != 0)
		{
			rcRectsrc = Bmp[POWER].rcSrc;
			rcRectsrc.right = rcRectsrc.left +
							  Bmp[POWER].Breite * Panzer[i].SchussEnergie / MAXSCHUSSENERGIE;
			rcRectdes.left = PanzSavPos[i].x + 1;
			rcRectdes.top = PanzSavPos[i].y + 17;
			rcRectdes.right = rcRectdes.left +
							  Bmp[POWER].Breite * Panzer[i].SchussEnergie / MAXSCHUSSENERGIE;
			rcRectdes.bottom = rcRectdes.top + Bmp[POWER].Hoehe;
			CalcRect(rcGesamt);
			Blitten2(Bmp[POWER].Surface.texture, lpDDSPanzSave.texture, false);
		}
		// SchussEnergieschrift
		sprintf(StdString, "%d", Panzer[i].SchussEnergie);
		DrawString(StdString, PanzSavPos[i].x + 4, PanzSavPos[i].y + 22, 2, lpDDSPanzSave.texture);
		// Lebensenergieanzeige
		rcRectsrc = Bmp[ENERGIE].rcSrc;
		rcRectsrc.top = rcRectsrc.bottom - Bmp[ENERGIE].Hoehe * Panzer[i].LebensEnergie / MAXLEBENSENERGIE;
		rcRectdes.left = PanzSavPos[i].x + 1;
		rcRectdes.top = PanzSavPos[i].y + 3 +
						Bmp[ENERGIE].Hoehe - Bmp[ENERGIE].Hoehe * Panzer[i].LebensEnergie / MAXLEBENSENERGIE;
		rcRectdes.right = rcRectdes.left + Bmp[ENERGIE].Breite;
		rcRectdes.bottom = rcRectdes.top + Bmp[ENERGIE].Hoehe * Panzer[i].LebensEnergie / MAXLEBENSENERGIE;
		;
		CalcRect(rcGesamt);
		Blitten2(Bmp[ENERGIE].Surface.texture, lpDDSPanzSave.texture, false);
		// Munition
		if ((Panzer[i].Munition == MUNGEWEHR) || (Panzer[i].Munition == MUNFASS))
		{
			ZeichneBmp(PanzSavPos[i].x + 7,
					   PanzSavPos[i].y + 5,
					   Munition[Panzer[i].Munition].Bild, rcGesamt, 8, lpDDSPanzSave.texture);
		}
		else
			ZeichneBmp(PanzSavPos[i].x + 7,
					   PanzSavPos[i].y + 5,
					   Munition[Panzer[i].Munition].Bild, rcGesamt, 0, lpDDSPanzSave.texture);
		// Munitionsanzahl
		if (Panzer[i].Lager[Panzer[i].Munition] >= 0)
		{
			sprintf(StdString, "%d", Panzer[i].Lager[Panzer[i].Munition]);
			DrawString(StdString, PanzSavPos[i].x + 5,
					   PanzSavPos[i].y + 4, 2, lpDDSPanzSave.texture);
		}
		break;
	case 2:
		// Panzer l�schen
		rcRectdes.left = PANZERBREITE * i;
		rcRectdes.top = 0;
		rcRectdes.right = rcRectdes.left + PANZERBREITE;
		rcRectdes.bottom = PANZERHOEHE;
		// ddbltfx.dwFillColor = Tansparent;
		lpDDSPanzSave.Blt(&rcRectdes, NULL, NULL);
		break;
	}
}

void ZeichneBmp(short x, short y, short i, RECT, short Version, SDL_Texture *lpDDSNach)
{
	if (i < 0) return;
	SDL_Rect src {
		.x = Bmp[i].rcSrc.left,
		.y = Bmp[i].rcSrc.top + Version * Bmp[i].Hoehe,
		.w = Bmp[i].Breite,
		.h = Bmp[i].Hoehe
	};
	SDL_Rect des {
        .x = x,
        .y = y,
        .w = Bmp[i].Breite,
        .h = Bmp[i].Hoehe
    };

	SDL_SetRenderTarget(renderer, lpDDSNach);
	SDL_RenderCopy(renderer, Bmp[i].Surface.texture, &src, &des);
	SDL_SetRenderTarget(renderer, nullptr);
}

void ZeichneLinie(short x1, short y1, short x2, short y2, DWORD color, LPDDSURFACEDESC2& ddsdtmp)
{
	SDL_SetRenderTarget(renderer, ddsdtmp.texture);
	SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 0xFF);
	// SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xFF);
	SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	SDL_SetRenderTarget(renderer, nullptr);
}

void DrawString(char *string, short x, short y, short Art, SDL_Texture *lpDDSNach)
{
	short length, index, cindex, Breite, Hoehe;

	if (lpDDSNach == lpDDSSchrift.texture)
		return; // F�r Fehlerausgabe aktivieren

	if (Art == 1)
	{
		Breite = S1XPIXEL;
		Hoehe = S1YPIXEL;
	}
	else if (Art == 2)
	{
		Breite = S2XPIXEL;
		Hoehe = S2YPIXEL;
	}
	else if (Art == 3)
	{
		Breite = S3XPIXEL;
		Hoehe = S3YPIXEL;
	}
	else
	{
		Breite = 0;
		Hoehe = 0;
	}

	// L�nge der Schrift ermitteln
	length = strlen(string);

	// Schriftsurface l�schen (Noch nicht entg�ltig)
	if (lpDDSNach == lpDDSSchrift.texture)
	{
		rcRectdes.left = x - 5;
		rcRectdes.top = y - 5;
		rcRectdes.right = x + length * Breite + 5;
		rcRectdes.bottom = y + Hoehe + 5;
		CalcRect(rcGesamt);
		lpDDSSchrift.Blt(&rcRectdes, NULL, NULL);
	}
	// Alle Zeichen durchgehen
	for (index = 0; index < length; index++)
	{
		// Korrekte indexNummer ermitteln
		cindex = string[index] - ' ';

		if ((string[index] >= ' ') && (string[index] <= '/'))
		{
			rcRectsrc.left = cindex * Breite;
			rcRectsrc.top = 0;
		}
		if ((string[index] >= '0') && (string[index] <= '?'))
		{
			rcRectsrc.left = (cindex - 16) * Breite;
			rcRectsrc.top = Hoehe;
		}
		if ((string[index] >= '@') && (string[index] <= 'O'))
		{
			rcRectsrc.left = (cindex - 16 * 2) * Breite;
			rcRectsrc.top = 2 * Hoehe;
		}
		if ((string[index] >= 'P') && (string[index] <= '_'))
		{
			rcRectsrc.left = (cindex - 16 * 3) * Breite;
			rcRectsrc.top = 3 * Hoehe;
		}
		if ((string[index] > '_') && (string[index] <= 'o'))
		{
			rcRectsrc.left = (cindex - 16 * 4) * Breite;
			rcRectsrc.top = 4 * Hoehe;
		}
		if ((string[index] >= 'p') && (string[index] <= '~'))
		{
			rcRectsrc.left = (cindex - 16 * 5) * Breite;
			rcRectsrc.top = 5 * Hoehe;
		}

		rcRectsrc.right = rcRectsrc.left + Breite;
		rcRectsrc.bottom = rcRectsrc.top + Hoehe;
		rcRectdes.left = x;
		rcRectdes.top = y;
		rcRectdes.right = x + Breite;
		rcRectdes.bottom = y + Hoehe;
		// Zeichen zeichnen
		if (Art == 1)
		{
			Blitten2(lpDDSSchrift1.texture, lpDDSNach, true);
			// x Position weiterschieben
			x += S1ABSTAND;
		}
		if (Art == 2)
		{
			Blitten2(lpDDSSchrift2.texture, lpDDSNach, true);
			// x Position weiterschieben
			x += S2ABSTAND;
		}
		if (Art == 3)
		{
			Blitten2(lpDDSSchrift3.texture, lpDDSNach, true);
			// x Position weiterschieben
			x += S3ABSTAND;
		}
	}
}

void CalcRect(RECT rcBereich)
{
	if (rcRectdes.left < rcBereich.left)
	{
		rcRectsrc.left = rcRectsrc.left + rcBereich.left - rcRectdes.left;
		rcRectdes.left = rcBereich.left;
	}
	if (rcRectdes.top < rcBereich.top)
	{
		rcRectsrc.top = rcRectsrc.top + rcBereich.top - rcRectdes.top;
		rcRectdes.top = rcBereich.top;
	}
	if (rcRectdes.right > rcBereich.right)
	{
		rcRectsrc.right = rcRectsrc.right + rcBereich.right - rcRectdes.right;
		rcRectdes.right = rcBereich.right;
	}
	if (rcRectdes.bottom > rcBereich.bottom)
	{
		rcRectsrc.bottom = rcRectsrc.bottom + rcBereich.bottom - rcRectdes.bottom;
		rcRectdes.bottom = rcBereich.bottom;
	}
}

void Compute() // Gebirge generieren
{
	short Anzahl, Abstand, i, x, y, ymax;
	short MaxPunkte = 1000;
	ZWEID Punkt[1000];

	// Linker und rechter Rand
	Punkt[1].x = 0;
	Punkt[1].y = MAXY - rand() % 200 - 1;
	Punkt[2].x = MAXX - 1;
	Punkt[2].y = MAXY - rand() % 200 - 1;
	Anzahl = 2;

	// PunkteListe erstellen
	while (1)
	{
		// Eintragungen spreizen
		for (i = Anzahl; i >= 2; i--)
		{
			Punkt[2 * i - 1] = Punkt[i];
		}
		// Zwischenwerte eintragen
		for (i = 1; i <= Anzahl - 1; i++)
		{
			Abstand = Punkt[2 * i + 1].x - Punkt[2 * i - 1].x;
			Punkt[2 * i].x = (Punkt[2 * i - 1].x + Punkt[2 * i + 1].x) / 2;
			if (i == 1)
				Punkt[2 * i].y = (Punkt[2 * i - 1].y + Punkt[2 * i + 1].y) / 2 - rand() % (Abstand / 2);
			else
				Punkt[2 * i].y = (Punkt[2 * i - 1].y + Punkt[2 * i + 1].y) / 2 - rand() % (Abstand) + Abstand / 2;
			// zu tief oder zu hoch
			if (Punkt[2 * i].y > MAXY - 50)
				Punkt[2 * i].y = MAXY - 50;
			if (Punkt[2 * i].y < 100)
				Punkt[2 * i].y = 100;
		}
		Anzahl = 2 * Anzahl - 1;
		if (2 * Anzahl - 1 > MaxPunkte)
			break;
	}
	// Landschaft auff�llen
	i = 2;
	for (x = 0; x < MAXX; x++)
	{
		if (x == Punkt[i].x)
			i++;
		// maximale y Koordinate rausfinden
		ymax = Punkt[i - 1].y +
			   (x - Punkt[i - 1].x) * (Punkt[i].y - Punkt[i - 1].y) / (Punkt[i].x - Punkt[i - 1].x);
		// Spalte auf�llen
		for (y = ymax; y < ymax + SCHNEEDICKE; y++)
		{
			if (y < MAXY)
				Pixel[x][y].p.Art = ARTSCHNEE;
		}
		for (y = ymax + SCHNEEDICKE; y < ymax + SCHNEEDICKE + GRASSDICKE; y++)
		{
			if (y < MAXY)
				Pixel[x][y].p.Art = ARTGRASS;
		}
		for (y = ymax + SCHNEEDICKE + GRASSDICKE; y < ymax + SCHNEEDICKE + GRASSDICKE + ERDDICKE; y++)
		{
			if (y < MAXY)
				Pixel[x][y].p.Art = ARTERDE;
		}
		for (y = ymax + SCHNEEDICKE + GRASSDICKE + ERDDICKE; y < MAXY; y++)
		{
			if (y < MAXY)
				Pixel[x][y].p.Art = ARTSTEIN;
		}
	}
	lpDDSScape.lock();
	// Farbe festlegen und zeichnen
	for (x = 0; x < MAXX; x++)
		for (y = 0; y < MAXY; y++)
			MakePixel(x, y, Pixel[x][y].p.Art, 0, 0, false, -1);
	lpDDSScape.unlock();
}

void PutPanzer()
{
	short x, y, i, Spieleranz, Nichtspieler;

	Spieleranz = 0;
	for (i = 0; i < MAXSPIELER; i++)
		if (Panzer[i].Aktiv)
			Spieleranz++;
	Nichtspieler = 0;
	for (i = 0; i <= MAXSPIELER; i++)
	{
		if (!Panzer[i].Aktiv)
		{
			Nichtspieler++;
			continue;
		}
		x = (i - Nichtspieler + 1) * MAXX / (Spieleranz + 1);
		for (y = 0; y < MAXY; y++)
		{
			if (Pixel[x][y].p.Art != ARTNICHTS)
			{
				y -= 1;
				break;
			}
		}
		Panzer[i].p.Pos.x = x;
		Panzer[i].p.Pos.y = y;
	}
}

void PanzerExpl(short PanzNr)
{
	short i, j;

	audio_manager->play(audio::TANK, audio::id::SFX, 100);

	// Splitter
	for (i = 0; i <= 10; i++)
		for (j = 0; j < MAXMUN; j++)
		{
			if (MunListe[j].Besitzer != -1)
				continue;
			MunListe[j].p.Art = MUNSPLITTER;
			MunListe[j].p.Pos.x = Panzer[PanzNr].p.Pos.x;
			MunListe[j].p.Pos.y = Panzer[PanzNr].p.Pos.y;
			MunListe[j].p.v.x = 20 - rand() % 40;
			MunListe[j].p.v.y = -rand() % 40;
			MunListe[j].Besitzer = PanzNr;
			MunListe[j].SoundBuffer = audio::NO_SOUND;
			MunListe[j].p.Lifetime = Munition[MunListe[j].p.Art].Dauer;
			break;
		}
	// Fetzen lassen
	Fetz(Panzer[PanzNr].p.Pos.x - Bmp[Panzer[PanzNr].Bild].Breite / 2,
		 Panzer[PanzNr].p.Pos.y - Bmp[Panzer[PanzNr].Bild].Hoehe, Panzer[PanzNr].Bild);
}

void Explosion(short MunNr)
{
	short x, y, Relx, Rely, i, j, Abstand, tmp;
	short PanzerDamage[MAXSPIELER]; // Liste des angerichteten Schadens
	short PanzerTreffer;			// Welcher Panzer wurde getroffen
	short DamageBetrag;				// Wie gro� ist die Besch�digung

	if (Munition[MunListe[MunNr].p.Art].Explosion == 0)
		return;

	for (i = 0; i < MAXSPIELER; i++)
		PanzerDamage[i] = 0;

	if (Munition[MunListe[MunNr].p.Art].Explosion == EXPKRATER)
	{
		audio_manager->play(audio::STONE, audio::id::SFX, 100);

		for (Relx = -10; Relx <= 10; Relx++)
			for (Rely = -10; Rely <= 10; Rely++)
			{
				if ((Relx * Relx + Rely * Rely) > (10 * 10))
					continue;
				x = MunListe[MunNr].p.Pos.x + Relx;
				y = MunListe[MunNr].p.Pos.y + Rely;
				if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
					continue;
				if (Pixel[x][y].p.Art != ARTNICHTS)
				{
					if (rand() % 2 == 0)
						DeletePixel(x, y);
					else
					{
						Pixel[x][y].p.Beschleunigung.x = -MunListe[MunNr].p.v.x * 10;
						Pixel[x][y].p.Beschleunigung.y = -MunListe[MunNr].p.v.y * 10;
						Aktivate(x, y, true);
					}
				}
				Abstand = (short)sqrt(Relx * Relx + Rely * Rely);
				// Besch�digungen?
				if ((Relx >= -3) && (Relx <= 3) && (Rely >= -3) && (Rely <= 3))
				{
					PanzerTreffer = CheckKollision(x, y, false);
					if ((PanzerTreffer >= 0) && (PanzerTreffer < MAXSPIELER))
						PanzerDamage[PanzerTreffer] = Munition[MunListe[MunNr].p.Art].MaxDamage;
				}
			}
		Fetz(MunListe[MunNr].p.Pos.x - Bmp[Munition[MunListe[MunNr].p.Art].Bild].Breite / 2,
			 MunListe[MunNr].p.Pos.y - Bmp[Munition[MunListe[MunNr].p.Art].Bild].Hoehe / 2,
			 Munition[MunListe[MunNr].p.Art].Bild);
	}
	else if (Munition[MunListe[MunNr].p.Art].Explosion == EXPEXPLOSION)
	{
		// Sound
		audio_manager->play(audio::EXPLOSION, audio::id::SFX, 100);

		for (Relx = -20; Relx <= 20; Relx++)
			for (Rely = -20; Rely <= 20; Rely++)
			{
				if ((Relx * Relx + Rely * Rely) > (20 * 20))
					continue;
				x = MunListe[MunNr].p.Pos.x + Relx;
				y = MunListe[MunNr].p.Pos.y + Rely;
				if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
					continue;
				if (Pixel[x][y].p.Art != ARTNICHTS)
				{
					DeletePixel(x, y);
				}
				Abstand = (short)sqrt(Relx * Relx + Rely * Rely);
				// Besch�digungen?
				PanzerTreffer = CheckKollision(x, y, false);
				if ((PanzerTreffer >= 0) && (PanzerTreffer < MAXSPIELER))
				{
					DamageBetrag = (short)(Munition[MunListe[MunNr].p.Art].MaxDamage *
										   (1 - Abstand / 20));
					if ((PanzerDamage[PanzerTreffer] < DamageBetrag) &&
						(PanzerTreffer != -1))
						PanzerDamage[PanzerTreffer] = DamageBetrag;
				}
				// Flammen
				MakeFXPixel(x, y, RGB2DWORD(255 - rand() % 100, Abstand * 10, 0), Abstand / 4 + rand() % 5, 0, 0);
			}
	}
	else if (Munition[MunListe[MunNr].p.Art].Explosion == EXPGRANATE)
	{
		audio_manager->play(audio::GRENADE, audio::id::SFX, 100);

		for (Relx = -10; Relx <= 10; Relx++)
			for (Rely = -10; Rely <= 10; Rely++)
			{
				if ((Relx * Relx + Rely * Rely) > (10 * 10))
					continue;
				x = MunListe[MunNr].p.Pos.x + Relx;
				y = MunListe[MunNr].p.Pos.y + Rely;
				if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
					continue;
				if (Pixel[x][y].p.Art != ARTNICHTS)
				{
					DeletePixel(x, y);
				}
				Abstand = (short)sqrt(Relx * Relx + Rely * Rely);
				// Besch�digungen?
				PanzerTreffer = CheckKollision(x, y, false);
				if ((PanzerTreffer >= 0) && (PanzerTreffer < MAXSPIELER))
				{
					DamageBetrag = (short)(Munition[MunListe[MunNr].p.Art].MaxDamage *
										   (1 - Abstand / 20));
					if ((PanzerDamage[PanzerTreffer] < DamageBetrag) &&
						(PanzerTreffer != -1))
						PanzerDamage[PanzerTreffer] = DamageBetrag;
				}
				// Flammen
				MakeFXPixel(x, y, RGB2DWORD(255 - rand() % 100, Abstand * 10, 0), (Abstand / 4 + rand() % 5) / 2, 0, 0);
			}
		// Splitter
		for (i = 0; i <= 5; i++)
			for (j = 0; j < MAXMUN; j++)
			{
				if (MunListe[j].Besitzer != -1)
					continue;
				MunListe[j].p.Art = MUNSPLITTER;
				MunListe[j].p.Pos.x = MunListe[MunNr].p.Pos.x;
				MunListe[j].p.Pos.y = MunListe[MunNr].p.Pos.y;
				MunListe[j].p.v.x = MunListe[MunNr].p.v.x + 20 - rand() % 40;
				MunListe[j].p.v.y = MunListe[MunNr].p.v.y + 20 - rand() % 40;
				MunListe[j].Besitzer = MunListe[MunNr].Besitzer;
				MunListe[j].SoundBuffer = audio::NO_SOUND;
				MunListe[j].p.Lifetime = Munition[MunListe[j].p.Art].Dauer;
				break;
			}
	}
	else if (Munition[MunListe[MunNr].p.Art].Explosion == EXPSPLITTER)
	{
		if (MunListe[MunNr].p.Art == MUNGEWEHR)
			audio_manager->play(audio::RICOCHET, audio::id::SFX, 100, false, false);
		else
			audio_manager->play(audio::SPLINTER, audio::id::SFX, 100, false, false);

		for (Relx = -5; Relx <= 5; Relx++)
			for (Rely = -5; Rely <= 5; Rely++)
			{
				if ((Relx * Relx + Rely * Rely) > (5 * 5))
					continue;
				x = MunListe[MunNr].p.Pos.x + Relx;
				y = MunListe[MunNr].p.Pos.y + Rely;
				if ((x >= MAXX) || (x < 0) || (y >= MAXY) || (y < 0))
					continue;
				if ((Pixel[x][y].p.Art != ARTNICHTS) && (rand() % 2 == 0))
				{
					DeletePixel(x, y);
				}
				Abstand = (short)sqrt(Relx * Relx + Rely * Rely);
				// Besch�digungen?
				PanzerTreffer = CheckKollision(x, y, false);
				if ((PanzerTreffer >= 0) && (PanzerTreffer < MAXSPIELER))
				{
					DamageBetrag = Munition[MunListe[MunNr].p.Art].MaxDamage;
					if ((PanzerDamage[PanzerTreffer] < DamageBetrag) &&
						(PanzerTreffer != -1))
						PanzerDamage[PanzerTreffer] = DamageBetrag;
				}
				// Flammen
				MakeFXPixel(x, y, RGB2DWORD(255 - rand() % 100, 100 + Abstand * 5, 0), (Abstand / 4 + rand() % 5) / 2, 0, 0);
				if (Pixel[x][y].p.Art != ARTNICHTS)
				{
					Pixel[x][y].p.Beschleunigung.x = -MunListe[MunNr].p.v.x * 10;
					Pixel[x][y].p.Beschleunigung.y = -MunListe[MunNr].p.v.y * 10;
					Aktivate(x, y, true);
				}
			}
	}
	else if (Munition[MunListe[MunNr].p.Art].Explosion == EXPFASS)
	{
		audio_manager->play(audio::ACID, audio::id::SFX, 100, false, false);

		// S�ure
		for (Relx = -10; Relx <= 10; Relx++)
			for (Rely = -10; Rely <= 10; Rely++)
			{
				if ((Relx * Relx + Rely * Rely) > (10 * 10))
					continue;
				x = MunListe[MunNr].p.Pos.x + Relx;
				y = MunListe[MunNr].p.Pos.y + Rely;

				if ((x >= MAXX) || (x < 0) || (y >= MAXY))
					continue;

				// Oberhalb des Bildschirms?
				if (y < 0)
				{
					for (i = 0; i < MAXHIMMELPIXEL; i++)
					{
						if (HimmelPixel[i].Art != ARTNICHTS)
							continue;
						HimmelPixel[i].Art = ARTSAEURE;
						HimmelPixel[i].v.x = -10 + rand() % 20;
						HimmelPixel[i].v.y = -10 + rand() % 20;
						HimmelPixel[i].Pos.x = x;
						HimmelPixel[i].Pos.y = y + HIMMELHOEHE - 1;
						HimmelPixel[i].Lifetime = rand() % 2000;
						break;
					}
				}
				else
				{
					if (Pixel[x][y].p.Art != ARTNICHTS)
					{
						DeletePixel(x, y);
					}
					MakePixel(x, y, ARTSAEURE, -10 + rand() % 20, -10 + rand() % 20, true, rand() % 2000);
				}
			}
	}
	else if (Munition[MunListe[MunNr].p.Art].Explosion == EXPMEGA)
	{
		// Inhalt herausfliegen lassen
		for (i = 0; i <= 7; i++)
			for (j = 0; j < MAXMUN; j++)
			{
				if (MunListe[j].Besitzer != -1)
					continue;

				while (1)
				{
					tmp = (rand() % (MUNANZAHL - 2)) + 1;
					if ((tmp == MUNRAKETE) || (tmp == MUNSTEIN) || (tmp == MUNGRANATE) ||
						(tmp == MUNMAGRAKETE) || (tmp == MUNFASS))
						break;
				}
				MunListe[j].p.Art = tmp;
				MunListe[j].p.Pos.x = MunListe[MunNr].p.Pos.x;
				MunListe[j].p.Pos.y = MunListe[MunNr].p.Pos.y;
				MunListe[j].p.v.x = 20 - rand() % 40;
				MunListe[j].p.v.y = -rand() % 40;
				MunListe[j].Besitzer = MunListe[MunNr].Besitzer;
				MunListe[j].SoundBuffer = audio::NO_SOUND;
				MunListe[j].p.Lifetime = Munition[MunListe[j].p.Art].Dauer;
				break;
			}
	}
	else if (Munition[MunListe[MunNr].p.Art].Explosion == EXPSCHNEEBALL)
	{
		// Schnee
		for (Relx = -20; Relx <= 20; Relx++)
			for (Rely = -20; Rely <= 20; Rely++)
			{
				if ((Relx * Relx + Rely * Rely) > (20 * 20))
					continue;
				x = MunListe[MunNr].p.Pos.x + Relx;
				y = MunListe[MunNr].p.Pos.y + Rely;

				if ((x >= MAXX) || (x < 0) || (y >= MAXY))
					continue;

				// Oberhalb des Bildschirms?
				if (y < 0)
				{
					for (i = 0; i < MAXHIMMELPIXEL; i++)
					{
						if (HimmelPixel[i].Art != ARTNICHTS)
							continue;
						HimmelPixel[i].Art = ARTSCHNEE;
						HimmelPixel[i].v.x = -10 + rand() % 20;
						HimmelPixel[i].v.y = -10 + rand() % 20;
						HimmelPixel[i].Pos.x = x;
						HimmelPixel[i].Pos.y = y + HIMMELHOEHE - 1;
						HimmelPixel[i].Lifetime = -1;
						break;
					}
				}
				else
				{
					if (Pixel[x][y].p.Art != ARTNICHTS)
					{
						continue;
					}
					MakePixel(x, y, ARTSCHNEE, -10 + rand() % 20, -10 + rand() % 20, true, -1);
				}
			}
	}
	// Energie abziehen
	for (i = 0; i < MAXSPIELER; i++)
		if (Panzer[i].Schild == 0)
			ChangeLebensEnergie(i, -PanzerDamage[i], MunListe[MunNr].Besitzer);
}

void Fetz(short x, short y, short Bild)
{
	short Relx, Rely;
	DWORD Farbe;

	for (Relx = 0; Relx < Bmp[Bild].Breite; Relx++)
		for (Rely = 0; Rely < Bmp[Bild].Hoehe; Rely++)
		{
			Farbe = GetPixel(Bmp[Bild].rcSrc.left + Relx, Bmp[Bild].rcSrc.top + Rely, &Bmp[Bild].Surface);
			if (Farbe == Tansparent)
				continue;
			MakeFXPixel(x + Relx, y + Rely, Farbe, rand() % 10,
						double(-(Bmp[Bild].Breite / 2 - Relx)) / 5 + double(rand() % 10) / 10 - 0.5,
						double(-(Bmp[Bild].Hoehe / 2 - Rely)) / 5 + double(rand() % 10) / 10 - 0.5);
		}
}

void Laser(short i)
{
	short j, k;
	float x, y;
	short Dx, Dy;
	float Sx, Sy;
	short Steps;
	bool BallonTreffer, Treffer[MAXSPIELER]; // Damit jeder Spieler nur einmal Energie abgezogen bekommt
	short erg;

	Dx = Panzer[i].KanonePos.x;
	Dy = Panzer[i].KanonePos.y;
	k = 1;
	BallonTreffer = false;
	for (j = 0; j < MAXSPIELER; j++)
		Treffer[j] = false;
	while (1)
	{
		x = (float)Panzer[i].p.Pos.x + k * Dx;
		y = (float)Panzer[i].p.Pos.y + k * Dy;
		if ((x < 0) || (x >= MAXX) ||
			(y < 0) || (y >= MAXY))
			break;

		if (abs(Dx) > abs(Dy))
		{
			if (Dx > 0)
				Sx = -1;
			else
				Sx = 1;
			if (Dx == 0)
				Sy = 0;
			else
				Sy = (float)Dy / ((float)(Dx * Sx));
			Steps = abs(Dx);
		}
		else
		{
			if (Dy > 0)
				Sy = -1;
			else
				Sy = 1;
			if (Dy == 0)
				Sx = 0;
			else
				Sx = (float)Dx / ((float)(Dy * Sy));
			Steps = abs(Dy);
		}

		for (j = 0; j < Steps; j++)
		{
			MakeFXPixel((short)x, (short)y, RGB2DWORD(255, 0, 0), rand() % Munition[MUNLASER].Dauer, 0, 0);
			DeletePixel((short)x, (short)y);
			DeletePixel((short)x - 1 + rand() % 2, (short)y - 1 + rand() % 2);

			erg = CheckKollision((short)x, (short)y, false);
			if ((erg >= 0) && (erg < MAXSPIELER))
				Treffer[erg] = true;
			else if (erg == -2)
				BallonTreffer = true;
			x += Sx;
			y += Sy;
		}
		k++;
	}
	// Lebensenergie abziehen
	for (j = 0; j < MAXSPIELER; j++)
		if ((Treffer[j]) && (Panzer[j].Schild == 0) && (i != j))
			ChangeLebensEnergie(j, -Munition[MUNLASER].MaxDamage, i);
	if (BallonTreffer)
		BallonAbschuss(i);
}

void BallonAbschuss(short i)
{
	if (!Ballon.Aktiv)
		return;

	Ballon.Aktiv = false;

	if (i != -1)
	{
		audio_manager->play(audio::PARACHUTE2, audio::id::SFX, 100);

		if (Panzer[i].Lager[Ballon.Munition] == -1)
			return;
		Panzer[i].Lager[Ballon.Munition] += Ballon.Anzahl;
		if (Panzer[i].Lager[Ballon.Munition] > 9)
			Panzer[i].Lager[Ballon.Munition] = 9;
	}
	else
		audio_manager->play(audio::ACID, audio::id::SFX, 100);
	// Explosion
	Fetz(Ballon.Pos.x, Ballon.Pos.y, BALLONBILD);
}

bool CheckShop(short i)
{
	short erg;
	short tmpNr;
	static short GKosten[MAXSPIELER] {};
	static short Anz[MUNANZAHL - 2] {};
	short j, k;

	tmpNr = (MUNANZAHL - 2) * 6 + 1;

	Menue[AktMenue].putBild(0, MROTER + i);
	//while (1)
	{
		// Mit Aktuellen Werten belegen
		for (j = 1; j < MUNANZAHL - 1; j++)
		{
			Menue[AktMenue].putZiffer(1 + (j - 1) * 6, Panzer[i].Lager[j]);
			Menue[AktMenue].putZiffer(4 + (j - 1) * 6, Anz[j - 1]);
		}
		Menue[AktMenue].putZiffer(tmpNr, Panzer[i].Konto);
		Menue[AktMenue].putZiffer(tmpNr + 1, GKosten[i]);

		erg = Menue[AktMenue].inputMouse(); // Mausaktionen abfragen

		if ((erg % 6 == 5) && (erg < tmpNr)) // Pfeil nach oben
		{
			k = erg / 6;
			if (Panzer[i].Lager[k + 1] == -1)
				return false;
			if ((Anz[k] + Panzer[i].Lager[k + 1] < 9) &&
				(GKosten[i] + Munition[k + 1].Preis <= Panzer[i].Konto))
			{
				GKosten[i] += Munition[k + 1].Preis;
				Anz[k]++;
			}
			return false;
		}
		if ((erg % 6 == 0) && (erg <= tmpNr)) // Pfeil nach unten
		{
			k = erg / 6 - 1;
			if (Panzer[i].Lager[k + 1] == -1)
				return false;
			if (Anz[k] > 0)
			{
				GKosten[i] -= Munition[k + 1].Preis;
				Anz[k]--;
			}
			return false;
		}
		if (erg == tmpNr + 2)
		{
			audio_manager->play(audio::REGISTER, audio::id::SFX, 100);

			for (j = 1; j < MUNANZAHL - 1; j++)
			{
				for (k = Anz[j - 1]; k > 0; k--)
				{
					Panzer[i].Konto -= Munition[j].Preis;
					Panzer[i].Lager[j]++;
				}
				Anz[j - 1] = 0;
			}
			GKosten[i] = 0;
			return true;
		}
		if (erg == tmpNr + 3)
			return true;

		Menue[AktMenue].zeige(); // Menue anzeigen
	}
	return false;
}

int CheckMMenue()
{
	int erg, i, anz;

	// while (1)
	{
		erg = Menue[AktMenue].inputMouse(); // Mausaktionen abfragen

		if (erg == -2)
		{
			SaveGame();
			return (0);
		}
		else if (erg == -3)
		{
			AktRunde = 0;
			NeuesSpiel();
		}
		else if (erg == -4)
		{
			InitCredits();
		}

		if (AktMenue == MENOPTION)
		{
			if (erg == 7)
			{
				i = Menue[AktMenue].getZiffer(6);
				i++;
				if (i <= 9)
					Menue[AktMenue].putZiffer(6, i);
			}
			else if (erg == 8)
			{
				i = Menue[AktMenue].getZiffer(6);
				i--;
				if (i >= 1)
					Menue[AktMenue].putZiffer(6, i);
			}
			else if (erg == 10)
			{
				if (Soundzustand == -1)
					Menue[AktMenue].putVersion(erg, 1);
				else if (Menue[AktMenue].getVersion(erg) == 0)
					Soundzustand = 1;
				else
					Soundzustand = 0;
			}
		}
		else if (AktMenue == MENSPIELER)
		{
			if ((erg == 2) || (erg == 4) || (erg == 6) || (erg == 8))
			{
				anz = 0;
				for (i = 2; i <= 8; i += 2)
					if (Menue[AktMenue].getVersion(i) == 2)
						anz++;
				if (anz >= 3)
					Menue[AktMenue].putVersion(erg, 0);
			}
		}
		if (AktMenue >= 0)
			Menue[AktMenue].zeige(); // Menue anzeigen
	}
	return 1;
}

void ComputerShop(short i)
{
	short j, k;

	for (j = 0; j < 100; j++)
	{
		k = (rand() % (MUNANZAHL - 2)) + 1;
		if (Panzer[i].Lager[k] >= 9)
			continue;
		if (Panzer[i].Konto < Munition[k].Preis)
			continue;
		if (k == MUNSTEIN)
			continue;

		Panzer[i].Lager[k]++;
		Panzer[i].Konto -= Munition[k].Preis;
	}
}

void pre_run()
{

	printf("Pre run init");

	if (Spielzustand == SZNICHTS)
	{
		Spielzustand = SZTITEL;
	}

	audio_manager->play(audio::TITLE_MUSIC, audio::id::MUSIC, 255, true, false);
}
bool bQuit = false;

short shopindex = 0;

void Run()
{
	SDL_Event event{};
	int Zeitdiv;
	int erg;
	int i, j;
	double delta = 0;
	auto Button0downbefore = Button0down;
	auto Button1downbefore = Button1down;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        	bQuit = true;
            return ;
        }

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_l)
            {
                Panzer[1].LebensEnergie = -1;
            }
        }
        if (CheckKey(&event) == 0)
            return; // Das Keyboard abfragen
        CheckMouse(&event);						// Den MouseZustand abchecken

    }

    if (Button0downbefore && Button0down)
        ButtonPush = 0;
    if (Button1downbefore && Button1down)
        ButtonPush = 0;

    // Zeit abfragen
    auto start = std::chrono::high_resolution_clock::now();
    SDL_RenderClear(renderer);

    // fps rausfinden
    Bild++;

    if (Spielzustand == SZSPIEL)
    {
        if (AktMenue == -1) // Kein Men� eingeblendet (normal)
        {
            // Wieviele Spieler leben noch
            erg = 0;
            for (i = 0; i < MAXSPIELER; i++)
            {
                if (Panzer[i].Aktiv)
                {
                    erg++;
                    j = i;
                }
            }
            if (erg <= 1)
            {
                AktMenue = MENRGEWONNEN;
                Menue[AktMenue].putBild(1, MROTER + j);
                Panzer[j].RGewonnen++;
                StopAllSound();
                PlaySound(WAVYIPPEE, 100, Panzer[j].p.Pos.x, -1, false, -1);
                audio_manager->play(audio::YIPPIEE, audio::id::SFX, 100, false);
            }

            if (Bild % (LastBild / 30 + 1) == 0) // unabh�nig von der Framerate
            {
                Computer();		// Die Computerspieler
            }

            if (Testmodus) // Beim Testmodus immer den gesamten Bildschirm l�schen
            {
                rcRectdes.left = 0;
                rcRectdes.top = 0;
                rcRectdes.right = MAXX;
                rcRectdes.bottom = MAXY;
                // ddbltfx.dwFillColor = Tansparent;
                lpDDSScape.Blt(&rcRectdes, NULL, NULL);
            }
        }

        if (Bild % (LastBild / 20 + 1) == 0) // unabh�nig von der Framerate
            CheckBallon();					 // Ballon erzeugen und bewegen
        CheckPanzer();						 // Panzer berechnen
        CheckHimmelPixel();					 // Nicht sichtbaren Bereich berechnen
        //
        lpDDSScape.lock();
        CheckMunListe();					 // Geschosse berechnen
        MakeWetter();						 // Wetter erzeugen
        FindActivePixel();					 // Pixel berechnen
        lpDDSScape.unlock();
        if (Bild % (LastBild / 10 + 1) == 0) // unabh�nig von der Framerate
            CheckFXPixel();					 // EffektPixel �berpr�fen

        if (AktMenue != -1) // ein Overlaymen� vorhanden
        {
            Zeige(false);						// Betrug (bild wird nicht angezeigt, sondern nur aufgebaut)
            erg = Menue[AktMenue].inputMouse(); // Mausaktionen abfragen
            if (erg != -1)
            {
                if ((AktMenue == MENRGEWONNEN) && (erg == 5))
                {
                    if (AktRunde == MaxRunde)
                    {
                        Spielzustand = SZMENUE;
                        AktMenue = MENGGEWONNEN;
                        PutGMenue();
                        StopAllSound();
                        audio_manager->play(audio::APPLAUSE, audio::id::SFX, 255, false);
                        return;
                    }
                    GetMenue();
                    Spielzustand = SZSHOP;
                    shopindex = 0;
                    AktMenue = MENSHOP;
                    StopAllSound();
                    return;
                }
                else if (AktMenue == MENBEENDEN)
                {
                    if (erg == 1)
                    {
                        AktMenue = -1;
                        return;
                    }
                    if (erg == 2)
                    {
                        AktMenue = MENTASTENO;
                        return;
                    }
                    if (erg == 3)
                    {
                        Spielzustand = SZMENUE;
                        AktMenue = MENMAIN;
                        StopAllSound();
                        return;
                    }
                }
                else if (((AktMenue == MENSTART) && (erg == 3)) ||
                         (AktMenue == MENTASTENO))
                {
                    AktMenue = -1;
                    return;
                }
            }
            Menue[AktMenue].zeige(); // Men� anzeigen

        }
        else
            Zeige(false); // Das Bild aufbauen
        SDL_RenderCopy(renderer, lpDDSBack.texture, NULL, NULL);
    }
    else if (Spielzustand == SZMENUE)
    {
        if (CheckMMenue() == 0) {
        	bQuit = true;
	        return;
        }

        SDL_RenderCopy(renderer, lpDDSBack.texture, NULL, NULL);

        // SDL_RenderCopy(renderer, lpDDSCredits.texture, NULL, NULL);
    }
    else if (Spielzustand == SZSHOP)
    {
        if (!Panzer[shopindex].Aktiv)
        {
            shopindex++;
        }
        else if (Panzer[shopindex].Computer)
        {
            ComputerShop(shopindex++);
        }
        else
        {
            if (CheckShop(shopindex))
                shopindex++;
        }

        if (shopindex >= MAXSPIELER)
        {
            NeuesSpiel();
        }
        SDL_RenderCopy(renderer, lpDDSBack.texture, NULL, NULL);
    }
    else if (Spielzustand == SZCREDITS)
    {
        lpDDSScape.lock();
        if (Bild%(LastBild/30+1) == 0)
        {
            CheckCredits();
        }

        FindActivePixel();					 // Pixel berechnen
        lpDDSScape.unlock();
        if (Bild % (LastBild / 10 + 1) == 0) // unabh�nig von der Framerate
            CheckFXPixel();					 // EffektPixel �berpr�fen

        Zeige(false); // Das Bild aufbauen
        SDL_RenderCopy(renderer, lpDDSBack.texture, NULL, NULL);
    }
    else if (Spielzustand == SZTITEL) {
        MakeTitel();
    }

    SDL_Rect dest {
        0, 0, 40, 40
    };
    // SDL_RenderCopy(renderer, lpDDSPanzSave.texture, NULL, &dest);

    SDL_RenderPresent(renderer);

    auto end = std::chrono::high_resolution_clock::now();

    auto diff_ms = std::chrono::duration<double, std::milli>(end - start).count();

    const auto frame_time = 1000.0 / 60.0;
    if (diff_ms < frame_time) {
        // Sleep(33 - diff_ms);
        SDL_Delay(SDL_min(frame_time - diff_ms, frame_time));
    }
    if (Bild % 30 == 0)
    {
        // fps rausfinden
        // printf("fps: %f\n", 1.0 / diff_ms);
    }
}

/*
 * doInit - do work required for every instance of the application:
 *                create the window, initialize data
 */
static BOOL doInit()
{
	printf("Initializing...\n");
	// init SDL
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
	{
		printf("Failed to init SDL: %s\n", SDL_GetError());
		return false;
	}

	/*
	 * create a window
	 */
	printf("Creating window...\n");
	window = SDL_CreateWindow(
		TITLE,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		MAXX * 2,
		MAXY * 2,
		SDL_WINDOW_SHOWN);

	if (!window)
	{
		printf("Failed to create window: %s\n", SDL_GetError());
		return false;
	}

	printf("Loading textures...\n");
	InitDDraw();
	printf("Loading sounds...\n");
	InitDSound();
	srand((unsigned)time(NULL));   // Random initialisieren
	// Structs initialisieren
	printf("Initializing data...\n");
	InitStructs(0);
	LoadGame();
	return true;

} /* doInit */


/*
 * Main - initialization, message loop
 */
#include <iostream>
void main_loop() {
	Run();
}

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __EMSCRIPTEN__
int main()
#else
int main(int argc, char* argv[])
#endif
{

	if (!doInit())
	{
		return 1;
	}
	pre_run();
#ifdef __EMSCRIPTEN__
	printf("Setting main loop\n");
	emscripten_set_main_loop(main_loop, 0, 1);
#else
	while (!bQuit)
	Run();
#endif
	finiObjects();
	return 0;
}