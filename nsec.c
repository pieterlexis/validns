/*
 * Part of DNS zone file validator `validns`.
 *
 * Copyright 2011, Anton Berezin <tobez@tobez.org>
 * Modified BSD license.
 * (See LICENSE file in the distribution.)
 *
 */
#include "common.h"
#include "rr.h"

static void* nsec_parse(char *name, long ttl, int type, char *s)
{
    struct rr_nsec *rr = getmem(sizeof(*rr));
	struct binary_data bitmap;
	struct binary_data type_bitmap;
	char *str_type = NULL;
	int ltype;

    rr->next_domain = extract_name(&s, "next domain");
	/* TODO: validate next_domain, http://tools.ietf.org/html/rfc4034#section-4.1.1 */

	bitmap = new_set();
	while (s && *s) {
		str_type = extract_label(&s, "type list", "temporary");
		if (!str_type) return NULL;
		ltype = str2rdtype(str_type);
		add_bit_to_set(&bitmap, ltype);
	}
	if (!s)
		return NULL;
	if (!str_type) {
		return bitch("NSEC type list should not be empty");
	}
	type_bitmap = compressed_set(&bitmap);
	rr->type_bitmap_len = type_bitmap.length;
	rr->type_bitmap = type_bitmap.data;

    return store_record(type, name, ttl, rr);
}

static char* nsec_human(void *rrv)
{
    struct rr_nsec *rr = rrv;
    char ss[1024];
	char *s = ss;
	int l;
	char *base;
	int i, k;
	int type;
	char *type_name;

    l = snprintf(s, 1024, "%s", rr->next_domain);
	s += l;
	base = rr->type_bitmap;
	while (base - rr->type_bitmap < rr->type_bitmap_len) {
		for (i = 0; i < base[1]; i++) {
			for (k = 0; k <= 7; k++) {
				if (base[2+i] & (0x80 >> k)) {
					type = base[0]*256 + i*8 + k;
					type_name = rdtype2str(type);
					l = snprintf(s, 1024-(s-ss), " %s", type_name);
					s += l;
				}
			}
		}
		base += base[1]+2;
	}
    return quickstrdup_temp(ss);
}

static void* nsec_wirerdata(void *rrv)
{
    struct rr_nsec *rr = rrv;

    return NULL;
}

struct rr_methods nsec_methods = { nsec_parse, nsec_human, nsec_wirerdata };
