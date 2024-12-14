/* pluralizer.c - make plural english nouns. */
/* Created: Sun May 16 10:07:03 PM EDT 2021 rahjiii */
/* Copyright © 1991-2021 The Last Outpost Project */
/* $Id: pluralizer.c,v 1.4 2022/12/12 05:16:42 malakai Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <gmodule.h>	/* glib header for hash table module */
#include <errno.h>

/* these three includes are needed for lo_log and lo_debug */
#include "structs.h"
#include "utils.h"
#include "debug.h"

#include "scnprintf.h"
#include "pluralizer.h"
#include "comm.h"

#ifndef MAX_STRING_LENGTH
#define MAX_STRING_LENGTH 4096
#endif

/* change this to whatever works for you.  Should be found in current working
 * directory at time pluralize_init() is called. */
#define NOUNS_S2P_FILENAME "plural_nouns.txt"
#define VERBS_S2P_FILENAME "plural_verbs.txt"

char *gender[]= {"neutral","masculine","feminine","plural"};
char *pronoun_possessive[]= {"its","his","hers","theirs"};
char *determiner_possessive[]= {"its","his","her","their"};
char *pronoun_personal[]= { "it","he","she","they" };
char *pronoun_thirdperson[]= { "it","him","her","them" };
char *pronoun_reflexive[]= { "itself", "himself","herself", "themselves" };

/* local variables */
GHashTable *nouns_s2p = NULL;	/* hash table dictionary will be created by pluralize_init() */
GHashTable *verbs_s2p = NULL;	/* hash table dictionary will be created by pluralize_init() */

/* local forward function declarations. */
GHashTable *pluralize_load(char *filename);
int is_suffix(char *a, char *b);

/* Code starts here. */

/* init the pluralizer noun singular to plural database. Call this once
 * somewhere early in your boot up routine.  If you call the other functions
 * without calling this init, you won't have irregualr pluralization lookups.*/
int pluralize_init(void) {

	/* load up the nouns singular to plural dictionary */
	if(!nouns_s2p) {
		nouns_s2p = pluralize_load(NOUNS_S2P_FILENAME);
	}

	/* load up the verbs singular to plural dictionary */
	if(!verbs_s2p) {
		verbs_s2p = pluralize_load(VERBS_S2P_FILENAME);
	}

	return((nouns_s2p!=0) && (verbs_s2p!=0));

}

/* Call this on shutdown, to free up the hash dictionary. */
void pluralize_free(void) {
	if(nouns_s2p) {
		g_hash_table_destroy(nouns_s2p);
	}
	if(verbs_s2p) {
		g_hash_table_destroy(nouns_s2p);
	}
}


/* load a hashtable of singular to plural forms from filename. */
GHashTable *pluralize_load(char *filename) {

	FILE *nin;
	char singular[MAX_STRING_LENGTH];
	char plural[MAX_STRING_LENGTH];
	char *c;
	GHashTable *new_s2p = NULL;

	/* load up a singular to plural dictionary */

	if( !(nin = fopen(filename,"r"))) {
		lo_log("%s: %s",filename,strerror(errno));
		return(NULL);
	}

	new_s2p = g_hash_table_new(g_str_hash, g_str_equal);
	while( (fscanf(nin,"%s %s",singular,plural) == 2) ) {
		for(c=singular;*c;*c++=tolower(*c));
		for(c=plural;*c;*c++=tolower(*c));
		lo_debug(DEBUG_DB,"Adding '%s'->'%s'",singular,plural);
		g_hash_table_insert(new_s2p,g_strdup(singular),g_strdup(plural));
	}
	lo_debug(DEBUG_DB,"loaded %d singular to plural nouns from %s",
		g_hash_table_size(new_s2p),
		filename
	);

	fclose(nin);
	return(new_s2p);
}


/* is b a suffix of a? */
int is_suffix(char *a, char *b) {
	int alen=strlen(a);
	int blen=strlen(b);
	if(blen>alen) return(0);
	return( !strcmp(a+alen-blen,b) );
}

/* does what it says on the tin. */
int is_vowel(char c) {
	switch (tolower(c)) {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	case 'y':
		return(1);
	default:
		return(0);
	}
}

/* spells out a number between -99 and 99.  Anything outside that range is
 * printed numerically.  Free whatever string this gives back to you. */
char *int_to_words(int num) {

	char *ones_str[] = {
		"zero",
		"one",
		"two",
		"three",
		"four",
		"five",
		"six",
		"seven",
		"eight",
		"nine",
		"ten",
		"eleven",
		"twelve",
		"thirteen",
		"fourteen",
		"fifteen",
		"sixteen",
		"seventeen",
		"eighteen",
		"nineteen"
	};

	char *tens_str[] = {
		"zero",
		"ten",
		"twenty",
		"thirty",
		"forty",
		"fifty",
		"sixty",
		"seventy",
		"eighty",
		"ninety"
	};

	char buf[MAX_STRING_LENGTH];

	int ones, tens;
	char *posneg;
	char positive[] = "";
	char negative_word[] = "minus ";
	char negative_sym[] = "-";
	int isneg=0;
	if(num < 0) {
		isneg = 1;
		num = -num;
	}
	posneg = positive;

	if(num>99) {
		if(isneg)
			posneg = negative_sym;
		scnprintf(buf,sizeof(buf),"%s%d",posneg,num);
	} else {
		if(isneg)
			posneg = negative_word;
		if(num < 20) {
			scnprintf(buf,sizeof(buf),"%s%s",posneg,ones_str[num]);
		} else {
			tens = num/10;
			ones = num%10;
			if(ones == 0) {
				scnprintf(buf,sizeof(buf),"%s%s",posneg,tens_str[tens]);
			} else {
				scnprintf(buf,sizeof(buf),"%s%s-%s",posneg,tens_str[tens],ones_str[ones]);
			}
		}
	}
	return(strdup(buf));
}

/* Pluralize a singular english word.  ex given "sword" returns "swords".  ONLY
 * works on single words, with no spaces.  Based on rules listed at
 * https://www.grammarly.com/blog/plural-nouns/ Be sure to free the string that
 * this gives back to you.*/
char *pluralize_noun(char *singular,int count) {

	int slen;
	char *s,*send, *s1,*s2;
	int r1,r2;
	char plural[MAX_STRING_LENGTH];
	char *p,*pend;
	char *word,*w;

	if(!singular) return(NULL); /* GIGO. */
	
	s = singular;
	slen = strlen(singular);
	send = s+slen;

	p = plural;
	pend = plural + sizeof(plural);

	/* if there's exactly 1, there's nothing to do. */
	if(count==1) {
		return(strdup(singular));
	}

	/*irregular pluralizaation dictionary lookup first. */
	if(nouns_s2p) {
		word = strdup(singular);
		for(w=word;*w;*w++=tolower(*w)); /*downcase a copy of the word.*/
		w = g_hash_table_lookup(nouns_s2p,word);
		free(word);
		if(w) {
			return(strdup(w));
		}
	}

	/* ok... derive the plural from some rules. */

	r1 = MAX(slen-1,0); /* root-1 of singular */
	r2 = MAX(slen-2,0); /* root-2 of singular */
	s1 = singular + r1; /* last char of word */
	s2 = singular + r2; /* last two chars of word */

	/* RULE: If the singular noun ends in -ss, -sh, -ch, -x, or -z, add ‑es
	 * to the end to make it plural.  (In some cases, singular nouns ending in
	 * -s or -z, require that you double the -s or -z prior to adding the -es
	 * for pluralization, but skipping that for now.) */
	if( !strcasecmp(s2,"ss") ||
		!strcasecmp(s2,"sh") ||
		!strcasecmp(s2,"ch") ||
		!strcasecmp(s1,"x") ||
		!strcasecmp(s1,"o") ||
		!strcasecmp(s1,"z")
	) {
		scnprintf(p,pend-p,"%ses",singular);
		return(strdup(plural));
	}

	/* RULE: If the noun ends with ‑f or ‑fe, the f is often changed to ‑ve before
	 * adding the -s to form the plural version. */
	if( !strcasecmp(s2,"fe") ) {
		scnprintf(p,pend-p,"%.*sves",r2,singular);
		return(strdup(plural));
	}
	if( !strcasecmp(s1,"f") &&
		strcasecmp(s2,"ff")  /* if it ends if ff, leave it alone. */
	) {
		scnprintf(p,pend-p,"%.*sves",r1,singular);
		return(strdup(plural));
	}

	/* RULE: If the singular noun ends in -y... */
	if(!strcasecmp(s1,"y")) {
		if(is_vowel(*s2)) {
			/* ...and the letter before the -y is a vowel, simply add an -s to
			 * make it plural. */
			scnprintf(p,pend-p,"%ss",singular);
			return(strdup(plural));
		} else {
			/* ...and the letter before the -y is a consonant, change the
			 * ending to ‑ies to make the noun plural. */
			scnprintf(p,pend-p,"%.*sies",r1,singular);
			return(strdup(plural));
		}
	}

	/* RULE: If the singular noun ends in ‑us, the plural ending is frequently ‑i. */
	if( !strcasecmp(s2,"us") ) {
		scnprintf(p,pend-p,"%.*si",r2,singular);
		return(strdup(plural));
	}

	/* RULE: If the singular noun ends in ‑is, the plural ending is changed to -es. */
	if( !strcasecmp(s2,"is") ) {
		scnprintf(p,pend-p,"%.*ses",r2,singular);
		return(strdup(plural));
	}

	/* RULE: If the singular noun ends in ‑on, the plural ending is ‑a. 
	if( !strcasecmp(s2,"on") ) {
		snprintf(p,pend-p,"%.*sa",r2,singular);
		return(strdup(plural));
	}
	*/

	/* RULE: If the singular noun ends in ‑s, the plural ending is -es. */
	if( !strcasecmp(s1,"s") ) {
		scnprintf(p,pend-p,"%ses",singular);
		return(strdup(plural));
	}
	
	/* RULE: To make regular nouns plural, add ‑s to the end. */
	scnprintf(p,pend-p,"%ss",singular); 
	return(strdup(plural)); 
}

/* pluralize a singlular english noun phrase.  Turns a phrase like "a short
 * sword" into "six short swords" If count is zero, returns pluralized word
 * without a count.  This code uses heuristics and may sometimes produce
 * laughable crap.  Be sure to free the string that this gives back to you.*/
char *pluralize_noun_phrase(char *singular, int count) {

	int slen;
	char *s,*send;

	char plural[MAX_STRING_LENGTH];
	char *p,*pend;

	char *pluralized;
	char *lastblank;

	char *written_count;
	
	s = singular;
	slen = strlen(singular);
	send = s+slen;

	p = plural;
	pend = plural + sizeof(plural);

	/* if there is an article prefix of a, an, the, one, skip over it.*/
	for (;*s && isblank(*s);*p++ = *s++); /* copy whitespace to start of next word. */
	if( !strncasecmp(s,"a ",2) ) {
		s+=2;
	}
	if( !strncasecmp(s,"an ",3) ) {
		s+=3;
	}
	if( !strncasecmp(s,"the ",4) ) {
		s+=4;
	}
	if( !strncasecmp(s,"one ",4) ) {
		s+=4;
	}

	/* if count >=0, write the count to the result. */
	if(count >=0) {
		written_count = int_to_words(count);
		p+=scnprintf(p,pend-p,"%s ",written_count);
		free(written_count);
	}

	/* copy into the result, up to end of string or the preposition " of ", so
	 * that noun prhases like "a bag of holding" are pluralized as bags, not
	 * holdings. */
	for(lastblank=p;*s && strncasecmp(s," of ",4) && p<pend; *p++ = *s++) {
		if(isblank(*s)) {
			lastblank = p+1;
		}
	}
	*p='\0';

	/* back up, find the space, and pluralize that last word. */
	p = lastblank;
	pluralized = pluralize_noun(p,count);
	p+=scnprintf(p,pend-p,"%s",pluralized);
	free(pluralized);

	/* copy the rest of the line into the result unchanged. */
	for(;*s && p<pend; *p++ = *s++);
	*p='\0';
	return(strdup(plural));

}

/* like pluralize_noun, but for verbs.  Also based on rules from Grammarly,
 * https://www.grammarly.com/blog/grammar-basics-what-is-subject-verb-agreement/
 * .  I say based on, becuase I had to compose the converse of the rules listed
 * there to get what I wanted.  May not be quite the same. */
char *pluralize_verb(char *singular) {

	char plural[MAX_STRING_LENGTH];
	char *p,*pend;
	char *word,*w;

	if(!singular) return(NULL); /* GIGO. */
	
	p = plural;
	pend = plural + sizeof(plural);

	/*irregular pluralization dictionary lookup first. */
	if(verbs_s2p) {
		word = strdup(singular);
		for(w=word;*w;*w++=tolower(*w)); /*downcase a copy of the word.*/
		w = g_hash_table_lookup(verbs_s2p,word);
		free(word);
		if(w) {
			return(strdup(w));
		}
	}

	/* ok... derive the plural from some rules. */
	int singularlen = strlen(singular);

	/* Grammarly RULE 1: If the verb ends in -x, –ss, –sh, –ch, –tch, or –zz,
	 * you add –es to the end to match the third-person singular. 
	 * Converse: verb ends in -xes -sses -shes -ches -tches or -zzes, remove
	 * -es.
	 * test word: assesses -> assess
	 * test word: hisses -> hiss
	 * test word: catches -> catch
	 * test word: mashes -> mash
	 * test word: razzes -> razz
	 */

	if(is_suffix(singular,"xes") ||
		is_suffix(singular,"sses") ||
		is_suffix(singular,"shes") ||
		is_suffix(singular,"ches") ||
		is_suffix(singular,"tches") ||
		is_suffix(singular,"zzes") 
	) {
		return(strndup(singular,singularlen-2));
	}

	/* Grammarly RULE 2: If the verb ends in a consonant + y, remove the y and
	 * add –ies.
	 * Converse: if the verb ends in consonant +ies, remove -ies and add +y 
	 * test word: allies -> ally, glorifies -> glorify
	 */
	if((singularlen >=4) &&
		is_suffix(singular,"ies") &&
		!is_vowel(*(singular + singularlen - 4))
	) {
		p += scnprintf(p,singularlen-2,"%s",singular);
		p += scnprintf(p,pend-p,"y");
		return(strdup(plural));
	}
	
	/* Grammarly RULE 3: with words that end in a vowel + y, follow the normal
	 * format and add only –s.
	 * Converse: if the verb ends in vowel +ys remove -s
	 * test word: toys -> toy, stays -> stay
	 */
	if((singularlen >=3) &&
		is_suffix(singular,"ys") &&
		is_vowel(*(singular + singularlen - 3))
	) {
		return(strndup(singular,singularlen-1));
	}

	/* RULE 4: Add s.  
	 * Converse: if it ends is -s, remove -s 
	 * test word: steals -> steal, eats -> eat
	 */
	if( is_suffix(singular,"s") ) {
		return(strndup(singular,singularlen-1));
	}

	/* gah.  who knows. Suggest you update the dictionary.*/
	lo_debug(DEBUG_DB,"'%s' wasn't in the dictionary, and didnt follow the rules.",singular);
	return(strdup(singular));
}


/* Diku gamma has these macros for getting at the appropriate string, located
 * in utils.h.  You can change the #define in utils.h to call these functions
 * directly instead of doing an inline test. They are a little easier to read
 * and to extend, and put some guardrails on the value of sex. (No further
 * comment required.)   Note that if you pass them a NULL ch, you are doomed.
 * Don't do that.  */

/* functional replacement for the HSHR diku macro */
char *diku_hshr(struct char_data *ch) {
	int sex = MAX(0,MIN(ARRAY_SIZE(determiner_possessive)-1,GET_SEX(ch)));
	return(determiner_possessive[sex]);
}

/* functional replacement for the HSSH diku macro */
char *diku_hssh(struct char_data *ch) {
	int sex = MAX(0,MIN(ARRAY_SIZE(pronoun_personal)-1,GET_SEX(ch)));
	return(pronoun_personal[sex]);
}

/* functional replacement for the HMHR diku macro */
char *diku_hmhr(struct char_data *ch) {
	int sex = MAX(0,MIN(ARRAY_SIZE(pronoun_thirdperson)-1,GET_SEX(ch)));
	return(pronoun_thirdperson[sex]);
}

/* A generic act variable argument list parser.  src points to a list of
 * substrings to choose from, option determines which one gets copied into dst.
 * The substrings are separated by ':' or ','.  A substring may contain ':' if
 * it is quoted, or parenthesized.  The list is terminated by a space, or by a
 * close paren.  dst is a location to copy the chosen string into, not to
 * excede len. Function returns the number of characters in src that were
 * parsed.
 *
 * Example src strings:  
 * 'is:are remainder'
 * '(is:are)remainder'
 * '("choice zero":one:"option two") was taken'
 */
int act_parse_option(char *src,int option, char *dst, size_t len) {

	char *s = src; 
	char *d = dst;
	char *eod = dst+len;
	int inparen=0;
	int inquote=0;
	int inopt=0;

	if(d && len>0) *d='\0';

	for(s=src;*s;s++) {
		if(*s=='(' && !inquote) {
			inparen++;
		} else if ( *s==')' && !inquote) {
			if((--inparen)==0) {
				s++;
				break;
			}
		} else if (*s=='\"') {
			inquote ^=1;
		} else if ( *s==':' && !inquote) {
			inopt++;
		} else if ( *s==',' && !inquote) {
			inopt++;
		} else if ( *s==' ' && !inquote && !inparen) {
			break;
		} else if ( option == inopt ) {
			if(d && d<eod-1) { 
				*d++ = *s;
			}
		}
	}
	if(d) *d = '\0';

	return(s-src);

}

/* transform a verb, provided as an act_parse_option() compatible list, from
 * Third-Person Singular to Second-Person Plural.  This is directly compatible
 * with act_parse_options, except that if the option isn't found, a heuristic
 * is invoked.*/
int act_verb_s2p(char *src,int plurality, char *dst, size_t len) {

	int parsed=0;
	char *pluralverb;

	/* get the requested option. */
	parsed = act_parse_option(src,plurality,dst,len);
	if(*dst) {
		/* something was supplied, so we are done here.*/
		return(parsed);
	}
	/* reparse just for the first option. */
	parsed = act_parse_option(src,0,dst,len);
	pluralverb = pluralize_verb(dst);
	strncpy(dst,pluralverb,len);
	free(pluralverb);
	return(parsed);
}

/* is the pronoun/sex of the player one that requires a plural verb to match? */
int get_sex_plurality(struct char_data *ch) {
	if(GET_SEX(ch) == SEX_PLURAL) {
		return(1);
	} 
	return(0);
}

/* Testing code goes here. */

/* some test case code using act().  */
void test_verb_s2p(struct char_data *ch) {

	int chsex = ch->player.sex;
	for(int sex =0;sex < SEX_MAX;sex++) {
		ch->player.sex = sex;
		act("Option: plurality is $v(single:plural)", FALSE, ch, 0, 0, TO_CHAR);
		act("Lookup: $n... $e $v(does) $s own thing.", FALSE, ch, 0, 0, TO_CHAR);
		act("Option: $n... $e $v(does:\"work out\") $s own thing.", FALSE, ch, 0, 0, TO_CHAR);
		act("Lookup: $e $v(isn't) able to talk to you.", FALSE, ch, 0, 0, TO_CHAR);
		act("Option: $e $v(ain't:ain't) able to talk to you.", FALSE, ch, 0, 0, TO_CHAR);
		act("Rule 1: $e $v(hisses) to say I love you.", FALSE, ch, 0, 0, TO_CHAR);
		act("Rule 1: $e $v(teaches) advanced thievery.", FALSE, ch, 0, 0, TO_CHAR);
		act("Rule 1: I wonder if $e $v(razzes) everyone like that?", FALSE, ch, 0, 0, TO_CHAR);
		act("Rule 2: $e $v(glorifies) violence.", FALSE, ch, 0, 0, TO_CHAR);
		act("Rule 3: $e $v(stays) in town square all the time.", FALSE, ch, 0, 0, TO_CHAR);
		act("Rule 4: $e $v(eats) with the ferocity of a tiger.", FALSE, ch, 0, 0, TO_CHAR);
		act("default: $e $v(squigglfonox) the bungshwee.", FALSE, ch, 0, 0, TO_CHAR);
		act("quotes: $e $v(uses) $v(\"the option before the : character\":\"that second option after (:)\")", FALSE, ch, 0, 0, TO_CHAR);
		act("noparen: $e $vwishes $e had $s parens.", FALSE, ch, 0, 0, TO_CHAR);
		act("quotes: $e $v\"wishes\" $e didn't have quotes.", FALSE, ch, 0, 0, TO_CHAR);
		act("---", FALSE, ch, 0, 0, TO_CHAR);
	}
	ch->player.sex = chsex;

}


