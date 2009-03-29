/*
 * vadorz.c (1.0)
 * -vk
 *
 * a ncurses space-invaders game
 *
 * Arrow Keys or WASD to move your ship.
 * F or Space to fire bullets.
 * Z for MegaKill.
 * Q to exit.
 * 
 * To-do:
 * > Add fancy colors / graphics. [0%]
*/
 
#define AUP_ART     "<{$^$}>"           // must be 7 chars
#define UFO_ART     "(@#@)"             // must be 5 chars
#define CONSTANT    80000
 
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
 
typedef unsigned short int num;

num NUM_UFO;
uint LATENCY = CONSTANT-10000;

struct Posn {
    num x;
    num y;
};
 
struct Ufo {
    struct Posn pos;
    num alive;
    num bounce; // 0 = has not reached right wall
};
 
struct Shot {
    struct Posn pos;
    num alive;
    num isGoingUp;
};
 
struct shot_list {
    num max;
    num cur;
    struct Shot* dat;
};
 
num rows;
num cols;
 
struct Posn aup;
struct Ufo ufos[((CONSTANT/5000)*2)+10];
struct shot_list shots;
 
void quit(char* seq) {
    clear();
    curs_set(2);
    endwin();
    printf(seq);
    free(shots.dat);
    exit(0);
}
 
struct Shot mk_shot(struct Posn obj, num uphuh) {
    struct Shot datum;
    datum.pos.x = obj.x + 3;
    datum.pos.y = (uphuh != 0) ? obj.y-1 : obj.y+1; // vert pos
    datum.alive = 1;
    datum.isGoingUp = uphuh;
    return datum;
}
 
void add_shot(struct Shot datum) {
    if (shots.cur < shots.max) {
        shots.dat[shots.cur] = datum;
        shots.cur += 1;
    } else if (shots.cur == shots.max) {
        shots.max += shots.max / 2;
        shots.dat = realloc(shots.dat, shots.max * sizeof(struct Shot));
        
        if (shots.dat == NULL) {
            quit(":: shot_list is out of memory!\n");
        }
          
        add_shot(datum);
    } else {
        quit(":: shot_list overflow!\n");
    }
}
 
int in; // input
num i; // inner-itr
num x; // inner-itr
 
inline void sprite_draw(struct Posn obj, const char* art) {
    mvprintw(obj.y, obj.x, art);
}
 
void draw_all() {
    clear();
    
    sprite_draw(aup, AUP_ART);
    
    for (i=0; i < shots.cur; ++i) {
        if (shots.dat[i].alive == 0) {
            continue;
        }
        
        sprite_draw(shots.dat[i].pos, "^");
    }
    
    for (i=0; i < NUM_UFO; ++i) {
        if (ufos[i].alive == 0) {
            continue;
        }
        
        sprite_draw(ufos[i].pos, UFO_ART);
    }
    
    refresh();
}
 
void run_ufos() {
    for (i=0; i < NUM_UFO; ++i) {
        if (ufos[i].alive == 0) {
            continue;
        }
        
        if (ufos[i].pos.x + 5 == cols) { // + 5 to adjust for the ufo length
            ufos[i].bounce = 1;
            ufos[i].pos.y += 1;
        }
        
        if (ufos[i].pos.x == 1) {
            ufos[i].bounce = 0;
            ufos[i].pos.y += 1;
        }
 
        if (ufos[i].bounce == 0) {
            ufos[i].pos.x += 1;
        } else {
            ufos[i].pos.x -= 1;
        }
        
        if (rand() % 100 < 5) {
            add_shot(mk_shot(ufos[i].pos, 0));
        }
        
        if (ufos[i].pos.y == rows-1) {
            quit("You lost the game!\n");
        }
    }
}
 
void run_aup() {
    in = getch();
    
	if (in == KEY_LEFT || in == 'a' || in == 'A') {
        aup.x -= (aup.x == 0) ? 0 : 1;
    } else if (in == KEY_UP || in == 'w' || in == 'W') {
        aup.y -= (aup.y == 0) ? 0 : 1;
    } else if (in == KEY_RIGHT || in == 'd' || in == 'D') {
        aup.x += (aup.x == cols-7) ? 0 : 1;
    } else if (in == KEY_DOWN || in == 's' || in == 'S') {
        aup.y += (aup.y == rows-1) ? 0 : 1;
    } else if (in == ' ' || in == 'f' || in == 'F') {
        add_shot(mk_shot(aup, 1)); // register a shot going up
    } else if (in == 'z' || in == 'Z') {
        for (i=0; i < cols; ++i) {
            struct Shot t;
            t.alive = 1;
            t.isGoingUp = 1;
            t.pos.x = i;
            t.pos.y = aup.y-1;
            add_shot(t);
        }
    } else if (in == 'q' || in =='Q' || in == KEY_EXIT) {
        quit("User Exit.\n");
    }
	
	usleep(LATENCY);
 
    draw_all();
}
 
void run_shots() {
    for (i=0; i < shots.cur; ++i) {        
        in = shots.dat[i].pos.y;
        if (in < 0 || in > rows-1) {
            shots.dat[i].alive = 0;
        }
        
        if (shots.dat[i].isGoingUp != 0) {
            shots.dat[i].pos.y -= 1;
        } else {
            shots.dat[i].pos.y += 1;
        }
    }
}
 
inline int is_hit(struct Posn obj, struct Posn pos) { // ship : bullet
    return (obj.y == pos.y
            && pos.x <= obj.x+7
            && pos.x >= obj.x)
        ? 1 : 0;
}

void populate() {
    if (LATENCY < 5000) {
        quit("Wow. I can't believe you just won this game.\n");
    }
    
    if (shots.dat) {
        free(shots.dat);
    }
    
    shots.cur = 0;
    shots.max = 100;
    shots.dat = (struct Shot*) malloc(shots.max * sizeof(struct Shot));
    
    aup.x = (cols / 2) - 7;
    aup.y = rows-1;

    num itr;
    
    for (itr=0; itr < NUM_UFO; ++itr) {
        struct Ufo t;
        t.pos.x = (itr * 7) + 2;
        t.pos.y = 0;
        t.alive = 1;
        t.bounce = (t.pos.x + 5 >= cols) ? 1 : 0; // fix this
        ufos[itr] = t;
    }
}

num u; // count of live ufos
 
void update_state() {
    for (i=0; i < shots.cur; ++i) {        
        if (shots.dat[i].alive == 0) { // skip out-of-bounds shots
            continue;
        }
        
        if (shots.dat[i].isGoingUp != 0) { // shot is headed to ufos
            u = 0;
            for (x=0; x < NUM_UFO; ++x) {
                if (ufos[x].alive != 0) {
                    if (is_hit(ufos[x].pos, shots.dat[i].pos)) {
                        ufos[x].alive = 0;
                    } else {
                        u += 1;
                    }
                }
            }
            
            if (u == 0) {
                //! todo: lvl_upd(); // disp "LVL UP %d" in the center, pause
                
                LATENCY -= 5000;
                NUM_UFO += 2;
                
                populate();
            }
        } else { // shot is headed to the aup
            if (is_hit(aup, shots.dat[i].pos)) {
                quit("You lost the Game... Better luck next time!\n");
            }
        }
    }
}

int main() {
    initscr();
    cbreak(); // handle OS signals, and don't wait for \n's on input
    nodelay(stdscr, TRUE); // non-blocking getch()
    noecho(); // don't print crap to scr
    curs_set(0); // disable cursor
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, rows, cols);
    
    srand(time(NULL));
    
    NUM_UFO = 2;
    
    populate();
    
    while (1) {
        draw_all();
        
        run_ufos();
        run_aup();
        run_shots();
        
        update_state();
    }
 
    quit("Game Over...\n");
    return 0;
}
