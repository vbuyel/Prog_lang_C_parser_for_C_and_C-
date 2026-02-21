%option noyywrap

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int next;

/*---------- TOKENS (TERMINALS) -----------*/

enum {
    T_TYPE = 1,
    T_IDENTIFIER,
    T_NUMBER,
    T_STRING,
    T_SEMICOLON,
    T_LBRACE,
    T_RBRACE,
    T_LPAREN,
    T_RPAREN,
    T_ASSIGN,
    T_OP,
    T_IF,
    T_ELSE,
    T_WHILE,
    T_RETURN,
    T_COMMA
};

/*------------- PROTOTYPES ----------------*/

int Program();
int FuncList();
int Func();
int Params();
int ParamList();
int Block();
int StmtList();
int Stmt();
int Expr();
int Term();
int Factor();

int check(int tok);
void error(const char *msg);

/*------------- HELPERS ----------------*/

int check(int tok) {
    if (next == tok) {
        next = yylex();
        return 1;
    }
    return 0;
}

void error(const char *msg) {
    printf("Syntax error: %s\n", msg);
    exit(1);
}

%}

TYPE        int|void|char|float|double
IDENT       [a-zA-Z_][a-zA-Z0-9_]*
NUMBER      [0-9]+
STRING      \"([^"])*\"

WS          [ \t\r\n]+

%%

{TYPE}              { return T_TYPE; }
"if"                { return T_IF; }
"else"              { return T_ELSE; }
"while"             { return T_WHILE; }
"return"        { return T_RETURN; }

{IDENT}             { return T_IDENTIFIER; }
{NUMBER}            { return T_NUMBER; }
{STRING}            { return T_STRING; }

"="                 { return T_ASSIGN; }
";"                 { return T_SEMICOLON; }
","                 { return T_COMMA; }

"{"                 { return T_LBRACE; }
"}"                 { return T_RBRACE; }

"("                 { return T_LPAREN; }
")"                 { return T_RPAREN; }

"+"|"-"|"*"|"/"     { return T_OP; }

"/*"[^"*/"]*"*/"    { /* comment */ }
"//".*              { /* line comment */ }

{WS}                { /* skip */ }

.                   { /* ignore unknown */ }

%%

/*---------------- PARSER ----------------*/

int Program() {
    if (next == 0) return 1;
    if (!FuncList()) error("expected function list");
    return 1;
}

int FuncList() {
    while (next == T_TYPE) {
        if (!Func()) error("bad function");
    }
    return 1;
}

int Func() {
    if (!check(T_TYPE)) error("expected return type");
    if (!check(T_IDENTIFIER)) error("expected function name");
    if (!check(T_LPAREN)) error("expected '('");
    Params();
    if (!check(T_RPAREN)) error("expected ')'");
    if (!Block()) error("bad block");
    return 1;
}

int Params() {
    if (next == T_RPAREN) return 1;
    return ParamList();
}

int ParamList() {
    if (!check(T_TYPE)) error("expected type");
    if (!check(T_IDENTIFIER)) error("expected identifier");

    while (check(T_COMMA)) {
        if (!check(T_TYPE)) error("expected type");
        if (!check(T_IDENTIFIER)) error("expected identifier");
    }
    return 1;
}

int Block() {
    if (!check(T_LBRACE)) error("expected '{'");
    StmtList();
    if (!check(T_RBRACE)) error("missing '}'");
    return 1;
}

int StmtList() {
    while (next != T_RBRACE && next != 0) {
        Stmt();
    }
    return 1;
}

int Stmt() {

    if (check(T_TYPE)) {
        if (!check(T_IDENTIFIER)) error("expected identifier in declaration");
        if (check(T_ASSIGN)) Expr();
        if (check(T_LPAREN)) {
            Expr();
            while (check(T_COMMA)) {
                Expr();
            }
            if (!check(T_RPAREN)) error("expected ')'");
        }
        if (!check(T_SEMICOLON)) error("missing ';'");
        return 1;
    }

    if (check(T_RETURN)) {
        Expr();
        if (check(T_LPAREN)) {
            Expr();
            while (check(T_COMMA)) {
                Expr();
            }
            if (!check(T_RPAREN)) error("expected ')'");
        }
        if (!check(T_SEMICOLON)) error("missing ';'");
        return 1;
    }

    if (check(T_IDENTIFIER)) {
        if (check(T_LPAREN)) {
            Expr();
            while (check(T_COMMA)) {
                Expr();
            }
            if (!check(T_RPAREN)) error("expected ')'");
        } else {
            if (!check(T_ASSIGN)) error("expected '='");
            Expr();
        }
        if (!check(T_SEMICOLON)) error("missing ';'");
        return 1;
    }

    if (check(T_IF)) {
        if (!check(T_LPAREN)) error("expected '('");
        Expr();
        if (!check(T_RPAREN)) error("expected ')'");
        Block();
        if (check(T_ELSE)) Block();
        return 1;
    }

    if (check(T_WHILE)) {
        if (!check(T_LPAREN)) error("expected '('");
        Expr();
        if (!check(T_RPAREN)) error("expected ')'");
        Block();
        return 1;
    }

    if (next == T_LBRACE) {
        Block();
        return 1;
    }

    error("unknown statement");
    return 0;
}

int Expr() {
    if (!Term()) error("expected expression");
    while (next == T_OP) {
        check(T_OP);
        if (!Term()) error("expected term");
    }
    return 1;
}

int Term() {
    if (check(T_IDENTIFIER)) return 1;
    if (check(T_RETURN)) return 1;
    if (check(T_NUMBER)) return 1;
    if (check(T_STRING)) return 1;

    if (check(T_LPAREN)) {
        Expr();
        if (!check(T_RPAREN)) error("expected ')'");
        return 1;
    }

    return 0;
}

/*---------------- MAIN ----------------*/

int main() {
    next = yylex();
    Program();
    printf("Parsing completed successfully.\n");
}
