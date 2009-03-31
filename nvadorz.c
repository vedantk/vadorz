/*
 * nvadorz.c
 * -vk
 *
 * a ncurses space-invaders game
 *
 * Arrow Keys or WASD to move your ship.
 * F or Space to fire bullets.
 * Z for MegaKill.
 * Q to exit.
 * DO NOT HOLD DOWN KEYS, Tap them
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
#define _UNICODE
#define UNICODE
#include "arraylist.h"

#if defined (__WIN32__) && ! defined (__CYGWIN__) 
# include <curses.h>
typedef unsigned int uint;

#else
# include <ncurses.h>
#endif

#include <time.h>
#include <string.h>
#include <wchar.h>	
#include <locale.h>

#ifndef _WIN32
# include <unistd.h>
#define msleep(t) usleep((t) * 1000)
#else
# if defined(_NEED_SLEEP_ONLY) && (defined(_MSC_VER) || defined(__MINGW32__))
#  include <stdlib.h>
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

uint32_t FPS = 10; //Frames per Seconds
uint32_t PPF = 60; //Polls per frame
const wchar_t UFO_ART[] = L"{%Y%}";
uint16_t UFO_ART_SIZE;
const uint16_t BUFF = 1;

const wchar_t AUP_ART[] = L"(|+|)";
uint16_t AUP_ART_SIZE;

const uint32_t SECOND = 1000 * 1000;
const uint32_t MAX_KEYS = 3; //MAXIMUM number of duplicate keys allowed
int16_t MAX_AUP_SHOTS = 8; //not sure I want to be const...
uint32_t SCORE = 0;
const uint32_t UFO_POINTS = 100;
uint32_t SHOOT_PERCENT = 3;

int level = 0;
int rows;
int cols;

typedef struct Posn Posn;
struct Posn {
	uint16_t x;
	uint16_t y;
};

typedef struct UFO UFO;

struct UFO {
	Posn pos;
	bool isAlive;
	bool goRight;
};

ArrayList ufos;
ArrayList ufo_shots;

typedef struct AUP AUP;
 
struct AUP{
	Posn pos;
	ArrayList shots;
}aup;

typedef struct Shot Shot;

struct Shot {
	Posn pos;
};

const wchar_t * AUP_SHOT_ART = L"I";
const wchar_t * UFO_SHOT_ART = L"\xae";
//AUP and shots are cyan, UFOs are Green

ArrayList key_stack;

int stackContains(int c){
	//returns how many of character c are in the key_stack
	int num = 0;
	for(int i = 0; i < key_stack.length(&key_stack); i++){
		if(c == ((int)key_stack.get(&key_stack, i))){
			num++;
		}
	}
	return num;
}

bool addKeyToStack(int c){
	if(stackContains(c) < MAX_KEYS && c > -1){
		key_stack.add(&key_stack, (void *) c);
		return true;
	}else{
		return false;
	}
}

bool isHit(Posn bullet, Posn target,int tsize){
	if(bullet.y == target.y && bullet.x >= target.x && bullet.x <= (target.x + tsize)){
		return true;
	}else{
		return false;
	}
}

void waitAndRead(){
	//waits Seconds/FPS
	//reads into key_stack during sleep
	uint32_t myTime = 0;
	uint32_t comTime = SECOND / FPS;
	while(myTime < comTime){
		int c = getch();
		addKeyToStack(c);
		myTime += comTime / PPF;
		usleep(comTime/PPF);
	}
	
}

void initCurses(){
	initscr();
	start_color();
    cbreak();
    nodelay(stdscr, TRUE); // non-blocking getch()
    noecho(); 
    curs_set(0);
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, rows, cols);
}	


void endCurses() {
    clear();
    curs_set(2);
    endwin();
}

void scores(){
	printf("==============CONGRATULATIONS!==========\n");
	printf("You Scored %d Points!\n", SCORE);
	printf("Try Again to see if you can improve!\n");
}

void bootload(){
	
	new_ArrayList(&key_stack,sizeof(int)); 
	new_ArrayList(&aup.shots,sizeof(Shot));
	new_ArrayList(&ufos, sizeof(UFO));
	new_ArrayList(&ufo_shots, sizeof(Shot));
	UFO_ART_SIZE = wcslen(UFO_ART);
	AUP_ART_SIZE = wcslen(AUP_ART);
	initCurses();
	if (!setlocale(LC_ALL, "en_GB.UTF8")) {
    	fprintf(stderr, "Can't set the specified locale! "
            "Check LANG, LC_CTYPE, LC_ALL.\n");
    	exit(0);
  	}
	init_pair(6, COLOR_BLUE,COLOR_BLACK);
	init_pair(2, COLOR_GREEN,COLOR_BLACK);
	init_pair(1, COLOR_RED,COLOR_BLACK);
	init_pair(5, COLOR_MAGENTA,COLOR_BLACK);
	init_pair(7,COLOR_WHITE,COLOR_BLACK);
	
	srand(time(NULL));
	clear();
}

void draw(Posn pos, const wchar_t * art){
	mvprintw(pos.y, pos.x, "%ls",art);
}

Shot * makeAUPShot(){
	Shot * s = (Shot *)malloc(sizeof(Shot));
	s->pos.x = aup.pos.x + (AUP_ART_SIZE/2);
	s->pos.y = aup.pos.y - 1;
	return s;
}

bool addAUPShot(Shot * s){
	if(c(aup.shots, length) < MAX_AUP_SHOTS || MAX_AUP_SHOTS <= 0){ //0 or less is infiite
		c(aup.shots, add, (void *) s);
		return true;
	}else{
		return false;
	}
}


void draw_world(){
	clear();
	attrset(COLOR_PAIR(7));
	attron(A_BOLD|A_PROTECT);
	attron(COLOR_PAIR(6));
	draw(aup.pos, AUP_ART);

	for(int i = 0; i < c(aup.shots,length); i++){
		Shot *  s = c(aup.shots,get,i);
		draw(s->pos,AUP_SHOT_ART);
	}
	attroff(COLOR_PAIR(6));

	
	attron(COLOR_PAIR(2));
	for(int i = 0; i < c(ufos,length); i++){
		UFO * u = c(ufos,get,i);
		draw(u->pos, UFO_ART);
	}
	
	for(int i = 0; i < c(ufo_shots,length); i++){
		Shot * s = c(ufo_shots,get,i);
		draw(s->pos,UFO_SHOT_ART);
	}
	attroff(COLOR_PAIR(2));
	attroff(A_BOLD|A_PROTECT);
		
}

void runAUPShots(){
	for(int i = 0; i < c(aup.shots,length); i++){
		Shot * s = c(aup.shots,get,i);
		if(s->pos.y > rows - 1 || s->pos.y < 0){
			free(s);
			c(aup.shots,remove, i);
			i--;
		}else{
			Shot * n = (Shot *) malloc(sizeof(Shot));
			n->pos.x = s->pos.x;
			n->pos.y = s->pos.y - 1;
			c(aup.shots, set ,i,n);
			free(s);
		}
	}
}

void runUFOShots(){
	for(int i = 0; i < c(ufo_shots,length); i++){
		Shot * s = c(ufo_shots,get,i);
		if(s->pos.y > rows - 1 || s->pos.y < 0 ){
			free(s);
			c(ufo_shots,remove, i);
			i--;
		}else{
			Shot * n = (Shot *) malloc(sizeof(Shot));
			n->pos.x = s->pos.x;
			n->pos.y = s->pos.y + 1;
			c(ufo_shots, set ,i,n);
			free(s);
		}
	}
}

void runAUP(){
	waitAndRead();
	while(!c(key_stack,isEmpty)){
		int in = (int) c(key_stack, pop);
		
		if (in == KEY_LEFT || in == 'a' || in == 'A') {
            aup.pos.x -= (aup.pos.x == 0) ? 0 : 1;
        } else if (in == KEY_UP || in == 'w' || in == 'W') {
            aup.pos.y -= (aup.pos.y == 0) ? 0 : 1;
        } else if (in == KEY_RIGHT || in == 'd' || in == 'D') {
            aup.pos.x += (aup.pos.x == cols-AUP_ART_SIZE) ? 0 : 1;
        } else if (in == KEY_DOWN || in == 's' || in == 'S') {
            aup.pos.y += (aup.pos.y == rows-1) ? 0 : 1;
        } else if (in == 'q' || in =='Q' || in == KEY_EXIT) {
            endCurses();
			scores();
			sleep(1);
			exit(0);
        } else if (in == ' ' || in == 'f' || in == 'F') {
           addAUPShot(makeAUPShot());
		} else if (in == 'z' || in == 'Z') { // MegaKill
            for (int x=0; x < cols; ++x) {
                Shot * t = (Shot *) malloc(sizeof(Shot));
                t->pos.x = x;
                t->pos.y = aup.pos.y -1;
                c(aup.shots, add, t);
            }
        }else if(in == 'p' || in == 'P'){
			nodelay(stdscr,FALSE);
			attron(COLOR_PAIR(5));
			clear();
			mvprintw(rows/2, cols/2 -3, "Paused");
			int k = getch();
			if(k == 'q' || k == 'Q'){
				endCurses();
				scores();
				sleep(1);
				exit(0);
			}
			attroff(COLOR_PAIR(5));
			draw_world();
			nodelay(stdscr,TRUE);
		} 
	}
}



void runUFOs(){
	for(int i = 0; i < c(ufos,length); i++){
		UFO * u = c(ufos,get,i);
		
		if (u->pos.x >= cols - (UFO_ART_SIZE+BUFF)) {
            u->goRight = false;
            u->pos.y += 1; 
        }
        
        if (u->pos.x <= BUFF) {
            u->goRight = true;
            u->pos.y += 1;
        }
 
        if (u->goRight == false) {
            u->pos.x -= 1;
        } else {
            u->pos.x += 1;
        }
        
        if (rand() % 100 < SHOOT_PERCENT) {
        	Shot * s = (Shot *) malloc(sizeof(Shot));
			s->pos.x = u->pos.x;
			s->pos.y = u->pos.y + 1;
			c(ufo_shots, add, (void *) s);
        }
        
        if (u->pos.y == rows-1) {
            endCurses();
			scores();
			sleep(1);
			exit(0);
        }
    }
}
	
	
	

void populate() {
    if (level > 10) {
        endCurses();
		printf("Just Kidding!\n");
		printf("YOU WON!\n");
		printf("SCORE : %d\n", SCORE);
		sleep(3);
		exit(0);
    }
    
    delete_ArrayList(&(aup.shots));
	delete_ArrayList(&ufo_shots);
	new_ArrayList(&(aup.shots),sizeof(Shot));
	new_ArrayList(&ufo_shots,sizeof(Shot));
    
    aup.pos.x = (cols / 2) - AUP_ART_SIZE;
    aup.pos.y = rows-1;

    for (int itr=0; itr < (level * 2 + 3); ++itr) {
        struct UFO * t = (UFO *) malloc(sizeof(UFO));
        t->pos.y = 0;
        t->pos.x = (itr*(UFO_ART_SIZE+3));

        adjust_ufo_pos:
        if (t->pos.x >= cols-UFO_ART_SIZE) {
            t->pos.y += 1;
            t->pos.x /= 2;
            if (t->pos.x >= cols-UFO_ART_SIZE) {
                goto adjust_ufo_pos;
            }
        }
        
        t->goRight = (t->pos.x >= cols - (UFO_ART_SIZE + 3) ? false : true);
        c(ufos, add, (void *) t);
    }
}

void levelUp(){
	level++;
	SHOOT_PERCENT += 3;
	clear();
	attron(COLOR_PAIR(1));
	mvprintw(rows/2 - 5, cols/2 - 10, "You beat level %d!\n",level);
	mvprintw(rows/2 - 4, cols/2 - 10 , "Get Ready for level %d!",level+1);
	mvprintw(rows/2 - 3, cols/2 - 10 ,"Current Score : %d", SCORE);
	refresh();
	sleep(3);
	populate();
}
	

void update(){
	for(int i = 0; i < c(aup.shots,length); i++){
		Shot * s = c(aup.shots,get,i);
		for(int q = 0; q < c(ufos,length); q++){
			UFO * u = c(ufos,get,q);
			if(isHit(s->pos,u->pos,UFO_ART_SIZE)){
				free(u);
				free(s);
				c(ufos,remove,q);
				c(aup.shots,remove,i);
				q--;
				i--;
				SCORE += UFO_POINTS;
			}
		}
	}
	
	if(c(ufos,isEmpty)){
		levelUp();
	}
	
	for(int i = 0; i < c(ufo_shots,length); i++){
		Shot * s = c(ufo_shots,get,i);
		if(isHit(s->pos,aup.pos,AUP_ART_SIZE)){
			endCurses();
			scores();
			sleep(1);
			exit(0);
		}
	}
}


	
			
		
int main(){
	
	bootload();
	populate();
	while(true){
		draw_world();
		runAUP();
		runUFOs();
		runAUPShots();
		runUFOShots();
		update();
		draw_world();
	}
	endCurses();
	scores();
	sleep(3);
	return 0;
}
