#ifndef HASHOVANI
#define HASHOVANI


typedef enum {
	EMPTY_HASH,
	FULL_HASH,
	DELETED_HASH,
} HASH_TYPE;


// pri zmene treba zmenit clone
typedef struct SHashMember {
	char *name;
	int info;
	HASH_TYPE full;
	unsigned long int hash;
	void *link;
} HashMember;


typedef struct SHash {
	HashMember *hashes;
	unsigned int size;
	unsigned int used;
} Hash;


Hash *new_Hash();

HashMember *add_Hash(Hash *h, unsigned long int hash, void *p);
HashMember *add_string_Hash(Hash *h, char *name, void *p);

HashMember *del_Hash(Hash *h, unsigned int hash);
HashMember *del_string_Hash(Hash *h, char *s);

HashMember *get_Hash(Hash *h, unsigned long int hash);
HashMember *get_string_Hash(Hash *h, char *s);

Hash *clone_Hash(Hash *h);
int free_Hash(Hash *h);

#endif
