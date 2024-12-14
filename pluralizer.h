/* pluralizer.h - make plural english nouns. */
/* Created: Sun May 16 10:07:03 PM EDT 2021 rahjiii */
/* Copyright © 1991-2021 The Last Outpost Project */
/* $Id: pluralizer.h,v 1.4 2024/01/29 03:48:46 malakai Exp $ */

#ifndef LO_PLURALIZER_H
#define LO_PLURALIZER_H

/* global #defines */
#ifndef SEX_MAX
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2
#define SEX_PLURAL    3
#define SEX_MAX       4
#endif

/* structs and typedefs */

/* exported global variable declarations */

/* exported function declarations */
int pluralize_init(void);
char *pluralize_noun(char *singular,int count);
char *pluralize_noun_phrase(char *singular, int count);
int act_parse_option(char *src,int option, char *dst, size_t len);
int act_verb_s2p(char *src,int plurality, char *dst, size_t len);
void test_verb_s2p(struct char_data *ch);
int get_sex_plurality(struct char_data *ch);
char *int_to_words(int num);
int is_vowel(char c);

char *diku_hshr(struct char_data *ch);
char *diku_hssh(struct char_data *ch);
char *diku_hmhr(struct char_data *ch);

#endif /* LO_PLURALIZER_H */
