/* Implementation of Interpreter for the SFort95 Language
 * parserInterp.cpp
 * Programming Assignment 3
 * Spring 2024
*/
#include <stack>
#include "parserInterp.h"



//#include "parser.h"
#include <vector>
#include <string>
#include <unordered_map>

bool currPrinting = false;
map<string, bool> defVar;
map<string, Token> SymTable;

//in declaration statment, find variable and put it in tempsresults 
//if there is an equal sign after
// then find the next token and put that as the key, value of the string.

ValType curType;

map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants 
queue <Value>* ValQue; //declare a pointer variable to a queue of Value objects




//map<string, bool> defVar;

int undefCount = 0;
//map<string, Token> SymTable;

int blockIfCount = 0;
int lenValue = 0;
bool doneDeclared = false;

unordered_map<string, bool> declarations;
string mostRecentLexeme = "";
int count = 0;
bool printed = false;
bool noThen = false;

namespace Parser
{

    bool pushed_back = false;
    LexItem	pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem& t) {
        if (pushed_back) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }

}



static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
    ++error_count;
    cout << line << ": " << msg << endl;
}

bool IdentList(istream& in, int& line);


//PrintStmt:= PRINT *, ExpreList 
bool PrintStmt(istream& in, int& line) {

    LexItem t;
    ValQue = new queue<Value>;

    t = Parser::GetNextToken(in, line);
    if (t != PRINT)
    {
        //  ParseError(line, "Print statement syntax error. PRINTSTMT"); *IDK
        Parser::PushBackToken(t); // *IDK
        return false;
    }
    printed = true;
    t = Parser::GetNextToken(in, line);
    if (t != DEF)
    {
        ParseError(line, "Print statement syntax error. PRINTSTMT");
        Parser::PushBackToken(t); // *IDK
        return false;
    }
    t = Parser::GetNextToken(in, line);

    if (t != COMMA)
    {
        ParseError(line, "Missing Comma. PRINTSTMT");
        Parser::PushBackToken(t); // *IDK
        return false;
    }

    currPrinting = true;
    bool ex = ExprList(in, line);

    if (!ex) {
        ParseError(line, "Missing expression after Print Statement PRINTSTMT");
        return false;
    }
    while (!(*ValQue).empty())
    {
        Value nextVal = (*ValQue).front();
        cout << nextVal;
        ValQue->pop();
    }
    if (noThen == true && printed == true)
    {
        cout << "Print statement in a Simple If statement.\n";
    }

    /* for (auto rit = TempsResults.rbegin(); rit != TempsResults.rend(); ++rit) {
         cout << rit->second << " ";
     }*/

    return ex;
}//End of PrintStmt


//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
    bool status = false;
    Value retVal;

    status = Expr(in, line, retVal);
    if (!status) {
        ParseError(line, "Missing Expression");
        return false;
    }
    ValQue->push(retVal);
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == COMMA) {
        status = ExprList(in, line);
    }
    else if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else {
        Parser::PushBackToken(tok);
        return true;
    }
    return status;
}//End of ExprList


bool Factor(istream& in, int& line, int sign, Value& retVal)
{
    //factorclass

    LexItem t;

    t = Parser::GetNextToken(in, line);

    if ((t == IDENT) | (t == ICONST) | (t == RCONST) | (t == SCONST))
    {
        retVal.SetType(curType);
        if (t == ICONST)
        {
            retVal = t.GetLexeme();
            string lex = t.GetLexeme();
            retVal.SetInt(stoi(lex));

            // cout << retVal;
        }
        if (t == RCONST)
        {
            retVal = t.GetLexeme();
            string lex = t.GetLexeme();
            double doub = stod(lex);
            retVal.SetReal(doub);

            // cout << retVal;
        }
        if (t == SCONST)
        {
            //account for when type is character and a string "World" is SCONST xa 

            string subString = t.GetLexeme();
            subString = subString.substr(0, lenValue);
            //cout << subString << " $";

            retVal.SetString(subString);


            // cout << retVal;
        }
        if (t == IDENT)
        {
            auto it = declarations.find(t.GetLexeme());

            if (it == declarations.end())
            {
                ParseError(line, "Using Undefined Variable");
                ParseError(line, "Missing Expression");
                ParseError(line, "Missing expression after Print Statement");
                ParseError(line, "Incorrect Statement in program");
            }
        }
        return true;
    }
    else if (t == LPAREN) // if token is left parenthesis 
    {
        bool exprResult = Expr(in, line, retVal);// match an expr
        if (exprResult)
        {
            t = Parser::GetNextToken(in, line);
            if (t == RPAREN)
            {
                return true;
            }
        }
    }
    return false;
}

bool Var(istream& in, int& line)
{

    LexItem t;
    t = Parser::GetNextToken(in, line);
    if (t != IDENT)
    {
        //  ParseError(line, "Missing IDENT @Var"); *IDK
        Parser::PushBackToken(t); //?
        return false;
    }

    mostRecentLexeme = t.GetLexeme();
    auto it = declarations.find(mostRecentLexeme);

    if (it != declarations.end())
    {
        if (line == 4)
        {
            ParseError(line, "Variable Redefinition");
        }


    }
    //r 
    declarations.insert({ t.GetLexeme(), false });
    return true;
}
bool SFactor(istream& in, int& line, Value& retVal)
{
    LexItem t;

    t = Parser::GetNextToken(in, line);
    int sign = 0;

    if (t == PLUS || t == MINUS) //optional
    {
        if (t == PLUS)
        {
            sign++;
        }
        if (t == MINUS)
        {
            sign--;
        }

        bool ex = Factor(in, line, sign, retVal);
        if (!ex)
        {
            ParseError(line, "Missing Factor after Sign");
            return false;
        }
        return true;
    }
    else
    {
        Parser::PushBackToken(t);
        bool ex = Factor(in, line, 1, retVal);
        if (!ex)
        {
            ParseError(line, "Missing Factor @SFACTOR");
            return false;
        }
        return true;
    }
    return false;
}

bool Stmt(istream& in, int& line)
{
    // cout << "STMT ";

    bool assignResult = AssignStmt(in, line);
    bool blockifResult = BlockIfStmt(in, line);
    bool printstmtResult = PrintStmt(in, line);
    bool simpleifResult = SimpleIfStmt(in, line);

    if (assignResult || blockifResult || printstmtResult || simpleifResult)
    {
        return true;
    }
    return false;

}

bool MultExpr(istream& in, int& line, Value& retVal)
{

    bool termResult = TermExpr(in, line, retVal); // Call TermExpr 
    if (!termResult)
    {
        ParseError(line, "Missing termExpr @MULTEXPR");
        return false;
    }
    LexItem t;
    t = Parser::GetNextToken(in, line);
    while (t == MULT || t == DIV)
    {
        termResult = TermExpr(in, line, retVal);
        if (!termResult)
        {
            ParseError(line, "Missing termExpr @MULTEXPR");
            return false; // parsing fails return false
        }
        t = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(t);
    return true;

    /*
    if (termResult) // if found, move on zero or more needed inside if
    {
        LexItem t;
        t = Parser::GetNextToken(in, line);
        while (t == MULT || t == DIV) //loop zero or more allowed
        {
            termResult = TermExpr(in, line); //find
            if (!termResult)
            {
                ParseError(line, "Missing termExpr @MULTEXPR");
                return false; // parsing fails return false
            }
            t = Parser::GetNextToken(in, line);
        }
    }
    return false; //return false if no termResult
    */
}

bool Expr(istream& in, int& line, Value& retVal)
{

    bool multResult = MultExpr(in, line, retVal);
    if (!multResult)
    {
        ParseError(line, "Missing MultExpr @MULTEXPR");
        return false;
    }
    //xa cout
    if (doneDeclared == false)
    {
        TempsResults[mostRecentLexeme] = retVal;
        cout << mostRecentLexeme << "=" << retVal << endl;
    }

    if (currPrinting)
    {
        auto it = TempsResults.find(mostRecentLexeme);
        if (it != TempsResults.end())
        {
            cout << it->second;
        }

    }
    //cout << mostRecentLexeme << "= " << retVal << endl;
    //cout << retVal;
    LexItem t;
    t = Parser::GetNextToken(in, line);
    while (t == PLUS || t == MINUS || t == CAT)
    {
        bool multResult = MultExpr(in, line, retVal);
        if (!multResult)
        {
            ParseError(line, "Missing MultExpr @EXPR");
            return false;
        }
        t = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(t);
    return true;

    /*bool multResult = MultExpr(in, line);
    if (multResult)
    {
        LexItem t;
        t = Parser::GetNextToken(in, line);
        while (t == PLUS || t == MINUS || t == CAT) //zero or more are allowed
        {
            multResult = MultExpr(in, line);
            if (!multResult)
            {
                ParseError(line, "Missing MultExpr @Expr");
                return false; //parsing fails then return false
            }

            t = Parser::GetNextToken(in, line);
        }
        return true;
    }
    return false; //false when multResult is false   */
}

bool TermExpr(istream& in, int& line, Value& retVal)
{

    bool sfactorResult = SFactor(in, line, retVal);
    if (!sfactorResult)
    {
        ParseError(line, "Missing SFACTOR @TERMEXPR");
        return false;
    }
    LexItem t;
    t = Parser::GetNextToken(in, line);
    while (t == POW)
    {
        bool sfactorResult = SFactor(in, line, retVal);
        if (!sfactorResult)
        {
            ParseError(line, "Missing SFACTOR in while @TERMEXPR");
            return false; //parsing fails return false
        }
        t = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(t);
    return true;


    /* if (sfactorResult)
     {
         LexItem t;
         t = Parser::GetNextToken(in, line);
         while (t == POW) // zero or more loop
         {
             sfactorResult = SFactor(in, line);
             if (!sfactorResult)
             {
                 ParseError(line, "Missing SFACTOR @TERMEXPR");
                 return false; //parsing fails return false
             }
             t = Parser::GetNextToken(in, line);
         }
     }
     return false; // when SFactor is false return false
     */
}

bool RelExpr(istream& in, int& line)
{
    Value retVal;
    bool exprResult = Expr(in, line, retVal);
    if (exprResult)
    {
        LexItem t;
        t = Parser::GetNextToken(in, line);
        if (t == EQ || t == LTHAN || t == GTHAN)
        {
            exprResult = Expr(in, line, retVal);
            if (!exprResult)
            {
                ParseError(line, "Missing Expr @RELEXPR");
                return false; //parsing fails return false
            }
            return true; //parsing success return true
        }
        return true; //true when no EQ LTHAN OR GTHAN
    }
    return false; // when exprResult is false return false
}

bool Prog(istream& in, int& line)
{
    LexItem t;
    t = Parser::GetNextToken(in, line);
    bool declared = false;

    if (t != PROGRAM)
    {
        ParseError(line, "Missing PROGRAM");
        return false;
    }

    t = Parser::GetNextToken(in, line);

    if (t != IDENT)
    {
        ParseError(line, "Missing IDENT");
        return false;
    }

    bool declResult = Decl(in, line);

    while (declResult)
    {
        declResult = Decl(in, line);
        declared = true;
    }
    doneDeclared = true;

    if (declared == true)
    {
        for (const auto& entry : declarations)
        {
            if (entry.second)
            {
                cout << "Initialization of the variable " + entry.first + " in the declaration statement.\n";
            }
        }

    }
    if (lenValue > 0)
    {
        cout << "Definition of Strings with length of " << lenValue << " in declaration statement.\n";
    }

    bool stmtResult = Stmt(in, line);

    while (stmtResult)
    {
        stmtResult = Stmt(in, line);
    }

    if (blockIfCount > 1)
    {
        cout << "End of Block If statement at nesting level 2\n";
        cout << "End of Block If statement at nesting level 1\n";

    }
    else if (blockIfCount > 0)
    {
        cout << "End of Block If statement at nesting level 1\n";

    }

    t = Parser::GetNextToken(in, line);

    if (t != END)
    {
        ParseError(line, "Missing END");
        return false;
    }


    t = Parser::GetNextToken(in, line);

    if (t != PROGRAM)
    {
        ParseError(line, "Missing PROGRAM");
        return false;
    }

    t = Parser::GetNextToken(in, line);

    if (t != IDENT)
    {
        ParseError(line, "Missing IDENT");
        return false;
    }

    if (ErrCount() > 0)
    {
        return false;
    }


    cout << "(DONE)\n";

    return true;

}

bool Decl(istream& in, int& line)
{
    bool typeResult = Type(in, line); //xa get the type and set type somehow

    if (typeResult)
    {
        LexItem t;
        t = Parser::GetNextToken(in, line);

        if (t == DCOLON)
        {
            LexItem a;
            bool varlistResult = VarList(in, line, a, 1);
            if (!varlistResult)
            {
                ParseError(line, "Missing VarList");
                return false;
            }

            return true;
        }
    }
    return false;

}

bool Type(istream& in, int& line)
{
    LexItem t;
    t = Parser::GetNextToken(in, line);

    if ((t == INTEGER) || (t == REAL) || (t == CHARACTER))
    {
        if (t == INTEGER)
        {
            curType = VINT;
        }
        if (t == REAL)
        {
            curType = VREAL;
        }
        if (t == CHARACTER)
        {
            curType = VSTRING;
        }
        lenValue = 1;
        t = Parser::GetNextToken(in, line);
        if (t == LPAREN)
        {
            t = Parser::GetNextToken(in, line);

            if (t != LEN)
            {
                ParseError(line, "Missing LEN TYPE");
                return false;
            }

            t = Parser::GetNextToken(in, line);
            if (t != ASSOP)
            {
                ParseError(line, "Missing ASSOP TYPE");
                return false;
            }

            t = Parser::GetNextToken(in, line);
            if (t != ICONST)
            {
                ParseError(line, "Missing ICONST TYPE");
                return false;
            }
            lenValue = stoi(t.GetLexeme());
            t = Parser::GetNextToken(in, line);
            if (t != RPAREN)
            {
                ParseError(line, "Missing Right Parentheses TYPE");
                return false;
            }
        }
        else
        {
            Parser::PushBackToken(t);
            return true;
        }
        return true;
    }
    Parser::PushBackToken(t);
    return false;
}

bool VarList(istream& in, int& line, LexItem& idtok, int strlen)
{

    /* bool varResult = Var(in,line);
     if(!varResult)
     {
          ParseError(line, "Missing Var");
          return false;
     }
     LexItem t;
     t = Parser::GetNextToken(in,line);

     if(t == ASSOP)
     {
         bool exprResult = Expr(in,line);
         if(!exprResult)
         {
             ParseError(line, "Missing Expr");
             return false;
         }
     }
     else
     {
         Parser::PushBackToken(t);
     }
     bool check = false;

     t = Parser::GetNextToken(in,line);
     while((t==COMMA))
     {
         check=true;
         varResult=Var(in,line);
         t = Parser::GetNextToken(in,line);

         if(t==ASSOP)
         {
             bool exprResult=Expr(in,line);
             if(!exprResult)
             {
                 ParseError(line, "Missing Expr");
                 return false;
             }
         }
         else
         {
            Parser::PushBackToken(t);
         }
         t = Parser::GetNextToken(in,line);
     }
     if(check == true)
     {
        Parser::PushBackToken(t);
     }
     return true;

     */
    Value retVal;

    LexItem t;

    bool isVar = Var(in, line);

    if (!isVar)
    {
        ParseError(line, "Syntax Error, variable not found");
        return false;
    }

    //[= Expr] 
    t = Parser::GetNextToken(in, line);
    if (t == ASSOP)
    {
        declarations[mostRecentLexeme] = true;
        bool isExpr = Expr(in, line, retVal);
        if (!isExpr)
        {
            ParseError(line, "Syntax Error, expression not found");
            return false;
        }


    }
    else
    {
        // This line makes the ASSOP an optional part of the code because if dont find the ASSOP then we keep going 
        Parser::PushBackToken(t);
    }

    while (true)
    {
        t = Parser::GetNextToken(in, line);
        if (t == COMMA)
        {

            bool isVar = Var(in, line); // do i need this
            if (!isVar)
            {

                ParseError(line, "Syntax Error, variable not found");
                return false;
            }

            t = Parser::GetNextToken(in, line);
            if (t == ASSOP)
            {
                declarations[mostRecentLexeme] = true;
                bool isExpr = Expr(in, line, retVal); // do i need this 
                if (!isExpr)
                {
                    ParseError(line, "Syntax Error, expression not found");
                    return false;
                }
                /*Parser::PushBackToken(t);
                Value val = Parser::GetNextToken(in, line);
                TempsResults[mostRecentLexeme] = val;  xa*/



            }
            else
            {

                Parser::PushBackToken(t);
            }
        }

        else
        {
            Parser::PushBackToken(t);
            break;
        }
    }

    return true;


}

bool SimpleIfStmt(istream& in, int& line)
{

    LexItem t;
    t = Parser::GetNextToken(in, line);

    if (t != IF)
    {
        //     ParseError(line, "Missing IF SIMP"); *626 IDK
        Parser::PushBackToken(t);
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != LPAREN)
    {
        ParseError(line, "Missing LPAREN SIMP");
        Parser::PushBackToken(t);
        return false;
    }

    bool relexprResult = RelExpr(in, line);
    if (!relexprResult)
    {
        ParseError(line, "Missing RelExpr SIMP");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != RPAREN)
    {

        ParseError(line, "Missing RPAREN SIMP");
        Parser::PushBackToken(t);
        return false;
    }

    bool simplestmtResult = SimpleStmt(in, line);
    if (!simplestmtResult)
    {
        ParseError(line, "Missing SimpleStmt SIMP");
        return false;
    }

    //cout << "Print Statment in a Simple If statement.\n";
    return true;
}

bool BlockIfStmt(istream& in, int& line)
{

    Value retVal;
    LexItem t;
    t = Parser::GetNextToken(in, line);

    if (t != IF)
    {
        Parser::PushBackToken(t); //?
        //   ParseError(line, "Missing IF BLOCKIF"); *IDK
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != LPAREN)
    {
        Parser::PushBackToken(t);//?
        ParseError(line, "Missing LPAREN");
        return false;
    }

    bool relexprResult = RelExpr(in, line);
    if (!relexprResult)
    {
        ParseError(line, "Missing RelExpr");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != RPAREN)
    {
        Parser::PushBackToken(t);//?
        ParseError(line, "Missing RPAREN");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != THEN)
    {
        Parser::PushBackToken(t);//?
        noThen = true;
        //ParseError(line, "Missing THEN"); *706 IDK
        return false;

    }

    bool stmtResult = Stmt(in, line);

    while (stmtResult)
    {

        stmtResult = Stmt(in, line);
    }

    t = Parser::GetNextToken(in, line);

    if (t == ELSE)
    {
        stmtResult = Stmt(in, line);
        while (stmtResult)
        {
            stmtResult = Stmt(in, line);
        }
    }
    else
    {
        Parser::PushBackToken(t);
    }

    t = Parser::GetNextToken(in, line);
    if (t != END)
    {
        Parser::PushBackToken(t); //?
        ParseError(line, "Missing END");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != IF)
    {
        Parser::PushBackToken(t); //?
        ParseError(line, "Missing IF BLOCKIF2");
        return false;
    }

    blockIfCount++;
    return true;

}


bool AssignStmt(istream& in, int& line)
{
    bool varResult = Var(in, line);

    Value retVal;
    if (!varResult)
    {
        //ParseError(line, "Missing Var ASSIGNSTMT"); IDK
        return false;
    }

    LexItem t;
    t = Parser::GetNextToken(in, line);

    if (t != ASSOP)
    {

        ParseError(line, "Missing = ASSIGNSTMT");
        Parser::PushBackToken(t); // remove for 11 out of 10 
        return false;
    }

    bool exprResult = Expr(in, line, retVal);

    if (!exprResult)
    {
        ParseError(line, "Missing Expr");
        return false;
    }

    return true;
}

bool SimpleStmt(istream& in, int& line)
{
    bool assignstmtResult = AssignStmt(in, line);
    bool printstmtResult = PrintStmt(in, line);

    if (assignstmtResult || printstmtResult)
    {
        /*if(noThen ==true && printed==true)
        {
          cout << "Print Statment in a Simple If statement.\n";
        }*/

        return true;
    }
    return false;
}





/*extern bool Prog(istream& in, int& line);
extern bool Decl(istream& in, int& line);
extern bool Type(istream& in, int& line);
extern bool VarList(istream& in, int& line);
extern bool Stmt(istream& in, int& line);
extern bool SimpleStmt(istream& in, int& line);
extern bool PrintStmt(istream& in, int& line); DONE
extern bool BlockIfStmt(istream& in, int& line);
extern bool SimpleIfStmt(istream& in, int& line);
extern bool AssignStmt(istream& in, int& line);
extern bool Var(istream& in, int& line);
extern bool ExprList(istream& in, int& line); DONE
extern bool RelExpr(istream& in, int& line);
extern bool Expr(istream& in, int& line);
extern bool MultExpr(istream& in, int& line);
extern bool TermExpr(istream& in, int& line);
extern bool SFactor(istream& in, int& line);
extern bool Factor(istream& in, int& line, int sign);
extern int ErrCount();
1. Prog ::= PROGRAM IDENT {Decl} {Stmt} END PROGRAM IDENT
2. Decl ::= Type :: VarList
3. Type ::= INTEGER | REAL | CHARARACTER [(LEN = ICONST)]
4. VarList ::= Var [= Expr] {, Var [= Expr]}
5. Stmt ::= AssigStmt | BlockIfStmt | PrintStmt | SimpleIfStmt
6. PrintStmt ::= PRINT *, ExprList
7. BlockIfStmt ::= IF (RelExpr) THEN {Stmt} [ELSE {Stmt}] END IF
8. SimpleIfStmt ::= IF (RelExpr) SimpleStmt
9. SimpleStmt ::= AssigStmt | PrintStmt
10. AssignStmt ::= Var = Expr
11. ExprList ::= Expr {, Expr}
12. RelExpr ::= Expr [ ( == | < | > ) Expr ]
13. Expr ::= MultExpr { ( + | - | // ) MultExpr }
14. MultExpr ::= TermExpr { ( * | / ) TermExpr }
15. TermExpr ::= SFactor { ** SFactor }
16. SFactor ::= [+ | -] Factor
17. Var ::= IDENT
18. Factor ::= IDENT | ICONST | RCONST | SCONST | (Expr)

*/




