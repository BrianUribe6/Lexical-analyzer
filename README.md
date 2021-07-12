# Grammar Tokenizer

Grammar Tokenizer is a lexical analyzer, i.e given a grammar it classifies each individual token into it's corresponding functionality. Each token must be defined using it's corresponding regular expression and its type. By default a C-like grammar is used.
# Defining a grammar
A grammar file is formatted as <**regex**>  <**description/name**>. Note the single space in between the two. Each token definition must be in its own separate line or else they would be assumed to correspond to the same token. An example grammar file would look like this:

    [a-zA-Z] alphabetic character
    [0-9] digit
			    ...
	while RESERVED_KEYWORD
	\^ bitwise XOR
Please note the need for "\\"in the last regular expression. The tokenizer uses POSIX Standard regular expressions, and thus the following characters must be escaped: `.^$*+?()[{\|`

## Usage
To run test Grammar Tokenizer against the default C grammar run     `$ ./tokenizer --test`

    usage: ./tokenizer [my_string [--grammar filename num_tokens]| --test]
    optional arguments:
      --grammar grammar_file num_tokens     tokenizes my_string using your own custom grammar
    
    examples:
    ./tokenizer my_string   //This would tokenize my_string using default grammar
    ./tokenizer my_string --grammar my_grammar.tok 10  //tokenize using my_grammar
    ./tokenizer --test  //run default testcases on default grammar.