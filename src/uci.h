#ifndef _UCI_H
#define _UCI_H

int stringEquals(char * s1, char * s2);
int stringStartsWith(char * str, char * key);
int stringContains(char * str, char * key);
void getInput(char * str);
void moveToString(char * str, uint16_t move);

#endif