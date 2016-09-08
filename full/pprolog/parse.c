/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 3 "parse.y"

#include "ptc.h"
#include <stdlib.h>

#define YYDEBUG 1


/* Line 268 of yacc.c  */
#line 79 "parse.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENT = 258,
     MONOP = 259,
     MULOP = 260,
     ADDOP = 261,
     RELOP = 262,
     WRITE = 263,
     NUMBER = 264,
     STRING = 265,
     CHAR = 266,
     ASSIGN = 267,
     DOTDOT = 268,
     NEQ = 269,
     GEQ = 270,
     LEQ = 271,
     AND = 272,
     ARRAY = 273,
     BEGN = 274,
     CASE = 275,
     CONST = 276,
     DIV = 277,
     DO = 278,
     DOWNTO = 279,
     IF = 280,
     ELSE = 281,
     END = 282,
     FOR = 283,
     FORWARD = 284,
     FUNC = 285,
     GOTO = 286,
     LABEL = 287,
     MOD = 288,
     NOT = 289,
     OF = 290,
     OR = 291,
     PROC = 292,
     PROGRAM = 293,
     RECORD = 294,
     REPEAT = 295,
     SHL = 296,
     SHR = 297,
     THEN = 298,
     TO = 299,
     TYPE = 300,
     UNTIL = 301,
     VAR = 302,
     WHILE = 303,
     CALL = 304,
     CONS = 305,
     SUB = 306,
     DOT = 307,
     FIELD = 308,
     RANGE = 309,
     PRIMTYPE = 310,
     CPARAM = 311,
     VPARAM = 312,
     NEG = 313,
     BITAND = 314,
     BITOR = 315,
     LIBCALL = 316,
     APARAM = 317,
     POINTER = 318,
     ADDRESS = 319,
     BUILTIN = 320,
     UMINUS = 321,
     N_TOKENS = 322
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 10 "parse.y"

	tree E;
	type T;
	ident I;
	literal L;



/* Line 293 of yacc.c  */
#line 191 "parse.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 203 "parse.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   280

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  78
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  68
/* YYNRULES -- Number of rules.  */
#define YYNRULES  131
/* YYNRULES -- Number of states.  */
#define YYNSTATES  252

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      72,    73,     2,     2,    75,    67,    70,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    74,    71,
       2,    66,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    76,     2,    77,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    68,    69
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     8,    13,    14,    18,    19,    22,    25,
      26,    29,    33,    36,    39,    41,    44,    46,    49,    54,
      56,    59,    64,    66,    69,    74,    75,    76,    83,    87,
      88,    94,    95,   103,   107,   108,   112,   114,   118,   122,
     127,   128,   133,   137,   139,   143,   145,   146,   151,   152,
     155,   156,   161,   162,   163,   168,   172,   175,   178,   181,
     182,   186,   187,   193,   194,   200,   201,   211,   212,   222,
     223,   232,   233,   239,   240,   241,   246,   247,   252,   253,
     254,   255,   259,   261,   265,   266,   271,   273,   274,   276,
     280,   282,   286,   287,   291,   293,   297,   299,   303,   305,
     307,   309,   314,   318,   321,   324,   328,   332,   336,   340,
     344,   345,   349,   351,   355,   357,   362,   366,   368,   372,
     379,   380,   386,   388,   390,   394,   398,   400,   402,   404,
     407,   409
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      79,     0,    -1,    80,    82,   101,    70,    -1,    38,     3,
      81,    71,    -1,    -1,    72,   130,    73,    -1,    -1,    82,
      84,    -1,    82,    85,    -1,    -1,    83,    84,    -1,    32,
     131,    71,    -1,    21,    86,    -1,    47,    90,    -1,    92,
      -1,    45,    88,    -1,    87,    -1,    86,    87,    -1,     3,
      66,   144,    71,    -1,    89,    -1,    88,    89,    -1,     3,
      66,   139,    71,    -1,    91,    -1,    90,    91,    -1,   130,
      74,   139,    71,    -1,    -1,    -1,    95,    93,    83,    94,
     103,    71,    -1,    95,    29,    71,    -1,    -1,    37,     3,
      96,    98,    71,    -1,    -1,    30,     3,    97,    98,    74,
     141,    71,    -1,    30,     3,    71,    -1,    -1,    72,    99,
      73,    -1,   100,    -1,    99,    71,   100,    -1,   130,    74,
     141,    -1,    47,   130,    74,   141,    -1,    -1,    19,   102,
     104,    27,    -1,    19,   104,    27,    -1,   105,    -1,   104,
      71,   105,    -1,   110,    -1,    -1,    19,   106,   104,    27,
      -1,    -1,   108,   110,    -1,    -1,    19,   109,   104,    27,
      -1,    -1,    -1,     9,    74,   111,   105,    -1,   138,    12,
     135,    -1,     3,   136,    -1,     8,   132,    -1,    31,     9,
      -1,    -1,    25,   112,   118,    -1,    -1,    48,   135,    23,
     113,   107,    -1,    -1,    40,   114,   104,    46,   135,    -1,
      -1,    28,   138,    12,   135,    44,   135,    23,   115,   107,
      -1,    -1,    28,   138,    12,   135,    24,   135,    23,   116,
     107,    -1,    -1,    20,   135,    35,   117,   126,   129,   124,
      27,    -1,    -1,   135,    43,   119,   107,   120,    -1,    -1,
      -1,    26,   123,   121,   107,    -1,    -1,    26,    25,   122,
     118,    -1,    -1,    -1,    -1,    26,   125,   104,    -1,   127,
      -1,   126,    71,   127,    -1,    -1,   145,    74,   128,   105,
      -1,    71,    -1,    -1,     3,    -1,   130,    75,     3,    -1,
       9,    -1,   131,    75,     9,    -1,    -1,    72,   133,    73,
      -1,   134,    -1,   133,    75,   134,    -1,   135,    -1,   135,
      74,   135,    -1,   138,    -1,     9,    -1,    10,    -1,     3,
      72,   137,    73,    -1,    72,   135,    73,    -1,     4,   135,
      -1,    67,   135,    -1,   135,     5,   135,    -1,   135,     6,
     135,    -1,   135,    67,   135,    -1,   135,     7,   135,    -1,
     135,    66,   135,    -1,    -1,    72,   137,    73,    -1,   135,
      -1,   137,    75,   135,    -1,     3,    -1,   138,    76,   135,
      77,    -1,   138,    70,     3,    -1,   141,    -1,   144,    13,
     144,    -1,    18,    76,   139,    77,    35,   139,    -1,    -1,
      39,   140,   142,   129,    27,    -1,     3,    -1,   143,    -1,
     142,    71,   143,    -1,   130,    74,   139,    -1,     9,    -1,
      10,    -1,     3,    -1,    67,   144,    -1,   144,    -1,   145,
      75,   144,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    45,    45,    47,    51,    52,    54,    55,    56,    58,
      59,    61,    62,    63,    65,    66,    68,    69,    71,    73,
      74,    76,    83,    84,    86,    91,    93,    90,    97,   100,
     100,   102,   102,   104,   106,   107,   109,   110,   112,   114,
     117,   117,   120,   125,   126,   128,   129,   129,   131,   131,
     132,   132,   134,   135,   135,   136,   137,   139,   141,   143,
     143,   145,   144,   148,   148,   153,   152,   156,   155,   159,
     158,   166,   165,   170,   171,   171,   172,   172,   174,   176,
     177,   177,   179,   180,   182,   182,   185,   185,   187,   188,
     190,   191,   193,   194,   196,   197,   199,   200,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     216,   217,   219,   220,   223,   224,   225,   227,   228,   229,
     231,   231,   234,   236,   237,   239,   241,   242,   243,   244,
     249,   250
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENT", "MONOP", "MULOP", "ADDOP",
  "RELOP", "WRITE", "NUMBER", "STRING", "CHAR", "ASSIGN", "DOTDOT", "NEQ",
  "GEQ", "LEQ", "AND", "ARRAY", "BEGN", "CASE", "CONST", "DIV", "DO",
  "DOWNTO", "IF", "ELSE", "END", "FOR", "FORWARD", "FUNC", "GOTO", "LABEL",
  "MOD", "NOT", "OF", "OR", "PROC", "PROGRAM", "RECORD", "REPEAT", "SHL",
  "SHR", "THEN", "TO", "TYPE", "UNTIL", "VAR", "WHILE", "CALL", "CONS",
  "SUB", "DOT", "FIELD", "RANGE", "PRIMTYPE", "CPARAM", "VPARAM", "NEG",
  "BITAND", "BITOR", "LIBCALL", "APARAM", "POINTER", "ADDRESS", "BUILTIN",
  "'='", "'-'", "UMINUS", "N_TOKENS", "'.'", "';'", "'('", "')'", "':'",
  "','", "'['", "']'", "$accept", "program", "prog.heading", "file.args",
  "glo.decls", "local.decls", "decl", "glo.decl", "const.decls",
  "const.decl", "type.decls", "type.decl", "var.decls", "var.decl",
  "proc.decl", "$@1", "$@2", "proc.heading", "$@3", "$@4", "formal.params",
  "formal.decls", "formal.decl", "main.prog", "$@5", "body", "stmt.list",
  "stmt", "$@6", "sub.stmt", "$@7", "$@8", "basic.stmt", "$@9", "$@10",
  "$@11", "$@12", "$@13", "$@14", "$@15", "if.tail", "$@16", "else.part",
  "$@17", "$@18", "marker", "default.part", "$@19", "cases", "case",
  "$@20", "opt.semi", "ident.list", "number.list", "write.params",
  "write.list", "write.item", "expr", "actuals", "expr.list", "variable",
  "type", "$@21", "type.ident", "fields", "field.decl", "const",
  "const.list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,    61,    45,   321,   322,
      46,    59,    40,    41,    58,    44,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    78,    79,    80,    81,    81,    82,    82,    82,    83,
      83,    84,    84,    84,    85,    85,    86,    86,    87,    88,
      88,    89,    90,    90,    91,    93,    94,    92,    92,    96,
      95,    97,    95,    95,    98,    98,    99,    99,   100,   100,
     102,   101,   103,   104,   104,   105,   106,   105,   108,   107,
     109,   107,   110,   111,   110,   110,   110,   110,   110,   112,
     110,   113,   110,   114,   110,   115,   110,   116,   110,   117,
     110,   119,   118,   120,   121,   120,   122,   120,   123,   124,
     125,   124,   126,   126,   128,   127,   129,   129,   130,   130,
     131,   131,   132,   132,   133,   133,   134,   134,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     136,   136,   137,   137,   138,   138,   138,   139,   139,   139,
     140,   139,   141,   142,   142,   143,   144,   144,   144,   144,
     145,   145
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     4,     4,     0,     3,     0,     2,     2,     0,
       2,     3,     2,     2,     1,     2,     1,     2,     4,     1,
       2,     4,     1,     2,     4,     0,     0,     6,     3,     0,
       5,     0,     7,     3,     0,     3,     1,     3,     3,     4,
       0,     4,     3,     1,     3,     1,     0,     4,     0,     2,
       0,     4,     0,     0,     4,     3,     2,     2,     2,     0,
       3,     0,     5,     0,     5,     0,     9,     0,     9,     0,
       8,     0,     5,     0,     0,     4,     0,     4,     0,     0,
       0,     3,     1,     3,     0,     4,     1,     0,     1,     3,
       1,     3,     0,     3,     1,     3,     1,     3,     1,     1,
       1,     4,     3,     2,     2,     3,     3,     3,     3,     3,
       0,     3,     1,     3,     1,     4,     3,     1,     3,     6,
       0,     5,     1,     1,     3,     3,     1,     1,     1,     2,
       1,     3
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     6,     4,     1,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     7,     8,    14,    25,
       0,    88,     0,     3,    52,     0,    12,    16,    31,    90,
       0,    29,     0,    15,    19,    13,    22,     0,     0,     9,
       2,     5,     0,   110,    92,     0,    46,     0,    59,     0,
       0,    63,     0,     0,    43,    45,     0,     0,    17,    33,
      34,    11,     0,    34,     0,    20,    23,     0,    28,    26,
      89,     0,    56,     0,    57,    53,    52,   114,     0,    99,
     100,     0,     0,     0,    98,     0,   114,     0,    58,    52,
       0,    41,    52,     0,     0,     0,   128,   126,   127,     0,
       0,     0,     0,    91,     0,   122,     0,   120,     0,   117,
       0,     0,    10,     0,   112,     0,     0,    94,    96,    52,
       0,     0,   103,   104,     0,     0,     0,     0,    69,     0,
       0,    60,     0,     0,     0,    61,    44,    55,   116,     0,
     129,    18,     0,     0,    36,     0,     0,    30,     0,     0,
      21,     0,    24,    52,     0,   111,     0,    93,     0,     0,
      54,    47,     0,   102,   105,   106,   108,     0,   109,   107,
      71,     0,     0,    48,   115,     0,     0,    35,     0,   122,
       0,     0,     0,    87,   123,   118,     0,    27,   113,    95,
      97,   101,    87,    82,   130,     0,    48,     0,     0,    64,
      50,    62,    52,     0,    37,    38,    32,     0,     0,    86,
       0,    42,    86,    79,    84,     0,    73,     0,     0,    52,
      49,    39,     0,   125,   124,   121,    83,    80,     0,    52,
     131,    78,    72,    67,    65,     0,   119,    52,    70,    85,
      76,    74,    48,    48,    51,    81,     0,    48,    68,    66,
      77,    75
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     8,     6,    69,    16,    17,    26,    27,
      33,    34,    35,    36,    18,    39,   113,    19,    63,    60,
     102,   143,   144,    20,    24,   154,    53,    54,    76,   201,
     202,   219,    55,   119,    85,   173,    89,   243,   242,   167,
     131,   196,   232,   247,   246,   241,   228,   237,   192,   193,
     229,   210,    37,    30,    74,   116,   117,   114,    72,   115,
      84,   108,   149,   109,   183,   184,   110,   195
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -131
static const yytype_int16 yypact[] =
{
     -31,    21,    43,  -131,   -41,  -131,   229,    59,   -16,  -131,
      69,    73,    88,    97,   104,    59,  -131,  -131,  -131,    87,
      57,  -131,    31,  -131,   214,    91,    69,  -131,    64,  -131,
     -54,  -131,   107,   104,  -131,    59,  -131,    -8,    68,  -131,
    -131,  -131,   174,    33,   120,   123,  -131,    19,  -131,   185,
     190,  -131,    19,   -15,  -131,  -131,    41,    48,  -131,  -131,
     129,  -131,   201,   129,    51,  -131,  -131,    51,  -131,   143,
    -131,    19,  -131,    19,  -131,  -131,   214,   139,    19,  -131,
    -131,    19,    19,   118,   -50,    19,  -131,    52,  -131,   214,
     127,  -131,   214,    19,   209,    19,  -131,  -131,  -131,    48,
     147,    16,   146,  -131,   150,   213,   138,  -131,   156,  -131,
     215,   160,  -131,   218,   140,    92,   101,  -131,     4,   214,
       6,    19,  -131,  -131,    35,    19,    19,    19,  -131,    19,
      19,  -131,   136,    19,   -12,  -131,  -131,   140,  -131,     8,
    -131,  -131,    59,   125,  -131,    18,   232,  -131,    51,    59,
    -131,    48,  -131,   214,   161,  -131,    19,  -131,    19,    19,
    -131,  -131,   116,  -131,  -131,   233,    32,    48,    32,   233,
    -131,   114,    19,   221,  -131,    56,    16,  -131,   232,  -131,
     172,   169,    86,   178,  -131,  -131,    17,  -131,   140,  -131,
     140,  -131,   180,  -131,  -131,   134,   221,    19,    19,   140,
    -131,  -131,   216,   232,  -131,  -131,  -131,   217,    51,    59,
     226,  -131,    48,   231,  -131,    48,   234,   149,   163,   214,
    -131,  -131,    51,  -131,  -131,  -131,  -131,  -131,   228,   214,
    -131,   238,  -131,  -131,  -131,    23,  -131,   214,  -131,  -131,
    -131,  -131,   221,   221,  -131,   187,    19,   221,  -131,  -131,
    -131,  -131
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -131,  -131,  -131,  -131,  -131,  -131,   196,  -131,  -131,   241,
    -131,   235,  -131,   236,  -131,  -131,  -131,  -131,  -131,  -131,
     206,  -131,    94,  -131,  -131,  -131,   -71,   -89,  -131,   -60,
    -131,  -131,    70,  -131,  -131,  -131,  -131,  -131,  -131,  -131,
      27,  -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,    63,
    -131,    85,    -5,  -131,  -131,  -131,   121,   -46,  -131,   157,
     -24,   -59,  -131,  -130,  -131,    71,   -53,  -131
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -129
static const yytype_int16 yytable[] =
{
      56,    83,    22,   136,   100,   120,    90,     1,   111,   125,
     126,   127,    91,   125,   126,   127,   180,    61,   134,    21,
      94,    62,    77,    78,     4,    87,    95,   118,    79,    80,
     160,     7,   122,   161,   172,   123,   124,   125,   126,   132,
     125,   126,   127,     5,   211,  -114,   140,   137,   205,   139,
     244,    96,    56,    93,   105,    23,    92,    97,    98,    92,
      97,    98,    21,   142,   133,    56,    67,    42,    56,   106,
     129,   130,    25,   221,   129,   130,    28,    92,   159,   164,
     165,   166,   186,   168,   169,   174,    81,   171,    92,   181,
     107,    82,   178,    42,    92,    56,   145,    29,   185,   130,
      31,   129,   130,  -114,    41,    71,    42,    32,   163,  -114,
     188,    94,   118,   190,   194,    99,    38,    95,    99,   125,
     126,   127,    94,   125,   126,   127,   199,    40,    95,    56,
     203,    42,   125,   126,   127,    59,   216,   175,   197,    68,
     239,   125,   126,   127,   182,   125,   126,   127,   235,   223,
     135,   217,   218,   128,   125,   126,   127,    57,   198,   194,
     208,    42,   230,   236,    10,   155,   245,   156,   125,   126,
     127,   145,   233,    64,   157,    12,   158,    70,    56,   170,
     129,   130,   248,   249,   129,   130,   234,   251,    86,   191,
      15,   156,    73,   129,   130,    56,   176,    75,   177,    88,
     132,   101,   129,   130,   182,    56,   129,   130,   214,   215,
     103,   121,   138,    56,   148,   129,   130,    43,   141,    43,
     146,   147,    44,    45,    44,    45,  -128,   150,   151,   129,
     130,   152,   187,    46,    47,   179,    47,   153,   125,    48,
     200,    48,    49,   206,    49,    50,   207,    50,     9,   209,
      10,   212,   222,   225,    51,   238,    51,   227,    92,    11,
     231,    12,    52,   240,    52,   112,    13,    58,    65,   104,
     204,    66,   220,   250,    14,   226,    15,   213,   162,   189,
     224
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-131))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_uint8 yycheck[] =
{
      24,    47,     7,    92,    57,    76,    52,    38,    67,     5,
       6,     7,    27,     5,     6,     7,   146,    71,    89,     3,
      70,    75,     3,     4,     3,    49,    76,    73,     9,    10,
     119,    72,    78,    27,    46,    81,    82,     5,     6,    85,
       5,     6,     7,     0,    27,    12,    99,    93,   178,    95,
      27,     3,    76,    12,     3,    71,    71,     9,    10,    71,
       9,    10,     3,    47,    12,    89,    74,    75,    92,    18,
      66,    67,     3,   203,    66,    67,     3,    71,    74,   125,
     126,   127,   153,   129,   130,    77,    67,   133,    71,   148,
      39,    72,    74,    75,    71,   119,   101,     9,   151,    67,
       3,    66,    67,    70,    73,    72,    75,     3,    73,    76,
     156,    70,   158,   159,   167,    67,    29,    76,    67,     5,
       6,     7,    70,     5,     6,     7,   172,    70,    76,   153,
      74,    75,     5,     6,     7,    71,   196,   142,    24,    71,
     229,     5,     6,     7,   149,     5,     6,     7,   219,   208,
      23,   197,   198,    35,     5,     6,     7,    66,    44,   212,
      74,    75,   215,   222,    21,    73,   237,    75,     5,     6,
       7,   176,    23,    66,    73,    32,    75,     3,   202,    43,
      66,    67,   242,   243,    66,    67,    23,   247,     3,    73,
      47,    75,    72,    66,    67,   219,    71,    74,    73,     9,
     246,    72,    66,    67,   209,   229,    66,    67,    74,    75,
       9,    72,     3,   237,    76,    66,    67,     3,    71,     3,
      74,    71,     8,     9,     8,     9,    13,    71,    13,    66,
      67,    71,    71,    19,    20,     3,    20,    19,     5,    25,
      19,    25,    28,    71,    28,    31,    77,    31,    19,    71,
      21,    71,    35,    27,    40,    27,    40,    26,    71,    30,
      26,    32,    48,    25,    48,    69,    37,    26,    33,    63,
     176,    35,   202,   246,    45,   212,    47,   192,   121,   158,
     209
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    38,    79,    80,     3,     0,    82,    72,    81,    19,
      21,    30,    32,    37,    45,    47,    84,    85,    92,    95,
     101,     3,   130,    71,   102,     3,    86,    87,     3,     9,
     131,     3,     3,    88,    89,    90,    91,   130,    29,    93,
      70,    73,    75,     3,     8,     9,    19,    20,    25,    28,
      31,    40,    48,   104,   105,   110,   138,    66,    87,    71,
      97,    71,    75,    96,    66,    89,    91,    74,    71,    83,
       3,    72,   136,    72,   132,    74,   106,     3,     4,     9,
      10,    67,    72,   135,   138,   112,     3,   138,     9,   114,
     135,    27,    71,    12,    70,    76,     3,     9,    10,    67,
     144,    72,    98,     9,    98,     3,    18,    39,   139,   141,
     144,   139,    84,    94,   135,   137,   133,   134,   135,   111,
     104,    72,   135,   135,   135,     5,     6,     7,    35,    66,
      67,   118,   135,    12,   104,    23,   105,   135,     3,   135,
     144,    71,    47,    99,   100,   130,    74,    71,    76,   140,
      71,    13,    71,    19,   103,    73,    75,    73,    75,    74,
     105,    27,   137,    73,   135,   135,   135,   117,   135,   135,
      43,   135,    46,   113,    77,   130,    71,    73,    74,     3,
     141,   139,   130,   142,   143,   144,   104,    71,   135,   134,
     135,    73,   126,   127,   144,   145,   119,    24,    44,   135,
      19,   107,   108,    74,   100,   141,    71,    77,    74,    71,
     129,    27,    71,   129,    74,    75,   107,   135,   135,   109,
     110,   141,    35,   139,   143,    27,   127,    26,   124,   128,
     144,    26,   120,    23,    23,   104,   139,   125,    27,   105,
      25,   123,   116,   115,    27,   104,   122,   121,   107,   107,
     118,   107
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:

/* Line 1806 of yacc.c  */
#line 48 "parse.y"
    { emit("/* Program '%s' */\n\n", (yyvsp[(2) - (4)].I)->i_text);
		  emit("#include \"ptclib.i\"\n\n"); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 63 "parse.y"
    { emit("\n"); }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 71 "parse.y"
    { const_decl((yyvsp[(1) - (4)].I), (yyvsp[(3) - (4)].E)); }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 77 "parse.y"
    { declare(TYPE, (yyvsp[(1) - (4)].I), (yyvsp[(3) - (4)].T));
		  if ((yyvsp[(3) - (4)].T)->x_name == NULL) {
			gen_decl(TYPE, (yyvsp[(1) - (4)].I), (yyvsp[(3) - (4)].T));
			emit("\n");
			(yyvsp[(3) - (4)].T)->x_name = (yyvsp[(1) - (4)].I); } }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 87 "parse.y"
    { decl_list(VAR, (yyvsp[(1) - (4)].E), (yyvsp[(3) - (4)].T));
		  gen_decl_list(VAR, (yyvsp[(1) - (4)].E), (yyvsp[(3) - (4)].T)); }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 91 "parse.y"
    { gen_heading(); emit("{\n%i"); push_scope(); }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 93 "parse.y"
    { if (return_type != void_type) {
		    gen_decl(VAR, enter("__R__", BUILTIN), return_type);
		    emit("\n"); } }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 96 "parse.y"
    { pop_scope(); pop_scope(); }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 98 "parse.y"
    { gen_forward(); forward_def(); }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 100 "parse.y"
    { begin_proc((yyvsp[(2) - (2)].I)); }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 101 "parse.y"
    { def_proc(void_type); }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 102 "parse.y"
    { begin_proc((yyvsp[(2) - (2)].I)); }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 103 "parse.y"
    { def_proc((yyvsp[(6) - (7)].T)); }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 104 "parse.y"
    { begin_proc((yyvsp[(2) - (3)].I)); def_proc(void_type); }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 113 "parse.y"
    { decl_list(CPARAM, (yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].T)); }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 115 "parse.y"
    { decl_list(VPARAM, (yyvsp[(2) - (4)].E), (yyvsp[(4) - (4)].T)); }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 117 "parse.y"
    { emit("void p_main()\n{\n%i"); }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 118 "parse.y"
    { emit("%o}\n"); }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 121 "parse.y"
    { if (return_type != void_type) 
			emit("return __R__;\n");
		  emit("%o}\n\n"); }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 129 "parse.y"
    { emit("{\n%i"); }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 129 "parse.y"
    { emit("%o}\n"); }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 131 "parse.y"
    { emit("\n%i"); }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 131 "parse.y"
    { emit("%o"); }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 132 "parse.y"
    { emit(" {\n%i"); }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 132 "parse.y"
    { emit("%o}\n"); }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 134 "parse.y"
    { emit("/* skip */;\n"); }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 135 "parse.y"
    { emit("%oL%s:\n%i", (yyvsp[(1) - (2)].L)->l_value); }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 136 "parse.y"
    { assign((yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].E)); }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 138 "parse.y"
    { emit("%e;\n", func_call(PROC, (yyvsp[(1) - (2)].I), (yyvsp[(2) - (2)].E))); }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 140 "parse.y"
    { emit("%e;\n", func_call(PROC, (yyvsp[(1) - (2)].I), (yyvsp[(2) - (2)].E))); }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 142 "parse.y"
    { emit("goto L%s;\n", (yyvsp[(2) - (2)].L)->l_value); }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 143 "parse.y"
    { emit("if "); }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 145 "parse.y"
    { check_type("while statement", bool_type, (yyvsp[(2) - (3)].E)->t_type);
		  emit("while (%e)", (yyvsp[(2) - (3)].E)); }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 148 "parse.y"
    { emit("do {\n%i"); }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 150 "parse.y"
    { check_type("repeat statement", bool_type, (yyvsp[(5) - (5)].E)->t_type);
		  emit("%o} while (! (%e));\n", (yyvsp[(5) - (5)].E)); }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 153 "parse.y"
    { for_loop((yyvsp[(2) - (7)].E), (yyvsp[(4) - (7)].E), (yyvsp[(6) - (7)].E), TO); }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 156 "parse.y"
    { for_loop((yyvsp[(2) - (7)].E), (yyvsp[(4) - (7)].E), (yyvsp[(6) - (7)].E), DOWNTO); }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 159 "parse.y"
    { check_type("case statement", scalar_type, (yyvsp[(2) - (3)].E)->t_type);
		  enter_case((yyvsp[(2) - (3)].E)->t_type);
		  emit("switch (%e) {\n%i", (yyvsp[(2) - (3)].E)); }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 163 "parse.y"
    { emit("%o}\n"); exit_case(); }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 166 "parse.y"
    { check_type("if statement", bool_type, (yyvsp[(1) - (2)].E)->t_type);
		  emit("(%e)", (yyvsp[(1) - (2)].E)); }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 171 "parse.y"
    { emit("else"); }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 172 "parse.y"
    { emit("else if "); }
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 177 "parse.y"
    { emit("%odefault:\n%i"); }
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 182 "parse.y"
    { case_labels((yyvsp[(1) - (2)].E)); }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 183 "parse.y"
    { emit("break;\n"); }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 187 "parse.y"
    { (yyval.E) = list1((tree) (yyvsp[(1) - (1)].I)); }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 188 "parse.y"
    { (yyval.E) = snoc((yyvsp[(1) - (3)].E), (tree) (yyvsp[(3) - (3)].I)); }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 190 "parse.y"
    { (yyval.E) = list1((tree) (yyvsp[(1) - (1)].L)); }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 191 "parse.y"
    { (yyval.E) = snoc((yyvsp[(1) - (3)].E), (tree) (yyvsp[(3) - (3)].L)); }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 193 "parse.y"
    { (yyval.E) = nil; }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 194 "parse.y"
    { (yyval.E) = (yyvsp[(2) - (3)].E); }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 196 "parse.y"
    { (yyval.E) = list1((yyvsp[(1) - (1)].E)); }
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 197 "parse.y"
    { (yyval.E) = snoc((yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].E)); }
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 200 "parse.y"
    { (yyval.E) = (yyvsp[(1) - (3)].E); }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 203 "parse.y"
    { (yyval.E) = var_as_exp((yyvsp[(1) - (1)].E)); }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 204 "parse.y"
    { (yyval.E) = (tree) (yyvsp[(1) - (1)].L); }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 205 "parse.y"
    { (yyval.E) = (tree) (yyvsp[(1) - (1)].L); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 206 "parse.y"
    { (yyval.E) = func_call(FUNC, (yyvsp[(1) - (4)].I), (yyvsp[(3) - (4)].E)); }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 207 "parse.y"
    { (yyval.E) = (yyvsp[(2) - (3)].E); }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 208 "parse.y"
    { (yyval.E) = eval((yyvsp[(1) - (2)].I)->i_op, (yyvsp[(2) - (2)].E), nil); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 209 "parse.y"
    { (yyval.E) = eval(UMINUS, (yyvsp[(2) - (2)].E), nil); }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 210 "parse.y"
    { (yyval.E) = eval((yyvsp[(2) - (3)].I)->i_op, (yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].E)); }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 211 "parse.y"
    { (yyval.E) = eval((yyvsp[(2) - (3)].I)->i_op, (yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].E)); }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 212 "parse.y"
    { (yyval.E) = eval('-', (yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].E)); }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 213 "parse.y"
    { (yyval.E) = eval((yyvsp[(2) - (3)].I)->i_op, (yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].E)); }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 214 "parse.y"
    { (yyval.E) = eval('=', (yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].E)); }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 216 "parse.y"
    { (yyval.E) = nil; }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 217 "parse.y"
    { (yyval.E) = (yyvsp[(2) - (3)].E); }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 219 "parse.y"
    { (yyval.E) = list1((yyvsp[(1) - (1)].E)); }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 220 "parse.y"
    { (yyval.E) = snoc((yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].E)); }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 223 "parse.y"
    { (yyval.E) = var_ref((yyvsp[(1) - (1)].I)); }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 224 "parse.y"
    { (yyval.E) = eval(SUB, (yyvsp[(1) - (4)].E), (yyvsp[(3) - (4)].E)); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 225 "parse.y"
    { (yyval.E) = eval(DOT, (yyvsp[(1) - (3)].E), (tree) (yyvsp[(3) - (3)].I)); }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 228 "parse.y"
    { (yyval.T) = range_type((yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].E)); }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 230 "parse.y"
    { (yyval.T) = array_type((yyvsp[(3) - (6)].T), (yyvsp[(6) - (6)].T)); }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 231 "parse.y"
    { start_rec_type(); }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 232 "parse.y"
    { (yyval.T) = end_rec_type(); }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 234 "parse.y"
    { (yyval.T) = const_type((yyvsp[(1) - (1)].I)); }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 239 "parse.y"
    { decl_list(FIELD, (yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].T)); }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 241 "parse.y"
    { (yyval.E) = (tree) (yyvsp[(1) - (1)].L); }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 242 "parse.y"
    { (yyval.E) = (tree) (yyvsp[(1) - (1)].L); }
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 243 "parse.y"
    { (yyval.E) = var_ref((yyvsp[(1) - (1)].I)); }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 245 "parse.y"
    { check_type("constant", int_type, (yyvsp[(2) - (2)].E)->t_type);
		  (yyval.E) = (tree) lit_cat(NUMBER, "-", ((literal) (yyvsp[(2) - (2)].E))->l_value, 
				      int_type); }
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 249 "parse.y"
    { (yyval.E) = list1((yyvsp[(1) - (1)].E)); }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 250 "parse.y"
    { (yyval.E) = snoc((yyvsp[(1) - (3)].E), (yyvsp[(3) - (3)].E)); }
    break;



/* Line 1806 of yacc.c  */
#line 2293 "parse.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 252 "parse.y"


PUBLIC tree nconc(tree a, tree b)
{
     if (a == nil)
	  return b;
     else {
	  tree t = a;
	  while (cdr(t) != nil) t = cdr(t);
	  cdr(t) = b;
	  return a;
     }
}

PUBLIC tree node_t(int kind, tree left, tree right, type t)
{
     tree p;

     p = (tree) malloc(sizeof(struct tree));
     p->t_kind = kind;
     p->t_left = left;
     p->t_right = right;
     p->t_type = t;
     return p;
}

PUBLIC void yyerror(char *msg)
{
     error(msg);
}

