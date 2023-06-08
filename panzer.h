#ifndef PANZER_H
#define PANZER_H

#define DIRECTINPUT_VERSION 0x0700
#define DIRECTDRAW_VERSION 0x0500

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>		//Fürs Random
#include <sys/timeb.h>	//Für genaue Zeitabfrage
#include <stdarg.h>
#include <math.h>

#include <fstream.h>
#include <fcntl.h>
#include <io.h>

#include "ddutil.h"
#include "resource.h"

#define NAME "Panzer"
#define TITLE "Panzer"
#define INITGUID
#define MAXX 640 //Bildschirmauflösung
#define MAXY 480
#define ROUND(s) ((short) ((short)(s-0.5) == (short) s ? s+1:s)) //Zahlen runden Macro
#define S1XPIXEL 24 //Breite der Schrift1
#define S1YPIXEL 34 //Höhe der Schrift1
#define S1ABSTAND 26 //Abstand zum nächsten Buchstaben
#define S2XPIXEL 5 //Breite der Schrift2
#define S2YPIXEL 8 //Höhe der Schrift2
#define S2ABSTAND		6		//Abstand zum nächsten Buchstaben
#define S3XPIXEL		15		//Breite der Schrift3
#define S3YPIXEL		17		//Höhe der Schrift3
#define S3ABSTAND		15		//Abstand zum nächsten Buchstaben
#define BERANZAHL		32		//Anzahl der Bereiche
#define IMPULSABGABE	0.60	//Geschwindgkeitsabgabe an das getroffene Pixel zwischen 0-1
#define SCHNEEDICKE		3		//Dicke der Schneedecke
#define GRASSDICKE		2		//Dicke der Grassdecke
#define ERDDICKE		10		//Dicke der Erddecke
#define ZFARBE			15		//Um wievel weicht der Farbwert maximal vom Original ab
#define MAXSPIELER		4		//Wieviele Spieler
#define KANONENLAENGE	20		//Wieviel Pixel ist die Kanone lang
#define HIMMELHOEHE		470		//Wieviel Pixel sind oberhalb des Bildschirms (muß kleiner als MAXY sein)
#define MAXFX			10000	//Wieviel FX-Pixel sind möglich
#define MAXSCHUSSENERGIE 50		// Bestimmt die Einteilungen der Schussenergieleiste
#define MAXLEBENSENERGIE 1000	//Maximale Lebensenergie
#define MAXMUN			100		//Wieviele Geschosse können gleichzeitig rumfliegen
#define MAXHIMMELPIXEL	500		//Wieviele Pixel sind oberhalb des Bildschirms?
#define MAXGEGNERSCHUSS 20		// Wie oft wird ein Gegner mindestens hintereinander beschossen? 
#define PANZERBREITE	40		//Breite der Panzer mit allem Drumherum
#define PANZERHOEHE		61		//Hoehe der Panzer mit allem Drumherum
#define MAGNETKONSTANTE 50000	//Wie stark sind die Magnete
#define BALLONRANDOM	5000	//Die Wahrscheinlichkeit für einen Ballon
#define MAXCHANNELS		10		//Wieviele Sounds werden gleichzeitig abgespielt?
#define MAXCREDITS		50		//Wieviele Credits

//Bilder
#define CUKREUZ	    0
#define PANZERNULL  1
#define PANZEREINS  2
#define PANZERZWEI  3
#define PANZERDREI  4
#define	MUNNULL	    5
#define MUNEINS		6
#define MUNZWEI		7
#define MUNDREI		8
#define MUNVIER		9
#define MUNFUENF	10
#define MUNSECHS	11
#define MUNSIEBEN	12
#define MUNACHT		13
#define MUNNEUN		14
#define MUNZEHN		15
#define MUNELF		16
#define MUNZWOELF	17
#define BALLONBILD	18
#define POWER		19
#define SCHILD		POWER+1
#define MUNBOX		POWER+2
#define ENERGIE		POWER+3
#define ZAHLENNULL	POWER+4
#define ZAHLENEINS  POWER+5
#define TEXFELS		POWER+6
#define WINDHOSE	POWER+7
#define GEWEHR		POWER+8
#define FASS		POWER+9
#define MAGNET		POWER+10
#define MBACK		POWER+11
#define MOPTION		MBACK+1
#define MZURUECK	MBACK+2
#define MSTARTEN	MBACK+3
#define MHAUPTMENUE	MBACK+4
#define MOPTIONENG	MBACK+5
#define MWIND		MBACK+6
#define MAN			MBACK+7
#define	MWETTER		MBACK+8
#define	MSPIELER	MBACK+9
#define	MSPIELERG	MBACK+10
#define MMENSCH		MBACK+11
#define MROTER		MBACK+12
#define MGRUENER	MBACK+13
#define	MBLAUER		MBACK+14
#define MGELBER		MBACK+15
#define MSBACK		MBACK+16
#define MWEITER		MBACK+17
#define MKAUFEN		MBACK+18
#define MPFEILH		MBACK+19
#define MPFEILR		MBACK+20
#define MOBACK		MBACK+21
#define MRUNDE		MBACK+22
#define MRUNDEN		MBACK+23
#define MHATDIESE	MBACK+24
#define MGEWONNEN	MBACK+25
#define MGBACK		MBACK+26
#define MKEYBOARD	MBACK+27
#define MTASTEN		MBACK+28
#define MTASTENG	MBACK+29
#define MBEENDEN	MBACK+30
#define MLADESPIEL	MBACK+31
#define MSOUND		MBACK+32
#define MCREDITS	MBACK+33
#define MBBUNKER	MBACK+34
#define MABUNKER	MBACK+35
#define CRDIRKPLATE			MBACK+36
#define CRPROGRAMMIERUNG	CRDIRKPLATE+1
#define CRBETATESTER		CRDIRKPLATE+2
#define CRMATTHIASBUCHETICS	CRDIRKPLATE+3
#define CRMARCEBERHARDT		CRDIRKPLATE+4
#define CRSOUND				CRDIRKPLATE+5
#define CRMUSIK				CRDIRKPLATE+6
#define CRHEIKOPLATE		CRDIRKPLATE+7
#define CRGRAFIK			CRDIRKPLATE+8
#define CRDPSOFTWARE		CRDIRKPLATE+9
#define BILDANZ				CRDIRKPLATE+10

//Spielzustände
#define SZNICHTS		0
#define SZSPIEL			1
#define SZMENUE			2
#define SZSHOP			3
#define SZCREDITS		4
#define SZTITEL			5

//Menues
#define	MENMAIN			0
#define MENOPTION		1
#define MENSPIELER		2
#define	MENSHOP			3
#define	MENSTART		4
#define MENRGEWONNEN	5
#define MENGGEWONNEN	6
#define MENTASTEN		7
#define MENTASTENO		8
#define MENBEENDEN		9
#define MENANZ			10

//Wav
#define WAVNICHTS		0
#define WAVKLICK		1
#define WAVRAKETE		2
#define WAVMOUSEEINS	3
#define WAVEXPLOSIONG	4
#define WAVGEWEHR		5
#define WAVQUERSCHUSS	6
#define WAVGRANATE		7
#define WAVSPLITTER		8
#define WAVMAGNETSCHILD	9
#define WAVSTEIN		10
#define WAVSCHILD		11
#define WAVKNISTERN		12
#define WAVLASER		13
#define WAVKASSE		14
#define WAVSAEURE		15
#define WAVPANZER		16
#define WAVFALLSCHIRM	17
#define WAVFALLSCHIRMZWEI	18
#define WAVYIPPEE		19
#define WAVAPPLAUS		20
#define WAVCREDITS		21
#define WAVCREDITSMUSIK	22
#define WAVTITELMUSIK	23
#define WAVANZ			24

//Gesteinsarten
#define ARTNICHTS       0
#define ARTGRASS		1
#define ARTWASSER		2
#define ARTSTEIN		3
#define ARTSCHNEE		4				//Bei Erweiterungen CheckHimmelPixel() ändern
#define ARTERDE			5
#define ARTPANZER		6
#define ARTMUNITION		7
#define ARTSAEURE		8
#define ARTANZAHL		9	

//Munitionsarten
#define MUNNICHTS		0
#define MUNRAKETE		1
#define MUNSCHILD		2
#define MUNSTEIN		3
#define MUNGRANATE		4
#define MUNLASER		5
#define MUNGEWEHR		6
#define MUNFASS			7
#define MUNMEDI			8
#define MUNMAGRAKETE	9
#define MUNMAGNET		10
#define MUNMEGA			11
#define MUNSCHNEEBALL	12
#define MUNSPLITTER		13	//MUNSPLITTER MUSS DER LETZTE SEIN
#define MUNANZAHL		14

//Explosionsarten
#define EXPNICHTS		0
#define EXPKRATER		1
#define EXPEXPLOSION	2
#define EXPGRANATE		3
#define EXPSPLITTER		4
#define EXPFASS			5
#define EXPMEGA			6
#define EXPSCHNEEBALL	7

//Raucharten
#define SMOKENICHTS		0
#define SMOKERAUCH		1
#define SMOKEFEUER		2

struct ZWEID
{
	short x;
	short y;
}; 	
struct ZWEIDGENAU
{
	double x;
	double y;
}; 
struct RGBSTRUCT
{
	BYTE r,g,b;
};
struct FXLISTE
{
	ZWEID			Pos;		//Bei x=-1 und y=-1 nicht belegt
	short			Lifetime;	//Wieviele Bilder soll das Pixel noch dableiben?
	DWORD			Farbe;		//Farbe
	ZWEIDGENAU		Rel;		//Relative Postition im Pixel
	ZWEIDGENAU		Bewegung;	//Bewegt sich der Pixel
}; 

struct BALLON
{
	bool			Aktiv;		///Gibts einen Ballon
	ZWEID			Pos;		//Postion
	short			Munition;	//Welche Munition hängt unten dran
	short			Anzahl;		//Wieviel von der Sorte?
	ZWEIDGENAU		Rel;		//Relative Postition im Pixel
};

struct PHYSIK
{
	short			Art;	// 0= nichts
	ZWEID			Pos;	//Position (für Pixel nicht benötigt
	double			Relx,Rely; //genauere Angaben innerhalb des Pixels 0.5 = Ruheposition
	ZWEIDGENAU		v;		//Geschwindikeitsvektor
	ZWEIDGENAU		Beschleunigung;
	bool			Kontakt;   //Liegt der Pixel auf einen anderen
	long			Lifetime;	//Lebensdauer
};

struct PIXEL
{
	bool			Aktiv;   //Muss der Pixel berechnet werden?
	short			CheckNr;	//Checknr, damit jeder Pixel nur ein berechnet wird
	DWORD           Farbe;		//Farbe
	PHYSIK			p;			//Physikvariablen
}; 

struct COMP
{
	short LastSchussEnergie; //Wie stark war der letzte Schuss 
	short Genauigkeit;		 //Um wieviel wird die Schussenergie das nächste mal verändert
	bool  Zuweit;			 //true = zuweit	false = nicht weit genug
	short Zaehler;			 //Wie oft wurde schon auf diesen Gegner geschossen?
	ZWEID Richtung;			//Letzte Kanonenrichtung für dieses Ziel
};

struct PANZER
{
	bool			Aktiv;
	DWORD			Farbe;		//Farbe
	short			Konto;		//Wieviel Geld besitzt man
	short			RGewonnen;	//Wieviel Runden gewonnen?
	short			Treffer;	//Wieviel Treffer geschafft?
	short			Abschuesse;	//Wieviel Panzer abgeschossen?
	short			Bild;		//Welches Bild
	short			Version;	//Welche Bildversion
	ZWEID			KanonePos;	//Relative Zielkoordinaten der Kanonenspitze
	PHYSIK			p;			//Physikvariablen
	bool			SchussAktiv;
	short			Munition;	//Welche Munition ist gewählt
	short			Lager[MUNANZAHL];	//Wieviel der Munition hat man noch (-1 unendlich)
	bool			ButtonDownUp; //Knopf nach oben gedrückt
	bool			ButtonDownDown; //Knopf nach unten gedrückt
	bool			ButtonDownFire; //Feuerknopf gedrückt
	short			SchussEnergie; //Wieviel lange ist der Feuerknopf gedrückt
	short			LebensEnergie; //Gesundheitszustand
	short			Schild;		//Wie lange wirkt das Schutzschild noch
	short			Magnet;		//Wie lange ist das Magnet noch da
	bool			Computer;	//Ist der Spieler ein Computer
	short			Ziel;		//Das Aktuelle Ziel (für den Computer)
	short			WunschMunition;	 //Diese Munition wird gewählt (nur für den Computer)
	ZWEID			WunschRichtung;	 //Welche Richtung ist erwünscht (nur für den Computer)
	COMP			Comp[MAXSPIELER]; //Die Sachen, die sich der Computerspieler merken muss
	short			KeyLeft;		//Die Tasten der Spieler
	short			KeyRight;
	short			KeyUp;
	short			KeyDown;
	short			KeyFire;
} ;

struct MUNITION
{
	short			Bild;		//Welches Bild
	short			Explosion;  //Welche Explosion
	short			Smoke[2];		//Nicht nur Rauch
	short			MaxDamage;  //Wieviel Schaden richtet sie maximal an?
	short			Dauer;
	short			Preis;		//Wieviel kostet diese Waffe?
	short			FlugSound;
	short			AbschussSound;
} ;

struct MUNLISTE
{
	PHYSIK			p;
	short			Besitzer;	//Wem gehört das Geschoss
	bool			Ausserhalb;	//Oberhalb des Bildschirmrandes
	short			SoundBuffer; //-1 = noch kein Soundbuffer sonst der aktive Soundbuffer
};

struct ART
{
	RGBSTRUCT	Farbe;
	double		m;			//Gewicht eines Pixel in kg
	double      Reibung;	    //Ab dieser Geschwindgkeit bleibt er stehen
}; 

struct BMP
{
	LPDIRECTDRAWSURFACE4 Surface; //in welcher Surface gespeichert?
	RECT rcSrc;     //Quelle des Bildes
	short Breite;   //Die Breite des Bildes
	short Hoehe;	//Die Hoehe des Bildes
};

struct WAV
{
	char *Dateiname; //Dateiname der Wavdatei
	short Volume;	//Die Standardlautstärke in Prozent
};

struct Menuepunkt
{
	Menuepunkt* next;
	ZWEID		pos;
	short		bild;
	bool		mouseover; 
	short		link;
	short		change;
	short		version;
	short		ziffer;
	short		schriftart;
};

struct CREDITSLISTE
{
	short		Bild;
	short		Zeit;
	bool		Ueberschrift;
};

class Menueseite
{
public:
	Menueseite();
	~Menueseite();
	void setzeMenuepunkt(short x,short y,short bild, bool mouseover, short link,
						 short change,short ziffer, short schriftart);
	void setzeBackground(short bild);
	int	 getVersion(short punktnr);
	void putVersion(short punktnr, short version);
	int  getZiffer(short punktnr);
	void putZiffer(short punktnr, short ziffer);
	void putBild(short punktnr, short bild);
	int  inputMouse();
	void zeige();
private:
	Menuepunkt* firstmenuepunkt;
	short background;
};

//Deklarationen
void InitDDraw();					//DirectDraw initialisieren
void InitDInput();					//DirectInput initialisieren
void InitDSound();					//DirectSound initialisieren
void CheckMouse();					//Mouse nachprüfen
void SetAcquire();					//Maus aktivieren

void InitStructs(short zustand);						// Intitialisiert die Structuren
void NeuesSpiel();									// Neues Spiel starten
void Compute();										// Zufallslandschaft erzeugen
void Zeige(bool flippen);							// Bild zusammenbauen anzeigen
void ZeichneBmp(short x, short y, short i,RECT Ziel,short Version,LPDIRECTDRAWSURFACE4 lpDDSNach);   // Zeichnet die Bmp-Bilder
void CheckMouse();									// Maus abfragen
short CheckKey();									// Tastatur abfragen
inline DWORD RGB2DWORD(BYTE r, BYTE g, BYTE b);		// Rechnet 24Bit RGB in 16 Bit um (Für ddbltfx.dwFillColor)
inline void DWORD2RGB(DWORD color);					//Rechnet 16Bit in 24Bit RGB um
void CalcRect(RECT rcBereich);						//Beschneidet das BlitRechteck so, daß es nur innerhalb des Bereiches gemalt wird
void DrawString(char *string,short x,short y, short Art,LPDIRECTDRAWSURFACE4 lpDDSNach); //Schreibt einen String an eine gewünschte Stelle
short Refresh();									//Wird ständig ausgeführt und aktuallisiert alles;
void Blitten(LPDIRECTDRAWSURFACE4 lpDDSVon, LPDIRECTDRAWSURFACE4 lpDDSNach,bool Transp);
void PutPixel(short x, short y, DWORD color, LPDDSURFACEDESC2 ddsdtmp); //Schreibt Pixel in eine Surface (davor lock())
DWORD GetPixel(short x, short y, LPDDSURFACEDESC2 ddsdtmp); //Das Ergebnis wird in rgbStruct gespeichert
void LoadSound(short Sound);						//Lädt einen Sound in den Speicher		
short PlaySound(short Sound, short Volume, short Pan, short SoundBuffer, bool Loop, short Frequenz);//Spiel eine Wavedatei mit Prozentualer Lautstärke, und Stereo abhängig von Bildschirmbreite (0-640)
void StopSound(short Channel);						//Stoppt ein Sound
void StopAllSound();								//Stoppt alle Sounds
void Aktivate(short x, short y, bool Aktiv);		//De-,Aktiviert einen Pixel
void FindActivePixel();								//Berechnet alle aktiven Pixel
void MovePixel(short x, short y, short xneu, short yneu); //Erledigt das Verschieben der Pixel
void MakePixel(short x, short y, short Typ, short vx, short vy, bool Active, long Lifetime); //Erledigt das eintragen der Pixel
void DeletePixel(short x,short y);					//Löscht ein Pixel
double Steigung(short x, short y);					//Ermittelt die Hangsteigung von einem Punkt (liefert den Winkel)
void CheckPanzer();									//Bearbeitet die Panzer
void ZeichnePanzer(short i,short Teil);				//Zeichnet die Panzer
void ZeigePanzer();									//Blittet die Panzer
void MoveKanone(short i, short Richtung);			//Berechnet die neue Position der Kanone (negativ=links)		
void ZeichneLinie(short x1, short y1, short x2, short y2, DWORD color, LPDDSURFACEDESC2 ddsdtmp); //Zeichnet eine Linie
void PutPanzer();									//Setzt die Panzer in die Landschaft
void Explosion(short MunNr);						//Berechnet für die Munition die passende Explosion
void CheckHimmelPixel();							//Checked die Pixel oberhalb des Bildschirms durch
inline double Absdouble(double Zahl);				//Liefert Betrag von einer Zahl
void CheckFXPixel();								//Prüft löscht und zeichnet die FX-Pixel
void Abschuss(short i);								//Schiesst eine Munition los (mit Angabe des Panzers)
void ChangeLebensEnergie(short i,short Anzahl,short Verursacher);//Zählt Lebensenergie ab und dazu
short CheckKollision(short x,short y, bool genau);	//Welcher Panzer beinhaltet den Pixel??
void TranspBlt(short Bild, short Prozent);			//Halbtansparentes blitten
void ChangeMunition(short i, short add);			//Wechsel die Munition (entweder nach oben oder nach unten)
void CheckMunListe();								//Steuert die umherfliegende Munition
void ZeichneMunition();								//Zeichnet die Munition
void PanzerExpl(short PanzNr);						//Lässt einen Panzer explodieren
void Computer();									//Berechnet die Computerspieler
void CalcSchussErgebnis(short x, short Besitzer);	//Findet heraus, wie gut der Schuss war (für die Computer)
void MakeWetter();									//Erzeugt das Wetter
void MakeFXPixel(short x, short y, DWORD Farbe, short Lifetime, double Bewx, double Bewy); //Erzeugt ein FX-Pixel
void Laser(short i);								//Schiesst einen Laserstrahl ab
ZWEID LineIntersect(short x, short y, short xneu, short yneu, bool gesamt); //Linie unterbrochen?
bool LoadGame();									//Lädt die Savegamedatei
void SaveGame();									//Speichert die Einstellungen
void GetMenue();									//Holt die Einstellungen aus dem Menü
void PutMenue();									//Stellt die Voreinstellungen ein
void CheckShop(short i);							//Managed den Shop
int  CheckMMenue();									//Managed das Hauptmenü
void ComputerShop(short i);							//Der Computer kauft ein
void PutGMenue();									//Stellt das Gewonnenbild ein	
void MakeMagRakete();								//Berechnet die Beschleunigung durch den Magnetismus
void MakeMagnet(short i);							//Berechnet die Beschleunigung durch den Magnetismus				
void CheckBallon();									//Erzeugt und berechnet die Bewegung vom Ballon
void BallonAbschuss(short i);						//Ballon explodiert und gibt dem Verantwortlichen die Waffen
void Fetz(short x, short y, short Bild);			//Zerfetzt ein Bild an gegebener linken/oberen Ecke
void InitCredits();									//Initialisiert die Credits
void CheckCredits();								//Steuert die Credits
void MakeTitel();									//Zeichnet das Titelbild

#endif