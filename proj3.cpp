#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <string>
#include <array>
#include <algorithm>
using namespace std;

static const int WYS_UI   = 220;
static const int SZER      = 480;
static const int KOMÓRKA_MAX = 120;
static const int KOMÓRKA_MIN = 40;
static const char PUSTE = 0;
static const char GRACZ = 1;
static const char AI    = 2;
using Plansza = vector<vector<char>>;
struct Kolor { Uint8 r, g, b, a; };
static const Kolor KOL_TLO= {255,255,255,255};
static const Kolor KOL_LINIA= {0,0,0,255};
static const Kolor KOL_X= {200,40,40,255};
static const Kolor KOL_O= {30,90,200,255};
static const Kolor KOL_TLO_WYG_X = {255,220,220,255};
static const Kolor KOL_TLO_WYG_O = {210,230,255,255};
static const Kolor KOL_BTN = {59,130,246,255};
static const Kolor KOL_BTN_NORM= {209,213,219,255};
static const Kolor KOL_TLO_UI= {243,244,246,255};
static const Kolor KOL_TEKST_CIEMNY = {17,17,17,255};
static const Kolor KOL_TEKST_SZARY = {85,85,85,255};
static const Kolor KOL_BIALY= {255,255,255,255};
static const Kolor KOL_INPUT_AKTYW = {59,130,246,255};
static const Kolor KOL_INPUT_NORM = {180,180,180,255};

static char sprawdzWygrana(const Plansza& p, int N, int K) {
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            char kto = p[r][c];
            if (!kto){
                continue;
            }
            if (c + K <= N) {
                bool ok = true;
                for (int i = 1; i < K; i++){
                    if (p[r][c+i] != kto){
                        ok=false;
                        break;
                    }
                }
                if (ok){
                    return kto;
                }
            }
            if (r + K <= N) {
                bool ok = true;
                for (int i = 1; i < K; i++){
                    if (p[r+i][c] != kto){
                        ok=false;
                        break;
                    }
                }
                if (ok){
                    return kto;
                }
            }
            if (r + K <= N && c + K <= N) {
                bool ok = true;
                for (int i = 1; i < K; i++){
                    if (p[r+i][c+i] != kto){
                        ok=false;
                        break;
                    }
                }
                if (ok){
                    return kto;
                }
            }
            if (r + K <= N && c - K + 1 >= 0) {
                bool ok = true;
                for (int i = 1; i < K; i++){
                    if (p[r+i][c-i] != kto){
                        ok=false;
                        break;
                    }
                }
                if (ok){
                    return kto;
                }
            }
        }
    }
    return PUSTE;
}

static bool czyPelna(const Plansza& p, int N) {
    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++){
            if (p[r][c] == PUSTE){
                return false;
            }
        }
    return true;
}

static int ocenPozycje(const Plansza& p, int N, int K) {
    int wynik = 0;
    auto ocenOkno = [&](vector<char> okno) {
        int ileAI = 0, ileGracz = 0;
        for (char x : okno) {
            if (x == AI){
                ileAI++;
            }
            if (x == GRACZ){
                ileGracz++;
            }
        }
        int val = 1;
        for (int i = 1; i < (ileAI > 0 ? ileAI : ileGracz); i++){
            val *= 10;
        }
        if (ileAI > 0){
            wynik += val;
        }
        if (ileGracz > 0){
            wynik -= val;
        }
    };

    for (int r = 0; r < N; r++) {
        for (int c = 0; c <= N - K; c++) {
            vector<char> okno(K);
            for (int i = 0; i < K; i++){
                okno[i] = p[r][c+i];
            }
            ocenOkno(okno);
        }
    }
    for (int c = 0; c < N; c++) {
        for (int r = 0; r <= N - K; r++) {
            vector<char> okno(K);
            for (int i = 0; i < K; i++){
                okno[i] = p[r+i][c];
            }
            ocenOkno(okno);
        }
    }
    for (int r = 0; r <= N - K; r++) {
        for (int c = 0; c <= N - K; c++) {
            vector<char> okno(K);
            for (int i = 0; i < K; i++){
                okno[i] = p[r+i][c+i];
            }
            ocenOkno(okno);
        }
    }
    for (int r = 0; r <= N - K; r++) {
        for (int c = K-1; c < N; c++) {
            vector<char> okno(K);
            for (int i = 0; i < K; i++){
                okno[i] = p[r+i][c-i];
            }
            ocenOkno(okno);
        }
    }
    return wynik;
}

static int minmax(Plansza& p, int N, int K, int gleb, int maxGleb, bool maks, int alfa, int beta) {
    char wynik = sprawdzWygrana(p, N, K);
    if (wynik == AI){
        return 10000 - gleb;
    }
    if (wynik == GRACZ){
        return gleb - 10000;
    }
    if (czyPelna(p, N)){
        return 0;
    }
    if (gleb >= maxGleb){
        return ocenPozycje(p, N, K);
    }

    int najlepszy = maks ? INT_MIN : INT_MAX;
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (p[r][c] != PUSTE){
                continue;
            }
            p[r][c] = maks ? AI : GRACZ;
            int ocena = minmax(p, N, K, gleb+1, maxGleb, !maks, alfa, beta);
            p[r][c] = PUSTE;
            if (maks) {
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

static int limitGlebokosci(int N) {
    if (N <= 3){
        return 9;
    }
    if (N <= 4){
        return 5;
    }
    if (N <= 5){
        return 4;
    }
    return 3;
}

static pair<int,int> wybierzRuchAI(Plansza plansza, int N, int K, double trudnosc) {
    vector<pair<int,int>> wolne;
    for (int r = 0; r < N; r++){
        for (int c = 0; c < N; c++){
            if (plansza[r][c] == PUSTE){
                wolne.push_back({r,c});
            }
        }
    }

    if ((double)rand()/RAND_MAX > trudnosc)
        return wolne[rand() % wolne.size()];

    int maxGleb = limitGlebokosci(N);
    int najlepszyWynik = INT_MIN;
    pair<int,int> najlepszyRuch = wolne[0];
    for (auto [r,c] : wolne) {
        plansza[r][c] = AI;
        int ocena = minmax(plansza, N, K, 0, maxGleb, false, INT_MIN, INT_MAX);
        plansza[r][c] = PUSTE;
        if (ocena > najlepszyWynik) {
            najlepszyWynik = ocena;
            najlepszyRuch  = {r,c};
        }
    }
    return najlepszyRuch;
}

static void ustawKolor(SDL_Renderer* rd, Kolor k) {
    SDL_SetRenderDrawColor(rd, k.r, k.g, k.b, k.a);
}
static void rysujFill(SDL_Renderer* rd, int x,int y,int w,int h, Kolor k) {
    ustawKolor(rd,k);
    SDL_Rect r={x,y,w,h};
    SDL_RenderFillRect(rd,&r);
}
static void rysujRamke(SDL_Renderer* rd, int x,int y,int w,int h, Kolor k, int gr=1) {
    ustawKolor(rd,k);
    for (int i=0;i<gr;i++){
        SDL_Rect r={x+i,y+i,w-2*i,h-2*i};
        SDL_RenderDrawRect(rd,&r);
    }
}
static void rysujLinie(SDL_Renderer* rd, int x1,int y1,int x2,int y2,int gr, Kolor k) {
    ustawKolor(rd,k);
    bool poz=(y1==y2);
    for (int o=-gr/2;o<=gr/2;o++){
        int ox=poz?0:o, oy=poz?o:0;
        SDL_RenderDrawLine(rd,x1+ox,y1+oy,x2+ox,y2+oy);
    }
}
static void rysujOkrag(SDL_Renderer* rd, int cx,int cy,int pr,int gr, Kolor k) {
    ustawKolor(rd,k);
    for (int t=-gr/2;t<=gr/2;t++){
        int R=pr+t, x=R, y=0, err=0;
        while(x>=y){
            SDL_RenderDrawPoint(rd,cx+x,cy+y); SDL_RenderDrawPoint(rd,cx-x,cy+y);
            SDL_RenderDrawPoint(rd,cx+x,cy-y); SDL_RenderDrawPoint(rd,cx-x,cy-y);
            SDL_RenderDrawPoint(rd,cx+y,cy+x); SDL_RenderDrawPoint(rd,cx-y,cy+x);
            SDL_RenderDrawPoint(rd,cx+y,cy-x); SDL_RenderDrawPoint(rd,cx-y,cy-x);
            y++;
            err+=2*y+1;
            if(2*err-2*x+1>0){
                x--;
                err+=-2*x+1;
            }
        }
    }
}
static void renderujTekst(SDL_Renderer* rd, TTF_Font* font, const string& txt, int rx,int ry,int rw,int rh, Kolor k) {
    SDL_Color sc={k.r,k.g,k.b,k.a};
    SDL_Surface* s=TTF_RenderUTF8_Blended(font,txt.c_str(),sc);
    if(!s){
        return;
    }
    SDL_Texture* t=SDL_CreateTextureFromSurface(rd,s);
    SDL_Rect cel={rx+(rw-s->w)/2, ry+(rh-s->h)/2, s->w, s->h};
    SDL_RenderCopy(rd,t,nullptr,&cel);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
}

struct Przycisk {
    SDL_Rect obszar;
    string etykieta;
    bool aktywny=false;
    bool klik(int x,int y) const {
        return x>=obszar.x && x<obszar.x+obszar.w && y>=obszar.y && y<obszar.y+obszar.h;
    }
    void rysuj(SDL_Renderer* rd, TTF_Font* f) const {
        Kolor kt = aktywny ? KOL_BTN : KOL_BIALY;
        Kolor kr = aktywny ? KOL_BTN : KOL_BTN_NORM;
        Kolor kte= aktywny ? KOL_BIALY: KOL_TEKST_SZARY;
        rysujFill(rd,obszar.x,obszar.y,obszar.w,obszar.h,kt);
        rysujRamke(rd,obszar.x,obszar.y,obszar.w,obszar.h,kr,aktywny?2:1);
        renderujTekst(rd,f,etykieta,obszar.x,obszar.y,obszar.w,obszar.h,kte);
    }
};

struct PoleTekstowe {
    SDL_Rect obszar;
    string bufor;
    bool focus=false;
    int maxZnakow=2;
    bool klik(int x,int y){
        bool trafiony = x>=obszar.x && x<obszar.x+obszar.w && y>=obszar.y && y<obszar.y+obszar.h;
        focus=trafiony;
        return trafiony;
    }
    void wprowadzZnak(SDL_Keycode key){
        if(!focus){
            return;
        }
        if(key==SDLK_BACKSPACE){
            if(!bufor.empty()) bufor.pop_back();
            return;
        }
        if(key>=SDLK_1 && key<=SDLK_9 && (int)bufor.size()<maxZnakow){
            bufor += (char)('0'+(key-SDLK_0));
        }
        if(key>=SDLK_KP_1 && key<=SDLK_KP_9 && (int)bufor.size()<maxZnakow){
            bufor += (char)('0'+(key-SDLK_KP_0));
        }
    }
    int wartosc(int domyslna) const {
        if(bufor.empty()){
            return domyslna;
        }
        return stoi(bufor);
    }
    void rysuj(SDL_Renderer* rd, TTF_Font* f, const string& etykieta) const {
        renderujTekst(rd,f,etykieta,obszar.x-70,obszar.y,68,obszar.h,KOL_TEKST_SZARY);
        rysujFill(rd,obszar.x,obszar.y,obszar.w,obszar.h,KOL_BIALY);
        Kolor ramka = focus ? KOL_INPUT_AKTYW : KOL_INPUT_NORM;
        rysujRamke(rd,obszar.x,obszar.y,obszar.w,obszar.h,ramka,focus?2:1);
        string wyswietl = bufor.empty() ? "-" : bufor;
        renderujTekst(rd,f,wyswietl,obszar.x,obszar.y,obszar.w,obszar.h,KOL_TEKST_CIEMNY);
    }
};

struct StanGry {
    int N=3, K=3;
    Plansza plansza;
    char biezacyGracz=GRACZ;
    bool koniecGry=false;
    bool tryb_vsAI=true;
    double trudnoscAI=0.85;
    char wynikKoncowy=PUSTE;
    vector<pair<int,int>> komorkiWygranej;
    bool maPodswietlenie=false;
    bool aiCzeka=false;
    Uint32 aiCzasRuchu=0;
    int wygraneGracza=0, wygraneAI=0, remisy=0;
    void inicjujPlansze() {
        plansza.assign(N, vector<char>(N, PUSTE));
    }
    void reset() {
        inicjujPlansze();
        biezacyGracz=GRACZ;
        koniecGry=false;
        maPodswietlenie=false;
        wynikKoncowy=PUSTE;
        komorkiWygranej.clear();
        aiCzeka=false;
    }
    void zapamietajWygrana() {
        char zw = sprawdzWygrana(plansza, N, K);
        if (!zw) return;
        int dirs[4][2]={{0,1},{1,0},{1,1},{1,-1}};
        for (int r=0;r<N;r++){
            for (int c=0;c<N;c++) {
                if (plansza[r][c]!=zw){
                    continue;
                }
            for (auto& d:dirs) {
                vector<pair<int,int>> seg;
                bool ok=true;
                for (int k=0;k<K;k++){
                    int nr=r+d[0]*k, nc=c+d[1]*k;
                    if (nr<0||nr>=N||nc<0||nc>=N||plansza[nr][nc]!=zw){
                        ok=false;
                        break;
                    }
                    seg.push_back({nr,nc});
                }
                if (ok){
                    komorkiWygranej=seg;
                    maPodswietlenie=true;
                    return;
                }
            }
        }
    }
}

    bool wykonajRuch(int r,int c) {
        if (koniecGry||plansza[r][c]!=PUSTE){
            return false;
        }
        plansza[r][c]=biezacyGracz;
        char zw=sprawdzWygrana(plansza,N,K);
        if (zw){
            zapamietajWygrana();
            koniecGry=true;
            wynikKoncowy=zw;
            if (zw==GRACZ){
                wygraneGracza++;
            }else{
                wygraneAI++;
            }
            return true;
        }
        if (czyPelna(plansza,N)){
            koniecGry=true;
            wynikKoncowy=PUSTE;
            remisy++;
            return true;
        }
        biezacyGracz=(biezacyGracz==GRACZ)?AI:GRACZ;
        return true;
    }
};

int main() {
    srand((unsigned)time(nullptr));
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    char sciezkaFontu[512]="";
    FILE* fc=popen("fc-match --format=%{file} sans 2>/dev/null","r");
    if(fc){
        fgets(sciezkaFontu,sizeof(sciezkaFontu),fc);
        pclose(fc);
        for(char* p=sciezkaFontu;*p;++p){
          if(*p=='\n'||*p=='\r'){
              *p=0;
              break;
          }
        }
    }
    const char* fonty[]={
        sciezkaFontu,
        "/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
        "/usr/share/fonts/open-sans/OpenSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        nullptr
    };
    TTF_Font *fDuza=nullptr, *fNorm=nullptr, *fMala=nullptr;
    for(int i=0;fonty[i]&&(!fDuza||!fNorm||!fMala);i++){
    if(!fDuza){
        fDuza=TTF_OpenFont(fonty[i],21);
    }
        if(!fNorm){
            fNorm=TTF_OpenFont(fonty[i],15);
        }
        if(!fMala){
            fMala=TTF_OpenFont(fonty[i],12);
        }
    }
    if(!fDuza){
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Blad","Brak czcionki!",nullptr);
        return 1;
    }
    const int WYS_PLANSZA_MAX = 480;
    const int WYS_OKNA = WYS_UI + WYS_PLANSZA_MAX + 60;
    SDL_Window* okno=SDL_CreateWindow("Kolko i Krzyzyk", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, SZER, WYS_OKNA, 0);
    SDL_Renderer* rd=SDL_CreateRenderer(okno,-1,SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(rd,SDL_BLENDMODE_BLEND);
    StanGry stan;
    stan.inicjujPlansze();
    const int MARG=20;
    Przycisk btnVsAI ={{MARG,14,185,32},"Gracz vs AI",true};
    Przycisk btnVsGracz ={{MARG+193,14,197,32},"Gracz vs Gracz",false};

    const char* nazwyTr[]={"Latwy","Sredni","Trudny","Niemozliwy"};
    const double wartTr[]={0.20,0.55, 0.85, 1.00};
    array<Przycisk,4> btnTr;
    int szTr=(SZER-2*MARG-18)/4;
    for(int i=0;i<4;i++){
        btnTr[i]={{MARG+i*(szTr+6),60,szTr,26},nazwyTr[i]};
    }
    btnTr[1].aktywny=true;
    const int INPUT_Y = 98;
    const int INPUT_W = 42;
    const int INPUT_H = 24;

    PoleTekstowe inpN, inpK;
    inpN.obszar = {MARG+80, INPUT_Y, INPUT_W, INPUT_H};
    inpN.bufor  = "3";
    inpK.obszar = {MARG+80+INPUT_W+90, INPUT_Y, INPUT_W, INPUT_H};
    inpK.bufor  = "3";
    Przycisk btnZastosuj={{MARG+80+2*(INPUT_W)+90+10+15, INPUT_Y-1, 80, INPUT_H+2},"Zastosuj"};
    Przycisk btnReset={{SZER/2-70, WYS_UI+WYS_PLANSZA_MAX+10,140,32},"Nowa runda"};
    bool dzialaj=true;
    while(dzialaj) {
        int rozmKom = min(KOMÓRKA_MAX, WYS_PLANSZA_MAX / stan.N);
        rozmKom = max(rozmKom, KOMÓRKA_MIN);
        int rozmSiat = stan.N * rozmKom;
        int plX = (SZER - rozmSiat)/2;
        int plY = WYS_UI + (WYS_PLANSZA_MAX - rozmSiat)/2;
        if(stan.aiCzeka && SDL_GetTicks()>=stan.aiCzasRuchu){
            stan.aiCzeka=false;
            auto [r,c]=wybierzRuchAI(stan.plansza, stan.N, stan.K, stan.trudnoscAI);
            stan.wykonajRuch(r,c);
        }
        SDL_Event ev;
        while(SDL_PollEvent(&ev)){
            if(ev.type==SDL_QUIT){
                dzialaj=false;
                break;
            }
            if(ev.type==SDL_KEYDOWN){
                inpN.wprowadzZnak(ev.key.keysym.sym);
                inpK.wprowadzZnak(ev.key.keysym.sym);
            }
            if(ev.type==SDL_MOUSEBUTTONDOWN && ev.button.button==SDL_BUTTON_LEFT){
                int mx=ev.button.x, my=ev.button.y;
                inpN.klik(mx,my);
                inpK.klik(mx,my);
                if(btnVsAI.klik(mx,my)&&!stan.tryb_vsAI){
                    stan.tryb_vsAI=true;
                    stan.wygraneGracza=stan.wygraneAI=stan.remisy=0;
                    btnVsAI.aktywny=true;
                    btnVsGracz.aktywny=false;
                    stan.reset();
                }
                if(btnVsGracz.klik(mx,my)&&stan.tryb_vsAI){
                    stan.tryb_vsAI=false;
                    stan.wygraneGracza=stan.wygraneAI=stan.remisy=0;
                    btnVsAI.aktywny=false;
                    btnVsGracz.aktywny=true;
                    stan.reset();
                }
                for(int i=0;i<4;i++) if(btnTr[i].klik(mx,my)){
                    stan.trudnoscAI=wartTr[i];
                    for(int j=0;j<4;j++){
                        btnTr[j].aktywny=(j==i);
                    }
                }
                if(btnZastosuj.klik(mx,my)){
                    int noweN = inpN.wartosc(3);
                    int noweK = inpK.wartosc(3);
                    noweN = max(3,min(12,noweN));
                    noweK = max(3,min(noweN,noweK));
                    inpN.bufor = to_string(noweN);
                    inpK.bufor = to_string(noweK);
                    stan.N=noweN;
                    stan.K=noweK;
                    stan.wygraneGracza=stan.wygraneAI=stan.remisy=0;
                    stan.reset();
                }
                if(btnReset.klik(mx,my)){
                    stan.reset();
                }
                bool graczMaRuch=!stan.tryb_vsAI || stan.biezacyGracz==GRACZ;
                if(!stan.koniecGry&&!stan.aiCzeka&&graczMaRuch){
                    int px=mx-plX, py=my-plY;
                    if(px>=0&&px<rozmSiat&&py>=0&&py<rozmSiat){
                        int kol=px/rozmKom, wiersz=py/rozmKom;
                        bool ok=stan.wykonajRuch(wiersz,kol);
                        if(ok&&!stan.koniecGry&&stan.tryb_vsAI&&stan.biezacyGracz==AI){
                            stan.aiCzeka=true;
                            stan.aiCzasRuchu=SDL_GetTicks()+300;
                        }
                    }
                }
            }
        }
        ustawKolor(rd,KOL_TLO);
        SDL_RenderClear(rd);
        rysujFill(rd,0,0,SZER,WYS_UI,KOL_TLO_UI);
        btnVsAI.rysuj(rd,fNorm);
        btnVsGracz.rysuj(rd,fNorm);
        if(stan.tryb_vsAI){
            renderujTekst(rd,fMala,"Trudnosc:",MARG,44,70,16,KOL_TEKST_SZARY);
            for(auto& b:btnTr){
                b.rysuj(rd,fMala);
            }
        }
        inpN.rysuj(rd,fNorm,"N (plansza):");
        inpK.rysuj(rd,fNorm,"K (seria):");
        btnZastosuj.rysuj(rd,fMala);
        {
            string etO=stan.tryb_vsAI?"AI (O)":"Gracz O";
            string wyr="X:"+to_string(stan.wygraneGracza) +"  Remis:"+to_string(stan.remisy)+"  "+etO+":"+to_string(stan.wygraneAI);
            rysujFill(rd,MARG,128,SZER-2*MARG,22,KOL_TLO_UI);
            renderujTekst(rd,fNorm,wyr,MARG,128,SZER-2*MARG,22,KOL_TEKST_CIEMNY);
        }
        {
            Kolor kst=KOL_TLO_UI;
            string msg;
            if(stan.koniecGry){
                if(stan.wynikKoncowy==GRACZ){
                    msg=stan.tryb_vsAI?"Wygrales!":"Gracz X wygrywa!";
                    kst=KOL_TLO_WYG_X;
                } else if(stan.wynikKoncowy==AI){
                    msg=stan.tryb_vsAI?"AI wygrywa!":"Gracz O wygrywa!";
                    kst=KOL_TLO_WYG_O;
                } else msg="Remis!";
            } else if(stan.aiCzeka){
                msg="AI mysli...";
            } else {
                bool aiNaRuchu=stan.tryb_vsAI&&stan.biezacyGracz==AI;
                msg=aiNaRuchu?"Ruch: AI...":
                string("Ruch: Gracz ")+(stan.biezacyGracz==GRACZ?"X":"O");
            }
            rysujFill(rd,MARG,155,SZER-2*MARG,30,kst);
            renderujTekst(rd,fDuza,msg,MARG,155,SZER-2*MARG,30,KOL_TEKST_CIEMNY);
        }

        rysujFill(rd,plX,plY,rozmSiat,rozmSiat,KOL_TLO);
        if(stan.maPodswietlenie){
            char zw=sprawdzWygrana(stan.plansza,stan.N,stan.K);
            Kolor kp=(zw==GRACZ)?KOL_TLO_WYG_X:KOL_TLO_WYG_O;
            for(auto [r,c]:stan.komorkiWygranej)
                rysujFill(rd, plX+c*rozmKom+2, plY+r*rozmKom+2, rozmKom-4, rozmKom-4, kp);
        }

        int grLin = max(1, rozmKom/30);
        for(int i=1;i<stan.N;i++){
            rysujLinie(rd, plX+i*rozmKom, plY+4,plX+i*rozmKom, plY+rozmSiat-4, grLin, KOL_LINIA);
            rysujLinie(rd, plX+4, plY+i*rozmKom,plX+rozmSiat-4, plY+i*rozmKom, grLin, KOL_LINIA);
        }
        for(int r=0;r<stan.N;r++){
            for(int c=0;c<stan.N;c++){
            int sx=plX+c*rozmKom+rozmKom/2;
            int sy=plY+r*rozmKom+rozmKom/2;
            int wc=max(6, rozmKom/5);
            int grSym=max(2, rozmKom/20);
            if(stan.plansza[r][c]==GRACZ){
                rysujLinie(rd,sx-rozmKom/2+wc, sy-rozmKom/2+wc,sx+rozmKom/2-wc, sy+rozmKom/2-wc, grSym, KOL_X);
                rysujLinie(rd,sx+rozmKom/2-wc, sy-rozmKom/2+wc,sx-rozmKom/2+wc, sy+rozmKom/2-wc, grSym, KOL_X);
            } else if(stan.plansza[r][c]==AI){
                rysujOkrag(rd,sx,sy, rozmKom/2-wc, grSym, KOL_O);
            }
        }
        }

        btnReset.rysuj(rd,fNorm);
        SDL_RenderPresent(rd);
    }
    TTF_CloseFont(fDuza);
    TTF_CloseFont(fNorm);
    TTF_CloseFont(fMala);
    TTF_Quit();
    SDL_DestroyRenderer(rd); SDL_DestroyWindow(okno);
    SDL_Quit();
    return 0;
}
