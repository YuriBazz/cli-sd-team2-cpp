%{
#include "ast.hpp"
#include "executor.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern int yylineno;
void yyerror(const char *s);
int yylex();

extern Executor* g_executor;
%}

%code requires {
    #include "ast.hpp"
}

%union {
    char* str;
    int num;
    Argument* arg;
    ArgumentList* arglist;
    Command* cmd;
    Statement* stmt;
    std::vector<Argument*>* argvec;
}

%token <str> WORD STRING VARIABLE STRING_SEGMENT WORD_ASSIGN
%token <str> TOKEN_CAT TOKEN_ECHO TOKEN_WC TOKEN_PWD TOKEN_EXIT TOKEN_GREP TOKEN_LS TOKEN_CD
%token SPACE
%token <num> NUMBER
%token PIPE SEMICOLON ASSIGN NEWLINE

%type <arg> argument atom
%type <arglist> argument_list
%type <argvec> argument_sequence
%type <cmd> command
%type <stmt> pipeline statement assignment

%start program

%%

program:
    /* empty */
    | program optional_spaces statement NEWLINE {
        if ($3) {
            g_executor->execute($3);
            delete $3;
        }
        if (g_executor->shouldExit()) {
            YYABORT;
        }
    }
    | program optional_spaces NEWLINE
    | program optional_spaces error NEWLINE {
        yyerror("Syntax error");
        yyerrok;
    }
    ;

optional_spaces:
    /* empty */
    | spaces
    ;

spaces:
    SPACE
    | spaces SPACE
    ;

statement:
    pipeline      { $$ = $1; }
    | assignment  { $$ = $1; }
    ;

assignment:
    WORD_ASSIGN argument_list {
        $$ = new Assignment($1, $2);
        free($1);
    }
    | WORD ASSIGN argument_list {
        $$ = new Assignment($1, $3);
        free($1);
    }
    ;

pipeline:
    command                     { $$ = new Pipeline({$1}); }
    | pipeline optional_spaces PIPE optional_spaces command     {
        Pipeline* p = dynamic_cast<Pipeline*>($1);
        if (p) {
            p->commands.push_back($5);
            $$ = p;
        } else {
            yyerror("Invalid pipeline");
            $$ = nullptr;
        }
    }
    ;

command:
    TOKEN_CAT optional_spaces argument_list     { $$ = new CatCommand($3); free($1); }
    | TOKEN_ECHO optional_spaces argument_list  { $$ = new EchoCommand($3); free($1); }
    | TOKEN_WC optional_spaces argument_list    { $$ = new WcCommand($3); free($1); }
    | TOKEN_PWD optional_spaces argument_list   { $$ = new PwdCommand($3); free($1); }
    | TOKEN_EXIT optional_spaces argument_list  { $$ = new ExitCommand($3); free($1); }
    | TOKEN_GREP optional_spaces argument_list  { $$ = new GrepCommand($3); free($1); }
    | TOKEN_LS optional_spaces argument_list    { $$ = new LsCommand($3); free($1); }
    | TOKEN_CD optional_spaces argument_list    { $$ = new CdCommand($3); free($1); }
    | WORD optional_spaces argument_list        {
        $$ = new ExternalCommand($1, $3);
        free($1);
    }
    | STRING optional_spaces argument_list      {
        $$ = new ExternalCommand($1, $3);
        free($1);
    }
    | VARIABLE optional_spaces argument_list    {
        std::string varName = std::string("$") + $1;
        $$ = new ExternalCommand(varName, $3);
        free($1);
    }
    ;

argument_list:
    /* empty */                 { $$ = new ArgumentList(); }
    | argument_sequence         {
        $$ = new ArgumentList();
        for (auto arg : *$1) {
            $$->push_back(arg);
        }
        delete $1;
    }
    ;

argument_sequence:
    argument {
        $$ = new std::vector<Argument*>();
        $$->push_back($1);
    }
    | argument_sequence spaces argument {
        $1->push_back($3);
        $$ = $1;
    }
    ;

argument:
    atom { $$ = $1; }
    | argument atom {
        if ($1->type == Argument::COMPOSITE) {
            $1->parts.push_back($2);
            $$ = $1;
        } else {
            std::vector<Argument*> parts;
            parts.push_back($1);
            parts.push_back($2);
            $$ = new Argument(Argument::COMPOSITE, parts);
        }
    }
    ;

atom:
    WORD        { $$ = new Argument(Argument::WORD, $1); free($1); }
    | STRING    { $$ = new Argument(Argument::STRING, $1); free($1); }
    | STRING_SEGMENT { $$ = new Argument(Argument::STRING, $1); free($1); }
    | NUMBER    {
        char buf[32];
        sprintf(buf, "%d", $1);
        $$ = new Argument(Argument::WORD, strdup(buf));
    }
    | VARIABLE  { $$ = new Argument(Argument::VARIABLE, $1); free($1); }
    | TOKEN_CAT   { $$ = new Argument(Argument::WORD, $1); free($1); }
    | TOKEN_ECHO  { $$ = new Argument(Argument::WORD, $1); free($1); }
    | TOKEN_WC    { $$ = new Argument(Argument::WORD, $1); free($1); }
    | TOKEN_PWD   { $$ = new Argument(Argument::WORD, $1); free($1); }
    | TOKEN_EXIT  { $$ = new Argument(Argument::WORD, $1); free($1); }
    | TOKEN_GREP  { $$ = new Argument(Argument::WORD, $1); free($1); }
    | TOKEN_LS    { $$ = new Argument(Argument::WORD, $1); free($1); }
    | TOKEN_CD    { $$ = new Argument(Argument::WORD, $1); free($1); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error at line %d: %s\n", yylineno, s);
}
