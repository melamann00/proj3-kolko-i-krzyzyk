#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <array>
#include <vector>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <string>
using namespace std;
static const int ROZM_PLAN = 3;
static const int ROZM_KOM = 130;
static const int ROZM_SIAT = ROZM_PLAN * ROZM_KOM;
static const int WYS_UI = 185;
static const int SZER = 430;
static const int WYS = WYS_UI + ROZM_SIAT + 80;
static const char PUSTE = 0;
static const char GRACZ = 1;
static const char AI = 2;
using Plansza = array<array<char, ROZM_PLAN>, ROZM_PLAN>;
struct Kolor { Uint8 r, g, b, a; };
static const Kolor KOL_TLO = {255, 255,255,255};
static const Kolor KOL_LINIA = {0, 0, 0,255};
static const Kolor KOL_X = {200, 40,40,255};
static const Kolor KOL_O  = {30, 90, 200,255};
static const Kolor KOL_TLO_WYG_X= {255, 220, 220, 255};
static const Kolor KOL_TLO_WYG_O= {210, 230, 255, 255};
static const Kolor KOL_BTN= {59, 130, 246, 255};
static const Kolor KOL_BTN_NORM = {209, 213, 219, 255};
static const Kolor KOL_TLO_UI= {243, 244, 246, 255};
static const Kolor KOL_TEKST_CIEMNY={ 17, 17, 17, 255};
static const Kolor KOL_TEKST_SZARY={ 85, 85, 85, 255};
static const Kolor KOL_BIALY ={255, 255, 255, 255};

static char sprawdzWygrana(const Plansza& plansza) {
    for (int i = 0; i < ROZM_PLAN; i++) {
        if (plansza[i][0] && plansza[i][0] == plansza[i][1] && plansza[i][1] == plansza[i][2]){
            return plansza[i][0];
        }
        if (plansza[0][i] && plansza[0][i] == plansza[1][i] && plansza[1][i] == plansza[2][i]){
            return plansza[0][i];
        }
    }
    if (plansza[0][0] && plansza[0][0] == plansza[1][1] && plansza[1][1] == plansza[2][2])
        return plansza[0][0];
    if (plansza[0][2] && plansza[0][2] == plansza[1][1] && plansza[1][1] == plansza[2][0])
        return plansza[0][2];
    return PUSTE;
}
static bool czyPelna(const Plansza& plansza) {
    for (auto& wiersz : plansza){
        for (char pole : wiersz){
            if (pole == PUSTE){
                return false;
            }
        }
    }
    return true;
}

//Algorytm MinMax
static int minmax(Plansza& plansza, int glebokosc, bool maksymalizuje, int alfa, int beta) {
    char wynik = sprawdzWygrana(plansza);
    if (wynik == AI){
        return 10 - glebokosc;
    }
    if (wynik == GRACZ){
        return glebokosc - 10;
    }
    if (czyPelna(plansza)){
        return 0;
    }
    int najlepszy = maksymalizuje ? INT_MIN : INT_MAX;
    for (int r = 0; r < ROZM_PLAN; r++) {
        for (int c = 0; c < ROZM_PLAN; c++) {
            if (plansza[r][c] != PUSTE){
                continue;
            }
            plansza[r][c] = maksymalizuje ? AI : GRACZ;
            int ocena = minmax(plansza, glebokosc + 1, !maksymalizuje, alfa, beta);
            plansza[r][c] = PUSTE;
            if (maksymalizuje) {
                najlepszy = max(najlepszy, ocena);
                alfa = max(alfa, najlepszy);
            } else {
                najlepszy = min(najlepszy, ocena);
                beta = min(beta, najlepszy);
            }
            if (beta <= alfa){
                goto przytnij;
            }
        }
    }
    przytnij:
    return najlepszy;
}

static pair<int, int> wybierzRuchAI(Plansza plansza, double trudnosc){
    vector<pair<int, int>> wolnePola;
    for (int r = 0; r < ROZM_PLAN; r++){
        for (int c = 0; c < ROZM_PLAN; c++){
            if (plansza[r][c] == PUSTE){
                wolnePola.push_back({r, c});
            }
        }
    }

    double losowa = (double)rand() / RAND_MAX;
    if (losowa > trudnosc)
        return wolnePola[rand() % wolnePola.size()];
    int najlepszyWynik = INT_MIN;
    pair<int, int> najlepszyRuch = wolnePola[0];
    for (auto [r, c] : wolnePola) {
        plansza[r][c] = AI;
        int ocena = minmax(plansza, 0, false, INT_MIN, INT_MAX);
        plansza[r][c] = PUSTE;
        if (ocena > najlepszyWynik) {
            najlepszyWynik = ocena;
            najlepszyRuch = {r, c};
        }
    }
    return najlepszyRuch;
}

static void ustawKolor(SDL_Renderer* renderer, Kolor kolor) {
    SDL_SetRenderDrawColor(renderer, kolor.r, kolor.g, kolor.b, kolor.a);
}

static void rysujProstokatWypelniony(SDL_Renderer* renderer, int x, int y, int w, int h, Kolor kolor) {
    ustawKolor(renderer, kolor);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

static void rysujProstokatObramowanie(SDL_Renderer* renderer, int x, int y, int w, int h, Kolor kolor, int grubosc = 1) {
    ustawKolor(renderer, kolor);
    for (int i = 0; i < grubosc; i++) {
        SDL_Rect rect = {x + i, y + i, w - 2 * i, h - 2 * i};
        SDL_RenderDrawRect(renderer, &rect);
    }
}

static void rysujGrubaLinie(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int grubosc, Kolor kolor) {
    ustawKolor(renderer, kolor);
    bool pozioma = (y1 == y2);
    for (int offset = -grubosc / 2; offset <= grubosc / 2; offset++){
        int ox = pozioma ? 0 : offset;
        int oy = pozioma ? offset : 0;
        SDL_RenderDrawLine(renderer, x1 + ox, y1 + oy, x2 + ox, y2 + oy);
    }
}

static void rysujOkrag(SDL_Renderer* renderer, int cx, int cy, int promien, int grubosc, Kolor kolor) {
    ustawKolor(renderer, kolor);
    for (int t = -grubosc / 2; t <= grubosc / 2; ++t) {
        int r = promien + t;
        int x = r, y = 0, err = 0;
        while (x >= y) {
            SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            SDL_RenderDrawPoint(renderer, cx - x, cy + y);
            SDL_RenderDrawPoint(renderer, cx + x, cy - y);
            SDL_RenderDrawPoint(renderer, cx - x, cy - y);
            SDL_RenderDrawPoint(renderer, cx + y, cy + x);
            SDL_RenderDrawPoint(renderer, cx - y, cy + x);
            SDL_RenderDrawPoint(renderer, cx + y, cy - x);
            SDL_RenderDrawPoint(renderer, cx - y, cy - x);
            y++;
            err += 2 * y + 1;
            if (2 * err - 2 * x + 1 > 0) {
                x--;
                err += -2 * x + 1;
            }
        }
    }
}

static void renderujTekst(SDL_Renderer* renderer, TTF_Font* czcionka, const string& tekst,int rx, int ry, int rw, int rh, Kolor kolor) {
    SDL_Color sdlKolor = {kolor.r, kolor.g, kolor.b, kolor.a};
    SDL_Surface* powierzchnia = TTF_RenderUTF8_Blended(czcionka, tekst.c_str(), sdlKolor);
    if (!powierzchnia){
        return;
    }
    SDL_Texture* tekstura = SDL_CreateTextureFromSurface(renderer, powierzchnia);
    SDL_Rect cel = {
        rx + (rw - powierzchnia->w) / 2,
        ry + (rh - powierzchnia->h) / 2,
        powierzchnia->w,
        powierzchnia->h
    };
    SDL_RenderCopy(renderer, tekstura, nullptr, &cel);
    SDL_DestroyTexture(tekstura);
    SDL_FreeSurface(powierzchnia);
}

struct Przycisk {
    SDL_Rect obszar;
    string etykieta;
    bool aktywny = false;
    bool klik(int x, int y) const {
        return x >= obszar.x && x < obszar.x + obszar.w && y >= obszar.y && y < obszar.y + obszar.h;
    }
    void rysuj(SDL_Renderer* renderer, TTF_Font* czcionka) const {
        Kolor kolorTla  = aktywny ? KOL_BTN : KOL_BIALY;
        Kolor kolorRamki = aktywny ? KOL_BTN : KOL_BTN_NORM;
        Kolor kolorTekstu = aktywny ? KOL_BIALY : KOL_TEKST_SZARY;
        rysujProstokatWypelniony(renderer, obszar.x, obszar.y, obszar.w, obszar.h, kolorTla);
        rysujProstokatObramowanie(renderer, obszar.x, obszar.y, obszar.w, obszar.h, kolorRamki, aktywny ? 2 : 1);
        renderujTekst(renderer, czcionka, etykieta, obszar.x, obszar.y, obszar.w, obszar.h, kolorTekstu);
    }
};

struct StanGry {
    Plansza plansza{};
    char biezacyGracz = GRACZ;
    bool koniecGry = false;
    bool tryb_vsAI = true;
    double trudnoscAI = 0.85;
    char wynikKoncowy = PUSTE;
    array<pair<int, int>, 3> komorkiWygranej = {{{-1,-1},{-1,-1},{-1,-1}}};
    bool maPodswietlenie = false;
    bool aiCzeka= false;
    Uint32 aiCzasRuchu = 0;
    int wygraneGracza = 0, wygraneAI= 0, remisy= 0;
    void reset() {
        for (auto& wiersz : plansza){
            wiersz.fill(PUSTE);
        }
        biezacyGracz = GRACZ;
        koniecGry = false;
        maPodswietlenie = false;
        wynikKoncowy = PUSTE;
        aiCzeka= false;
    }
    void zapamietajWygrana() {
        char zwyciezca = sprawdzWygrana(plansza);
        if (!zwyciezca){
            return;
        }
        const int kierunki[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
        for (int r = 0; r < ROZM_PLAN; r++) {
            for (int c = 0; c < ROZM_PLAN; c++){
                if (plansza[r][c] != zwyciezca){
                    continue;
                }
                for (auto& kier : kierunki) {
                    array<pair<int,int>, 3> segment{};
                    bool ok = true;
                    for (int k = 0; k < ROZM_PLAN; ++k) {
                        int nr = r + kier[0] * k;
                        int nc = c + kier[1] * k;
                        bool poza = nr < 0 || nr >= ROZM_PLAN || nc < 0 || nc >= ROZM_PLAN;
                        if (poza || plansza[nr][nc] != zwyciezca){
                            ok = false;
                            break;
                        }
                        segment[k] = {nr, nc};
                    }
                    if (ok){
                        komorkiWygranej = segment;
                        maPodswietlenie = true;
                        return;
                    }
                }
            }
        }
    }

    bool wykonajRuch(int r, int c) {
        if (koniecGry || plansza[r][c] != PUSTE){
            return false;
        }

        plansza[r][c] = biezacyGracz;

        char zwyciezca = sprawdzWygrana(plansza);
        if (zwyciezca) {
            zapamietajWygrana();
            koniecGry = true;
            wynikKoncowy = zwyciezca;
            if (zwyciezca == GRACZ){
                wygraneGracza++;
            }else{
                wygraneAI++;
            }
            return true;
        }
        if (czyPelna(plansza)){
            koniecGry = true;
            wynikKoncowy = PUSTE;
            remisy++;
            return true;
        }
        biezacyGracz = (biezacyGracz == GRACZ) ? AI : GRACZ;
        return true;
    }
};

int main() {
    srand((unsigned)time(nullptr));
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    char sciezkaFontu[512] = "";
    FILE* fcmatch = popen("fc-match --format=%{file} sans 2>/dev/null", "r");
    if (fcmatch) {
        fgets(sciezkaFontu, sizeof(sciezkaFontu), fcmatch);
        pclose(fcmatch);
        for (char* p = sciezkaFontu; *p; ++p)
            if (*p == '\n' || *p == '\r'){
                *p = 0; break;
            }
    }
    const char* sciezkiFontow[] = {
        sciezkaFontu,
        "/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
        "/usr/share/fonts/open-sans/OpenSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        nullptr
    };
    TTF_Font* czcionkaDuza = nullptr;
    TTF_Font* czcionkaNorm = nullptr;
    TTF_Font* czcionkaMala = nullptr;
    for (int i = 0; sciezkiFontow[i] && (!czcionkaDuza || !czcionkaNorm || !czcionkaMala); i++){
        if (!czcionkaDuza){
            czcionkaDuza = TTF_OpenFont(sciezkiFontow[i], 21);
        }
        if (!czcionkaNorm){
            czcionkaNorm = TTF_OpenFont(sciezkiFontow[i], 15);
        }
        if (!czcionkaMala){
            czcionkaMala = TTF_OpenFont(sciezkiFontow[i], 12);
        }
    }
    if (!czcionkaDuza) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Blad", "Brak czcionki systemowej!", nullptr);
        return 1;
    }
    SDL_Window* okno = SDL_CreateWindow("Kolko i Krzyzyk", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,SZER, WYS, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(okno, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    StanGry stan;
    const int MARGINES = 20;
    Przycisk btnGraczVsAI = {{MARGINES, 14, 185, 32}, "Gracz vs AI", true};
    Przycisk btnGraczVsGracz= {{MARGINES + 193, 14, 197, 32}, "Gracz vs Gracz", false};
    const char* nazwyTrudnosci[] = {"Latwy", "Sredni", "Trudny", "Niemozliwy"};
    const double wartosciTrudnosci[] = {0.20, 0.55, 0.85, 1.00};

    array<Przycisk, 4> przyciski_trudnosci;
    int szerokoscPrzycisku = (SZER - 2 * MARGINES - 18) / 4;
    for (int i = 0; i < 4; i++){
        przyciski_trudnosci[i] = {
            {MARGINES + i * (szerokoscPrzycisku + 6), 60, szerokoscPrzycisku, 28},
            nazwyTrudnosci[i],
        };
    }
    const int PLANSZA_X = (SZER - ROZM_SIAT) / 2;
    const int PLANSZA_Y = WYS_UI;
    Przycisk btnNowaRunda = {
        {SZER / 2 - 80, PLANSZA_Y + ROZM_SIAT + 12, 160, 34},
        "Nowa runda",
        false
    };
    bool dzialaj = true;
    while (dzialaj) {
        if (stan.aiCzeka && SDL_GetTicks() >= stan.aiCzasRuchu) {
            stan.aiCzeka = false;
            auto [r, c] = wybierzRuchAI(stan.plansza, stan.trudnoscAI);
            stan.wykonajRuch(r, c);
        }
        SDL_Event zdarzenie;
        while (SDL_PollEvent(&zdarzenie)) {
            if (zdarzenie.type == SDL_QUIT) {
                dzialaj = false;
                break;
            }
            if (zdarzenie.type == SDL_MOUSEBUTTONDOWN && zdarzenie.button.button == SDL_BUTTON_LEFT) {
                int klikX = zdarzenie.button.x;
                int klikY = zdarzenie.button.y;
                if (btnGraczVsAI.klik(klikX, klikY) && !stan.tryb_vsAI) {
                    stan.tryb_vsAI = true;
                    stan.wygraneGracza = stan.wygraneAI = stan.remisy = 0;
                    btnGraczVsAI.aktywny = true;
                    btnGraczVsGracz.aktywny = false;
                    stan.reset();
                }
                if (btnGraczVsGracz.klik(klikX, klikY) && stan.tryb_vsAI) {
                    stan.tryb_vsAI = false;
                    stan.wygraneGracza = stan.wygraneAI = stan.remisy = 0;
                    btnGraczVsAI.aktywny = false;
                    btnGraczVsGracz.aktywny = true;
                    stan.reset();
                }
                for (int i = 0; i < 4; i++) {
                    if (przyciski_trudnosci[i].klik(klikX, klikY)){
                        stan.trudnoscAI = wartosciTrudnosci[i];
                        for (int j = 0; j < 4; j++){
                            przyciski_trudnosci[j].aktywny = (j == i);
                        }
                    }
                }
                if (btnNowaRunda.klik(klikX, klikY))
                    stan.reset();
                bool graczMaRuch = !stan.tryb_vsAI || stan.biezacyGracz == GRACZ;
                if (!stan.koniecGry && !stan.aiCzeka && graczMaRuch) {
                    int px = klikX - PLANSZA_X;
                    int py = klikY - PLANSZA_Y;
                    bool wObszarzePlanszy = px >= 0 && px < ROZM_SIAT && py >= 0 && py < ROZM_SIAT;
                    if (wObszarzePlanszy) {
                        int kolumna = px / ROZM_KOM;
                        int wiersz  = py / ROZM_KOM;
                        bool ruchWykonany = stan.wykonajRuch(wiersz, kolumna);
                        if (ruchWykonany && !stan.koniecGry && stan.tryb_vsAI && stan.biezacyGracz == AI) {
                            stan.aiCzeka= true;
                            stan.aiCzasRuchu = SDL_GetTicks() + 300;  // 300ms opóźnienia
                        }
                    }
                }
            }
        }

        ustawKolor(renderer, KOL_TLO);
        SDL_RenderClear(renderer);
        btnGraczVsAI.rysuj(renderer, czcionkaNorm);
        btnGraczVsGracz.rysuj(renderer, czcionkaNorm);
        if (stan.tryb_vsAI){
            renderujTekst(renderer, czcionkaMala, "Trudnosc AI:", MARGINES, 43, 120, 16, KOL_TEKST_SZARY);
            for (auto& btn : przyciski_trudnosci){
                btn.rysuj(renderer, czcionkaMala);
            }
        }
        {
            string etykietaO = stan.tryb_vsAI ? "AI (O)" : "Gracz O";
            string wyniki = "Gracz X: " + to_string(stan.wygraneGracza)+"   Remisy: " + to_string(stan.remisy)+"   " + etykietaO + ": " + to_string(stan.wygraneAI);
            rysujProstokatWypelniony(renderer, MARGINES, 98, SZER - 2 * MARGINES, 28, KOL_TLO_UI);
            renderujTekst(renderer, czcionkaNorm, wyniki, MARGINES, 98, SZER - 2 * MARGINES, 28, KOL_TEKST_CIEMNY);
        }
        {
            Kolor   kolorStatusu = KOL_TLO_UI;
            string komunikat;
            if (stan.koniecGry) {
                if (stan.wynikKoncowy == GRACZ) {
                    komunikat=stan.tryb_vsAI ? "Wygrales!" : "Gracz X wygrywa!";
                    kolorStatusu = KOL_TLO_WYG_X;
                } else if (stan.wynikKoncowy == AI) {
                    komunikat    = stan.tryb_vsAI ? "AI wygrywa!" : "Gracz O wygrywa!";
                    kolorStatusu = KOL_TLO_WYG_O;
                } else {
                    komunikat = "Remis!";
                }
            } else if (stan.aiCzeka) {
                komunikat = "AI mysli...";
            } else {
                bool aiNaRuchu = stan.tryb_vsAI && stan.biezacyGracz == AI;
                if (aiNaRuchu)
                    komunikat = "Ruch: AI...";
                else
                    komunikat = string("Ruch: Gracz ") + (stan.biezacyGracz == GRACZ ? "X" : "O");
            }
            rysujProstokatWypelniony(renderer, MARGINES, 132, SZER - 2 * MARGINES, 30, kolorStatusu);
            renderujTekst(renderer, czcionkaDuza, komunikat, MARGINES, 132, SZER - 2 * MARGINES, 30, KOL_TEKST_CIEMNY);
        }
        rysujProstokatWypelniony(renderer, PLANSZA_X, PLANSZA_Y, ROZM_SIAT, ROZM_SIAT, KOL_TLO);
        if (stan.maPodswietlenie) {
            char zwyciezca = sprawdzWygrana(stan.plansza);
            Kolor kolorPodswietlenia = (zwyciezca == GRACZ) ? KOL_TLO_WYG_X : KOL_TLO_WYG_O;
            for (auto [r, c] : stan.komorkiWygranej) {
                rysujProstokatWypelniony(renderer,
                    PLANSZA_X + c * ROZM_KOM + 2,
                    PLANSZA_Y + r * ROZM_KOM + 2,
                    ROZM_KOM - 4,
                    ROZM_KOM - 4,
                    kolorPodswietlenia);
            }
        }
        for (int i = 1; i < ROZM_PLAN; i++) {
            rysujGrubaLinie(renderer,
                PLANSZA_X + i * ROZM_KOM, PLANSZA_Y + 6,
                PLANSZA_X + i * ROZM_KOM, PLANSZA_Y + ROZM_SIAT - 6, 4, KOL_LINIA);
            rysujGrubaLinie(renderer,
                PLANSZA_X + 6, PLANSZA_Y + i * ROZM_KOM,
                PLANSZA_X + ROZM_SIAT - 6,  PLANSZA_Y + i * ROZM_KOM, 4, KOL_LINIA);
        }
        for (int r = 0; r < ROZM_PLAN; r++){
            for (int c = 0; c < ROZM_PLAN; c++){
                int srodekX = PLANSZA_X + c * ROZM_KOM + ROZM_KOM / 2;
                int srodekY = PLANSZA_Y + r * ROZM_KOM + ROZM_KOM / 2;
                int wcięcie = 26;
                if (stan.plansza[r][c] == GRACZ){
                    rysujGrubaLinie(renderer,
                        srodekX - ROZM_KOM/2 + wcięcie,
                        srodekY - ROZM_KOM/2 + wcięcie,
                        srodekX + ROZM_KOM/2 - wcięcie,
                        srodekY + ROZM_KOM/2 - wcięcie,
                        7, KOL_X);
                    rysujGrubaLinie(renderer,
                        srodekX + ROZM_KOM/2 - wcięcie,
                        srodekY - ROZM_KOM/2 + wcięcie,
                        srodekX - ROZM_KOM/2 + wcięcie,
                        srodekY + ROZM_KOM/2 - wcięcie,
                        7, KOL_X);
                }else if (stan.plansza[r][c] == AI){
                    rysujOkrag(renderer, srodekX, srodekY, ROZM_KOM/2 - wcięcie, 7, KOL_O);
                }
            }
        }
        btnNowaRunda.rysuj(renderer, czcionkaNorm);
        SDL_RenderPresent(renderer);
    }
    TTF_CloseFont(czcionkaDuza);
    TTF_CloseFont(czcionkaNorm);
    TTF_CloseFont(czcionkaMala);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(okno);
    SDL_Quit();
    return 0;
}
