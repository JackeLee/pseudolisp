#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// #include "funkce.h"
#include "parser.h"
#include "helpers.h"
#include "error.h"
#include "execute.h"
// #include "gc.h"

extern t_point f_plus(Cons *params);
extern t_point f_if(Cons *params);
extern t_point f_eq(Cons *params);

t_point parse_pipe(Hash *h, int level);
extern int is_whitespace(char c);


int prompt = 1;

int set_prompt(int set)
{
	prompt = set;
	return 0;
}


Hash *get_basic_hash()
{
	#define NUM_FUN sizeof(array_of_functions)/sizeof(struct function)

	static Hash *h = NULL;
	if (h != NULL) return h;

	h = new_Hash();
	Function *f;
	t_point s;

	struct function {
		char *name;
		t_point (*link)(Cons *);
		int params_count;
		int more_params;
	} array_of_functions[] = {
	//	name	function	params  more_params
		{"+",       f_plus,    2,     1},
		{"if",      f_if,      3,     0},
		{"=",       f_eq,      2,     1},
	};

	for (int i=0; i<NUM_FUN; i++) {
		f = new_Function(NULL, array_of_functions[i].params_count);
		f->built_in = BOOL_TRUE;
		f->body.link = array_of_functions[i].link;
		f->more_params = array_of_functions[i].more_params;
		s = make_Func(f);

		add_string_Hash(h, array_of_functions[i].name, s);
//		gc_inc_immortal(NIL, s);
	}

	add_string_Hash(h, "NIL",   NIL);
	add_string_Hash(h, "TRUE",  BOOL_TRUE);
	add_string_Hash(h, "FALSE", BOOL_FALSE);

	return h;
}

/*
	//	name	function	params
		{"+",       plus,    2},
		{"-",       minus,   2},
		{"*",       krat,    2},
		{"/",       deleno,  2},

		{"if",      op_if,   3},
		{"and",     op_and,  2},
		{"or",      op_or,   2},
		{"not",     op_not,  1},

		{"nil?",    op_nil,  1},
		{"list?",   op_list, 1},
		{"number?", op_num,  1},
		{"char?",   op_char, 1},
		{"bool?",   op_bool, 1},
		{"func?",   op_func, 1},

		{"=",       eq,      2},
		{">",       gt,      2},

		{"head",    head,    1},
		{"tail",    tail,    1},
		{"append",  append,  2},

		{"list",    list,    1},
		{"print",   f_print, 1},
		{"print-string", f_print_string, 1},
		{"env",     env,     0},
		{"apply",   apply,   2},
*/


t_point init_def(Hash *h, char *name, int level)
{
	Function *f = new_Function(NULL, 0);
	t_point s = make_Func(f);
	HashMember *hm = NULL;
	if (name != NULL) {
		hm = add_string_Hash(h, name, s);
		hm->info = level;
	}

	Hash *new_h = clone_Hash(h);
	char chars[101];
	int param_counter = level;

	// nacteni parametru funkce
	while (is_whitespace(chars[0] = read_char()));
	if (chars[0] != OPEN_TAG) ERROR(SYNTAX_ERROR);

	while (read_word(chars, 1)) {
		add_string_Hash(new_h, chars, pnew_Param(++param_counter));
	}

	if (chars[0] != '\0') {
		add_string_Hash(new_h, chars, pnew_Param(++param_counter));

		// nastaveni parametru se zbytkem z volani
		if (chars[0] == REMAIN_PARAMS_TAG) {
			f->more_params = 0;
			param_counter--;
		}
	}

	// asociace funkce
	f->built_in = 0;
	f->params_count = param_counter;

	// vytvoreni tela funkce
	s = parse_pipe(new_h, param_counter+f->more_params);
	if (is_Thunk(s))
		f->body.structure = get_Thunk(s);
	else
		f->body.structure = new_Thunk(s, NULL);

	if (f->body.structure == NULL) ERROR(SYNTAX_ERROR);
	free_Hash(new_h);

	return make_Func(f);
}


t_point get_Undefined()
{
	static t_point u = NIL;
/*
	if (u == NIL) {
		Function *f = new_Function(NULL, 0);
		f->built_in = 1;
		f->body.link = undefined;

		u = make_Func(f);
	}
*/
	return u;
}


static t_point kontext_params(t_point function, int level)
{
	if (level <= 0) return function;

	Cons params = {.b = NIL};
	Cons *l = &params;

	for (int i=1; i<=level; i++) {
		l->b = pnew_List(pnew_Param(i));
		l = next(l);
	}

	return pnew_Thunk(function, get_Cons(params.b));
}


/**
 * TODO Neumi tohle:
 *   [def b [] [+ a 3]]
 *   [def a [] 3]
 */
t_point create_token(Hash *h, char *string, int level)
{
	t_point s = NIL;
	
	if (('0' <= string[0] && string[0] <= '9')
			|| (string[0] == '-' && '0' <= string[1] && string[1] <= '9'))
	{
		s = make_Num((t_number) atof(string));
	}

	if (s == NIL) {
		HashMember *hp = get_string_Hash(h, string);

		if (hp == NULL) {
			hp = add_string_Hash(h, string, get_Undefined());
		}

		s = hp->link;

		// doplneni kontextu do volani funkce
		// TODO delat jen kdyz je treba
		if (is_Func(s))
			s = kontext_params(s, min(hp->info, level));
	}

	return s;
}


t_point parse_pipe(Hash *h, int level)
{
	char chars[101];
	char *c = chars;
	int whitespaces = 1; // cetli jsme whitespace?
	int first = 1;
	int is_def = 0;
	int is_close_tag = 0;
	Cons all = {NIL, NIL};
	Cons *l = &all;
	t_point s = NIL;


	while ((*c = read_char()) != EOF) {
		if (is_whitespace(*c) || (is_close_tag = (*c == CLOSE_TAG))) {
			if (whitespaces && !is_close_tag) continue;

			*c = '\0';
			whitespaces = 1;

			if (first && ((is_def = (strcmp(chars, "def") == 0))
						|| (strcmp(chars, "lambda") == 0)))
			{
				if (is_close_tag) ERROR(SYNTAX_ERROR);
				if (is_def) {
					// is def
					if (!read_word(chars, 0)) ERROR(SYNTAX_ERROR);
					init_def(h, chars, level);
				} else {
					// is lambda
					return kontext_params(init_def(h, NULL, level), level);
				}

				break;
			}

			// c is not empy
			if (c != chars) {
				l->b = create_token(h, chars, level);

				if (first) {
					l->a = l->b;
					l->b = NIL;
				} else {
					l->b = pnew_List(l->b);
					l = next(l);
				}
			}

			if (is_close_tag) break;

			c = chars;
			first = 0;
			continue;
		}

		// nacitame dal
		if (!whitespaces) { c++; continue; }

		switch (*c) {
			case '\'': s = parse_char(); break;
			case '"':  s = parse_string(); break;

			case OPEN_TAG:
				l->b = parse_pipe(h, level);

				if (l->b != NIL) {
					if (first) {
						l->a = l->b;
						l->b = NIL;
						first = 0;
					} else {
						l->b = pnew_List(l->b);
						l = next(l);
					}
				}
				break;

			default: c++; whitespaces = 0; break;
		}

		if (s != NIL) {
			l->b = pnew_List(s);
			l = next(l);
			s = NIL;
		}
	}
	
	if (all.a == NIL && all.b == NIL)
		return NIL;
	else
		return pnew_Thunk(all.a, get_Cons(all.b));
}


int play()
{
//	gc_init();
	Hash *h = get_basic_hash();
	t_point parsed;
	char c;

	if (prompt) printf(PROMPT);
	while ((c = read_char()) != EOF) {
		if (c == OPEN_TAG) {
			parsed = parse_pipe(h, 0);
			if (parsed != NIL) {
				print_Symbol(parsed);
			} else if (prompt)
				printf("OK\n");
//			gc();
		}
	}

	printf("\n\n"); // gc_score();
	return 0;
}
