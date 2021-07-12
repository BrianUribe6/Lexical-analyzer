#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>


typedef struct Token 
{
    char regex[255];
    char name[255];
} token_t;


void regexException(int status, regex_t *preg)
{
    size_t size = regerror(status, preg, (char *)NULL, (size_t)0);
    char errmsg[size];
    regerror(status, preg, errmsg, 255);
    puts(errmsg);
    exit(1);
}

/**
 * 
 * Matches every token in a string given a token list or regular expressions.
 * Each match is stored in a buffer and the caller is responsible for properly freeing memory after it's use.
 * Every unused entry of the buffer would be set to NULL.
 * 
 * char * string: The string to match.
 * token_t *tokens: array of structures containing regular expressions and their name.
 * size_t ntoken: number of tokes in the grammar.
 * char *tokenBuf[]: buffer to hold the string corresponding to each token.
 * size_t nbuf: size of tokenBuf, which must be at least equal to len(string)
 * 
 * returns a LinkedList containing every token matched.
 * throws: regexException
*/
void matcher(char* string, token_t *tokens, size_t ntoken, char *tokenBuf[], size_t nbuf)
{    
    regex_t preg;
    regmatch_t match;

    if (!string || *string =='\0' || ntoken < 1)
        return;
    
    int j = 0;
    while (*string) 
    {
        int i = 0;
        int hasMatched = 0;
        //skip whitespace
        while(*string && isspace(*string))  string++;
        
        while (i < ntoken && !hasMatched)  //find a matching regular expression 
        {
            token_t tok = tokens[i++];
            int status = regcomp(&preg, tok.regex, REG_EXTENDED);
            if (status != 0) //couldn't compile regular expression
            {
                regexException(status, &preg);
            }
            int error = regexec(&preg, string, 1, &match, REG_NOTBOL);
            if (!error && match.rm_so == 0) //There's a match at the current position
            {
                hasMatched = 1;
                int start = match.rm_so; 
                int end = match.rm_eo;
                size_t len = end - start;
                char *substr = malloc(sizeof(char) * (len + 1));
                substr[len] = '\0';
                
                strncpy(substr, string + start, len);
                tokenBuf[j++] = substr;
                printf("%.*s\t%s\n", (end - start), string + start, tok.name);
                string += end;
            }
            regfree(&preg);
        }
        if (!hasMatched)    string++;
    }
    //Set the remaining space of the token buffer to NULL in case the buffer was declared dynamically
    for (;j < nbuf; j++) tokenBuf[j] = NULL;
}

/**
 * Clears resulting buffer from a matcher call
*/
void freeTokens(char *toks[], size_t n) {
    for (int i = 0; i < n && toks[i]; i++)
        free(toks[i]);
}

/**
 * Loads from a file each token's regex and description into and array of token structs.
 * 
 * char *filename: name of the file on disk.
 * token_t *tokens: destination array which most contain at least 'ntokens' allocated.
 * int ntokens: number of tokens to be loaded.
 * 
 * Throws: File Not Found
*/
void loadGrammar(char* filename, token_t *tokens, int ntokens)
{
    FILE * grammar = fopen(filename, "r");
    if (!grammar)
    {
        printf("%s: File Not Found\n", filename);
        exit(1);
    }
    int i = 0;
    while (i < ntokens && fscanf(grammar, "%s %[^\n]", tokens[i].regex, tokens[i].name) != EOF) i++;
    
    fclose(grammar);
}

/**
 * Compares the contents of each node in a list its corresponding match in a string array. 
 * returns 1 if the content of the list matches the content of the string 0 Otherwise
 * 
 * list_t *lst: linked list of characters.
 * char *str: array of variable length strings.
 * size_t nstr: number of strings.
*/
int listEquals(char *a[], char *b[], size_t na, size_t nb)
{
    int i = 0, j = 0;
    while (i < na && j < nb)
    {
        if (strcmp(a[i++], b[j++]) != 0)
            return 0;
    }
    return 1;
}

/**
 * Tests the output of matcher(...) given an input string.
 * Throws assertion error if a given test case fails.
*/
void testMatcher(token_t *tokens, int ntokens) 
{
    const int ntests = 6;
    //test input strings
    char *t[] = {
        "for (int i = 0; i < 10; i++) {}", 
        "",
        "array[xyz ] += pi 3.14159e-10     ",
        "0x4356abdc 0777 []",
        "while (i >> 1 >= 0 && b & 2 == NULL)",
        "()[]{}.->sizeof,!~>><<^|++--+/||&&?:==!=<><=>==+=-=*=/=%=>>=<<=&=^=|=&-*\"'#"
        };
    //expected output
    char *res[][50] = {
        {"for", "(", "int", "i", "=", "0", ";", "i", "<", "10", ";", "i", "++", ")", "{", "}"},
        {},
        {"array", "[", "xyz", "]", "+=", "pi", "3.14159e-10"},
        {"0x4356abdc", "0777", "[", "]"},
        {"while", "(", "i", ">>", "1", ">=", "0", "&&", "b", "&", "2", "==", "NULL", ")"},
        //all symbols
        {"(",")","[","]","{","}",".","->","sizeof",",","!","~",">>","<<","^","|","++","--",
        "+","/","||","&&","?",":","==","!=","<",">","<=",">=","=","+=","-=",
        "*=","/=","%=",">>=","<<=","&=","^=","|=","&","-","*", "\"", "'", "#"}
        };
    const size_t resLen[] = {16, 0, 7, 4, 14, 46}; //Size of each expected output array
    for (int i = 0; i < ntests; i++)
    {
        printf("============BEGGINING TEST %d==========\n", (i + 1));
        printf("input: '%s'\n\n", t[i]);
        size_t testSize = strlen(t[i]);
        char *m[testSize];
        matcher(t[i], tokens, ntokens, m, testSize);

        assert(listEquals(m, res[i], testSize, resLen[i]));
        printf("=============TEST %d PASSED============\n", (i + 1));
        freeTokens(m, testSize);
        puts("");
    }
    puts("===============> All test passed <================");
}

/**
 * Tokenizer can be called with several arguments:
 * ex: ./tokenizer my_string  tokenizes my_string using the default grammar file.
 * ./tokenizer --test runs several test cases using the default grammar file.
 * additional flag:
 * --grammar filename num_tokens : uses a custom grammar file to tokenize your string. 
 *                                  The file must be properly formatted see (README)
 * ex: ./tokenizer my_string --grammar my_grammar.tok 10
*/
int main(int argc, char *argv[]) 
{   
    int ntokens;
    char *grammarFile;
    size_t len;
    //tokenizer --test
    if (argc == 2) 
    {
        ntokens = 87;
        grammarFile = "grammar.tok";
        if (strcmp(argv[1], "--test") == 0)
        {
            token_t tokens[ntokens];
            loadGrammar(grammarFile, tokens, ntokens);
            testMatcher(tokens, ntokens);
            return 0;
        }
    }
    //tokenizer somestring --grammar filename num_tokens
    else if (argc == 5 && strcmp(argv[2], "--grammar") == 0) 
    {
        ntokens = atoi(argv[4]);
        if (ntokens <= 0)
        {
            puts("number of tokens must be positive");
            return 1;
        }
        grammarFile = argv[3];
    }
    else 
    {
        puts("incorrect arguments correct usage: ./tokenizer \"somestring\" --grammar filename num_tokens");
        puts("if a custom grammar is not being used, just call ./tokenizer \"somestring\"");
        return 1;
    }
    len = strlen(argv[1]);
    char *tokenList[len];
    token_t tokens[ntokens];
    loadGrammar(grammarFile, tokens, ntokens);
    // char *test = "()[]{}.->sizeof,!~>><<^|++--+/||&&?:==!=<><=>==+=-=*=/=%=>>=<<=&=^=|=&-*";
    // for(int i = 0; /*This is a loop\t\n*/i < 5; i++) //start of the loop\n{print(i);}
    matcher(argv[1], tokens, ntokens, tokenList, len);
    
    freeTokens(tokenList, len);    
    return 0;
}
