#include <stdio.h>
// #include <stdlib.h>

#include "funkce.h"
#include "structs.h"


static int vypis_list(List *l);
static int listove_testy();
static int funkcni_testy();
static int testy_slozenych_funkci();
static int zkouska_erroru();

int test()
{
	listove_testy();
	funkcni_testy();
	testy_slozenych_funkci();
	zkouska_erroru();

	return 0;
}


static int zkouska_erroru()
{
	Funkce **f = get_array_of_funtions();

	List *telo = new_List(new_Symbol(FUNKCE, f[0]));
	telo->dalsi = new_List(new_Ordinal(PARAMETR, 1));
	telo->dalsi->dalsi = new_List(new_Ordinal(PARAMETR, 2));

	Funkce *a = new_Funkce(telo, 2);

	List *do_tanku = new_List(new_Symbol(FUNKCE, f[0]));
	do_tanku->dalsi = new_List(new_Ordinal(CISLO, 3));
	do_tanku->dalsi = new_List(new_Ordinal(CISLO, 5));


	List *volani = new_List(new_Ordinal(CISLO, 2));
	volani->dalsi = new_List(result(do_tanku));

	Symbol *vysledek = resolve_Tank(call(a, volani));

	printf("vysledek-error: %s\n", (vysledek == NULL) ? "JO" : "NE");
	printf("vysledku-error2: %s\n", (resolve_Tank(result(do_tanku)) == NULL) ? "JO" : "NE");

	return 0;
}


static int testy_slozenych_funkci()
{
	// 	plus minus krat deleno
	Funkce **f = get_array_of_funtions();

	List *telo = new_List(new_Symbol(FUNKCE, f[0]));
	telo->dalsi = new_List(new_Ordinal(CISLO, 3));
	telo->dalsi->dalsi = new_List(new_Ordinal(PARAMETR, 1));

	Funkce *a = new_Funkce(telo, 1);

	Symbol *vysledek = resolve_Tank(call(a, new_List(new_Ordinal(CISLO, 8))));
	printf("typ vysledku: %d ", vysledek->typ);
	printf("parametr: %d\n", (int)a->telo.struktura->dalsi->dalsi->symbol->s.znak);
	printf("vysledek: %d\n", (int) vysledek->s.cislo);

	return 0;
}


static int funkcni_testy()
{
	Funkce **f = get_array_of_funtions();
	const int n = 5;
	Symbol *s[n];

	for (int i=0; i<n; i++) {
		s[i] = new_Ordinal(CISLO, i+3);
	}

	List *l = array_to_List(s, n);
	vypis_list(l);

//	Symbol *vysl = krat(l);
	Symbol *vysl = call(f[0], l);
	printf("vysledek: %d\n", (int) vysl->s.cislo);

	return 0;
}


static int listove_testy()
{
	const int n = 10;
	Symbol *s[n];
	List *l;

	for (int i=0; i<n; i++) {
		s[i] = new_Ordinal(CISLO, i*100);
	}

	l = array_to_List(s, n);
	vypis_list(l);

/*	List *nl = new_List(new_Symbol(LIST, l));
	nl->dalsi = new_List(new_Symbol(LIST, l));
	Symbol *list = append(nl);
	if (list != NULL && list->typ == LIST) vypis_list((List *)list->s.odkaz);
*/
	return 0;
}


static int vypis_list(List *l)
{
	while (l != NULL) {
		printf("%d %3d\n", l->symbol->typ, (int) l->symbol->s.cislo);

		l = l->dalsi;
	}

	return 0;
}

