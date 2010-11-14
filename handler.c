
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"

/* Idea is to ask google translator API about translated word
 * Link that should be sent to socket
 * http://ajax.googleapis.com/ajax/services/language/translate?v=1.0&q=word&langpair=en%7Cpl
 */

/* Idea is to ask google online translator about translated word
 * Link that should be send to socket
 * http://translate.google.pl/translate_a/t?client=t&text=job&hl=pl&sl=en&tl=pl&multires=1&sc=1
 */

static int isConstant = 0;

static char *reqTop[2];
static char *reqBottom[2];
static size_t reqTopSize[2];
static size_t reqBottomSize[2];
static char *key[2];

void
initConstant()
{
    if (isConstant)
        return;

    reqTop[BASIC] = "GET http://ajax.googleapis.com/ajax/services/language/translate?v=1.0&q=";
    reqBottom[BASIC] = "&langpair=en%7Cpl\n";
    reqTopSize[BASIC] = strlen(reqTop[BASIC]);
    reqBottomSize[BASIC] = strlen(reqBottom[BASIC]);
    key[BASIC] = "\"translatedText\"";

    reqTop[ADDITIONAL] = "GET http://translate.google.pl/translate_a/t?client=t&text=";
    reqBottom[ADDITIONAL] = "&hl=pl&sl=en&tl=pl&multires=1&sc=1";
    reqTopSize[ADDITIONAL] = strlen(reqTop[ADDITIONAL]);
    reqBottomSize[ADDITIONAL] = strlen(reqBottom[ADDITIONAL]);
    key[ADDITIONAL] = "X-XSS-Protection: 1; mode=block";

    isConstant = 1;
}

char* buildRequest(int type, char *word)
{
    initConstant();
    int wordSize = strlen(word);
    int i, j=0, spaces = 0;
    for (i=0; i<wordSize; ++i)
        if (word[i] == ' ')
            spaces++;
    spaces = spaces*2;

    char *tmp = malloc(wordSize + spaces + 1);
    for (i=0; i<wordSize; ++i)
        if (word[i] != ' ')
        {
            tmp[j++] = word[i];
        }
        else
        {
            tmp[j++] = '%';
            tmp[j++] = '2';
            tmp[j++] = '0';
        }
    tmp[wordSize + spaces] = '\0';
    wordSize = strlen(tmp);
    word = tmp;

    tmp = malloc(reqTopSize[type] +
                        reqBottomSize[type] + 
                        wordSize + 1);
    memcpy(tmp, reqTop[type], reqTopSize[type]);
    memcpy(tmp + reqTopSize[type], word, wordSize);
    memcpy(tmp + reqTopSize[type] + wordSize, reqBottom[type], reqBottomSize[type]);
    tmp[reqTopSize[type] + wordSize + reqBottomSize[type]] = '\n';

    free(word);
    return tmp;
}

char*
getAdditionalText(char *word)
{
    int wordSize = strlen(word);
    int keySize = strlen(key[ADDITIONAL]);
    int i, found = 0;
    for (i=0; i<wordSize; i++) {
        //Find key
        if (word[i] == 'X' && i + 31 < wordSize &&
                memcmp(word + i, key[ADDITIONAL], keySize) == 0) {
            found = i + keySize;
            break;
        }
    }
    int leftStop, rightStop;
    leftStop = found+4;
    rightStop = wordSize;

    //printf("Found %d l:%d, r:%d size:%d\n", found, leftStop, rightStop, wordSize);
    word[rightStop-5] = '\0';

    if (!found) {
        return "Json parse error";
    }

    //printf("Translated word: %s \n", word+leftStop);
    return word+leftStop;
}

char*
getBasicText(char *word)
{
    int wordSize = strlen(word);
    int keySize = strlen(key[BASIC]);
    int i, found = 0;
    for (i=0; i<wordSize; i++) {
        //Find "translatedText"
        if (word[i] == '"' && i + 16 < wordSize &&
                memcmp(word + i, key[BASIC], keySize) == 0) {
            found = i + keySize;
            break;
        }
    }
    int leftStop, rightStop;
    //Find left "
    for (i=found; i<wordSize; i++)
        if (word[i] == '\"') {
            leftStop = i+1;
            break;
        }
    //Find right "
    for (i=leftStop; i<wordSize; i++)
        if (word[i] == '\"') {
            rightStop = i;
            break;
        }

    //printf("Found %d l:%d, r:%d size:%d\n", found, leftStop, rightStop, strlen(word));
    word[rightStop] = '\0';

    if (!found) {
        return "Json parse error";
    }

    printf("Translated word: %s \n", word+leftStop);
    return word+leftStop;
}

