#include "panzer.h"

extern ZWEID MousePosition;
extern BMP Bmp[];
extern BOOL Button0down;
extern short ButtonPush;
extern short AktMenue;
extern RECT rcGesamt;
extern LPDDSURFACEDESC2 lpDDSBack;
extern short CursorTyp;

Menueseite::Menueseite()
{
	firstmenuepunkt = NULL;
	background = -1;
}

Menueseite::~Menueseite()
{
	while (firstmenuepunkt != NULL)
	{
		Menuepunkt *t = firstmenuepunkt;
		firstmenuepunkt = t->next;
		delete t;
	}
}

void Menueseite::setzeMenuepunkt(short x, short y, short bild, bool mouseover, short link,
								 short change, short ziffer, short schriftart)
{
	Menuepunkt *t = firstmenuepunkt;
	Menuepunkt *h = new Menuepunkt;

	h->next = NULL;
	h->pos.x = x;
	h->pos.y = y;
	h->bild = bild;
	h->mouseover = mouseover;
	h->link = link;
	h->change = change;
	h->version = 0;
	h->ziffer = ziffer;
	h->schriftart = schriftart;

	if (t == NULL)
	{
		firstmenuepunkt = h;
		return;
	}

	while (t->next != NULL)
	{
		t = t->next;
	}
	t->next = h;
}

void Menueseite::setzeBackground(short bild)
{
	background = bild;
}

int Menueseite::getVersion(short punktnr)
{
	Menuepunkt *t = firstmenuepunkt;
	int i = 0;

	while (t != NULL)
	{
		if (i == punktnr)
			return t->version;
		t = t->next;
		i++;
	}
	return -1;
}

void Menueseite::putVersion(short punktnr, short version)
{
	Menuepunkt *t = firstmenuepunkt;
	short i = 0;

	while (t != NULL)
	{
		if (i == punktnr)
		{
			t->version = version;
			break;
		}
		t = t->next;
		i++;
	}
}

int Menueseite::getZiffer(short punktnr)
{
	Menuepunkt *t = firstmenuepunkt;
	int i = 0;

	while (t != NULL)
	{
		if (i == punktnr)
			return t->ziffer;
		t = t->next;
		i++;
	}
	return -1;
}

void Menueseite::putZiffer(short punktnr, short ziffer)
{
	Menuepunkt *t = firstmenuepunkt;
	short i = 0;

	while (t != NULL)
	{
		if (i == punktnr)
		{
			t->ziffer = ziffer;
			break;
		}
		t = t->next;
		i++;
	}
}

void Menueseite::putBild(short punktnr, short bild)
{
	Menuepunkt *t = firstmenuepunkt;
	short i = 0;

	while (t != NULL)
	{
		if (i == punktnr)
		{
			t->bild = bild;
			break;
		}
		t = t->next;
		i++;
	}
}

int Menueseite::inputMouse()
{
	Menuepunkt *t = firstmenuepunkt;
	short i = 0;
	short tmp = -1;

	while (t != NULL)
	{
		if ((MousePosition.x > t->pos.x) && (MousePosition.x < (t->pos.x + Bmp[t->bild].Breite)) &&
			(MousePosition.y > t->pos.y) && (MousePosition.y < (t->pos.y + Bmp[t->bild].Hoehe)))
			tmp = i;
		i++;
		t = t->next;
	}
	i = 0;
	t = firstmenuepunkt;
	if ((tmp != -1) && Button0down && (ButtonPush == 1))
	{
		while (t != NULL)
		{
			if (i == tmp)
				break;
			i++;
			t = t->next;
		}
		if (t->link < -1)
		{
			return t->link; // Sonderf�lle
		}
		if (t->link >= 0)
		{
			AktMenue = t->link;
		}
		if (t->change > 0) // Auswahlfl�chen
		{
			t->version++;
			if (t->version >= t->change)
				t->version = 0;
		}
		if (t->mouseover)
			audio_manager->play(audio::CLICK, audio::id::SFX, 100, false);
		return i;
	}
	return -1;
}

void Menueseite::zeige()
{
	int ddrval;
	char StdString[10];

	Menuepunkt *t = firstmenuepunkt;

	if (background != -1)
		ZeichneBmp(0, 0, background, rcGesamt, 0, lpDDSBack.texture); // Hintergrund

	while (t != NULL) // Menuepunkte zeichnen
	{
		if (t->ziffer == -1)
		{
			if ((MousePosition.x > t->pos.x) && (MousePosition.x < (t->pos.x + Bmp[t->bild].Breite)) &&
				(MousePosition.y > t->pos.y) && (MousePosition.y < (t->pos.y + Bmp[t->bild].Hoehe)) &&
				(t->mouseover))
			{
				ZeichneBmp(t->pos.x, t->pos.y, t->bild, rcGesamt, 2 * (t->version) + 1, lpDDSBack.texture);
				t = t->next;
				continue;
			}
			if (t->mouseover)
				ZeichneBmp(t->pos.x, t->pos.y, t->bild, rcGesamt, 2 * t->version, lpDDSBack.texture);
			else
				ZeichneBmp(t->pos.x, t->pos.y, t->bild, rcGesamt, t->version, lpDDSBack.texture);
		}
		else
		{
			sprintf(StdString, "%d", t->ziffer);
			DrawString(StdString, t->pos.x, t->pos.y, t->schriftart, lpDDSBack.texture);
		}
		t = t->next;
	}
}
