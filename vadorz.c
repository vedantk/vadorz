/*
 * vadorz.c
 *
 * an addicting ncurses space-invaders game
 * http://code.google.com/p/vadorz/
 *
 * Copyright (c) 2009, Vedant Kumar and Oreoluwa Babarinsa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * > Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * > Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * > The names of the authors may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if defined (__WIN32__) && !defined (__CYGWIN__)
# include <curses.h>
#else
# include <ncurses.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
# include <unistd.h>
#else
# if defined(_NEED_SLEEP_ONLY) && (defined(_MSC_VER) || defined(__MINGW32__))
#  define sleep(t) _sleep((t) * 1000)
# else
#  include <windows.h>
#  define sleep(t)  Sleep((t) * 1000)
# endif
# ifndef _NEED_SLEEP_ONLY
#  define msleep(t) Sleep(t)
#  define usleep(t) Sleep((t) / 1000)
# endif
#endif

typedef unsigned short int num;

/* Begin Tweak-able Settings */
#define AUP_SHOT_ART "!"
#define UFO_ART "(@-@)"
#define AUP_ART "$\\A/$"

#define AUP_LIVES 5
#define AUP_MEGAKILLS 5

#define SHOOT_FACTOR 2
num SHOOT_PERCENT = 2;

#define FPS 20
#define PPF 50
/* End Tweak-able Settings */

#define SECOND 1000000

num SCORE = 0;
num level = 0;

num UFO_ART_SIZE;
num AUP_ART_SIZE;

num rows;
num cols;

struct Posn {
    num x;
    num y;
};

struct Aup {
    struct Posn pos;
    num lives;
    num megakills;
};

struct Ufo {
    struct Posn pos;
    num alive;
    num goRight;
};

struct Shot {
    struct Posn pos;
    num alive;
    num isGoingUp;
};

num smax;
num scur;
struct Shot* shots;

struct Aup aup;
struct Ufo ufos[((100/SHOOT_FACTOR) * 2) + 4];

int in;
num u;

num i;
num x;

void quit(const char* seq) {
    clear();
    curs_set(2);
    endwin();
    printf(seq);
    free(shots);
    exit(0);
}

struct Shot mk_shot(struct Posn obj, num uphuh, num grace) {
    struct Shot datum;
    datum.pos.x = obj.x + grace;
    datum.pos.y = (uphuh != 0) ? obj.y-1 : obj.y+1;
    datum.alive = 1;
    datum.isGoingUp = uphuh;
    return datum;
}

void add_shot(struct Shot datum) {
    if (scur < smax) {
        shots[scur] = datum;
        scur += 1;
    } else if (scur == smax) {
        smax += smax / 2;
        shots = (struct Shot*) realloc(shots, smax * sizeof(struct Shot));

        if (shots == NULL) {
            quit(":: shot_list is out of memory!\n");
        }

        add_shot(datum);
    } else {
        quit(":: shot_list overflow!\n");
    }
}

inline void draw(struct Posn obj, const char* art) {
    mvprintw(obj.y, obj.x, art);
}

void draw_all() {
    clear();

    mvprintw(rows-2, 0, "Lives: %u", aup.lives);
    mvprintw(rows-1, 0, "MegaKills: %u", aup.megakills);    
    draw(aup.pos, AUP_ART);

    for (i=0; i < scur; ++i) {
        if (shots[i].alive == 0) {
            continue;
        }

        draw(shots[i].pos, (shots[i].isGoingUp != 0) ? AUP_SHOT_ART : "x");
    }

    for (i=0; i < level*2+3; ++i) {
        if (ufos[i].alive == 0) {
            continue;
        }

        draw(ufos[i].pos, UFO_ART);
    }
    
    refresh();
}

void gameover(const char* msg) {
    clear();
    
    mvprintw(0, 0, msg);
    mvprintw(rows-1, 0, "Final Score: %u...", SCORE);
    
    refresh();
    sleep(5);
}

void run_ufos() {
    for (i=0; i < level*2+3; ++i) {
        if (ufos[i].alive == 0) {
            continue;
        }

        if (ufos[i].pos.x >= cols - 8) {
            ufos[i].goRight = 0;
            ufos[i].pos.y += 1;
        }

        if (ufos[i].pos.x <= 1) {
            ufos[i].goRight = 1;
            ufos[i].pos.y += 1;
        }

        if (ufos[i].goRight == 0) {
            ufos[i].pos.x -= 2;
        } else {
            ufos[i].pos.x += 2;
        }

        if (rand() % 100 < SHOOT_PERCENT) {
            add_shot(mk_shot(ufos[i].pos, 0, UFO_ART_SIZE/2));
        }

        if (ufos[i].pos.y == rows-1) {
            gameover("The UFOs have landed. Earth will be destroyed.");
            quit("Better luck next time!\n");
        }
    }
}

void run_aup() {
    for (x=0; x < PPF; ++x) {
        in = getch();
        usleep((SECOND/FPS) / PPF);

        if (in == ERR) {
            continue;
        }

        if (in == KEY_LEFT || in == 'a' || in == 'h') {
            aup.pos.x -= (aup.pos.x == 0) ? 0 : 1;
        } else if (in == KEY_UP || in == 'w' || in == 'j') {
            aup.pos.y -= (aup.pos.y == 0) ? 0 : 1;
        } else if (in == KEY_RIGHT || in == 'd' || in == 'k') {
            aup.pos.x += (aup.pos.x == cols-AUP_ART_SIZE) ? 0 : 1;
        } else if (in == KEY_DOWN || in == 's' || in == 'l') {
            aup.pos.y += (aup.pos.y == rows-1) ? 0 : 1;
        } else if (in == ' ' || in == 'f' || in == 'F') {
            add_shot(mk_shot(aup.pos, 1, AUP_ART_SIZE/2));
        } else if (in == 'z' || in == 'Z') {
            if (aup.megakills == 0) {
                continue;
            } else {
                aup.megakills -= 1;
            }
            
            for (x=0; x < cols; ++x) {
                struct Shot t = mk_shot(aup.pos, 1, 0);
                t.pos.x = x;
                add_shot(t);
            }
        } else if (in == 'q' || in == KEY_EXIT) {
            quit("User Exit.\n");
        } else if (in == 'p') {
			nodelay(stdscr,FALSE);
            mvprintw(0, 0, "Paused...");
			getch();
			nodelay(stdscr,TRUE);
        }
    }
}

void run_shots() {
    for (i=0; i < scur; ++i) {
        if (shots[i].alive == 0) {
            continue;
        }

        in = shots[i].pos.y;
        if (in < 0 || in > rows-1) {
            shots[i].alive = 0;
        }

        if (shots[i].isGoingUp != 0) {
            shots[i].pos.y -= 1;
        } else {
            shots[i].pos.y += 1;
        }
    }
}

inline int is_hit(struct Posn obj, struct Posn pos, int grace) { // ship : bullet
    return (obj.y == pos.y
            && pos.x <= obj.x+grace
            && pos.x >= obj.x) ? 1 : 0;
}

void populate() {
    if (SHOOT_PERCENT >= 100) {
        gameover("The UFO threat has been thwarted! Earth is safe... For now!");
        quit("Wow. I can't believe you just won this game.\n");
    }

    if (shots) {
        free(shots);
    }

    scur = 0;
    smax = cols*3;
    shots = (struct Shot*) malloc(smax * sizeof(struct Shot));

    aup.pos.x = (cols / 2) - AUP_ART_SIZE;
    aup.pos.y = rows-1;

    for (x=0; x < level*2+3; ++x) {
        struct Ufo t;
        t.pos.y = 0;
        t.pos.x = (x*8);

        adjust_ufo_pos:
        if (t.pos.x >= cols-UFO_ART_SIZE) {
            t.pos.y += 1;
            t.pos.x /= 2;
            if (t.pos.x >= cols-UFO_ART_SIZE) {
                goto adjust_ufo_pos;
            }
        }

        t.alive = 1;
        t.goRight = (t.pos.x >= cols - 8) ? 0 : 1;
        ufos[x] = t;
    }
}

void lvld_up() {
    clear();

    level += 1;
    SHOOT_PERCENT += SHOOT_FACTOR;
    populate();

	mvprintw(rows-3, 0, "You beat level %d!\n",level);
	mvprintw(rows-2, 0, "Get ready for level %d!",level+1);
	mvprintw(rows-1, 0,"Current Score: %d...", SCORE);

    refresh();
    sleep(3);
}

void update_state() {
    for (i=0; i < scur; ++i) {
        if (shots[i].alive == 0) {
            continue;
        }

        if (shots[i].isGoingUp != 0) { // shot is headed to ufos
            u = 0;
            for (x=0; x < level*2+3; ++x) {
                if (ufos[x].alive != 0) {
                    if (is_hit(ufos[x].pos, shots[i].pos, UFO_ART_SIZE)) {
                        ufos[x].alive = 0;
                        shots[i].alive = 0;
                        SCORE += 100;
                    } else {
                        u += 1;
                    }
                }
            }

            if (u == 0) {
                lvld_up();
            }
        } else { // shot is headed to the aup
            if (is_hit(aup.pos, shots[i].pos, AUP_ART_SIZE)) {
                aup.lives -= 1;
                shots[i].alive = 0;
                
                if (aup.lives < 1) {
                    gameover("Your ship is destroyed! Earth has no hope!");
                    quit("You lost the Game... Better luck next time!\n");
                }
            }
        }
    }
}

int main() {
    initscr();
    cbreak();
    nodelay(stdscr, TRUE);
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, rows, cols);

    srand(time(NULL));
    UFO_ART_SIZE = strlen(UFO_ART);
    AUP_ART_SIZE = strlen(AUP_ART);

    aup.lives = AUP_LIVES;
    aup.megakills = AUP_MEGAKILLS;

    populate();

    while (1) {
        draw_all();

        run_ufos();
        run_aup();
        run_shots();

        draw_all();

        update_state();
    }

    return 0;
}
