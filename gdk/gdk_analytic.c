/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 1997 - July 2008 CWI, August 2008 - 2018 MonetDB B.V.
 */

#include "monetdb_config.h"
#include "gdk.h"
#include "gdk_analytic.h"
#include "gdk_calc_private.h"

#define ANALYTICAL_DIFF_IMP(TPE)              \
	do {                                      \
		TPE *bp = (TPE*)Tloc(b, 0);           \
		TPE prev = *bp, *end = bp + cnt;      \
		if(rp) {                              \
			for(; bp<end; bp++, rb++, rp++) { \
				*rb = *rp;                    \
				if (*bp != prev) {            \
					*rb = TRUE;               \
					prev = *bp;               \
				}                             \
			}                                 \
		} else {                              \
			for(; bp<end; bp++, rb++) {       \
				*rb = FALSE;                  \
				if (*bp != prev) {            \
					*rb = TRUE;               \
					prev = *bp;               \
				}                             \
			}                                 \
		}                                     \
	} while(0);

gdk_return
GDKanalyticaldiff(BAT *r, BAT *b, BAT *c, int tpe)
{
	BUN i, cnt = BATcount(b);
	bit *restrict rb = (bit*)Tloc(r, 0), *restrict rp = c ? (bit*)Tloc(c, 0) : NULL;
	int (*atomcmp)(const void *, const void *);

	switch(tpe) {
		case TYPE_bit:
			ANALYTICAL_DIFF_IMP(bit)
			break;
		case TYPE_bte:
			ANALYTICAL_DIFF_IMP(bte)
			break;
		case TYPE_sht:
			ANALYTICAL_DIFF_IMP(sht)
			break;
		case TYPE_int:
			ANALYTICAL_DIFF_IMP(int)
			break;
		case TYPE_lng:
			ANALYTICAL_DIFF_IMP(lng)
			break;
#ifdef HAVE_HGE
		case TYPE_hge:
			ANALYTICAL_DIFF_IMP(hge)
			break;
#endif
		case TYPE_flt:
			ANALYTICAL_DIFF_IMP(flt)
			break;
		case TYPE_dbl:
			ANALYTICAL_DIFF_IMP(dbl)
			break;
		default: {
			BATiter it = bat_iterator(b);
			ptr v = BUNtail(it, 0), next;
			atomcmp = ATOMcompare(tpe);
			if(rp) {
				for (i=0; i<cnt; i++, rb++, rp++) {
					*rb = *rp;
					next = BUNtail(it, i);
					if (atomcmp(v, next) != 0) {
						*rb = TRUE;
						v = next;
					}
				}
			} else {
				for(i=0; i<cnt; i++, rb++) {
					*rb = FALSE;
					next = BUNtail(it, i);
					if (atomcmp(v, next) != 0) {
						*rb = TRUE;
						v = next;
					}
				}
			}
		}
	}
	BATsetcount(r, cnt);
	r->tnonil = true;
	r->tnil = false;
	return GDK_SUCCEED;
}

#undef ANALYTICAL_DIFF_IMP

#define NTILE_CALC(TPE)               \
	do {                              \
		if((BUN)val >= ncnt) {        \
			i = 1;                    \
			for(; rb<rp; i++, rb++)   \
				*rb = i;              \
		} else if(ncnt % val == 0) {  \
			buckets = ncnt / val;     \
			for(; rb<rp; i++, rb++) { \
				if(i == buckets) {    \
					j++;              \
					i = 0;            \
				}                     \
				*rb = j;              \
			}                         \
		} else {                      \
			buckets = ncnt / val;     \
			for(; rb<rp; i++, rb++) { \
				*rb = j;              \
				if(i == buckets) {    \
					j++;              \
					i = 0;            \
				}                     \
			}                         \
		}                             \
	} while(0);

#define ANALYTICAL_NTILE_IMP(TPE)            \
	do {                                     \
		TPE i = 0, j = 1, *rp, *rb, buckets; \
		TPE val =  *(TPE *)ntile;            \
		rb = rp = (TPE*)Tloc(r, 0);          \
		if(is_##TPE##_nil(val)) {            \
			TPE *end = rp + cnt;             \
			has_nils = true;                 \
			for(; rp<end; rp++)              \
				*rp = TPE##_nil;             \
		} else if(p) {                       \
			pnp = np = (bit*)Tloc(p, 0);     \
			end = np + cnt;                  \
			for(; np<end; np++) {            \
				if (*np) {                   \
					i = 0;                   \
					j = 1;                   \
					ncnt = np - pnp;         \
					rp += ncnt;              \
					NTILE_CALC(TPE)          \
					pnp = np;                \
				}                            \
			}                                \
			i = 0;                           \
			j = 1;                           \
			ncnt = np - pnp;                 \
			rp += ncnt;                      \
			NTILE_CALC(TPE)                  \
		} else {                             \
			rp += cnt;                       \
			NTILE_CALC(TPE)                  \
		}                                    \
		goto finish;                         \
	} while(0);

gdk_return
GDKanalyticalntile(BAT *r, BAT *b, BAT *p, BAT *o, int tpe, const void* restrict ntile)
{
	BUN cnt = BATcount(b), ncnt = cnt;
	bit *np, *pnp, *end;
	bool has_nils = false;
	gdk_return gdk_res = GDK_SUCCEED;

	assert(ntile);

	(void) o;
	switch (tpe) {
		case TYPE_bte:
			ANALYTICAL_NTILE_IMP(bte)
			break;
		case TYPE_sht:
			ANALYTICAL_NTILE_IMP(sht)
			break;
		case TYPE_int:
			ANALYTICAL_NTILE_IMP(int)
			break;
		case TYPE_lng:
			ANALYTICAL_NTILE_IMP(lng)
			break;
#ifdef HAVE_HGE
		case TYPE_hge:
			ANALYTICAL_NTILE_IMP(hge)
			break;
#endif
		default:
			goto nosupport;
	}
nosupport:
	GDKerror("ntile: type %s not supported.\n", ATOMname(tpe));
	return GDK_FAIL;
finish:
	BATsetcount(r, cnt);
	r->tnonil = !has_nils;
	r->tnil = has_nils;
	return gdk_res;
}

#undef ANALYTICAL_NTILE_IMP
#undef NTILE_CALC

#define FIRST_CALC(TPE)            \
	do {                           \
		for (;rb < rp; rb++)       \
			*rb = curval;          \
		if(is_##TPE##_nil(curval)) \
			has_nils = true;       \
	} while(0);

#define ANALYTICAL_FIRST_IMP(TPE)           \
	do {                                    \
		TPE *rp, *rb, *restrict bp, curval; \
		rb = rp = (TPE*)Tloc(r, 0);         \
		bp = (TPE*)Tloc(b, 0);              \
		curval = *bp;                       \
		if (p) {                            \
			pnp = np = (bit*)Tloc(p, 0);    \
			end = np + cnt;                 \
			for(; np<end; np++) {           \
				if (*np) {                  \
					ncnt = (np - pnp);      \
					rp += ncnt;             \
					bp += ncnt;             \
					FIRST_CALC(TPE)         \
					curval = *bp;           \
					pnp = np;               \
				}                           \
			}                               \
			ncnt = (np - pnp);              \
			rp += ncnt;                     \
			bp += ncnt;                     \
			FIRST_CALC(TPE)                 \
		} else {                            \
			rp += cnt;                      \
			FIRST_CALC(TPE)                 \
		}                                   \
	} while(0);

#define ANALYTICAL_FIRST_OTHERS                                         \
	do {                                                                \
		curval = BUNtail(bpi, j);                                       \
		if((*atomcmp)(curval, nil) == 0)                                \
			has_nils = true;                                            \
		for (;j < i; j++) {                                             \
			if ((gdk_res = BUNappend(r, curval, false)) != GDK_SUCCEED) \
				goto finish;                                            \
		}                                                               \
	} while(0);

gdk_return
GDKanalyticalfirst(BAT *r, BAT *b, BAT *p, BAT *o, int tpe)
{
	int (*atomcmp)(const void *, const void *);
	const void* restrict nil;
	bool has_nils = false;
	BUN i = 0, j = 0, ncnt, cnt = BATcount(b);
	bit *np, *pnp, *end;
	gdk_return gdk_res = GDK_SUCCEED;

	(void) o;
	switch(tpe) {
		case TYPE_bit:
			ANALYTICAL_FIRST_IMP(bit)
			break;
		case TYPE_bte:
			ANALYTICAL_FIRST_IMP(bte)
			break;
		case TYPE_sht:
			ANALYTICAL_FIRST_IMP(sht)
			break;
		case TYPE_int:
			ANALYTICAL_FIRST_IMP(int)
			break;
		case TYPE_lng:
			ANALYTICAL_FIRST_IMP(lng)
			break;
#ifdef HAVE_HUGE
		case TYPE_hge:
			ANALYTICAL_FIRST_IMP(hge)
			break;
#endif
		case TYPE_flt:
			ANALYTICAL_FIRST_IMP(flt)
			break;
		case TYPE_dbl:
			ANALYTICAL_FIRST_IMP(dbl)
			break;
		default: {
			BATiter bpi = bat_iterator(b);
			void *restrict curval;
			nil = ATOMnilptr(tpe);
			atomcmp = ATOMcompare(tpe);
			if (p) {
				pnp = np = (bit*)Tloc(p, 0);
				end = np + cnt;
				for(; np<end; np++) {
					if (*np) {
						i += (np - pnp);
						ANALYTICAL_FIRST_OTHERS
						pnp = np;
					}
				}
				i += (np - pnp);
				ANALYTICAL_FIRST_OTHERS
			} else { /* single value, ie no ordering */
				i += cnt;
				ANALYTICAL_FIRST_OTHERS
			}
		}
	}
finish:
	BATsetcount(r, cnt);
	r->tnonil = !has_nils;
	r->tnil = has_nils;
	return gdk_res;
}

#undef ANALYTICAL_FIRST_IMP
#undef FIRST_CALC
#undef ANALYTICAL_FIRST_OTHERS

#define LAST_CALC(TPE)             \
	do {                           \
		curval = *(bp - 1);        \
		if(is_##TPE##_nil(curval)) \
			has_nils = true;       \
		for (;rb < rp; rb++)       \
			*rb = curval;          \
	} while(0);

#define ANALYTICAL_LAST_IMP(TPE)            \
	do {                                    \
		TPE *rp, *rb, *restrict bp, curval; \
		rb = rp = (TPE*)Tloc(r, 0);         \
		bp = (TPE*)Tloc(b, 0);              \
		if (p) {                            \
			pnp = np = (bit*)Tloc(p, 0);    \
			end = np + cnt;                 \
			for(; np<end; np++) {           \
				if (*np) {                  \
					ncnt = (np - pnp);      \
					rp += ncnt;             \
					bp += ncnt;             \
					LAST_CALC(TPE)          \
					pnp = np;               \
				}                           \
			}                               \
			ncnt = (np - pnp);              \
			rp += ncnt;                     \
			bp += ncnt;                     \
			LAST_CALC(TPE)                  \
		} else {                            \
			rp += cnt;                      \
			bp += cnt;                      \
			LAST_CALC(TPE)                  \
		}                                   \
	} while(0);

#define ANALYTICAL_LAST_OTHERS                                          \
	do {                                                                \
		curval = BUNtail(bpi, i - 1);                                   \
		if((*atomcmp)(curval, nil) == 0)                                \
			has_nils = true;                                            \
		for (;j < i; j++) {                                             \
			if ((gdk_res = BUNappend(r, curval, false)) != GDK_SUCCEED) \
				goto finish;                                            \
		}                                                               \
	} while(0);

gdk_return
GDKanalyticallast(BAT *r, BAT *b, BAT *p, BAT *o, int tpe)
{
	int (*atomcmp)(const void *, const void *);
	const void* restrict nil;
	bool has_nils = false;
	BUN i = 0, j = 0, ncnt, cnt = BATcount(b);
	bit *np, *pnp, *end;
	gdk_return gdk_res = GDK_SUCCEED;

	(void) o;
	switch(tpe) {
		case TYPE_bit:
			ANALYTICAL_LAST_IMP(bit)
			break;
		case TYPE_bte:
			ANALYTICAL_LAST_IMP(bte)
			break;
		case TYPE_sht:
			ANALYTICAL_LAST_IMP(sht)
			break;
		case TYPE_int:
			ANALYTICAL_LAST_IMP(int)
			break;
		case TYPE_lng:
			ANALYTICAL_LAST_IMP(lng)
			break;
#ifdef HAVE_HUGE
		case TYPE_hge:
			ANALYTICAL_LAST_IMP(hge)
			break;
#endif
		case TYPE_flt:
			ANALYTICAL_LAST_IMP(flt)
			break;
		case TYPE_dbl:
			ANALYTICAL_LAST_IMP(dbl)
			break;
		default: {
			BATiter bpi = bat_iterator(b);
			void *restrict curval;
			nil = ATOMnilptr(tpe);
			atomcmp = ATOMcompare(tpe);
			if (p) {
				pnp = np = (bit*)Tloc(p, 0);
				end = np + cnt;
				for(; np<end; np++) {
					if (*np) {
						i += (np - pnp);
						ANALYTICAL_LAST_OTHERS
						pnp = np;
					}
				}
				i += (np - pnp);
				ANALYTICAL_LAST_OTHERS
			} else { /* single value, ie no ordering */
				i += cnt;
				ANALYTICAL_LAST_OTHERS
			}
		}
	}
finish:
	BATsetcount(r, cnt);
	r->tnonil = !has_nils;
	r->tnil = has_nils;
	return gdk_res;
}

#undef ANALYTICAL_LAST_IMP
#undef LAST_CALC
#undef ANALYTICAL_LAST_OTHERS

#define NTHVALUE_CALC(TPE)         \
	do {                           \
		if(nth > (BUN) (bp - pbp)) \
			curval = TPE##_nil;    \
		else                       \
			curval = *(pbp + nth); \
		if(is_##TPE##_nil(curval)) \
			has_nils = true;       \
		for(; rb<rp; rb++)         \
			*rb = curval;          \
	} while(0);

#define ANALYTICAL_NTHVALUE_IMP(TPE)     \
	do {                                 \
		TPE *rp, *rb, *pbp, *bp, curval; \
		pbp = bp = (TPE*)Tloc(b, 0);     \
		rb = rp = (TPE*)Tloc(r, 0);      \
		if(nth == BUN_NONE) {            \
			TPE* rend = rp + cnt;        \
			has_nils = true;             \
			for(; rp<rend; rp++)         \
				*rp = TPE##_nil;         \
		} else if(p) {                   \
			pnp = np = (bit*)Tloc(p, 0); \
			end = np + cnt;              \
			for(; np<end; np++) {        \
				if (*np) {               \
					ncnt = (np - pnp);   \
					rp += ncnt;          \
					bp += ncnt;          \
					NTHVALUE_CALC(TPE)   \
					pbp = bp;            \
					pnp = np;            \
				}                        \
			}                            \
			ncnt = (np - pnp);           \
			rp += ncnt;                  \
			bp += ncnt;                  \
			NTHVALUE_CALC(TPE)           \
		} else {                         \
			rp += cnt;                   \
			bp += cnt;                   \
			NTHVALUE_CALC(TPE)           \
		}                                \
		goto finish;                     \
	} while(0);

#define ANALYTICAL_NTHVALUE_OTHERS                                      \
	do {                                                                \
		if(nth > (i - j))                                               \
			curval = nil;                                               \
		else                                                            \
			curval = BUNtail(bpi, nth);                                 \
		if((*atomcmp)(curval, nil) == 0)                                \
			has_nils = true;                                            \
		for (;j < i; j++) {                                             \
			if ((gdk_res = BUNappend(r, curval, false)) != GDK_SUCCEED) \
				goto finish;                                            \
		}                                                               \
	} while(0);

gdk_return
GDKanalyticalnthvalue(BAT *r, BAT *b, BAT *p, BAT *o, BUN nth, int tpe)
{
	int (*atomcmp)(const void *, const void *);
	const void* restrict nil;
	BUN i = 0, j = 0, ncnt, cnt = BATcount(b);
	bit *np, *pnp, *end;
	gdk_return gdk_res = GDK_SUCCEED;
	bool has_nils = false;

	(void) o;
	switch (tpe) {
		case TYPE_bte:
			ANALYTICAL_NTHVALUE_IMP(bte)
			break;
		case TYPE_sht:
			ANALYTICAL_NTHVALUE_IMP(sht)
			break;
		case TYPE_int:
			ANALYTICAL_NTHVALUE_IMP(int)
			break;
		case TYPE_lng:
			ANALYTICAL_NTHVALUE_IMP(lng)
			break;
#ifdef HAVE_HGE
		case TYPE_hge:
			ANALYTICAL_NTHVALUE_IMP(hge)
			break;
#endif
		case TYPE_flt:
			ANALYTICAL_NTHVALUE_IMP(flt)
			break;
		case TYPE_dbl:
			ANALYTICAL_NTHVALUE_IMP(dbl)
			break;
		default: {
			BATiter bpi = bat_iterator(b);
			const void *restrict curval;
			nil = ATOMnilptr(tpe);
			atomcmp = ATOMcompare(tpe);
			if(nth == BUN_NONE) {
				has_nils = true;
				for(i=0; i<cnt; i++) {
					if ((gdk_res = BUNappend(r, nil, false)) != GDK_SUCCEED)
						goto finish;
				}
			} else if (p) {
				pnp = np = (bit*)Tloc(p, 0);
				end = np + cnt;
				for(; np<end; np++) {
					if (*np) {
						i += (np - pnp);
						ANALYTICAL_NTHVALUE_OTHERS
						pnp = np;
					}
				}
				i += (np - pnp);
				ANALYTICAL_NTHVALUE_OTHERS
			} else { /* single value, ie no ordering */
				i += cnt;
				ANALYTICAL_NTHVALUE_OTHERS
			}
		}
	}
finish:
	BATsetcount(r, cnt);
	r->tnonil = !has_nils;
	r->tnil = has_nils;
	return gdk_res;
}

#undef ANALYTICAL_NTHVALUE_IMP
#undef NTHVALUE_CALC
#undef ANALYTICAL_NTHVALUE_OTHERS

#define ANALYTICAL_LAG_CALC(TPE)            \
	do {                                    \
		for(i=0; i<lag && rb<rp; i++, rb++) \
			*rb = def;                      \
		if(lag > 0 && is_##TPE##_nil(def))  \
			has_nils = true;                \
		for(;rb<rp; rb++, bp++) {           \
			next = *bp;                     \
			*rb = next;                     \
			if(is_##TPE##_nil(next))        \
				has_nils = true;            \
		}                                   \
	} while(0);

#define ANALYTICAL_LAG_IMP(TPE)                   \
	do {                                          \
		TPE *rp, *rb, *bp, *rend,                 \
			def = *((TPE *) default_value), next; \
		bp = (TPE*)Tloc(b, 0);                    \
		rb = rp = (TPE*)Tloc(r, 0);               \
		rend = rb + cnt;                          \
		if(lag == BUN_NONE) {                     \
			has_nils = true;                      \
			for(; rb<rend; rb++)                  \
				*rb = TPE##_nil;                  \
		} else if(p) {                            \
			pnp = np = (bit*)Tloc(p, 0);          \
			end = np + cnt;                       \
			for(; np<end; np++) {                 \
				if (*np) {                        \
					ncnt = (np - pnp);            \
					rp += ncnt;                   \
					ANALYTICAL_LAG_CALC(TPE)      \
					bp += (lag < ncnt) ? lag : 0; \
					pnp = np;                     \
				}                                 \
			}                                     \
			rp += (np - pnp);                     \
			ANALYTICAL_LAG_CALC(TPE)              \
		} else {                                  \
			rp += cnt;                            \
			ANALYTICAL_LAG_CALC(TPE)              \
		}                                         \
		goto finish;                              \
	} while(0);

#define ANALYTICAL_LAG_OTHERS                                                  \
	do {                                                                       \
		for(i=0; i<lag && k<j; i++, k++) {                                     \
			if ((gdk_res = BUNappend(r, default_value, false)) != GDK_SUCCEED) \
				goto finish;                                                   \
		}                                                                      \
		if(lag > 0 && (*atomcmp)(default_value, nil) == 0)                     \
			has_nils = true;                                                   \
		for(l=k-lag; k<j; k++, l++) {                                          \
			curval = BUNtail(bpi, l);                                          \
			if ((gdk_res = BUNappend(r, curval, false)) != GDK_SUCCEED)        \
				goto finish;                                                   \
			if((*atomcmp)(curval, nil) == 0)                                   \
				has_nils = true;                                               \
		}                                                                      \
	} while (0);

gdk_return
GDKanalyticallag(BAT *r, BAT *b, BAT *p, BAT *o, BUN lag, const void* restrict default_value, int tpe)
{
	int (*atomcmp)(const void *, const void *);
	const void *restrict nil;
	BUN i = 0, j = 0, k = 0, l = 0, ncnt, cnt = BATcount(b);
	bit *np, *pnp, *end;
	gdk_return gdk_res = GDK_SUCCEED;
	bool has_nils = false;

	assert(default_value);

	(void) o;
	switch (tpe) {
		case TYPE_bte:
			ANALYTICAL_LAG_IMP(bte)
			break;
		case TYPE_sht:
			ANALYTICAL_LAG_IMP(sht)
			break;
		case TYPE_int:
			ANALYTICAL_LAG_IMP(int)
			break;
		case TYPE_lng:
			ANALYTICAL_LAG_IMP(lng)
			break;
#ifdef HAVE_HGE
		case TYPE_hge:
			ANALYTICAL_LAG_IMP(hge)
			break;
#endif
		case TYPE_flt:
			ANALYTICAL_LAG_IMP(flt)
			break;
		case TYPE_dbl:
			ANALYTICAL_LAG_IMP(dbl)
			break;
		default: {
			BATiter bpi = bat_iterator(b);
			const void *restrict curval;
			nil = ATOMnilptr(tpe);
			atomcmp = ATOMcompare(tpe);
			if(lag == BUN_NONE) {
				has_nils = true;
				for (j=0;j < cnt; j++) {
					if ((gdk_res = BUNappend(r, nil, false)) != GDK_SUCCEED)
						goto finish;
				}
			} else if(p) {
				pnp = np = (bit*)Tloc(p, 0);
				end = np + cnt;
				for(; np<end; np++) {
					if (*np) {
						j += (np - pnp);
						ANALYTICAL_LAG_OTHERS
						pnp = np;
					}
				}
				j += (np - pnp);
				ANALYTICAL_LAG_OTHERS
			} else {
				j += cnt;
				ANALYTICAL_LAG_OTHERS
			}
		}
	}
finish:
	BATsetcount(r, cnt);
	r->tnonil = !has_nils;
	r->tnil = has_nils;
	return gdk_res;
}

#undef ANALYTICAL_LAG_IMP
#undef ANALYTICAL_LAG_CALC
#undef ANALYTICAL_LAG_OTHERS

#define LEAD_CALC(TPE)                       \
	do {                                     \
		if(lead < ncnt) {                    \
			bp += lead;                      \
			l = ncnt - lead;                 \
			for(i=0; i<l; i++, rb++, bp++) { \
				next = *bp;                  \
				*rb = next;                  \
				if(is_##TPE##_nil(next))     \
					has_nils = true;         \
			}                                \
		} else {                             \
			bp += ncnt;                      \
		}                                    \
		for(;rb<rp; rb++)                    \
			*rb = def;                       \
		if(lead > 0 && is_##TPE##_nil(def))  \
			has_nils = true;                 \
	} while(0);

#define ANALYTICAL_LEAD_IMP(TPE)                  \
	do {                                          \
		TPE *rp, *rb, *bp, *rend,                 \
			def = *((TPE *) default_value), next; \
		bp = (TPE*)Tloc(b, 0);                    \
		rb = rp = (TPE*)Tloc(r, 0);               \
		rend = rb + cnt;                          \
		if(lead == BUN_NONE) {                    \
			has_nils = true;                      \
			for(; rb<rend; rb++)                  \
				*rb = TPE##_nil;                  \
		} else if(p) {                            \
			pnp = np = (bit*)Tloc(p, 0);          \
			end = np + cnt;                       \
			for(; np<end; np++) {                 \
				if (*np) {                        \
					ncnt = (np - pnp);            \
					rp += ncnt;                   \
					LEAD_CALC(TPE)                \
					pnp = np;                     \
				}                                 \
			}                                     \
			ncnt = (np - pnp);                    \
			rp += ncnt;                           \
			LEAD_CALC(TPE)                        \
		} else {                                  \
			ncnt = cnt;                           \
			rp += ncnt;                           \
			LEAD_CALC(TPE)                        \
		}                                         \
		goto finish;                              \
	} while(0);

#define ANALYTICAL_LEAD_OTHERS                                                 \
	do {                                                                       \
		j += ncnt;                                                             \
		if(lead < ncnt) {                                                      \
			m = ncnt - lead;                                                   \
			for(i=0,n=k+lead; i<m; i++, n++) {                                 \
				curval = BUNtail(bpi, n);                                      \
				if ((gdk_res = BUNappend(r, curval, false)) != GDK_SUCCEED)    \
					goto finish;                                               \
				if((*atomcmp)(curval, nil) == 0)                               \
					has_nils = true;                                           \
			}                                                                  \
			k += i;                                                            \
		}                                                                      \
		for(; k<j; k++) {                                                      \
			if ((gdk_res = BUNappend(r, default_value, false)) != GDK_SUCCEED) \
				goto finish;                                                   \
		}                                                                      \
		if(lead > 0 && (*atomcmp)(default_value, nil) == 0)                    \
			has_nils = true;                                                   \
	} while(0);

gdk_return
GDKanalyticallead(BAT *r, BAT *b, BAT *p, BAT *o, BUN lead, const void* restrict default_value, int tpe)
{
	int (*atomcmp)(const void *, const void *);
	const void* restrict nil;
	BUN i = 0, j = 0, k = 0, l = 0, ncnt, cnt = BATcount(b);
	bit *np, *pnp, *end;
	gdk_return gdk_res = GDK_SUCCEED;
	bool has_nils = false;

	assert(default_value);

	(void) o;
	switch (tpe) {
		case TYPE_bte:
			ANALYTICAL_LEAD_IMP(bte)
			break;
		case TYPE_sht:
			ANALYTICAL_LEAD_IMP(sht)
			break;
		case TYPE_int:
			ANALYTICAL_LEAD_IMP(int)
			break;
		case TYPE_lng:
			ANALYTICAL_LEAD_IMP(lng)
			break;
#ifdef HAVE_HGE
		case TYPE_hge:
			ANALYTICAL_LEAD_IMP(hge)
			break;
#endif
		case TYPE_flt:
			ANALYTICAL_LEAD_IMP(flt)
			break;
		case TYPE_dbl:
			ANALYTICAL_LEAD_IMP(dbl)
			break;
		default: {
			BUN m = 0, n = 0;
			BATiter bpi = bat_iterator(b);
			const void *restrict curval;
			nil = ATOMnilptr(tpe);
			atomcmp = ATOMcompare(tpe);
			if(lead == BUN_NONE) {
				has_nils = true;
				for (j=0;j < cnt; j++) {
					if ((gdk_res = BUNappend(r, nil, false)) != GDK_SUCCEED)
						goto finish;
				}
			} else if(p) {
				pnp = np = (bit*)Tloc(p, 0);
				end = np + cnt;
				for(; np<end; np++) {
					if (*np) {
						ncnt = (np - pnp);
						ANALYTICAL_LEAD_OTHERS
						pnp = np;
					}
				}
				ncnt = (np - pnp);
				ANALYTICAL_LEAD_OTHERS
			} else {
				ncnt = cnt;
				ANALYTICAL_LEAD_OTHERS
			}
		}
	}
finish:
	BATsetcount(r, cnt);
	r->tnonil = !has_nils;
	r->tnil = has_nils;
	return gdk_res;
}

#undef ANALYTICAL_LEAD_IMP
#undef LEAD_CALC
#undef ANALYTICAL_LEAD_OTHERS

#define ANALYTICAL_MIN_MAX_IMP_NO_OVERLAP(TPE, IMP) \
	do {                                          \
		curval = *pbp;                            \
		pbp++;                                    \
		for(; pbp<bp; pbp++) {                    \
			v = *pbp;                             \
			if(!is_##TPE##_nil(v)) {              \
				if(is_##TPE##_nil(curval))        \
					curval = v;                   \
				else                              \
					curval = IMP(v, curval);      \
			}                                     \
		}                                         \
		for (;rb < rp; rb++)                      \
			*rb = curval;                         \
		if(is_##TPE##_nil(curval))                \
			has_nils = true;                      \
	} while(0);

#define ANALYTICAL_MIN_MAX_IMP_ROWS(TPE, IMP)         \
	do {                                              \
		TPE *bs, *bl, *be;                            \
		bl = pbp;                                     \
		for(; pbp<bp;pbp++) {                         \
			bs = (pbp > bl+start) ? pbp - start : bl; \
			be = (pbp+end < bp) ? pbp + end + 1 : bp; \
			curval = *bs;                             \
			bs++;                                     \
			for(; bs<be; bs++) {                      \
				v = *bs;                              \
				if(!is_##TPE##_nil(v)) {              \
					if(is_##TPE##_nil(curval))        \
						curval = v;                   \
					else                              \
						curval = IMP(v, curval);      \
				}                                     \
			}                                         \
			*rb = curval;                             \
			rb++;                                     \
			if(is_##TPE##_nil(curval))                \
				has_nils = true;                      \
		}                                             \
	} while(0);

#define ANALYTICAL_MIN_MAX_CALC(TPE, IMP, REAL)    \
	do {                                           \
		TPE *rp, *rb, *pbp, *bp, *rend, curval, v; \
		rb = rp = (TPE*)Tloc(r, 0);                \
		pbp = bp = (TPE*)Tloc(b, 0);               \
		rend = rp + cnt;                           \
		if (p) {                                   \
			pnp = np = (bit*)Tloc(p, 0);           \
			nend = np + cnt;                       \
			for(; np<nend; np++) {                 \
				if (*np) {                         \
					ncnt = (np - pnp);             \
					rp += ncnt;                    \
					bp += ncnt;                    \
					REAL(TPE, IMP)                 \
					pnp = np;                      \
					pbp = bp;                      \
				}                                  \
			}                                      \
			ncnt = (np - pnp);                     \
			rp += ncnt;                            \
			bp += ncnt;                            \
			REAL(TPE, IMP)                         \
		} else if (o || force_order) {             \
			rp += cnt;                             \
			bp += cnt;                             \
			REAL(TPE, IMP)                         \
		} else {                                   \
			for(; rp<rend; rp++, bp++) {           \
				v = *bp;                           \
				if(is_##TPE##_nil(v))              \
					has_nils = true;               \
				*rp = v;                           \
			}                                      \
		}                                          \
	} while(0);

#ifdef HAVE_HUGE
#define ANALYTICAL_MIN_MAX_LIMIT(IMP, REAL)     \
	case TYPE_hge:                              \
		ANALYTICAL_MIN_MAX_CALC(hge, IMP, REAL) \
	break;
#else
#define ANALYTICAL_MIN_MAX_LIMIT(IMP, REAL)
#endif

#define ANALYTICAL_MIN_MAX_OTHERS_IMP_NO_OVERLAP(SIGN_OP)                     \
	do {                                                                      \
		l = j;                                                                \
		curval = BUNtail(bpi, j);                                             \
		j++;                                                                  \
		for (;j < i; j++) {                                                   \
			void *next = BUNtail(bpi, j);                                     \
			if((*atomcmp)(next, nil) != 0) {                                  \
				if((*atomcmp)(curval, nil) == 0)                              \
					curval = next;                                            \
				else                                                          \
					curval = atomcmp(next, curval) SIGN_OP 0 ? curval : next; \
			}                                                                 \
		}                                                                     \
		j = l;                                                                \
		for (;j < i; j++) {                                                   \
			if ((gdk_res = BUNappend(r, curval, false)) != GDK_SUCCEED)       \
				goto finish;                                                  \
		}                                                                     \
		if((*atomcmp)(curval, nil) == 0)                                      \
			has_nils = true;                                                  \
	} while(0);

#define ANALYTICAL_MIN_MAX_OTHERS_IMP_ROWS(SIGN_OP)                               \
	do {                                                                          \
		m = k;                                                                    \
		for(;k<i;k++) {                                                           \
			j = (k > m+start) ? k - start : m;                                    \
			l = (k+end < i) ? k + end + 1 : i;                                    \
			curval = BUNtail(bpi, j);                                             \
			j++;                                                                  \
			for (;j < l; j++) {                                                   \
				void *next = BUNtail(bpi, j);                                     \
				if((*atomcmp)(next, nil) != 0) {                                  \
					if((*atomcmp)(curval, nil) == 0)                              \
						curval = next;                                            \
					else                                                          \
						curval = atomcmp(next, curval) SIGN_OP 0 ? curval : next; \
				}                                                                 \
			}                                                                     \
			if ((gdk_res = BUNappend(r, curval, false)) != GDK_SUCCEED)           \
				goto finish;                                                      \
			if((*atomcmp)(curval, nil) == 0)                                      \
				has_nils = true;                                                  \
		}                                                                         \
	} while(0);

#define ANALYTICAL_MIN_MAX_OTHERS_CALC(SIGN_OP, REAL)                     \
	do {                                                                  \
		BATiter bpi = bat_iterator(b);                                    \
		void *restrict curval = BUNtail(bpi, 0);                          \
		nil = ATOMnilptr(tpe);                                            \
		atomcmp = ATOMcompare(tpe);                                       \
		if (p) {                                                          \
			pnp = np = (bit*)Tloc(p, 0);                                  \
			nend = np + cnt;                                              \
			for(; np<nend; np++) {                                        \
				if (*np) {                                                \
					i += (np - pnp);                                      \
					REAL(SIGN_OP)                                         \
					pnp = np;                                             \
				}                                                         \
			}                                                             \
			i += (np - pnp);                                              \
			REAL(SIGN_OP)                                                 \
		} else if (o || force_order) {                                    \
			i += cnt;                                                     \
			REAL(SIGN_OP)                                                 \
		} else {                                                          \
			for(i=0; i<cnt; i++) {                                        \
				void *next = BUNtail(bpi, i);                             \
				if((*atomcmp)(next, nil) == 0)                            \
					has_nils = true;                                      \
				if ((gdk_res = BUNappend(r, next, false)) != GDK_SUCCEED) \
					goto finish;                                          \
			}                                                             \
		}                                                                 \
	} while(0);

#define ANALYTICAL_MIN_MAX_BRANCHES(OP, IMP, SIGN_OP, FRAME) \
	switch(tpe) { \
		case TYPE_bit: \
			ANALYTICAL_MIN_MAX_CALC(bit, IMP, ANALYTICAL_MIN_MAX_IMP##FRAME) \
			break; \
		case TYPE_bte: \
			ANALYTICAL_MIN_MAX_CALC(bte, IMP, ANALYTICAL_MIN_MAX_IMP##FRAME) \
			break; \
		case TYPE_sht: \
			ANALYTICAL_MIN_MAX_CALC(sht, IMP, ANALYTICAL_MIN_MAX_IMP##FRAME) \
			break; \
		case TYPE_int: \
			ANALYTICAL_MIN_MAX_CALC(int, IMP, ANALYTICAL_MIN_MAX_IMP##FRAME) \
			break; \
		case TYPE_lng: \
			ANALYTICAL_MIN_MAX_CALC(lng, IMP, ANALYTICAL_MIN_MAX_IMP##FRAME) \
			break; \
		ANALYTICAL_MIN_MAX_LIMIT(IMP, ANALYTICAL_MIN_MAX_IMP##FRAME) \
		case TYPE_flt: \
			ANALYTICAL_MIN_MAX_CALC(flt, IMP, ANALYTICAL_MIN_MAX_IMP##FRAME) \
			break; \
		case TYPE_dbl: \
			ANALYTICAL_MIN_MAX_CALC(dbl, IMP, ANALYTICAL_MIN_MAX_IMP##FRAME) \
			break; \
		default: \
			ANALYTICAL_MIN_MAX_OTHERS_CALC(SIGN_OP, ANALYTICAL_MIN_MAX_OTHERS_IMP##FRAME) \
	}

#define ANALYTICAL_MIN_MAX(OP, IMP, SIGN_OP) \
gdk_return \
GDKanalytical##OP(BAT *r, BAT *b, BAT *p, BAT *o, bit force_order, int tpe, BUN start, BUN end) \
{ \
	int (*atomcmp)(const void *, const void *); \
	const void* restrict nil; \
	bool has_nils = false; \
	BUN i = 0, j = 0, l = 0, k = 0, m = 0, ncnt, cnt = BATcount(b); \
	bit *np, *pnp, *nend; \
	gdk_return gdk_res = GDK_SUCCEED; \
 \
	if(start == 0 && end == 0) { \
		ANALYTICAL_MIN_MAX_BRANCHES(OP, IMP, SIGN_OP, _NO_OVERLAP) \
	} else { \
		ANALYTICAL_MIN_MAX_BRANCHES(OP, IMP, SIGN_OP, _ROWS) \
	} \
finish: \
	BATsetcount(r, cnt); \
	r->tnonil = !has_nils; \
	r->tnil = has_nils; \
	return gdk_res; \
}

ANALYTICAL_MIN_MAX(min, MIN, >)
ANALYTICAL_MIN_MAX(max, MAX, <)

#undef ANALYTICAL_MIN_MAX_CALC
#undef ANALYTICAL_MIN_MAX_IMP_ROWS
#undef ANALYTICAL_MIN_MAX_IMP_NO_OVERLAP
#undef ANALYTICAL_MIN_MAX_OTHERS_CALC
#undef ANALYTICAL_MIN_MAX_OTHERS_IMP_ROWS
#undef ANALYTICAL_MIN_MAX_OTHERS_IMP_NO_OVERLAP
#undef ANALYTICAL_MIN_MAX
#undef ANALYTICAL_MIN_MAX_BRANCHES
#undef ANALYTICAL_MIN_MAX_LIMIT

#define ANALYTICAL_COUNT_IGNORE_NILS_IMP_NO_OVERLAP \
	do {                                            \
		for (;rb < rp; rb++)                        \
			*rb = curval;                           \
	} while(0);

#define ANALYTICAL_COUNT_IGNORE_NILS_IMP_ROWS       \
	do {                                            \
		lng *rs = rb, *fs, *fe;                     \
		for(; rb<rp;rb++) {                         \
			fs = (rb > rs+start) ? rb - start : rs; \
			fe = (rb+end < rp) ? rb + end + 1 : rp; \
			*rb = (fe - fs);                        \
		}                                           \
	} while(0);

#define ANALYTICAL_COUNT_IGNORE_NILS_CALC(FRAME)            \
	do {                                                    \
		lng *rp, *rb, curval = 0;                           \
		rb = rp = (lng*)Tloc(r, 0);                         \
		if (p) {                                            \
			np = pnp = (bit*)Tloc(p, 0);                    \
			nend = np + cnt;                                \
			for(; np < nend; np++) {                        \
				if (*np) {                                  \
					curval = np - pnp;                      \
					rp += curval;                           \
					ANALYTICAL_COUNT_IGNORE_NILS_IMP##FRAME \
					pnp = np;                               \
				}                                           \
			}                                               \
			curval = np - pnp;                              \
			rp += curval;                                   \
			ANALYTICAL_COUNT_IGNORE_NILS_IMP##FRAME         \
		} else {                                            \
			curval = cnt;                                   \
			rp += curval;                                   \
			ANALYTICAL_COUNT_IGNORE_NILS_IMP##FRAME         \
		}                                                   \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP_NO_OVERLAP(TPE) \
	do {                                                   \
		for (;pbp < bp; pbp++)                             \
			curval += !is_##TPE##_nil(*pbp);               \
		for (;rb < rp; rb++)                               \
			*rb = curval;                                  \
		curval = 0;                                        \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP_ROWS(TPE)\
	do {                                                \
		TPE *bs, *bl, *be;                              \
		bl = pbp;                                       \
		for(; pbp<bp;pbp++) {                           \
			bs = (pbp > bl+start) ? pbp - start : bl;   \
			be = (pbp+end < bp) ? pbp + end + 1 : bp;   \
			for(; bs<be; bs++)                          \
				curval += !is_##TPE##_nil(*bs);         \
			*rb = curval;                               \
			rb++;                                       \
			curval = 0;                                 \
		}                                               \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_CALC(TPE, IMP)\
	do {                                                 \
		TPE *pbp, *bp = (TPE*)Tloc(b, 0);                \
		lng *rp, *rb, curval = 0;                        \
		rb = rp = (lng*)Tloc(r, 0);                      \
		pbp = bp;                                        \
		if (p) {                                         \
			pnp = np = (bit*)Tloc(p, 0);                 \
			nend = np + cnt;                             \
			for(; np<nend; np++) {                       \
				if (*np) {                               \
					ncnt = np - pnp;                     \
					bp += ncnt;                          \
					rp += ncnt;                          \
					IMP(TPE)                             \
					pnp = np;                            \
					pbp = bp;                            \
				}                                        \
			}                                            \
			ncnt = np - pnp;                             \
			bp += ncnt;                                  \
			rp += ncnt;                                  \
			IMP(TPE)                                     \
		} else {                                         \
			bp += cnt;                                   \
			rp += cnt;                                   \
			IMP(TPE)                                     \
		}                                                \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_STR_IMP_NO_OVERLAP(TPE_CAST, OFFSET)  \
	do {                                                              \
		for(;j<i;j++)                                                 \
			curval += base[(var_t) ((TPE_CAST) bp) OFFSET] != '\200'; \
		for (;rb < rp; rb++)                                          \
			*rb = curval;                                             \
		curval = 0;                                                   \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_STR_IMP_ROWS(TPE_CAST, OFFSET)            \
	do {                                                                  \
		m = k;                                                            \
		for(;k<i;k++) {                                                   \
			j = (k > m+start) ? k - start : m;                            \
			l = (k+end < i) ? k + end + 1 : i;                            \
			for(; j<l; j++)                                               \
				curval += base[(var_t) ((TPE_CAST) bp) OFFSET] != '\200'; \
			*rb = curval;                                                 \
			rb++;                                                         \
			curval = 0;                                                   \
		}                                                                 \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_STR_CALC(TPE_CAST, OFFSET, IMP)\
	do {                                                       \
		const void *restrict bp = Tloc(b, 0);                  \
		lng *rp, *rb, curval = 0;                              \
		rb = rp = (lng*)Tloc(r, 0);                            \
		if (p) {                                               \
			pnp = np = (bit*)Tloc(p, 0);                       \
			nend = np + cnt;                                   \
			for(; np<nend; np++) {                             \
				if (*np) {                                     \
				    ncnt = (np - pnp);                         \
					rp += ncnt;                                \
					i += ncnt;                                 \
					IMP(TPE_CAST, OFFSET)                      \
					pnp = np;                                  \
				}                                              \
			}                                                  \
			ncnt = (np - pnp);                                 \
			rp += ncnt;                                        \
			i += ncnt;                                         \
			IMP(TPE_CAST, OFFSET)                              \
		} else {                                               \
			rp += cnt;                                         \
			i += cnt;                                          \
			IMP(TPE_CAST, OFFSET)                              \
		}                                                      \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_VARSIZED_TYPES_NO_OVERLAP               \
	do {                                                                \
		for(; j<i; j++)                                                 \
			curval += (*cmp)(nil, base + ((const var_t *) bp)[j]) != 0; \
		for (;rb < rp; rb++)                                            \
			*rb = curval;                                               \
		curval = 0;                                                     \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_FIXEDSIZE_TYPES_NO_OVERLAP \
	do {                                                   \
		for(; j<i; j++)                                    \
			curval += (*cmp)(Tloc(b, j), nil) != 0;        \
		for (;rb < rp; rb++)                               \
			*rb = curval;                                  \
		curval = 0;                                        \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_VARSIZED_TYPES_ROWS                         \
	do {                                                                    \
		m = k;                                                              \
		for(;k<i;k++) {                                                     \
			j = (k > m+start) ? k - start : m;                              \
			l = (k+end < i) ? k + end + 1 : i;                              \
			for(; j<l; j++)                                                 \
				curval += (*cmp)(nil, base + ((const var_t *) bp)[j]) != 0; \
			*rb = curval;                                                   \
			rb++;                                                           \
			curval = 0;                                                     \
		}                                                                   \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_FIXEDSIZE_TYPES_ROWS    \
	do {                                                \
		m = k;                                          \
		for(;k<i;k++) {                                 \
			j = (k > m+start) ? k - start : m;          \
			l = (k+end < i) ? k + end + 1 : i;          \
			for(; j<l; j++)                             \
				curval += (*cmp)(Tloc(b, j), nil) != 0; \
			*rb = curval;                               \
			rb++;                                       \
			curval = 0;                                 \
		}                                               \
	} while(0);

#define ANALYTICAL_COUNT_NO_NIL_OTHER_TYPES_CALC(IMP_VARSIZED, IMP_FIXEDSIZE) \
	do {                                                                 \
		const void *restrict nil = ATOMnilptr(tpe);                      \
		int (*cmp)(const void *, const void *) = ATOMcompare(tpe);       \
		lng *rp, *rb, curval = 0;                                        \
		rb = rp = (lng*)Tloc(r, 0);                                      \
		if (b->tvarsized) {                                              \
			const char *restrict base = b->tvheap->base;                 \
			const void *restrict bp = Tloc(b, 0);                        \
			if (p) {                                                     \
				pnp = np = (bit*)Tloc(p, 0);                             \
				nend = np + cnt;                                         \
				for(; np < nend; np++) {                                 \
					if (*np) {                                           \
						ncnt = (np - pnp);                               \
						rp += ncnt;                                      \
						i += ncnt;                                       \
						IMP_VARSIZED                                     \
						pnp = np;                                        \
					}                                                    \
				}                                                        \
				ncnt = (np - pnp);                                       \
				rp += ncnt;                                              \
				i += ncnt;                                               \
				IMP_VARSIZED                                             \
			} else {                                                     \
				rp += cnt;                                               \
				i += cnt;                                                \
				IMP_VARSIZED                                             \
			}                                                            \
		} else {                                                         \
			if (p) {                                                     \
				pnp = np = (bit*)Tloc(p, 0);                             \
				nend = np + cnt;                                         \
				for(; np < nend; np++) {                                 \
					if (*np) {                                           \
						ncnt = (np - pnp);                               \
						rp += ncnt;                                      \
						i += ncnt;                                       \
						IMP_FIXEDSIZE                                    \
						pnp = np;                                        \
					}                                                    \
				}                                                        \
				ncnt = (np - pnp);                                       \
				rp += ncnt;                                              \
				i += ncnt;                                               \
				IMP_FIXEDSIZE                                            \
			} else {                                                     \
				rp += cnt;                                               \
				i += cnt;                                                \
				IMP_FIXEDSIZE                                            \
			}                                                            \
		}                                                                \
	} while(0);

#ifdef HAVE_HGE
#define ANALYTICAL_COUNT_LIMIT(FRAME) \
	case TYPE_hge: \
		ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_CALC(hge, ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP##FRAME) \
		break;
#else
#define ANALYTICAL_COUNT_LIMIT(FRAME)
#endif

#if SIZEOF_VAR_T != SIZEOF_INT
#define ANALYTICAL_COUNT_STR_LIMIT(FRAME) \
	case 4: \
		ANALYTICAL_COUNT_NO_NIL_STR_CALC(const unsigned int *, [j], ANALYTICAL_COUNT_NO_NIL_STR_IMP##FRAME) \
		break;
#else
#define ANALYTICAL_COUNT_STR_LIMIT(FRAME)
#endif

#define ANALYTICAL_COUNT_BRANCHES(FRAME) \
	switch (tpe) { \
		case TYPE_bit: \
			ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_CALC(bit, ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP##FRAME) \
			break; \
		case TYPE_bte: \
			ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_CALC(bte, ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP##FRAME) \
			break; \
		case TYPE_sht: \
			ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_CALC(sht, ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP##FRAME) \
			break; \
		case TYPE_int: \
			ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_CALC(int, ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP##FRAME) \
			break; \
		case TYPE_lng: \
			ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_CALC(lng, ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP##FRAME) \
			break; \
		ANALYTICAL_COUNT_LIMIT(FRAME) \
		case TYPE_flt: \
			ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_CALC(flt, ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP##FRAME) \
			break; \
		case TYPE_dbl: \
			ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_CALC(dbl, ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP##FRAME) \
			break; \
		case TYPE_str: { \
			const char *restrict base = b->tvheap->base; \
			switch (b->twidth) { \
				case 1: \
					ANALYTICAL_COUNT_NO_NIL_STR_CALC(const unsigned char *, [j] + GDK_VAROFFSET, ANALYTICAL_COUNT_NO_NIL_STR_IMP##FRAME) \
					break; \
				case 2: \
					ANALYTICAL_COUNT_NO_NIL_STR_CALC(const unsigned short *, [j] + GDK_VAROFFSET, ANALYTICAL_COUNT_NO_NIL_STR_IMP##FRAME) \
					break; \
				ANALYTICAL_COUNT_STR_LIMIT(FRAME) \
				default: \
					ANALYTICAL_COUNT_NO_NIL_STR_CALC(const var_t *, [j], ANALYTICAL_COUNT_NO_NIL_STR_IMP##FRAME) \
					break; \
			} \
			break; \
		} \
		default: \
			ANALYTICAL_COUNT_NO_NIL_OTHER_TYPES_CALC(ANALYTICAL_COUNT_NO_NIL_VARSIZED_TYPES##FRAME, ANALYTICAL_COUNT_NO_NIL_FIXEDSIZE_TYPES##FRAME) \
	}

gdk_return
GDKanalyticalcount(BAT *r, BAT *b, BAT *p, BAT *o, const bit *ignore_nils, int tpe, BUN start, BUN end)
{
	BUN i = 0, j = 0, k = 0, l = 0, m = 0, ncnt, cnt = BATcount(b);
	bit *np, *pnp, *nend;
	gdk_return gdk_res = GDK_SUCCEED;

	assert(ignore_nils);
	(void) o;
	if(!*ignore_nils || b->T.nonil) {
		if(start == 0 && end == 0) {
			ANALYTICAL_COUNT_IGNORE_NILS_CALC(_NO_OVERLAP)
		} else {
			ANALYTICAL_COUNT_IGNORE_NILS_CALC(_ROWS)
		}
	} else if(start == 0 && end == 0) {
		ANALYTICAL_COUNT_BRANCHES(_NO_OVERLAP)
	} else {
		ANALYTICAL_COUNT_BRANCHES(_ROWS)
	}
	BATsetcount(r, cnt);
	r->tnonil = true;
	r->tnil = false;
	return gdk_res;
}

#undef ANALYTICAL_COUNT_IGNORE_NILS_CALC
#undef ANALYTICAL_COUNT_IGNORE_NILS_IMP_ROWS
#undef ANALYTICAL_COUNT_IGNORE_NILS_IMP_NO_OVERLAP
#undef ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_CALC
#undef ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP_ROWS
#undef ANALYTICAL_COUNT_NO_NIL_FIXED_SIZE_IMP_NO_OVERLAP
#undef ANALYTICAL_COUNT_NO_NIL_STR_CALC
#undef ANALYTICAL_COUNT_NO_NIL_STR_IMP_ROWS
#undef ANALYTICAL_COUNT_NO_NIL_STR_IMP_NO_OVERLAP
#undef ANALYTICAL_COUNT_NO_NIL_OTHER_TYPES_CALC
#undef ANALYTICAL_COUNT_NO_NIL_VARSIZED_TYPES_NO_OVERLAP
#undef ANALYTICAL_COUNT_NO_NIL_FIXEDSIZE_TYPES_NO_OVERLAP
#undef ANALYTICAL_COUNT_NO_NIL_VARSIZED_TYPES_ROWS
#undef ANALYTICAL_COUNT_NO_NIL_FIXEDSIZE_TYPES_ROWS
#undef ANALYTICAL_COUNT_LIMIT
#undef ANALYTICAL_COUNT_STR_LIMIT
#undef ANALYTICAL_COUNT_BRANCHES

#define ANALYTICAL_SUM_IMP_NO_OVERLAP(TPE1, TPE2) \
	do {                                      \
		for(; pbp<bp; pbp++) {                \
			v = *pbp;                         \
			if (!is_##TPE1##_nil(v)) {        \
				if(is_##TPE2##_nil(curval))   \
					curval = (TPE2) v;        \
				else                          \
					ADD_WITH_CHECK(TPE1, v, TPE2, curval, TPE2, curval, GDK_##TPE2##_max, goto calc_overflow); \
			}                                 \
		}                                     \
		for (;rb < rp; rb++)                  \
			*rb = curval;                     \
		if(is_##TPE2##_nil(curval))           \
			has_nils = true;                  \
		else                                  \
			curval = TPE2##_nil;              \
	} while(0);                               \

#define ANALYTICAL_SUM_IMP_ROWS(TPE1, TPE2)           \
	do {                                              \
		TPE1 *bs, *bl, *be;                           \
		bl = pbp;                                     \
		for(; pbp<bp;pbp++) {                         \
			bs = (pbp > bl+start) ? pbp - start : bl; \
			be = (pbp+end < bp) ? pbp + end + 1 : bp; \
			for(; bs<be; bs++) {                      \
				v = *bs;                              \
				if (!is_##TPE1##_nil(v)) {            \
					if(is_##TPE2##_nil(curval))       \
						curval = (TPE2) v;            \
					else                              \
						ADD_WITH_CHECK(TPE1, v, TPE2, curval, TPE2, curval, GDK_##TPE2##_max, goto calc_overflow); \
				}                                     \
			}                                         \
			*rb = curval;                             \
			rb++;                                     \
			if(is_##TPE2##_nil(curval))               \
				has_nils = true;                      \
			else                                      \
				curval = TPE2##_nil;                  \
		}                                             \
	} while(0);

#define ANALYTICAL_SUM_CALC(TPE1, TPE2, IMP)       \
	do {                                           \
		TPE1 *pbp, *bp, v;                         \
		TPE2 *rp, *rb, *rend, curval = TPE2##_nil; \
		pbp = bp = (TPE1*)Tloc(b, 0);              \
		rb = rp = (TPE2*)Tloc(r, 0);               \
		rend = rp + cnt;                           \
		if (p) {                                   \
			pnp = np = (bit*)Tloc(p, 0);           \
			nend = np + cnt;                       \
			for(; np<nend; np++) {                 \
				if (*np) {                         \
					ncnt = (np - pnp);             \
					bp += ncnt;                    \
					rp += ncnt;                    \
					IMP(TPE1, TPE2)                \
					pnp = np;                      \
					pbp = bp;                      \
				}                                  \
			}                                      \
			ncnt = (np - pnp);                     \
			bp += ncnt;                            \
			rp += ncnt;                            \
			IMP(TPE1, TPE2)                        \
		} else if (o || force_order) {             \
			ncnt = cnt;                            \
			bp += ncnt;                            \
			rp += ncnt;                            \
			IMP(TPE1, TPE2)                        \
		} else {                                   \
			for(; rb<rend; rb++, bp++) {           \
				v = *bp;                           \
				if(is_##TPE1##_nil(v)) {           \
					*rb = TPE2##_nil;              \
					has_nils = true;               \
				} else {                           \
					*rb = (TPE2) v;                \
				}                                  \
			}                                      \
		}                                          \
		goto finish;                               \
	} while(0);

#define ANALYTICAL_SUM_FP_IMP_NO_OVERLAP(TPE1, TPE2) \
	do {                                             \
		if(dofsum(pbp, 0, 0, ncnt, rb, 1, TYPE_##TPE1, TYPE_##TPE2, NULL, NULL, NULL, 0, 0, true, false, true, \
				  "GDKanalyticalsum") == BUN_NONE) { \
			goto bailout;                            \
		}                                            \
		curval = *rb;                                \
		for (;rb < rp; rb++)                         \
			*rb = curval;                            \
		if(is_##TPE2##_nil(curval))                  \
			has_nils = true;                         \
	} while(0);

#define ANALYTICAL_SUM_FP_IMP_ROWS(TPE1, TPE2)           \
	do {                                                 \
		TPE1 *bs, *bl, *be;                              \
		bl = pbp;                                        \
		(void) curval;                                   \
		for(; pbp<bp; pbp++) {                           \
			bs = (pbp > bl+start) ? pbp - start : bl;    \
			be = (pbp+end < bp) ? pbp + end + 1 : bp;    \
			if(dofsum(bs, 0, 0, (be - bs), rb, 1, TYPE_##TPE1, TYPE_##TPE2, NULL, NULL, NULL, 0, 0, true, false, true, \
					  "GDKanalyticalsum") == BUN_NONE) { \
				goto bailout;                            \
			}                                            \
			if(is_##TPE2##_nil(*rb))                     \
				has_nils = true;                         \
			rb++;                                        \
		}                                                \
	} while(0);

#ifdef HAVE_HGE
#define ANALYTICAL_SUM_LIMIT(FRAME) \
	case TYPE_hge: { \
		switch (tp1) { \
			case TYPE_bte: \
				ANALYTICAL_SUM_CALC(bte, hge, ANALYTICAL_SUM_IMP##FRAME); \
				break; \
			case TYPE_sht: \
				ANALYTICAL_SUM_CALC(sht, hge, ANALYTICAL_SUM_IMP##FRAME); \
				break; \
			case TYPE_int: \
				ANALYTICAL_SUM_CALC(int, hge, ANALYTICAL_SUM_IMP##FRAME); \
				break; \
			case TYPE_lng: \
				ANALYTICAL_SUM_CALC(lng, hge, ANALYTICAL_SUM_IMP##FRAME); \
				break; \
			case TYPE_hge: \
				ANALYTICAL_SUM_CALC(hge, hge, ANALYTICAL_SUM_IMP##FRAME); \
				break; \
			default: \
				goto nosupport; \
		} \
		break; \
	}
#else
#define ANALYTICAL_SUM_LIMIT(FRAME)
#endif

#define ANALYTICAL_SUM_BRANCHES(FRAME) \
	switch (tp2) { \
		case TYPE_bte: { \
			switch (tp1) { \
				case TYPE_bte: \
					ANALYTICAL_SUM_CALC(bte, bte, ANALYTICAL_SUM_IMP##FRAME); \
					break; \
				default: \
					goto nosupport; \
			} \
			break; \
		} \
		case TYPE_sht: { \
			switch (tp1) { \
				case TYPE_bte: \
					ANALYTICAL_SUM_CALC(bte, sht, ANALYTICAL_SUM_IMP##FRAME); \
					break; \
				case TYPE_sht: \
					ANALYTICAL_SUM_CALC(sht, sht, ANALYTICAL_SUM_IMP##FRAME); \
					break; \
				default: \
					goto nosupport; \
			} \
			break; \
		} \
		case TYPE_int: { \
			switch (tp1) { \
				case TYPE_bte: \
					ANALYTICAL_SUM_CALC(bte, int, ANALYTICAL_SUM_IMP##FRAME); \
					break; \
				case TYPE_sht: \
					ANALYTICAL_SUM_CALC(sht, int, ANALYTICAL_SUM_IMP##FRAME); \
					break; \
				case TYPE_int: \
					ANALYTICAL_SUM_CALC(int, int, ANALYTICAL_SUM_IMP##FRAME); \
					break; \
				default: \
					goto nosupport; \
			} \
			break; \
		} \
		case TYPE_lng: { \
			switch (tp1) { \
				case TYPE_bte: \
					ANALYTICAL_SUM_CALC(bte, lng, ANALYTICAL_SUM_IMP##FRAME); \
					break; \
				case TYPE_sht: \
					ANALYTICAL_SUM_CALC(sht, lng, ANALYTICAL_SUM_IMP##FRAME); \
					break; \
				case TYPE_int: \
					ANALYTICAL_SUM_CALC(int, lng, ANALYTICAL_SUM_IMP##FRAME); \
					break; \
				case TYPE_lng: \
					ANALYTICAL_SUM_CALC(lng, lng, ANALYTICAL_SUM_IMP##FRAME); \
					break; \
				default: \
					goto nosupport; \
			} \
			break; \
		} \
		ANALYTICAL_SUM_LIMIT(FRAME) \
		case TYPE_flt: { \
			switch (tp1) { \
				case TYPE_flt: \
					ANALYTICAL_SUM_CALC(flt, flt, ANALYTICAL_SUM_FP_IMP##FRAME); \
					break; \
				default: \
					goto nosupport; \
					break; \
			} \
		} \
		case TYPE_dbl: { \
			switch (tp1) { \
				case TYPE_flt: \
					ANALYTICAL_SUM_CALC(flt, dbl, ANALYTICAL_SUM_FP_IMP##FRAME); \
					break; \
				case TYPE_dbl: \
					ANALYTICAL_SUM_CALC(dbl, dbl, ANALYTICAL_SUM_FP_IMP##FRAME); \
					break; \
				default: \
					goto nosupport; \
					break; \
			} \
		} \
		default: \
			goto nosupport; \
	}

gdk_return
GDKanalyticalsum(BAT *r, BAT *b, BAT *p, BAT *o, bit force_order, int tp1, int tp2, BUN start, BUN end)
{
	bool has_nils = false;
	BUN ncnt, cnt = BATcount(b), nils = 0;
	bit *np, *pnp, *nend;
	int abort_on_error = 1;

	if(start == 0 && end == 0) {
		ANALYTICAL_SUM_BRANCHES(_NO_OVERLAP)
	} else {
		ANALYTICAL_SUM_BRANCHES(_ROWS)
	}
bailout:
	GDKerror("error while calculating floating-point sum\n");
	return GDK_FAIL;
nosupport:
	GDKerror("sum: type combination (sum(%s)->%s) not supported.\n", ATOMname(tp1), ATOMname(tp2));
	return GDK_FAIL;
calc_overflow:
	GDKerror("22003!overflow in calculation.\n");
	return GDK_FAIL;
finish:
	BATsetcount(r, cnt);
	r->tnonil = !has_nils;
	r->tnil = has_nils;
	return GDK_SUCCEED;
}

#undef ANALYTICAL_SUM_CALC
#undef ANALYTICAL_SUM_IMP_NO_OVERLAP
#undef ANALYTICAL_SUM_IMP_ROWS
#undef ANALYTICAL_SUM_FP_IMP_NO_OVERLAP
#undef ANALYTICAL_SUM_FP_IMP_ROWS
#undef ANALYTICAL_SUM_BRANCHES
#undef ANALYTICAL_SUM_LIMIT

#define ANALYTICAL_PROD_IMP_NO_OVERLAP(TPE1, TPE2, TPE3) \
	do {                                                 \
		for(; pbp<bp; pbp++) {                           \
			v = *pbp;                                    \
			if (!is_##TPE1##_nil(v)) {                   \
				if(is_##TPE2##_nil(curval))              \
					curval = (TPE2) v;                   \
				else                                     \
					MUL4_WITH_CHECK(TPE1, v, TPE2, curval, TPE2, curval, GDK_##TPE2##_max, TPE3, goto calc_overflow); \
			}                                            \
		}                                                \
		for (;rb < rp; rb++)                             \
			*rb = curval;                                \
		if(is_##TPE2##_nil(curval))                      \
			has_nils = true;                             \
		else                                             \
			curval = TPE2##_nil;                         \
	} while (0);

#define ANALYTICAL_PROD_IMP_ROWS(TPE1, TPE2, TPE3)    \
	do {                                              \
		TPE1 *bs, *bl, *be;                           \
		bl = pbp;                                     \
		for(; pbp<bp;pbp++) {                         \
			bs = (pbp > bl+start) ? pbp - start : bl; \
			be = (pbp+end < bp) ? pbp + end + 1 : bp; \
			for(; bs<be; bs++) {                      \
				v = *bs;                              \
				if (!is_##TPE1##_nil(v)) {            \
					if(is_##TPE2##_nil(curval))       \
						curval = (TPE2) v;            \
					else                              \
						MUL4_WITH_CHECK(TPE1, v, TPE2, curval, TPE2, curval, GDK_##TPE2##_max, TPE3, \
										goto calc_overflow); \
				}                                     \
			}                                         \
			*rb = curval;                             \
			rb++;                                     \
			if(is_##TPE2##_nil(curval))               \
				has_nils = true;                      \
			else                                      \
				curval = TPE2##_nil;                  \
		}                                             \
	} while(0);

#define ANALYTICAL_PROD_CALC(TPE1, TPE2, TPE3, IMP)\
	do {                                           \
		TPE1 *pbp, *bp, v;                         \
		TPE2 *rp, *rb, *rend, curval = TPE2##_nil; \
		pbp = bp = (TPE1*)Tloc(b, 0);              \
		rb = rp = (TPE2*)Tloc(r, 0);               \
		rend = rb + cnt;                           \
		if (p) {                                   \
			pnp = np = (bit*)Tloc(p, 0);           \
			nend = np + cnt;                       \
			for(; np<nend; np++) {                 \
				if (*np) {                         \
					ncnt = np - pnp;               \
					bp += ncnt;                    \
					rp += ncnt;                    \
					IMP(TPE1, TPE2, TPE3)          \
					pnp = np;                      \
					pbp = bp;                      \
				}                                  \
			}                                      \
			ncnt = np - pnp;                       \
			bp += ncnt;                            \
			rp += ncnt;                            \
			IMP(TPE1, TPE2, TPE3)                  \
		} else if (o || force_order) {             \
			ncnt = cnt;                            \
			bp += ncnt;                            \
			rp += ncnt;                            \
			IMP(TPE1, TPE2, TPE3)                  \
		} else {                                   \
			for(; rb<rend; rb++, bp++) {           \
				v = *bp;                           \
				if(is_##TPE1##_nil(v)) {           \
					*rb = TPE2##_nil;              \
					has_nils = true;               \
				} else {                           \
					*rb = (TPE2) v;                \
				}                                  \
			}                                      \
		}                                          \
		goto finish;                               \
	} while(0);

#define ANALYTICAL_PROD_IMP_LIMIT_NO_OVERLAP(TPE1, TPE2, REAL_IMP) \
	do {                                                           \
		for(; pbp<bp; pbp++) {                                     \
			v = *pbp;                                              \
			if (!is_##TPE1##_nil(v)) {                             \
				if(is_##TPE2##_nil(curval))                        \
					curval = (TPE2) v;                             \
				else                                               \
					REAL_IMP(TPE1, v, TPE2, curval, curval, GDK_##TPE2##_max, goto calc_overflow); \
			}                                                      \
		}                                                          \
		for (;rb < rp; rb++)                                       \
			*rb = curval;                                          \
		if(is_##TPE2##_nil(curval))                                \
			has_nils = true;                                       \
		else                                                       \
			curval = TPE2##_nil;                                   \
	} while (0);

#define ANALYTICAL_PROD_IMP_LIMIT_ROWS(TPE1, TPE2, REAL_IMP)    \
	do {                                                        \
		TPE1 *bs, *bl, *be;                                     \
		bl = pbp;                                               \
		for(; pbp<bp;pbp++) {                                   \
			bs = (pbp > bl+start) ? pbp - start : bl;           \
			be = (pbp+end < bp) ? pbp + end + 1 : bp;           \
			for(; bs<be; bs++) {                                \
				v = *bs;                                        \
				if (!is_##TPE1##_nil(v)) {                      \
					if(is_##TPE2##_nil(curval))                 \
						curval = (TPE2) v;                      \
					else                                        \
						REAL_IMP(TPE1, v, TPE2, curval, curval, GDK_##TPE2##_max, goto calc_overflow); \
				}                                               \
			}                                                   \
			*rb = curval;                                       \
			rb++;                                               \
			if(is_##TPE2##_nil(curval))                         \
				has_nils = true;                                \
			else                                                \
				curval = TPE2##_nil;                            \
		}                                                       \
	} while(0);

#define ANALYTICAL_PROD_CALC_LIMIT(TPE1, TPE2, IMP, REAL_IMP)\
	do {                                                     \
		TPE1 *pbp, *bp, v;                                   \
		TPE2 *rp, *rb, *rend, curval = TPE2##_nil;           \
		pbp = bp = (TPE1*)Tloc(b, 0);                        \
		rb = rp = (TPE2*)Tloc(r, 0);                         \
		rend = rp + cnt;                                     \
		if (p) {                                             \
			pnp = np = (bit*)Tloc(p, 0);                     \
			nend = np + cnt;                                 \
			for(; np<nend; np++) {                           \
				if (*np) {                                   \
					ncnt = np - pnp;                         \
					bp += ncnt;                              \
					rp += ncnt;                              \
					IMP(TPE1, TPE2, REAL_IMP)                \
					pbp = bp;                                \
					pnp = np;                                \
				}                                            \
			}                                                \
			ncnt = np - pnp;                                 \
			bp += ncnt;                                      \
			rp += ncnt;                                      \
			IMP(TPE1, TPE2, REAL_IMP)                        \
		} else if (o || force_order) {                       \
			ncnt = cnt;                                      \
			bp += ncnt;                                      \
			rp += ncnt;                                      \
			IMP(TPE1, TPE2, REAL_IMP)                        \
		} else {                                             \
			for(; rp<rend; rp++, bp++) {                     \
				v = *bp;                                     \
				if(is_##TPE1##_nil(v)) {                     \
					*rp = TPE2##_nil;                        \
					has_nils = true;                         \
				} else {                                     \
					*rp = (TPE2) v;                          \
				}                                            \
			}                                                \
		}                                                    \
		goto finish;                                         \
	} while(0);

#define ANALYTICAL_PROD_IMP_FP_REAL(TPE1, TPE2)                                          \
	do {                                                                                 \
		if (ABSOLUTE(curval) > 1 && GDK_##TPE2##_max / ABSOLUTE(v) < ABSOLUTE(curval)) { \
			if (abort_on_error)                                                          \
				goto calc_overflow;                                                      \
			curval = TPE2##_nil;                                                         \
			nils++;                                                                      \
		} else {                                                                         \
			curval *= v;                                                                 \
		}                                                                                \
	} while(0);

#define ANALYTICAL_PROD_IMP_FP_NO_OVERLAP(TPE1, TPE2)       \
	do {                                                    \
		for(; pbp<bp; pbp++) {                              \
			v = *pbp;                                       \
			 if (!is_##TPE1##_nil(v)) {                     \
				if(is_##TPE2##_nil(curval))                 \
					curval = (TPE2) v;                      \
				else                                        \
					ANALYTICAL_PROD_IMP_FP_REAL(TPE1, TPE2) \
			}                                               \
		}                                                   \
		for (;rb < rp; rb++)                                \
			*rb = curval;                                   \
		if(is_##TPE2##_nil(curval))                         \
			has_nils = true;                                \
		else                                                \
			curval = TPE2##_nil;                            \
	} while (0);

#define ANALYTICAL_PROD_IMP_FP_ROWS(TPE1, TPE2)                 \
	do {                                                        \
		TPE1 *bs, *bl, *be;                                     \
		bl = pbp;                                               \
		for(; pbp<bp;pbp++) {                                   \
			bs = (pbp > bl+start) ? pbp - start : bl;           \
			be = (pbp+end < bp) ? pbp + end + 1 : bp;           \
			for(; bs<be; bs++) {                                \
				v = *bs;                                        \
				if (!is_##TPE1##_nil(v)) {                      \
					if(is_##TPE2##_nil(curval))                 \
						curval = (TPE2) v;                      \
					else                                        \
						ANALYTICAL_PROD_IMP_FP_REAL(TPE1, TPE2) \
				}                                               \
			}                                                   \
			*rb = curval;                                       \
			rb++;                                               \
			if(is_##TPE2##_nil(curval))                         \
				has_nils = true;                                \
			else                                                \
				curval = TPE2##_nil;                            \
		}                                                       \
	} while(0);

#define ANALYTICAL_PROD_CALC_FP(TPE1, TPE2, IMP)   \
	do {                                           \
		TPE1 *pbp, *bp, v;                         \
		TPE2 *rp, *rb, *rend, curval = TPE2##_nil; \
		pbp = bp = (TPE1*)Tloc(b, 0);              \
		rb = rp = (TPE2*)Tloc(r, 0);               \
		rend = rp + cnt;                           \
		if (p) {                                   \
			pnp = np = (bit*)Tloc(p, 0);           \
			nend = np + cnt;                       \
			for(; np<nend; np++) {                 \
				if (*np) {                         \
					ncnt = np - pnp;               \
					bp += ncnt;                    \
					rp += ncnt;                    \
					IMP(TPE1, TPE2)                \
					pbp = bp;                      \
					pnp = np;                      \
				}                                  \
			}                                      \
			ncnt = np - pnp;                       \
			bp += ncnt;                            \
			rp += ncnt;                            \
			IMP(TPE1, TPE2)                        \
		} else if (o || force_order) {             \
			ncnt = cnt;                            \
			bp += ncnt;                            \
			rp += ncnt;                            \
			IMP(TPE1, TPE2)                        \
		} else {                                   \
			for(; rp<rend; rp++, bp++) {           \
				v = *bp;                           \
				if(is_##TPE1##_nil(v)) {           \
					*rp = TPE2##_nil;              \
					has_nils = true;               \
				} else {                           \
					*rp = (TPE2) v;                \
				}                                  \
			}                                      \
		}                                          \
		goto finish;                               \
	} while(0);

#ifdef HAVE_HGE
#define ANALYTICAL_PROD_LIMIT(FRAME) \
	case TYPE_lng: { \
		switch (tp1) { \
			case TYPE_bte: \
				ANALYTICAL_PROD_CALC(bte, lng, hge, ANALYTICAL_PROD_IMP##FRAME); \
				break; \
			case TYPE_sht: \
				ANALYTICAL_PROD_CALC(sht, lng, hge, ANALYTICAL_PROD_IMP##FRAME); \
				break; \
			case TYPE_int: \
				ANALYTICAL_PROD_CALC(int, lng, hge, ANALYTICAL_PROD_IMP##FRAME); \
				break; \
			case TYPE_lng: \
				ANALYTICAL_PROD_CALC(lng, lng, hge, ANALYTICAL_PROD_IMP##FRAME); \
				break; \
			default: \
				goto nosupport; \
		} \
		break; \
	} \
	case TYPE_hge: { \
		switch (tp1) { \
			case TYPE_bte: \
				ANALYTICAL_PROD_CALC_LIMIT(bte, hge, ANALYTICAL_PROD_IMP_LIMIT##FRAME, HGEMUL_CHECK); \
				break; \
			case TYPE_sht: \
				ANALYTICAL_PROD_CALC_LIMIT(sht, hge, ANALYTICAL_PROD_IMP_LIMIT##FRAME, HGEMUL_CHECK); \
				break; \
			case TYPE_int: \
				ANALYTICAL_PROD_CALC_LIMIT(int, hge, ANALYTICAL_PROD_IMP_LIMIT##FRAME, HGEMUL_CHECK); \
				break; \
			case TYPE_lng: \
				ANALYTICAL_PROD_CALC_LIMIT(lng, hge, ANALYTICAL_PROD_IMP_LIMIT##FRAME, HGEMUL_CHECK); \
				break; \
			case TYPE_hge: \
				ANALYTICAL_PROD_CALC_LIMIT(hge, hge, ANALYTICAL_PROD_IMP_LIMIT##FRAME, HGEMUL_CHECK); \
				break; \
			default: \
				goto nosupport; \
		} \
		break; \
	}
#else
#define ANALYTICAL_PROD_LIMIT(FRAME) \
	case TYPE_lng: { \
		switch (tp1) { \
			case TYPE_bte: \
				ANALYTICAL_PROD_CALC_LIMIT(bte, lng, ANALYTICAL_PROD_IMP_LIMIT##FRAME, LNGMUL_CHECK); \
				break; \
			case TYPE_sht: \
				ANALYTICAL_PROD_CALC_LIMIT(sht, lng, ANALYTICAL_PROD_IMP_LIMIT##FRAME, LNGMUL_CHECK); \
				break; \
			case TYPE_int: \
				ANALYTICAL_PROD_CALC_LIMIT(int, lng, ANALYTICAL_PROD_IMP_LIMIT##FRAME, LNGMUL_CHECK); \
				break; \
			case TYPE_lng: \
				ANALYTICAL_PROD_CALC_LIMIT(lng, lng, ANALYTICAL_PROD_IMP_LIMIT##FRAME, LNGMUL_CHECK); \
				break; \
			default: \
				goto nosupport; \
		} \
		break; \
	}
#endif

#define ANALYTICAL_PROD_BRANCHES(FRAME) \
		switch (tp2) { \
			case TYPE_bte: { \
				switch (tp1) { \
					case TYPE_bte: \
						ANALYTICAL_PROD_CALC(bte, bte, sht, ANALYTICAL_PROD_IMP##FRAME); \
						break; \
					default: \
						goto nosupport; \
				} \
				break; \
			} \
			case TYPE_sht: { \
				switch (tp1) { \
					case TYPE_bte: \
						ANALYTICAL_PROD_CALC(bte, sht, int, ANALYTICAL_PROD_IMP##FRAME); \
						break; \
					case TYPE_sht: \
						ANALYTICAL_PROD_CALC(sht, sht, int, ANALYTICAL_PROD_IMP##FRAME); \
						break; \
					default: \
						goto nosupport; \
				} \
				break; \
			} \
			case TYPE_int: { \
				switch (tp1) { \
					case TYPE_bte: \
						ANALYTICAL_PROD_CALC(bte, int, lng, ANALYTICAL_PROD_IMP##FRAME); \
						break; \
					case TYPE_sht: \
						ANALYTICAL_PROD_CALC(sht, int, lng, ANALYTICAL_PROD_IMP##FRAME); \
						break; \
					case TYPE_int: \
						ANALYTICAL_PROD_CALC(int, int, lng, ANALYTICAL_PROD_IMP##FRAME); \
						break; \
					default: \
						goto nosupport; \
				} \
				break; \
			} \
			ANALYTICAL_PROD_LIMIT(FRAME) \
			case TYPE_flt: { \
				switch (tp1) { \
					case TYPE_flt: \
						ANALYTICAL_PROD_CALC_FP(flt, flt, ANALYTICAL_PROD_IMP_FP##FRAME); \
						break; \
					default: \
						goto nosupport; \
						break; \
				} \
			} \
			case TYPE_dbl: { \
				switch (tp1) { \
					case TYPE_flt: \
						ANALYTICAL_PROD_CALC_FP(flt, dbl, ANALYTICAL_PROD_IMP_FP##FRAME); \
						break; \
					case TYPE_dbl: \
						ANALYTICAL_PROD_CALC_FP(dbl, dbl, ANALYTICAL_PROD_IMP_FP##FRAME); \
						break; \
					default: \
						goto nosupport; \
						break; \
				} \
			} \
			default: \
				goto nosupport; \
		}

gdk_return
GDKanalyticalprod(BAT *r, BAT *b, BAT *p, BAT *o, bit force_order, int tp1, int tp2, BUN start, BUN end)
{
	bool has_nils = false;
	BUN ncnt, cnt = BATcount(b), nils = 0;
	bit *pnp, *np, *nend;
	int abort_on_error = 1;

	if(start == 0 && end == 0) {
		ANALYTICAL_PROD_BRANCHES(_NO_OVERLAP)
	} else {
		ANALYTICAL_PROD_BRANCHES(_ROWS)
	}
nosupport:
	GDKerror("prod: type combination (prod(%s)->%s) not supported.\n", ATOMname(tp1), ATOMname(tp2));
	return GDK_FAIL;
calc_overflow:
	GDKerror("22003!overflow in calculation.\n");
	return GDK_FAIL;
finish:
	BATsetcount(r, cnt);
	r->tnonil = !has_nils;
	r->tnil = has_nils;
	return GDK_SUCCEED;
}

#undef ANALYTICAL_PROD_IMP_ROWS
#undef ANALYTICAL_PROD_IMP_NO_OVERLAP
#undef ANALYTICAL_PROD_CALC
#undef ANALYTICAL_PROD_IMP_LIMIT_ROWS
#undef ANALYTICAL_PROD_IMP_LIMIT_NO_OVERLAP
#undef ANALYTICAL_PROD_CALC_LIMIT
#undef ANALYTICAL_PROD_IMP_FP_ROWS
#undef ANALYTICAL_PROD_IMP_FP_NO_OVERLAP
#undef ANALYTICAL_PROD_CALC_FP
#undef ANALYTICAL_PROD_IMP_FP_REAL
#undef ANALYTICAL_PROD_BRANCHES
#undef ANALYTICAL_PROD_LIMIT

#define ANALYTICAL_AVERAGE_IMP_NO_OVERLAP(TPE,lng_hge,LABEL)  \
	do {                                                      \
		for(; pbp<bp; pbp++) {                                \
			v = *pbp;                                         \
			if (!is_##TPE##_nil(v)) {                         \
				ADD_WITH_CHECK(TPE, v, lng_hge, sum, lng_hge, sum, GDK_##lng_hge##_max, \
							   goto avg_overflow##TPE##LABEL##no_overlap); \
				/* count only when no overflow occurs */      \
				n++;                                          \
			}                                                 \
		}                                                     \
		if(0) {                                               \
avg_overflow##TPE##LABEL##no_overlap:                         \
			assert(n > 0);                                    \
			if (sum >= 0) {                                   \
				a = (TPE) (sum / (lng_hge) n);                \
				rr = (BUN) (sum % (SBUN) n);                  \
			} else {                                          \
				sum = -sum;                                   \
				a = - (TPE) (sum / (lng_hge) n);              \
				rr = (BUN) (sum % (SBUN) n);                  \
				if (r) {                                      \
					a--;                                      \
					rr = n - rr;                              \
				}                                             \
			}                                                 \
			for(; pbp<bp; pbp++) {                            \
				v = *pbp;                                     \
				if (is_##TPE##_nil(v))                        \
					continue;                                 \
				AVERAGE_ITER(TPE, v, a, rr, n);               \
			}                                                 \
			curval = a + (dbl) rr / n;                        \
			goto calc_done##TPE##LABEL##no_overlap;           \
		}                                                     \
		curval = n > 0 ? (dbl) sum / n : dbl_nil;             \
calc_done##TPE##LABEL##no_overlap:                            \
		has_nils = has_nils || (n == 0);                      \
		for (;rb < rp; rb++)                                  \
			*rb = curval;                                     \
		n = 0;                                                \
		sum = 0;                                              \
	} while(0);

#define ANALYTICAL_AVERAGE_IMP_ROWS(TPE,lng_hge,LABEL)            \
	do {                                                          \
		TPE *bs, *bl, *be;                                        \
		bl = pbp;                                                 \
		for(; pbp<bp;pbp++) {                                     \
			bs = (pbp > bl+start) ? pbp - start : bl;             \
			be = (pbp+end < bp) ? pbp + end + 1 : bp;             \
			for(; bs<be; bs++) {                                  \
				v = *bs;                                          \
				if (!is_##TPE##_nil(v)) {                         \
					ADD_WITH_CHECK(TPE, v, lng_hge, sum, lng_hge, sum, GDK_##lng_hge##_max, \
								   goto avg_overflow##TPE##LABEL##rows); \
					/* count only when no overflow occurs */      \
					n++;                                          \
				}                                                 \
			}                                                     \
			if(0) {                                               \
avg_overflow##TPE##LABEL##rows:                                   \
				assert(n > 0);                                    \
				if (sum >= 0) {                                   \
					a = (TPE) (sum / (lng_hge) n);                \
					rr = (BUN) (sum % (SBUN) n);                  \
				} else {                                          \
					sum = -sum;                                   \
					a = - (TPE) (sum / (lng_hge) n);              \
					rr = (BUN) (sum % (SBUN) n);                  \
					if (r) {                                      \
						a--;                                      \
						rr = n - rr;                              \
					}                                             \
				}                                                 \
				for(; bs<be; bs++) {                              \
					v = *bs;                                      \
					if (is_##TPE##_nil(v))                        \
						continue;                                 \
					AVERAGE_ITER(TPE, v, a, rr, n);               \
				}                                                 \
				curval = a + (dbl) rr / n;                        \
				goto calc_done##TPE##LABEL##rows;                 \
			}                                                     \
			curval = n > 0 ? (dbl) sum / n : dbl_nil;             \
calc_done##TPE##LABEL##rows:                                      \
			has_nils = has_nils || (n == 0);                      \
			*rb = curval;                                         \
			rb++;                                                 \
			n = 0;                                                \
			sum = 0;                                              \
		}                                                         \
	} while(0);

#define ANALYTICAL_AVERAGE_LNG_HGE(TPE,lng_hge,IMP)   \
	do {                                              \
		TPE *pbp, *bp, a, v;                          \
		dbl *rp, *rb;                                 \
		pbp = bp = (TPE*)Tloc(b, 0);                  \
		rb = rp = (dbl*)Tloc(r, 0);                   \
		if (p) {                                      \
			pnp = np = (bit*)Tloc(p, 0);              \
			nend = np + cnt;                          \
			for(; np<nend; np++) {                    \
				if (*np) {                            \
					ncnt = np - pnp;                  \
					bp += ncnt;                       \
					rp += ncnt;                       \
					IMP(TPE,lng_hge,middle_partition) \
					pnp = np;                         \
					pbp = bp;                         \
				}                                     \
			}                                         \
			ncnt = np - pnp;                          \
			bp += ncnt;                               \
			rp += ncnt;                               \
			IMP(TPE,lng_hge,final_partition)          \
		} else if (o || force_order) {                \
			bp += cnt;                                \
			rp += cnt;                                \
			IMP(TPE,lng_hge,single_partition)         \
		} else {                                      \
			dbl* rend = rp + cnt;                     \
			for(; rp<rend; rp++, bp++) {              \
				v = *bp;                              \
				if(is_##TPE##_nil(v)) {               \
					*rp = dbl_nil;                    \
					has_nils = true;                  \
				} else {                              \
					*rp = (dbl) v;                    \
				}                                     \
			}                                         \
		}                                             \
		goto finish;                                  \
	} while(0);

#ifdef HAVE_HGE
#define ANALYTICAL_AVERAGE_CALC(TYPE,IMP) ANALYTICAL_AVERAGE_LNG_HGE(TYPE,hge,IMP)
#else
#define ANALYTICAL_AVERAGE_CALC(TYPE,IMP) ANALYTICAL_AVERAGE_LNG_HGE(TYPE,lng,IMP)
#endif

#define ANALYTICAL_AVERAGE_FLOAT_IMP_NO_OVERLAP(TPE) \
	do {                                          \
		for(; pbp<bp; pbp++) {                    \
			v = *pbp;                             \
			if (!is_##TPE##_nil(v))               \
				AVERAGE_ITER_FLOAT(TPE, v, a, n); \
		}                                         \
		curval = n > 0 ? a : dbl_nil;             \
		has_nils = has_nils || (n == 0);          \
		for (;rb < rp; rb++)                      \
			*rb = curval;                         \
		n = 0;                                    \
		a = 0;                                    \
	} while(0);

#define ANALYTICAL_AVERAGE_FLOAT_IMP_ROWS(TPE)        \
	do {                                              \
		TPE *bs, *bl, *be;                            \
		bl = pbp;                                     \
		for(; pbp<bp;pbp++) {                         \
			bs = (pbp > bl+start) ? pbp - start : bl; \
			be = (pbp+end < bp) ? pbp + end + 1 : bp; \
			for(; bs<be; bs++) {                      \
				v = *bs;                              \
				if (!is_##TPE##_nil(v))               \
					AVERAGE_ITER_FLOAT(TPE, v, a, n); \
			}                                         \
			curval = n > 0 ? a : dbl_nil;             \
			has_nils = has_nils || (n == 0);          \
			*rb = curval;                             \
			rb++;                                     \
			n = 0;                                    \
			a = 0;                                    \
		}                                             \
	} while(0);

#define ANALYTICAL_AVERAGE_CALC_FLOAT(TPE, IMP) \
	do {                                   \
		TPE *pbp, *bp, v;                  \
		dbl *rp, *rb, a = 0;               \
		pbp = bp = (TPE*)Tloc(b, 0);       \
		rb = rp = (dbl*)Tloc(r, 0);        \
		if (p) {                           \
			pnp = np = (bit*)Tloc(p, 0);   \
			nend = np + cnt;               \
			for(; np<nend; np++) {         \
				if (*np) {                 \
					ncnt = np - pnp;       \
					bp += ncnt;            \
					rp += ncnt;            \
					IMP(TPE)               \
					pbp = bp;              \
					pnp = np;              \
				}                          \
			}                              \
			ncnt = np - pnp;               \
			bp += ncnt;                    \
			rp += ncnt;                    \
			IMP(TPE)                       \
		} else if (o || force_order) {     \
			bp += cnt;                     \
			rp += cnt;                     \
			IMP(TPE)                       \
		} else {                           \
			dbl *rend = rp + cnt;          \
			for(; rp<rend; rp++, bp++) {   \
				if(is_##TPE##_nil(*bp)) {  \
					*rp = dbl_nil;         \
					has_nils = true;       \
				} else {                   \
					*rp = (dbl) *bp;       \
				}                          \
			}                              \
		}                                  \
		goto finish;                       \
	} while(0);

#ifdef HAVE_HGE
#define ANALYTICAL_AVERAGE_LIMIT(FRAME) \
	case TYPE_hge: \
		ANALYTICAL_AVERAGE_CALC(hge, ANALYTICAL_AVERAGE_IMP##FRAME); \
		break;
#else
#define ANALYTICAL_AVERAGE_LIMIT(FRAME)
#endif

#define ANALYTICAL_AVERAGE_BRANCHES(FRAME) \
	switch (tpe) { \
		case TYPE_bte: \
			ANALYTICAL_AVERAGE_CALC(bte, ANALYTICAL_AVERAGE_IMP##FRAME); \
			break; \
		case TYPE_sht: \
			ANALYTICAL_AVERAGE_CALC(sht, ANALYTICAL_AVERAGE_IMP##FRAME); \
			break; \
		case TYPE_int: \
			ANALYTICAL_AVERAGE_CALC(int, ANALYTICAL_AVERAGE_IMP##FRAME); \
			break; \
		case TYPE_lng: \
			ANALYTICAL_AVERAGE_CALC(lng, ANALYTICAL_AVERAGE_IMP##FRAME); \
			break; \
		ANALYTICAL_AVERAGE_LIMIT(FRAME) \
		case TYPE_flt: \
			ANALYTICAL_AVERAGE_CALC_FLOAT(flt, ANALYTICAL_AVERAGE_FLOAT_IMP##FRAME); \
			break; \
		case TYPE_dbl: \
			ANALYTICAL_AVERAGE_CALC_FLOAT(dbl, ANALYTICAL_AVERAGE_FLOAT_IMP##FRAME); \
			break; \
		default: \
			GDKerror("GDKanalyticalavg: average of type %s unsupported.\n", ATOMname(tpe)); \
			return GDK_FAIL; \
	}

gdk_return
GDKanalyticalavg(BAT *r, BAT *b, BAT *p, BAT *o, bit force_order, int tpe, BUN start, BUN end)
{
	bool has_nils = false;
	BUN ncnt, cnt = BATcount(b), nils = 0, n = 0, rr;
	bit *np, *pnp, *nend;
	bool abort_on_error = true;
	dbl curval;
#ifdef HAVE_HGE
	hge sum = 0;
#else
	lng sum = 0;
#endif

	if(start == 0 && end == 0) {
		ANALYTICAL_AVERAGE_BRANCHES(_NO_OVERLAP)
	} else {
		ANALYTICAL_AVERAGE_BRANCHES(_ROWS)
	}
finish:
	BATsetcount(r, cnt);
	r->tnonil = !has_nils;
	r->tnil = has_nils;
	return GDK_SUCCEED;
}

#undef ANALYTICAL_AVERAGE_CALC
#undef ANALYTICAL_AVERAGE_LNG_HGE
#undef ANALYTICAL_AVERAGE_IMP_ROWS
#undef ANALYTICAL_AVERAGE_IMP_NO_OVERLAP
#undef ANALYTICAL_AVERAGE_CALC_FLOAT
#undef ANALYTICAL_AVERAGE_FLOAT_IMP_ROWS
#undef ANALYTICAL_AVERAGE_FLOAT_IMP_NO_OVERLAP
#undef ANALYTICAL_AVERAGE_BRANCHES
#undef ANALYTICAL_AVERAGE_LIMIT
