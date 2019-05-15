#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

//BIBLIOTEKI

#include<math.h>
#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include <time.h>
extern "C" 
{
	#include"./sdl-2.0.7/include/SDL.h"
	#include"./sdl-2.0.7/include/SDL_main.h"
}

//STALE

#define SZEROKOSC_EKRANU 1000
#define WYSOKOSC_EKRANU	700
#define DL_INFORMACJE 128
#define DL_NAPIS 32
#define WYSOKOSC_NAGLOWKA 100
#define WIELKOSC_ODSTEPU_NAGLOWKA 4
#define ROZMIAR_PLANSZY 450
#define ROZMIAR_KOMORKI 100
#define POCZ_WYSOKOSC_PLANSZY 150
#define WIELOSC_ODSTEPU_KOMORKI 10 //odstep miedzy komorkami na planszy
#define ILOSC_KOMOREK 4
#define ILOSC_NUMERKOW 17
#define KONIEC_GRY 2048
#define BRAK_OBRAZKA 0
#define NIE_ZWYCIESTWO 0
#define ZWYCIESTWO 1
#define PO_ZWYCIESTWIE 2

//STRUKTURY

typedef struct
{
	int pierwszy;
	int drugi;
}para_t;

typedef enum
{
	FALSZ,
	PRAWDA
}moj_bool_t;

typedef enum
{
	GORA,
	DOL,
	PRAWO,
	LEWO
}kierunek_t;

typedef struct
{
	//struktura opisujaca wlasnosci danego obrazka - powierzchnie i wartosc, czyli liczbe jaka zawiera obrazek
	SDL_Surface *obrazek;
	int wartosc;
}komorka_t;

typedef struct
{
	//pola opisujace plansze
	para_t wspolrzedne;//stale - okreslaja wspolrzedne gornego rogu danej komorki
	int wartosc;//liczba jaka zawiera obrazek znajdujacy sie na danym kafelku, 0 - brak obrazka
	para_t wspolrzedne_nowe;//wspolrzedne, na ktore ma pojsc dana komorka poprzez animacje
	moj_bool_t zloczone;//okresla czy komorka jest zloczona w jedna, np. dwie dwojki tworza czworke itp.
	moj_bool_t stale;//okresla czy na danej komorce bedzie animacja
}komorki_planszy_t;

typedef struct
{
	SDL_Texture *tekstura;
	SDL_Window *okno;
	SDL_Renderer *renderer;
	SDL_Surface *ekran;
}glowna_t;

//NAGLOWKI FUNKCJI

//funckje z szablonu
void DrawString(SDL_Surface *ekran, int poczatek_x, int poczatek_y, const char *napis, SDL_Surface *kodowanie);
void DrawSurface(SDL_Surface *ekran, SDL_Surface *sprite, int poczatek_x, int poczatek_y);
void DrawSurface(SDL_Surface *ekran, SDL_Surface *sprite, int poczatek_x, int poczatek_y);
void DrawPixel(SDL_Surface *powierzchnia, int poczatek_x, int poczatek_y, Uint32 kolor);
void DrawLine(SDL_Surface *ekran, int poczatek_x, int poczatek_y, int dlugosc, int poziomo, int pionowo, Uint32 kolor);
void DrawRectangle(SDL_Surface *ekran, int poczatek_x, int poczatek_y, int dlugosc, int szerokosc, Uint32 kolor_ramki, Uint32 wypelnienie);

//moje funkcje
void wstaw_naglowek(SDL_Surface *ekran, SDL_Surface *kodowanie, double czas_glowny, int punkty);
//funckja wstawia plansze i rysuje stale obrazki, czyli te ktore nie podlegaja animacji
void wstaw_plansze(glowna_t obraz, komorki_planszy_t **plansza, komorka_t *numerki, moj_bool_t *zwyciestwo);
void plansza_init(komorki_planszy_t **plansza);
//funckja pobiera obrazek BMP z pliku
void obrazek_bmp_init(SDL_Surface **obrazek, char *nazwa_obrazka, glowna_t obraz);
void numerki_init(komorka_t *numerki, glowna_t obraz);
//funckja losuje polozenie dwojki i umieszcza ja w tablicy plansza
void losuj_2(komorka_t *numerki, komorki_planszy_t **plansza);
//funckja odpowiada za sprawdzenie, gdzie dana komoroka moze dojsc, wykonuje laczenie komorek
void przejscie(komorki_planszy_t **plansza, kierunek_t kierunek, int *punkty);
//funckja ustawia poczatek iteracji po planszy przy sprawdzaniu w zaleznosci od kieruneku
para_t ustaw_poczatek(para_t poczatek, kierunek_t kierunek);
//funckja iteruje po planszy w zaleznosci od kierunku
int iteracja(int it, kierunek_t kierunek);
//warunek konca iteracji po planszy
moj_bool_t warunek(int it, kierunek_t kierunek);
//funckja ustawia odwrotny 
kierunek_t odwrotny(kierunek_t kierunek);
void animacja_init(komorki_planszy_t *animacja);
void zrob_animacje(komorki_planszy_t *animacja, kierunek_t kierunek, int *ile_do_animacji, int *ile_wyzerowanych, glowna_t obraz, moj_bool_t *wyzerowane, komorki_planszy_t **plansza, komorka_t *numerki);
//funckja sprawdza dla jakich kafelkow ma byc wykonana animacja
void czy_animacja(komorki_planszy_t **plansza, komorka_t *numerki, komorki_planszy_t *animacja, int *ile_do_animacji);
//funckja odpowiada za interfejs przy zwyciestwie
void wypisz_zwyciestwo(SDL_Surface *ekran, SDL_Surface *kodowanie);
//funckja kopiuje zwartosc planszy do drugiej, pomocniczej planszy - wykorzystane przy cofaniu ruchu
void kopiuj_plansze(komorki_planszy_t **plansza, komorki_planszy_t **plansza_cofania);
//funckja zwalnia pamiec
void zwolnic_wszystko(komorka_t *numerki, SDL_Surface *kodowanie, glowna_t obraz);

#ifdef __cplusplus
	extern "C"
#endif

int main(int argc, char **argv)
{
	//inicjacja SDL'a
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	//ile_wyzerowanych okresla ilosc komorek, ktorych animacja zostala zakonczona 
	int rc, czas_p, czas_k, punkty = 0, ile_do_animacji = 0,ile_wyzerowanych=0, punkty_cofania=0;
	//tablica mowiaca, czy animacja danej komorki juz sie zakonczyla
	moj_bool_t *wyzerowane = (moj_bool_t*)calloc(ILOSC_KOMOREK*ILOSC_KOMOREK, sizeof(moj_bool_t));
	moj_bool_t czy_przejsc;//flaga informujaca czy wykonac przejscie w jakims kierunku po planszy
	moj_bool_t koniec;//flaga oznaczajaca koniec gry, zamkniecie okna
	moj_bool_t zwyciestwo, czy_zwyciestwo;
	moj_bool_t czy_cofanie;//flaga okreslajaca czy uzytkownik wcisnal u w celu cofniecia ruchu
	double roznica_czasu, czas_glowny;
	kierunek_t kierunek;//kierunek przechodzenia po planszy
	//tablica zawierajaca wszystkie obrazki, ideks do wartosci odbrazka to i=log2(wartosc)-1
	komorka_t *numerki = (komorka_t*)malloc(ILOSC_NUMERKOW * sizeof(komorka_t));
	//tablica zawierajaca te komorki tablicy, ktore wymagaja animacji
	komorki_planszy_t *animacja = (komorki_planszy_t*)malloc(ILOSC_KOMOREK*ILOSC_KOMOREK * sizeof(komorki_planszy_t));
	SDL_Event event;
	SDL_Surface *kodowanie=NULL;//zawiera znaki do wypisywania na ekran
	glowna_t obraz;
	//tablica opisujaca, co zawiera plansza
	komorki_planszy_t **plansza = (komorki_planszy_t**)malloc(ILOSC_KOMOREK * sizeof(komorki_planszy_t*));
	for (int i = 0; i < ILOSC_KOMOREK; i++)plansza[i] = (komorki_planszy_t *)malloc(ILOSC_KOMOREK * sizeof(komorki_planszy_t));
	//pomocnicza plansza zawierajaca informacje sprzed danego ruchu - wykorzystane przy cofaniu
	komorki_planszy_t **plansza_cofania = (komorki_planszy_t**)malloc(ILOSC_KOMOREK * sizeof(komorki_planszy_t*));
	for (int i = 0; i < ILOSC_KOMOREK; i++)plansza_cofania[i] = (komorki_planszy_t *)malloc(ILOSC_KOMOREK * sizeof(komorki_planszy_t));
	rc = SDL_CreateWindowAndRenderer(SZEROKOSC_EKRANU, WYSOKOSC_EKRANU, 0,&obraz.okno, &obraz.renderer);
	if(rc != 0) 
	{
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		free(plansza);
		free(plansza_cofania);
		free(numerki);
		free(wyzerowane);
		free(animacja);
		return 1;
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(obraz.renderer, SZEROKOSC_EKRANU, WYSOKOSC_EKRANU);
	SDL_SetRenderDrawColor(obraz.renderer, 0, 0, 0, 255);
	obraz.ekran = SDL_CreateRGBSurface(0, SZEROKOSC_EKRANU, WYSOKOSC_EKRANU, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	obraz.tekstura = SDL_CreateTexture(obraz.renderer, SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,SZEROKOSC_EKRANU, WYSOKOSC_EKRANU);
	SDL_ShowCursor(SDL_DISABLE);// wy³¹czenie widocznoœci kursora myszy
	obrazek_bmp_init(&kodowanie, "cs8x8", obraz);
	SDL_SetColorKey(kodowanie, true, 0x000000);
	int kolor_tla = SDL_MapRGB(obraz.ekran->format, 0xFA, 0xEB, 0xD7);//antic white
	//inicjalizacja tablic i zerowanie zmiennych
	numerki_init(numerki, obraz);
	plansza_init(plansza);
	plansza_init(plansza_cofania);
	animacja_init(animacja);
	koniec = czy_przejsc = FALSZ;
	czy_cofanie = zwyciestwo = FALSZ;
	czy_zwyciestwo = FALSZ;
	czas_glowny = roznica_czasu = 0;
	srand(time(NULL));//uruchomienie ziarna do losowania
	//na poczatku losujemy polozenie dwoch dwojek
	losuj_2(numerki, plansza);
	losuj_2(numerki, plansza);
	czas_p = SDL_GetTicks();
	while(koniec==FALSZ) 
	{
		//obsluga czasu
		czas_k = SDL_GetTicks();
		roznica_czasu = (czas_k - czas_p) * 0.001;
		czas_p = czas_k;
		czas_glowny += roznica_czasu;
		if (czy_cofanie == PRAWDA)//jezeli gracz chce cofnac ruch
		{
			//kopiuje zawartosc planszy pomocniczej do oryginalnej planszy, co wykonuje cofanie ruchu
			kopiuj_plansze(plansza_cofania, plansza);
			punkty = punkty_cofania;//przywracam poprzedni stan punktow
			czy_cofanie = FALSZ;//cofnalem, wiec przy kolejnych iteracjach nie moge cofac do kolejnego nacisniecia u
		}
		if (czy_przejsc==PRAWDA)//jezeli gracz wcisnal strzalki to oznacza, ze mamy przejsc po planszy i przesuwac kafelki
		{
			kopiuj_plansze(plansza, plansza_cofania); punkty_cofania = punkty;//kopiuje zawartosc planszy do planszy pomocniczej
			przejscie(plansza, kierunek, &punkty);//przejscie po planszy
			czy_przejsc = FALSZ;//w kolejnych iteracjach nie wykonuje przejscia do nacisniecia strzalek
		}
		SDL_FillRect(obraz.ekran, NULL, kolor_tla);//nadanie ekranowi koloru
		wstaw_naglowek(obraz.ekran, kodowanie, czas_glowny,punkty);
		//jezeli nie jest uruchomiony interfejs zwyciestwa
		wstaw_plansze(obraz, plansza, numerki,&zwyciestwo);
		//po uruchomieniu interfejsu zwyciestwa i kontynuacji gry nie mozna dalej wygrac
		if (zwyciestwo == PRAWDA&&czy_zwyciestwo == PRAWDA)zwyciestwo = FALSZ;
		//jezeli gracz wygral po raz pierwszy to ma zostac uruchomiony interfejs zwyciestwa
		else if (zwyciestwo == PRAWDA&&czy_zwyciestwo==FALSZ)wypisz_zwyciestwo(obraz.ekran, kodowanie);
		//jezeli nie jestesmy w trakcie animacji to sprawdzamy, czy po przejsciu nie nalezy zrobic animacji
		if (ile_do_animacji == 0)czy_animacja(plansza, numerki, animacja, &ile_do_animacji);
		else
		{
			//jezeli mamy cos do animacji to ja wykonujemy
			zrob_animacje(animacja, kierunek, &ile_do_animacji, &ile_wyzerowanych, obraz, wyzerowane, plansza, numerki);
			//oznacza to koniec animacji - animacje wszystkich komorek zostaly zakonczone
			if (ile_do_animacji == ile_wyzerowanych)
			{
				//zerujemy wszystko, co zwiazane z animacja i losujemy dwojke - koniec pojedynczej tury
				ile_do_animacji = ile_wyzerowanych = 0;
				for (int i = 0; i < ILOSC_KOMOREK*ILOSC_KOMOREK; i++)wyzerowane[i] = FALSZ;
				animacja_init(animacja);
				losuj_2(numerki, plansza);
			}
		}
		SDL_UpdateTexture(obraz.tekstura, NULL, obraz.ekran->pixels, obraz.ekran->pitch);
		SDL_RenderClear(obraz.renderer);
		SDL_RenderCopy(obraz.renderer, obraz.tekstura, NULL, NULL);
		SDL_RenderPresent(obraz.renderer);
		while(SDL_PollEvent(&event)) 
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) koniec = PRAWDA;//escape - wyjscie z gry
					//blokada podczas animacji, animacja musi sie wykonac, a dopiero potem nastepuje nowy ruch
					if (ile_do_animacji == 0&&zwyciestwo==FALSZ)
					{
						if (event.key.keysym.sym == SDLK_n)
						{
							//zeruje czas, punkty, na nowo inicjalizuje plansze i losuje dwie dwojki
							roznica_czasu = czas_glowny = 0;
							plansza_init(plansza);
							losuj_2(numerki, plansza);
							losuj_2(numerki, plansza);
							punkty = 0;
						}
						else if (event.key.keysym.sym == SDLK_LEFT)
						{
							kierunek = LEWO;
							czy_przejsc = PRAWDA;
						}
						else if (event.key.keysym.sym == SDLK_RIGHT)
						{
							kierunek = PRAWO;
							czy_przejsc = PRAWDA;
						}
						else if (event.key.keysym.sym == SDLK_UP)
						{
							kierunek = GORA;
							czy_przejsc = PRAWDA;
						}
						else if (event.key.keysym.sym == SDLK_DOWN)
						{
							kierunek = DOL;
							czy_przejsc = PRAWDA;
						}
						else if (event.key.keysym.sym == SDLK_u)czy_cofanie = PRAWDA;
					}
					if (zwyciestwo == PRAWDA)
					{
						if (event.key.keysym.sym == SDLK_t)
						{
							czy_zwyciestwo = PRAWDA;
							zwyciestwo = FALSZ;
						}
						else if (event.key.keysym.sym == SDLK_n)koniec = PRAWDA;
					}
					break;
				case SDL_QUIT:
					koniec = PRAWDA;
					break;
				}
			}
		}
		zwolnic_wszystko(numerki, kodowanie,obraz);
		free(plansza); 
		free(plansza_cofania);
		free(numerki);
		free(wyzerowane);
		free(animacja);
		return 0;
	}

	void zwolnic_wszystko( komorka_t *numerki, SDL_Surface *kodowanie, glowna_t obraz)
	{
		SDL_FreeSurface(kodowanie);
		SDL_FreeSurface(obraz.ekran);
		SDL_DestroyTexture(obraz.tekstura);
		SDL_DestroyRenderer(obraz.renderer);
		SDL_DestroyWindow(obraz.okno);
		for (int i = 0; i < ILOSC_NUMERKOW; i++)SDL_FreeSurface(numerki[i].obrazek);
		SDL_Quit();
	}

	void kopiuj_plansze(komorki_planszy_t **plansza, komorki_planszy_t **plansza_cofania)
	{
		int i, j;
		for (i = 0; i < ILOSC_KOMOREK; i++)
		{
			for (j = 0; j < ILOSC_KOMOREK; j++)
			{
				plansza_cofania[i][j].wartosc = plansza[i][j].wartosc;
				plansza_cofania[i][j].wspolrzedne_nowe = {};
				plansza_cofania[i][j].stale = PRAWDA;
				plansza_cofania[i][j].zloczone = FALSZ;
			}
		}
	}

	void wypisz_zwyciestwo(SDL_Surface *ekran, SDL_Surface *kodowanie)
	{
		int kolor_zwyciestwa = SDL_MapRGB(ekran->format, 0x1B, 0x1B, 0x1B);//fajny czerny kolor;
		SDL_FillRect(ekran, NULL, kolor_zwyciestwa);
		char *informacje = (char*)malloc(DL_INFORMACJE * sizeof(char));
		sprintf(informacje, "GRATULACJE WYGRALES!");
		DrawString(ekran, ekran->w / 2 - strlen(informacje) * 8 / 2, WYSOKOSC_EKRANU / 2, informacje, kodowanie);
		sprintf(informacje, "Czy chcesz kontynuowac gre? T/N");
		DrawString(ekran, ekran->w / 2 - strlen(informacje) * 8 / 2, WYSOKOSC_EKRANU / 2 + 16, informacje, kodowanie);
		free(informacje);
	}

	para_t konwertowanie(para_t wspolrzedne)
	{
		//funckja konwertuje wspolrzedne w konsoli na indeksy w tablicy plansza
		para_t wynik;
		wynik.pierwszy = (wspolrzedne.drugi - POCZ_WYSOKOSC_PLANSZY - WIELOSC_ODSTEPU_KOMORKI) / (ROZMIAR_KOMORKI + WIELOSC_ODSTEPU_KOMORKI);
		wynik.drugi = (wspolrzedne.pierwszy - SZEROKOSC_EKRANU / 4 - WIELOSC_ODSTEPU_KOMORKI) / (ROZMIAR_KOMORKI + WIELOSC_ODSTEPU_KOMORKI);
		return wynik;
	}

	void animacja_init(komorki_planszy_t *animacja)
	{
		//inicjalizacja tablicy animacja
		int i;
		for (i = 0; i < ILOSC_KOMOREK*ILOSC_KOMOREK; i++)
		{
			animacja[i].wspolrzedne = {};
			animacja[i].wspolrzedne_nowe = {};
			animacja[i].wartosc = BRAK_OBRAZKA;
			animacja[i].zloczone = FALSZ;
		}
	}

	void plansza_init(komorki_planszy_t **plansza)
	{
		//funckja zeruje zawartosc tablicy plansza oraz oblicza wpolrzedne poszczegolnych komorek
		int poczatek = SZEROKOSC_EKRANU / 4;
		int i, j, przesuniecie, rzad;
		for (i = 0; i < ILOSC_KOMOREK; i++)
		{
			rzad = i*(ROZMIAR_KOMORKI + WIELOSC_ODSTEPU_KOMORKI) + WIELOSC_ODSTEPU_KOMORKI;
			for (j = 0; j < ILOSC_KOMOREK; j++)
			{
				przesuniecie = WIELOSC_ODSTEPU_KOMORKI * (j + 1) + ROZMIAR_KOMORKI * j;
				plansza[i][j].wspolrzedne.pierwszy = poczatek + przesuniecie;
				plansza[i][j].wspolrzedne.drugi = POCZ_WYSOKOSC_PLANSZY + rzad;
				plansza[i][j].wspolrzedne_nowe = {};
				plansza[i][j].wartosc = BRAK_OBRAZKA;
				plansza[i][j].zloczone = FALSZ;
				plansza[i][j].stale = PRAWDA;
			}
		}
	}

	void obrazek_bmp_init(SDL_Surface **obrazek, char *nazwa_obrazka, glowna_t obraz)
	{
		//funckja czyta obrazek z pliku
		char sciezka[DL_NAPIS] = "./GRAFIKA/";
		//strcat dopisuje napis drugiego argumentu do pierwszego 
		strcat(sciezka, nazwa_obrazka);
		strcat(sciezka, ".bmp");
		*obrazek = SDL_LoadBMP(sciezka);
		//oblsuga bledu
		if (*obrazek == NULL)
		{
			printf("SDL_LoadBMP() error: %s\n", SDL_GetError());
			SDL_FreeSurface(obraz.ekran);
			SDL_DestroyTexture(obraz.tekstura);
			SDL_DestroyWindow(obraz.okno);
			SDL_DestroyRenderer(obraz.renderer);
			SDL_Quit();
			return;
		}
	}

	void wstaw_naglowek(SDL_Surface *ekran, SDL_Surface *kodowanie, double czas_glowny, int punkty)
	{
		//funkcja wypisuje informacje w naglowku
		int kolor_naglowka = SDL_MapRGB(ekran->format, 0xCD, 0x85, 0x3F);//peru
		char *informacje = (char*)malloc(DL_INFORMACJE * sizeof(char));
		//rysowanie kwadratu z naglowkiem
		DrawRectangle(ekran, WIELKOSC_ODSTEPU_NAGLOWKA, WIELKOSC_ODSTEPU_NAGLOWKA, SZEROKOSC_EKRANU - 2*WIELKOSC_ODSTEPU_NAGLOWKA, WYSOKOSC_NAGLOWKA, kolor_naglowka, kolor_naglowka);

		sprintf(informacje, "2048");
		DrawString(ekran, ekran->w / 2 - strlen(informacje) * 8 / 2, 10, informacje, kodowanie);
		sprintf(informacje, "AUTOR: Michal Sieczczynski 175989");
		DrawString(ekran, ekran->w / 2 - strlen(informacje) * 8 / 2, 26, informacje, kodowanie);
		sprintf(informacje, "Czas od poczatku gry: %.1lf s", czas_glowny);
		DrawString(ekran, ekran->w / 2 - strlen(informacje) * 8 / 2, 42, informacje, kodowanie);
		sprintf(informacje, "Punkty: %d", punkty);
		DrawString(ekran, ekran->w / 2 - strlen(informacje) * 8 / 2, 58, informacje, kodowanie);
		sprintf(informacje, "Strzalki - przesuniecie klockow w kierunku, Esc - wyjscie, n - nowa gra, u - cofniecie pojedynczego ruchu");
		DrawString(ekran, ekran->w / 2 - strlen(informacje) * 8 / 2, 74, informacje, kodowanie);
		free(informacje);
	}

	void wstaw_plansze(glowna_t obraz, komorki_planszy_t **plansza, komorka_t *numerki, moj_bool_t *zwyciestwo)
	{
		int kolor_komorki = SDL_MapRGB(obraz.ekran->format, 0xD3, 0xD3, 0xD3);//Lightgray
		int kolor_planszy = SDL_MapRGB(obraz.ekran->format, 0xA9, 0xA9, 0xA9);//Darkgray
		int poczatek = SZEROKOSC_EKRANU / 4;
		int i,j,it,x,y;
		//rysowanie duzego ciemniejszego kwadratu jako tla dla komorek
		DrawRectangle(obraz.ekran, poczatek, POCZ_WYSOKOSC_PLANSZY, ROZMIAR_PLANSZY, ROZMIAR_PLANSZY, kolor_planszy, kolor_planszy);
		for (i = 0; i < ILOSC_KOMOREK; i++)
		{
			for (j = 0; j < ILOSC_KOMOREK; j++)
			{
				plansza[i][j].zloczone = FALSZ;//zerowanie zloczonych komorek
				if (plansza[i][j].wartosc == BRAK_OBRAZKA||plansza[i][j].stale==FALSZ)
				{
					//jezeli wartosc jest rowna 0 to oznacza, ze obrazek nie powinien znajdowac sie na tej komorce
					//jezeli stale jest FALSZ to oznacza, ze w tej komorce bedzie obrazek z animacja, ktora jeszcze trwa
					DrawRectangle(obraz.ekran, plansza[i][j].wspolrzedne.pierwszy, plansza[i][j].wspolrzedne.drugi, ROZMIAR_KOMORKI, ROZMIAR_KOMORKI, kolor_komorki, kolor_komorki);
				}
				else if (plansza[i][j].stale==PRAWDA)
				{
					//dla obrazkow, ktore nie podlegaja animacji, a maja byc - ich wartosc rozna od 0
					//okreslenie wspolrzednych
					//+ROZMIAR_KOMORKI/2 - poniewaz DrawSurface przyjmuje srodek obrazka
					x = plansza[i][j].wspolrzedne.pierwszy + ROZMIAR_KOMORKI / 2;
					y = plansza[i][j].wspolrzedne.drugi + ROZMIAR_KOMORKI / 2;
					if (plansza[i][j].wartosc == KONIEC_GRY)(*zwyciestwo) = PRAWDA;
					it = log2(double(plansza[i][j].wartosc)) - 1;//wyznaczenie indeksu w tablicy numerki na podstawie wartosci komorki
					//plansza[i][j].komorka = numerki[it];//przekazanie obrazka, gdyby wartosc sie zwiekszyla
					DrawSurface(obraz.ekran, numerki[it].obrazek, x, y);//rysowanie obrazka
				}	
			}
		}
	}

	void numerki_init(komorka_t *numerki,glowna_t obraz)
	{
		int i;
		char *napis = (char*)malloc(DL_NAPIS * sizeof(char));
		for (i = 0; i < ILOSC_NUMERKOW; i++)
		{
			//exp2(x) = 2^x
			numerki[i].wartosc = exp2((double)(i + 1));
			numerki[i].obrazek = NULL;
			sprintf(napis, "%d", numerki[i].wartosc);//konwersja wartosci na napis
			obrazek_bmp_init(&numerki[i].obrazek, napis, obraz);//inicjalizacja obrazka
		}
		free(napis);
	}

	void losuj_2(komorka_t *numerki, komorki_planszy_t **plansza)
	{
		int ilosc_pustych = 0,i,j,wylosowany_indeks=0;
		for (i = 0; i < ILOSC_KOMOREK; i++)
		{
			for (j = 0; j < ILOSC_KOMOREK; j++)
			{
				//poszukiwanie pustych komorek i zliczenie ich ilosci
				if (plansza[i][j].wartosc == BRAK_OBRAZKA)ilosc_pustych++;
			}
		}
		if (ilosc_pustych > 0)//jezeli sa jakies puste komorki
		{
			int losowa = rand() % ilosc_pustych+1;//losuje z przedzialu <1;ilosc_pustych>
			for (i = 0; i < ILOSC_KOMOREK; i++)
			{
				for (j = 0; j < ILOSC_KOMOREK; j++)
				{
					//szukamy gdzie lezy wylosowana komorka
					if (plansza[i][j].wartosc == BRAK_OBRAZKA)wylosowany_indeks++;
					if (wylosowany_indeks == losowa)//jezeli ja znalazlem to zapisuje tam dwojke
					{
						plansza[i][j].wartosc = numerki[0].wartosc;
						return;
					}
				}
			}
		}
	}

	para_t ustaw_poczatek(para_t poczatek, kierunek_t kierunek)
	{
		//funckja ustawia poczatek itecji po planszy
		//omijam ostatni/pierwszy rzad/kolumne poniewaz nie mozna z tamtad przsuwac komorek
		switch (kierunek)
		{
			case LEWO:
				poczatek.pierwszy = 0;
				poczatek.drugi = 1;
				break;
			case PRAWO:
				poczatek.pierwszy = 0;
				poczatek.drugi = ILOSC_KOMOREK - 2;
				break;
			case GORA:
				poczatek.pierwszy = 1;
				poczatek.drugi = 0;
				break;
			case DOL:
				poczatek.pierwszy = ILOSC_KOMOREK - 2;
				poczatek.drugi = 0;
				break;
		}
		return poczatek;
	}
	
	int iteracja(int it, kierunek_t kierunek)
	{
		//iteruje w zaleznosci od kierunku
		if (kierunek == LEWO || kierunek == GORA)it++;
		if (kierunek == PRAWO || kierunek == DOL)it--;
		return it;
	}

	moj_bool_t warunek(int it, kierunek_t kierunek)
	{
		//warunek okreslajacy koniec iteracji w zaleznosci od kierunku
		moj_bool_t flaga = PRAWDA;
		if (kierunek == LEWO || kierunek == GORA)
			if(it >= ILOSC_KOMOREK)flaga=FALSZ;
		if (kierunek == PRAWO || kierunek == DOL)
			if(it <0 )flaga = FALSZ;
		return flaga;
	}

	kierunek_t odwrotny(kierunek_t kierunek)
	{
		//okreslamy kierunek odwrotny
		if (kierunek == PRAWO)return LEWO;
		if (kierunek == LEWO)return PRAWO;
		if (kierunek == GORA)return DOL;
		if (kierunek == DOL)return GORA;
	}

	void przejscie(komorki_planszy_t **plansza, kierunek_t kierunek, int* punkty)
	{
		para_t poczatek = {};
		poczatek = ustaw_poczatek(poczatek, kierunek);//ustawienie poczatku
		kierunek_t odw = odwrotny(kierunek);//wyznaczenie kierunku odwrotnego
		int i, j, k;
		komorki_planszy_t* komorka_pom;//wskazik do przechodzenia po planszy
		if (kierunek == LEWO || kierunek == PRAWO)
		{
			for (i = poczatek.pierwszy; i < ILOSC_KOMOREK; i++)
			{
				for (j = poczatek.drugi; warunek(j, kierunek); j = iteracja(j, kierunek))
				{
					if (plansza[i][j].wartosc != BRAK_OBRAZKA)//jezeli mamy gdzies jakis obrazek
					{
						komorka_pom = &(plansza[i][j]);//wskazuje na ten obrazek
						k = iteracja(j, odw);//przechodze dalej w zaleznosci od kierunku
						while (warunek(k, odw))
						{
							//jezeli wartosci beda takie same i nie wskazujemy na zloczona komorke
							if (plansza[i][k].wartosc == komorka_pom->wartosc&&komorka_pom->zloczone==FALSZ)
							{
								plansza[i][k].wartosc = 2 * komorka_pom->wartosc;//wartosc w nowej komorce przepisuje na dwa razy wieksza
								komorka_pom->zloczone = PRAWDA;//okreslamy, ze jeden przed bedzie zaznaczony, aby nie poloczylo sie wiecej
								*punkty += plansza[i][k].wartosc;//doliczam punkty
								//zmieniam wspolrzedne nowe, czyli te na ktore pojdziemy podczas aniamcji
								plansza[i][j].wspolrzedne_nowe = plansza[i][k].wspolrzedne; 
								komorka_pom->wartosc = BRAK_OBRAZKA;// obrazek sie przesunal wiec w pierwotnej komorce nie ma juz obrazka
								komorka_pom->stale = PRAWDA;//zaznaczamy, ze pierwotna komorka jest stala
								komorka_pom = &(plansza[i][k]);//wskazujemy na nastepna komorke
								komorka_pom->zloczone = PRAWDA;//mowimy, ze jest zaznaczona
								komorka_pom->stale = FALSZ;//mimo ze wartosc jest to bedzie podlegala animacji
							}
							else if (plansza[i][k].wartosc == BRAK_OBRAZKA)//jezeli nastepna komorka nie ma obrazka
							{
								plansza[i][k].wartosc = komorka_pom->wartosc;//zapisujemy do niej wartosc
								plansza[i][j].wspolrzedne_nowe = plansza[i][k].wspolrzedne;//zapisujemy wspolrzedne do animacji
								komorka_pom->wartosc = BRAK_OBRAZKA;// przesuwamy obrazek
								komorka_pom->stale = PRAWDA;//pierwotna komorka bedzie stala
								komorka_pom = &(plansza[i][k]);//wskazuje na nastepna komorke
								komorka_pom->stale = FALSZ;//bedzie tam animacja
							}
							else break;//jezeli bedzie inna wartosc to dalej nie pojdziemy
							k = iteracja(k, odw);
						}
					}

				}
			}
		}
		else
		{
			//przehodzimy gora dol - owrocowane iteratory
			for (j = poczatek.drugi; j < ILOSC_KOMOREK; j++)
			{
				for (i = poczatek.pierwszy; warunek(i, kierunek); i = iteracja(i, kierunek))
				{
					if (plansza[i][j].wartosc != BRAK_OBRAZKA)//jezeli jest obrazek
					{
						komorka_pom = &(plansza[i][j]);//wskazujemy na dany obrazek
						k = iteracja(i, odw);//przechodzimy dalej
						while (warunek(k, odw))
						{
							//jezeli wartosc jest rowna naszej i nie jestesmy zloczonym kafelkiem
							if (plansza[k][j].wartosc == komorka_pom->wartosc&&komorka_pom->zloczone == FALSZ)
							{
								plansza[k][j].wartosc = 2 * komorka_pom->wartosc;//podwajamy nasza wartosc
								komorka_pom->zloczone = PRAWDA;//okreslamy, ze jeden przed bedzie zaznaczony, aby nie poloczylo sie wiecej
								*punkty += plansza[k][j].wartosc;//zwiekszamy punkty
								plansza[i][j].wspolrzedne_nowe = plansza[k][j].wspolrzedne;//zapisujemy wsppolrzedne do animacji
								komorka_pom->stale = PRAWDA;//na danej komorce nie bedzie aniamcji
								komorka_pom->wartosc = BRAK_OBRAZKA;//przesuwamy obrazek
								komorka_pom = &(plansza[k][j]);//wskazujemy na nastepna komorke planszy
								komorka_pom->zloczone = PRAWDA;//jest ona zloczona
								komorka_pom->stale = FALSZ;//nie jest stala przejdzie przez nia animacja
							}
							else if (plansza[k][j].wartosc == BRAK_OBRAZKA)//jezeli nie ma obrazka
							{
								plansza[k][j].wartosc = komorka_pom->wartosc;//przepisujemy tam wartosc
								plansza[i][j].wspolrzedne_nowe = plansza[k][j].wspolrzedne;//okreslamy wspolrzedne do animacji
								komorka_pom->wartosc = BRAK_OBRAZKA;//przesuwamy obrazk
								komorka_pom->stale = PRAWDA;//na danej komorce nie bedzie animacji
								komorka_pom = &(plansza[k][j]);//wskazujemy na nastepna komorke
								komorka_pom->stale = FALSZ;//bedzie uczestniczyc w animacji
							}
							else break;//jezeli bedzie inna wartos to nie mozemy isc dalej
							k = iteracja(k, odw);
						}
					}

				}
			}
		}
	}

	void zrob_animacje(komorki_planszy_t *animacja, kierunek_t kierunek, int *ile_do_animacji, int *ile_wyzerowanych, glowna_t obraz, moj_bool_t *wyzerowane, komorki_planszy_t **plansza, komorka_t *numerki)
	{
		//animacja polega na wyswietlaniu wszystkich obrazkow co 10 px do momentu osiagniecia docelowej komorki przez wszyskie obrazki
		int i = 0,it;
		para_t indeksy;
		for (i = 0; i < (*ile_do_animacji); i++)
		{
			//jezeli animacja doszla do konca
			if (animacja[i].wspolrzedne.pierwszy == animacja[i].wspolrzedne_nowe.pierwszy &&animacja[i].wspolrzedne.drugi == animacja[i].wspolrzedne_nowe.drugi)
			{
				if (wyzerowane[i] == FALSZ)//jezeli nie zaznaczylismy, ze sie zakonczyla
				{
					(*ile_wyzerowanych)++;
					wyzerowane[i] = PRAWDA;//zaznaczamy, ze animacja sie zakonczyla
					//od teraz ta komorka jest stala, czyli bedzie wyswietlana w funckji wstaw_plansze
					indeksy = konwertowanie(animacja[i].wspolrzedne_nowe);
					plansza[indeksy.pierwszy][indeksy.drugi].stale = PRAWDA;
				}
			}
			else
			{
				//zwiekszam wspolrzedne w zaleznosci od kierunku
				if (kierunek == PRAWO)animacja[i].wspolrzedne.pierwszy+=10;
				if (kierunek == LEWO)animacja[i].wspolrzedne.pierwszy-=10;
				if (kierunek == GORA)animacja[i].wspolrzedne.drugi-=10;
				if (kierunek == DOL)animacja[i].wspolrzedne.drugi+=10;
				//wypisuje obrazek
				it = log2((double)(animacja[i].wartosc)) - 1;
				DrawSurface(obraz.ekran, numerki[it].obrazek, animacja[i].wspolrzedne.pierwszy, animacja[i].wspolrzedne.drugi);
			}
		}
	}

	void czy_animacja(komorki_planszy_t **plansza, komorka_t *numerki, komorki_planszy_t *animacja, int *ile_do_animacji)
	{
		int i, j, it, x, y;
		para_t nowa_komorka;
		int it_animacja = 0;
		for (i = 0; i < ILOSC_KOMOREK; i++)
		{
			for (j = 0; j < ILOSC_KOMOREK; j++)
			{
					//jezeli dana komorka ma podlegac animacji
					if (plansza[i][j].wspolrzedne_nowe.pierwszy != 0 && plansza[i][j].wspolrzedne_nowe.drugi != 0)
					{
						(*ile_do_animacji)++;//zwiekszamy ilosc do animacji
						//+ROZMIAR_KOMORKI/2 - aby okreslic wspolrzedne srodka 
						animacja[it_animacja].wspolrzedne.pierwszy = plansza[i][j].wspolrzedne.pierwszy + ROZMIAR_KOMORKI / 2;
						animacja[it_animacja].wspolrzedne.drugi = plansza[i][j].wspolrzedne.drugi + ROZMIAR_KOMORKI / 2;
						//konwertowanie na indeksy w tablicy plansza
						nowa_komorka = konwertowanie(plansza[i][j].wspolrzedne_nowe);
						//okreslenie wspolrzednych koncowych animacji
						x = plansza[nowa_komorka.pierwszy][nowa_komorka.drugi].wspolrzedne.pierwszy + ROZMIAR_KOMORKI / 2;
						y = plansza[nowa_komorka.pierwszy][nowa_komorka.drugi].wspolrzedne.drugi + ROZMIAR_KOMORKI / 2;
						animacja[it_animacja].wspolrzedne_nowe.pierwszy = x;
						animacja[it_animacja].wspolrzedne_nowe.drugi = y;
						plansza[i][j].wspolrzedne_nowe = {};//wyzerowanie wspolrzednych nowych
						//obliczenie indeksu w tablicy numerki
						it = log2(double(plansza[nowa_komorka.pierwszy][nowa_komorka.drugi].wartosc)) - 1;
						plansza[nowa_komorka.pierwszy][nowa_komorka.drugi].wartosc = numerki[it].wartosc;
						animacja[it_animacja].wartosc = numerki[it].wartosc;
						it_animacja++;
					}
			}
		}
	}

	void DrawString(SDL_Surface *ekran, int x, int y, const char *napis, SDL_Surface *charset)
	{
		int px, py, c;
		SDL_Rect s, d;
		s.w = 8;
		s.h = 8;
		d.w = 8;
		d.h = 8;
		while (*napis)
		{
			c = *napis & 255;
			px = (c % 16) * 8;
			py = (c / 16) * 8;
			s.x = px;
			s.y = py;
			d.x = x;
			d.y = y;
			SDL_BlitSurface(charset, &s, ekran, &d);
			x += 8;
			napis++;
		}
	}

	void DrawSurface(SDL_Surface *ekran, SDL_Surface *sprite, int x, int y) 
	{
		SDL_Rect dest;
		dest.x = x - sprite->w / 2;
		dest.y = y - sprite->h / 2;
		dest.w = sprite->w;
		dest.h = sprite->h;
		SDL_BlitSurface(sprite, NULL, ekran, &dest);
	}

	void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 kolor)
	{
		int bpp = surface->format->BytesPerPixel;
		Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
		*(Uint32 *)p = kolor;
	}

	void DrawLine(SDL_Surface *ekran, int x, int y, int l, int dx, int dy, Uint32 color) 
	{
		for (int i = 0; i < l; i++)
		{
			DrawPixel(ekran, x, y, color);
			x += dx;
			y += dy;
		}
	}

	void DrawRectangle(SDL_Surface *ekran, int x, int y, int l, int k,Uint32 kolor_ramki, Uint32 wypelnienie) 
	{
		int i;
		DrawLine(ekran, x, y, k, 0, 1, kolor_ramki);
		DrawLine(ekran, x + l - 1, y, k, 0, 1, kolor_ramki);
		DrawLine(ekran, x, y, l, 1, 0, kolor_ramki);
		DrawLine(ekran, x, y + k - 1, l, 1, 0, kolor_ramki);
		for (i = y + 1; i < y + k - 1; i++)
			DrawLine(ekran, x + 1, i, l - 2, 1, 0, wypelnienie);
	};


