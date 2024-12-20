// pluralizer, translated from https://www.last-outpost.com/LO/pubcode/
// Ported to Go from pluralizer.c by Dr Pogi (drpogi@icloud.com)
package plural

import (
	"fmt"
	"log"
	"strings"
	"unicode"
)


func IsVowel(r byte) bool {
	switch unicode.ToLower(rune(r)) {
	case 'a', 'e', 'i', 'o', 'u', 'y': return true
	default: return false
	}
}

var onesStr = []string{
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
	"nineteen",
}

var tensStr = []string{
	"zero",
	"ten",
	"twenty",
	"thirty",
	"forty",
	"fifty",
	"sixty",
	"seventy",
	"eighty",
	"ninety",
}

// Spells out a number between -99 and 99.  Anything outside that range is
// printed numerically.  Free whatever string this gives back to you.
func IntToWords(num int) string {
	posNeg := ""
	isNeg := false

	if num < 0 {
		isNeg = true
		num = -num
	}

	if num > 99 {
		if isNeg { posNeg = "-" }
		return fmt.Sprintf("%s%d", posNeg, num)
	} else {
		if isNeg { posNeg = "minus " }

		if num < 20 {
			return posNeg + onesStr[num]
		} else {
			tens, ones := num / 10, num % 10

			if ones == 0 {
				return posNeg + tensStr[tens]
			} else {
				return fmt.Sprintf("%s%s-%s",
						posNeg, tensStr[tens], onesStr[ones])
			}
		}
	}
}

func PluralizePronoun(singular string, count int) string {
	if count == 1 {
		return singular
	}

	if strings.EqualFold(singular, "it") {
		return "them"
	}

	return singular
}

// Pluralize a singular english word.  ex given "sword" returns "swords".  ONLY
// works on single words, with no spaces.  Based on rules listed at
// https://www.grammarly.com/blog/plural-nouns/
func PluralizeNoun(singular string, count int) string {
	if len(singular) < 2 { return singular }

	// If there's exactly 1, there's nothing to do.
	if count == 1 { return singular }

	//NOTE assumes the lowercase form of the suffix runes are the same
	// byte length as the original runes.
	ls := strings.ToLower(singular)
	if len(singular) != len(ls) {
		log.Printf("PluralizeNoun: unsupported unicode in input")
		return singular
	}

	// Irregular pluralization dictionary lookup first.
	if w, ok := nounToPlural[ls]; ok {
		return w
	}

	// OK... derive the plural from some rules.
	r1 := ls[len(ls) - 1]
	s2 := ls[len(ls) - 2:]

	// RULE: If the singular noun ends in -ss, -sh, -ch, -x, or -z, add ‑es
	// to the end to make it plural.  (In some cases, singular nouns ending in
	// -s or -z, require that you double the -s or -z prior to adding the -es
	// for pluralization, but skipping that for now.)
	if s2 == "ss" || s2 == "sh" || s2 == "ch" ||
			r1 == 'x' || r1 == 'o' || r1 == 'z' {
		return singular + "es"
	}

	// RULE: If the noun ends with ‑f or ‑fe, the f is often changed to ‑ve
	// before adding the -s to form the plural version.
	if s2 == "fe" {
		return singular[:len(singular) - 2] + "ves"
	}

	if r1 == 'f' && s2 != "ff" { /* if it ends if ff, leave it alone. */
		return singular[:len(singular) - 1] + "ves"
	}

	// RULE: If the singular noun ends in -y...
	if r1 == 'y' {
		if IsVowel(s2[0]) {
			// ...and the letter before the -y is a vowel, simply add an -s to
			// make it plural.
			return singular + "s"
		} else {
			return singular[:len(singular) - 1] + "ies"
		}
	}

	// RULE: If the singular noun ends in ‑us, the plural ending is ‑i.
	if s2 == "us" {
		return singular[:len(singular) - 2] + "i"
	}

	// RULE: If the singular noun ends in ‑is, the plural ending is -es.
	if s2 == "is" {
		return singular[:len(singular) - 2] + "es"
	}

	// RULE: If the singular noun ends in ‑on, the plural ending is ‑a.
	/*if s2 == "on" {
		return singular[:len(singular) - 2] + "a"
	} */

	if r1 == 's' {
		return singular + "es"
	}

	// RULE: To make regular nouns plural, add ‑s to the end.
	return singular + "s"
}

// Pluralize a singlular english noun phrase.  Turns a phrase like "a short
// sword" into "six short swords" If count is zero, returns pluralized word
// without a count.  This code uses heuristics and may sometimes produce
// laughable crap.
func PluralizeNounPhrase(singular string, count int) string {
	//NOTE assumes the lowercase form of the runes are the same
	// byte length as the original runes.
	ls := strings.ToLower(singular)
	if len(singular) != len(ls) {
		return singular
	}

	bldr := strings.Builder{}
	bldr.Grow(len(singular) + 16)

	// Copy over leading whitespace.
	for _, r := range singular {
		if unicode.IsSpace(r) {
			bldr.WriteRune(r)
		} else {
			break
		}
	}

	singular = singular[bldr.Len():]

	// If there is an article prefix of a, an, the, one, skip over it.
	if strings.HasPrefix(ls, "a ") {
		singular = singular[2:]
		ls = ls[2:]
	} else if strings.HasPrefix(ls, "an ") {
		singular = singular[3:]
		ls = ls[3:]
	} else if strings.HasPrefix(ls, "the ") {
		singular = singular[4:]
		ls = ls[4:]
	} else if strings.HasPrefix(ls, "one ") {
		singular = singular[4:]
		ls = ls[4:]
	}

	// if count >=0, write the count to the result.
	if count >= 0 {
		bldr.WriteString(IntToWords(count))
		bldr.WriteRune(' ')
	}

	// Copy into the result, up to end of string or the preposition " of ", so
	// that noun prhases like "a bag of holding" are pluralized as bags, not
	// holdings.
	prepInd := strings.Index(ls, " of ")
	if prepInd == -1 { prepInd = len(singular) }

	spaceInd := strings.LastIndexFunc(singular[:prepInd], unicode.IsSpace)
	if spaceInd == -1 { spaceInd = 0 }

	//Write up to (not including) the last word to be pluralized.
	bldr.WriteString(singular[:spaceInd])

	//Pluralize and write the noun.
	pluralized := PluralizeNoun(singular[spaceInd:prepInd], count)
	bldr.WriteString(pluralized)

	//Write the remainder after the pluralized noun.
	bldr.WriteString(singular[prepInd:])

	return bldr.String()
}

// Like pluralize_noun, but for verbs.  Also based on rules from Grammarly,
// https://www.grammarly.com/blog/grammar-basics-what-is-subject-verb-agreement/
// .  I say based on, becuase I had to compose the converse of the rules listed
// there to get what I wanted.  May not be quite the same. */
func PluralizeVerb(singular string) string {
	if len(singular) < 2 { return singular }

	//NOTE assumes the lowercase form of the suffix runes are the same
	// byte length as the original runes.
	ls := strings.ToLower(singular)
	if len(singular) != len(ls) {
		log.Printf("PluralizeVerb: unsupported unicode in input")
		return singular
	}

	// Irregular pluralization dictionary lookup first.
	if w, ok := verbToPlural[ls]; ok {
		return w
	}

	// OK... derive the plural from some rules.

	// Grammarly RULE 1: If the verb ends in -x, –ss, –sh, –ch, –tch, or –zz,
	// you add –es to the end to match the third-person singular.
	// Converse: verb ends in -xes -sses -shes -ches -tches or -zzes, remove
	// -es.
	if strings.HasSuffix(ls, "xes") ||
			strings.HasSuffix(ls, "sses") ||
			strings.HasSuffix(ls, "shes") ||
			strings.HasSuffix(ls, "ches") ||
			strings.HasSuffix(ls, "tches") ||
			strings.HasSuffix(ls, "zzes") {
		return singular[:len(singular) - 2]
	}

	// Grammarly RULE 2: If the verb ends in a consonant + y, remove the y and
	// add –ies.
	// Converse: if the verb ends in -consonant +ies, remove -ies and add +y
	if len(singular) >= 4 &&
			strings.HasSuffix(ls, "ies") &&
			!IsVowel(ls[len(ls) - 4]) {
		return singular[:len(singular) - 3] + "y"
	}

	// Grammarly RULE 3: with words that end in a vowel + y, follow the normal
	// format and add only –s.
	// Converse: if the verb ends in -vowel +ys remove -s
	if len(singular) >= 3 &&
			strings.HasSuffix(ls, "ys") &&
			IsVowel(ls[len(ls) - 3]) {
		return singular[:len(singular) - 1]
	}

	// RULE 4: Add s.
	// Converse: if it ends is -s, remove -s
	if strings.HasSuffix(ls, "s") {
		return singular[:len(singular) - 1]
	}

	log.Printf("PluralizeVerb: '%s' wasn't in the dictionary, and didn't follow the rules.", singular)
	return singular
}

