#include <stdlib.h>
#include <string.h>

#include "hashovani.h"


/**
 * djb2 string hashing alorithm
 *
 * http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long int hash_string(char *s)
{
	unsigned long int h = 5381;
	char *c = s;

	while (*c != '\0') {
		/* hash * 33 + c */
		h = ((h << 5) + h) + *c;
		c++;
	}

	return h;
}


#define set_Hash(hp, NAME, INFO, FULL, HASH, LINK) {\
	hp.name = NAME; \
	hp.info = INFO; \
	hp.full = FULL; \
	hp.hash = HASH; \
	hp.link = LINK; }

#define empty_Hash(h) \
	((h) != NULL ? memset((h)->hashes, 0, (h)->size * sizeof(HashMember)) : NULL)


static Hash *new_sized_Hash(int size)
{
	Hash *h = (Hash *) malloc(sizeof(Hash));

	h->hashes = (HashMember *) malloc(size * sizeof(HashMember));
	h->size = size;
	h->used = 0;

	empty_Hash(h);
	return h;
}


inline Hash *new_Hash()
{
	return new_sized_Hash(BASIC_HASH_SIZE);
}


static inline unsigned int free_space(Hash *h, unsigned long int hash)
{
	unsigned int index = (unsigned int) hash % h->size;

	while (h->hashes[index].full != EMPTY_HASH) {

		if (h->hashes[index].hash == hash
			|| h->hashes[index].full == DELETED_HASH)
		{
			return index;
		}

		index++;
		if (index >= (h->size)) index = 0;
	}

	return index;
}


static int resize_hash(Hash *h, int new_size)
{
	HashMember *old_hashes = h->hashes;
	HashMember *hm;
	int stara_size = h->size;

	h->size = new_size;
	h->used = 0;
	h->hashes = (HashMember *) malloc((h->size)*sizeof(HashMember));
	if (h->hashes == NULL) return 1;
	empty_Hash(h);

	for (int i=0; i<stara_size; i++) {
		if (old_hashes[i].full == FULL_HASH)
		{
			hm = add_Hash(h, old_hashes[i].hash, old_hashes[i].link);
			hm->name = old_hashes[i].name;
			hm->info = old_hashes[i].info;
		}

	}

	free(old_hashes);
	return 0;
}


HashMember *add_Hash(Hash *h, unsigned long int hash, unsigned long int p)
{
	if (h->used > (3 * (h->size)/4))
		if (resize_hash(h, h->size * 2)) return NULL;

	h->used++;

	unsigned int index = free_space(h, hash);
	if (h->hashes[index].full == FULL_HASH) h->used--;
	set_Hash(h->hashes[index], NULL, 0, FULL_HASH, hash, p);

	return &h->hashes[index];
}


HashMember *add_string_Hash(Hash *h, char *name, unsigned long int p)
{
	// copy name
	char *name_c = (char *) malloc((strlen(name)+1) * sizeof(char));
	strcpy(name_c, name);

	unsigned long int hash = hash_string(name);

	HashMember *hm = add_Hash(h, hash, p);
	if (hm) hm->name = name_c;

	return hm;
}


Hash *del_HashMember(Hash *h, HashMember *hm)
{
	if (hm && hm->full == FULL_HASH) {
		hm->full = DELETED_HASH;
		h->used--;
	}

	if (h->used > BASIC_HASH_SIZE && h->used < (h->size)/4)
		if (resize_hash(h, h->size / 2)) return NULL;

	return h;
}


HashMember *get_Hash(Hash *h, unsigned long int hash)
{
	unsigned int i = hash % h->size;

	while (h->hashes[i].full != EMPTY_HASH && h->hashes[i].hash != hash) {
		i++;
		if (i >= (h->size)) i = 0;
	}

	if (h->hashes[i].hash != hash || h->hashes[i].full != FULL_HASH)
		return NULL;

	return &h->hashes[i];
}


Hash *clone_Hash(Hash *recent)
{
	Hash *h = new_sized_Hash(recent->size);
	h->used = recent->used;
	memcpy(h->hashes, recent->hashes, h->size * sizeof(HashMember));

	return h;
}


void free_Hash(Hash *h)
{
//	for (int i=0; i<h->size; i++) {
//		if (h->hashes[i].name == NULL) continue;
//		free(h->hashes[i].name);
//	}

	free(h->hashes);
	free(h);
}
