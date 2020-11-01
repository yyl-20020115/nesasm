#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"


/* ----
 * symhash()
 * ----
 * calculate the hash value of a symbol
 */

int
symhash(void)
{
	int i;
	char c;
	int hash = 0;

	/* hash value */
	for (i = 1; i <= symbol[0]; i++) {
		c = symbol[i];
		hash += c;
		hash  = (hash << 3) + (hash >> 5) + c;
	}

	/* ok */
	return (hash & 0xFF);
}


/* ----
 * colsym()
 * ----
 * collect a symbol from prlnbuf into symbol[],
 * leaves prlnbuf pointer at first invalid symbol character,
 * returns 0 if no symbol collected
 */

int
colsym(int *ip)
{
	int  err = 0;
	int	 i = 0;
	char c;

	/* get the symbol */
	for (;;) {
		c = prlnbuf[*ip];
		if (isdigit(c) && (i == 0))
			break;
		if ((!isalnum(c)) && (c != '_') && (c != '.'))
			break;
		if (i < (SBOLSZ - 1))
			symbol[++i] = c;
		(*ip)++;
	}

	symbol[0] = i;
	symbol[i+1] = '\0';

	/* check if it's a reserved symbol */
	if (i == 1) {
		switch (toupper(symbol[1])) {
		case 'A':
		case 'X':
		case 'Y':
			err = 1;
			break;
		}
	}
	if (check_keyword())
		err = 1;

	/* error */
	if (err) {
		fatal_error("Reserved symbol!");
		return (0);
	}	

	/* ok */
	return (i);
}

FILE* stlist_file(FILE *fp, char * basename, int n)
{
	if (fp) return fp;
	char fname[256];
	strcpy(fname, basename);
	if (n < 0)
		strcat(fname, ".ram.nl");
	else {
		char ext[16];
		sprintf(ext, ".%X.nl", n);
		strcat(fname, ext);
	}
	if ((fp = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "can not open file '%s'!\n", fname);
		return 0;
	}
	return fp;
}

void stlist(char *file, int bank_offset)
{
	struct t_symbol *sym;
	struct t_symbol *local;
	FILE *files[256];
	memset(files, 0, sizeof(files)); 
	int i, j, bank, fnum;
	for (i = 0; i < sizeof(hash_tbl) / sizeof(hash_tbl[0]); i++)
	{
		sym = hash_tbl[i];
		if (!sym) continue;
		do
		{
			/* skipping reserved symbols and constants */
			if (sym->reserved || sym->equ) continue;

			bank = sym->value < 0x8000 ? -1 : sym->bank/2 + bank_offset;
			fnum = bank >= 0 ? bank : (sizeof(files) / sizeof(FILE*) - 1);
			files[fnum] = stlist_file(files[fnum], file, bank);
			//fprintf(files[fnum], "$%04X#%s# %d %d\n", sym->value, sym->name+1, sym->reserved, sym->equ);
      fprintf(files[fnum], "$%04X#%s#\n", sym->value, sym->name+1);
			for (j = 1; j < sym->data_size; j++)
				//fprintf(files[fnum], "$%04X#%s+%d# %d %d\n", sym->value+j, sym->name+1, j, sym->reserved, sym->equ);
        fprintf(files[fnum], "$%04X#%s+%d#\n", sym->value+j, sym->name+1, j);
			local = sym->local;
			while (local)
			{
				bank = local->value < 0x8000 ? -1 : local->bank/2 + bank_offset;
				fnum = bank >= 0 ? bank : (sizeof(files) / sizeof(FILE*) - 1);
				files[fnum] = stlist_file(files[fnum], file, bank);
				//fprintf(files[fnum], "$%04X#%s (%s)# %d %d\n", local->value, local->name+1, sym->name+1, local->reserved, local->equ);
        fprintf(files[fnum], "$%04X#%s (%s)#\n", local->value, local->name+1, sym->name+1);
				for (j = 1; j < sym->data_size; j++)
					//fprintf(files[fnum], "$%04X#%s+%d (%s)# %d %d\n", local->value+j, local->name+1, j, sym->name+1, local->reserved, local->equ);
          fprintf(files[fnum], "$%04X#%s+%d (%s)#\n", local->value+j, local->name+1, j, sym->name+1);
	       			local = local->next;
			}
		} while ((sym = sym->next) != NULL);
	}
	for (i = 0; i < sizeof(files) / sizeof(FILE*); i++)
		if (files[i])
			fclose(files[i]);
}

/* ----
 * stlook()
 * ----
 * symbol table lookup
 * if found, return pointer to symbol
 * else, install symbol as undefined and return pointer
 */

struct t_symbol *stlook(int flag)
{
	struct t_symbol *sym;
	int sym_flag = 0;
	int hash;

	/* local symbol */
	if (symbol[1] == '.') {
		if (glablptr) {
			/* search the symbol in the local list */
			sym = glablptr->local;

			while (sym) {
				if (!strcmp(symbol, sym->name))
					break;			
				sym = sym->next;
			}

			/* new symbol */
			if (sym == NULL) {
				if (flag) {
					sym = stinstall(0, 1);
					sym_flag = 1;
				}
			}
		}
		else {
			error("Local symbol not allowed here!");
			return (NULL);
		}
	}

	/* global symbol */
	else {
		/* search symbol */
		hash = symhash();
		sym  = hash_tbl[hash];
		while (sym) {
			if (!strcmp(symbol, sym->name))
				break;			
			sym = sym->next;
		}

		/* new symbol */
		if (sym == NULL) {
			if (flag) {
				sym = stinstall(hash, 0);
				sym_flag = 1;
			}
		}
	}

	/* incremente symbol reference counter */
	if (sym_flag == 0) {
		if (sym)
			sym->refcnt++;
	}

	/* ok */
	return (sym);
}


/* ----
 * stinstall()
 * ----
 * install symbol into symbol hash table
 */

struct t_symbol *stinstall(int hash, int type)
{
	struct t_symbol *sym;

	/* allocate symbol structure */
	if ((sym = (void *)malloc(sizeof(struct t_symbol))) == NULL) {
		fatal_error("Out of memory!");
		return (NULL);
	}

	/* init the symbol struct */
	sym->type  = if_expr ? IFUNDEF : UNDEF;
	sym->value = 0;
	sym->local = NULL;
	sym->proc  = NULL;
	sym->bank  = RESERVED_BANK;
	sym->nb    = 0;
	sym->size  = 0;
	sym->page  = -1;
	sym->vram  = -1;
	sym->pal   = -1;
	sym->refcnt = 0;
	sym->reserved = 0;
	sym->equ = 0;
	sym->data_type = -1;
	sym->data_size = 0;
	strcpy(sym->name, symbol);

	/* add the symbol to the hash table */
	if (type) {
		/* local */
		sym->next = glablptr->local;
		glablptr->local = sym;
	}
	else {
		/* global */
		sym->next = hash_tbl[hash];
		hash_tbl[hash] = sym;
	}

	/* ok */
	return (sym);
}


/* ----
 * labldef()
 * ----
 * assign <lval> to label pointed to by lablptr,
 * checking for valid definition, etc.
 */

int
labldef(int lval, int flag)
{
	char c;

	/* check for NULL ptr */
	if (lablptr == NULL)
		return (0);

	/* adjust symbol address */	
	if (flag)
		lval = (lval & 0x1FFF) | (page << 13);

	/* first pass */
	if (pass == FIRST_PASS) {
		switch (lablptr->type) {
		/* undefined */
		case UNDEF:
			lablptr->type = DEFABS;
			lablptr->value = lval;
			break;

		/* already defined - error */
		case IFUNDEF:
			error("Can not define this label, declared as undefined in an IF expression!");
			return (-1);

		case MACRO:
			error("Symbol already used by a macro!");
			return (-1);

		case FUNC:
			error("Symbol already used by a function!");
			return (-1);

		default:
			/* reserved label */
			if (lablptr->reserved) {
				fatal_error("Reserved symbol!");
				return (-1);
			}

			/* compare the values */
			if (lablptr->value == lval)
				break;

			/* normal label */
			lablptr->type = MDEF;
			lablptr->value = 0;
			error("Label multiply defined!");
			return (-1);
		}
	}

	/* second pass */
	else {
		if ((lablptr->value != lval) ||
		   ((flag) && (bank < bank_limit) && (lablptr->bank  != bank_base + bank)))
		{
			fatal_error("Internal error[1]!");
			return (-1);
		}
	}

	/* update symbol data */
	if (flag) {
		if (section == S_CODE)
			lablptr->proc = proc_ptr;
		lablptr->bank = bank_base + bank;
		lablptr->page = page;

		/* check if it's a local or global symbol */
		c = lablptr->name[1];
		if (c == '.')
			/* local */
			lastlabl = NULL;
		else {
			/* global */
			glablptr = lablptr;
			lastlabl = lablptr;
		}
	}

	/* ok */
	return (0);
}


/* ----
 * lablset()
 * ----
 * create/update a reserved symbol
 */

void
lablset(char *name, int val)
{
	int len;

	len = strlen(name);
	lablptr = NULL;

	if (len) {
		symbol[0] = len;
		strcpy(&symbol[1], name);
		lablptr = stlook(1);

		if (lablptr) {
			lablptr->type = DEFABS;
			lablptr->value = val;
			lablptr->reserved = 1;
		}
	}

	/* ok */
	return;
}


/* ----
 * lablremap()
 * ----
 * remap all the labels
 */

void
lablremap(void)
{
	struct t_symbol *sym;
	int i;

	/* browse the symbol table */
	for (i = 0; i < 256; i++) {
		sym = hash_tbl[i];
		while (sym) {
			/* remap the bank */
			if (sym->bank <= bank_limit)
				sym->bank += bank_base;
			sym = sym->next;
		}
	}
}

