#ifndef ERRCHK
#define ERRCHK

#include <stdio.h>
#include <string.h>

//STRING ERROR CHECKING

int strNotEmpty(char* str){ return strlen(str) > 0; }
int strMaxLenght(char* str, int len){ return strlen(str) <= len; }
int strMinLenght(char* str, int len){ return strlen(str) >= len; }
int strLenInterval(char* str, int min, int max){ return strMinLenght(str, min) && strMaxLenght(str, max); }

int isCapital(char c){return (c >= 'A' && c <= 'Z');}
int isLowercase(char c){return (c >= 'a' && c <= 'z');}
int isSpace(char c){return c == ' ';}
int isQMark(char c){return c == '\'';}

int isLetter(char c){ return isCapital(c) || isLowercase(c) || isSpace(c) || isQMark(c);}

int strOnlyLetters(char* str)
{
    int ret = 1;

    for(int i = 0; i < strlen(str); i++)
        ret *= isLetter(str[i]);

    return ret;
}

int strCapital(char* str)
{
    int ret = isCapital(str[0]);
    ret *= strOnlyLetters(str);
}

int isNumber(char c){return (c >= '0' && c <= '9');}

int strOnlyNumbers(char* str)
{
    int ret = 1;

    for(int i = 0; i < strlen(str); i++)
        ret *= isNumber(str[i]);

    return ret;
}

int strNot0(char* str)
{
    if(strlen(str) == 1) return str[0] != '0';
}

int ValidName(char* str) //used for players and teams
{
    return (strNotEmpty(str) && strCapital(str));
}

int ValidNumber(char* str)
{
    return (strLenInterval(str, 1, 2) && strOnlyNumbers(str) && strNot0(str));
}

//ERROR RAISING LOGIC:

/* 
enum errType {wrongInput};
char* errors[] =  {"Wrong Input"};
 */
#endif