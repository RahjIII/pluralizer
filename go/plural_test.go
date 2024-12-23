// pluralizer, translated from https://www.last-outpost.com/LO/pubcode/
// Ported to Go from pluralizer.c by Dr Pogi (drpogi@icloud.com)
package plural

import (
	"path"
	"runtime"
	"testing"
)

func Assert(t *testing.T, b bool) {
	if !b {
		_, file, line, _ := runtime.Caller(1)
		t.Errorf("Assert failed at %s:%d", path.Base(file), line)
	}
}

func TestIntToWords(t *testing.T) {
	Assert(t, IntToWords(0) == "zero")
	Assert(t, IntToWords(1) == "one")
	Assert(t, IntToWords(-1) == "minus one")
	Assert(t, IntToWords(7) == "seven")
	Assert(t, IntToWords(10) == "ten")
	Assert(t, IntToWords(12) == "twelve")
	Assert(t, IntToWords(17) == "seventeen")
	Assert(t, IntToWords(-29) == "minus twenty-nine")
	Assert(t, IntToWords(99) == "ninety-nine")
	Assert(t, IntToWords(100) == "100")
	Assert(t, IntToWords(1234) == "1234")
}

func TestPluralizeNoun(t *testing.T) {
	Assert(t, PluralizeNoun("", 2) == "")
	Assert(t, PluralizeNoun("foo", 1) == "foo")
	Assert(t, PluralizeNoun("fish", 1) == "fish")

	Assert(t, PluralizeNoun("fish", 2) == "fish")
	Assert(t, PluralizeNoun("person", 1) == "person")
	Assert(t, PluralizeNoun("person", 2) == "people")

	Assert(t, PluralizeNoun("boss", 7) == "bosses")
	Assert(t, PluralizeNoun("brush", 7) == "brushes")
	Assert(t, PluralizeNoun("punch", 2) == "punches")
	Assert(t, PluralizeNoun("fox", 2) == "foxes")
	Assert(t, PluralizeNoun("avocado", 2) == "avocadoes")
	Assert(t, PluralizeNoun("fez", 2) == "fezes")

	Assert(t, PluralizeNoun("life", 3) == "lives")

	Assert(t, PluralizeNoun("loaf", 2) == "loaves")
	Assert(t, PluralizeNoun("bluff", 2) == "bluffs")

	Assert(t, PluralizeNoun("entity", 2) == "entities")
	Assert(t, PluralizeNoun("tray", 2) == "trays")

	Assert(t, PluralizeNoun("virus", 2) == "viri")
	Assert(t, PluralizeNoun("terminus", 2) == "termini")

	Assert(t, PluralizeNoun("ellipsis", 2) == "ellipses")

	Assert(t, PluralizeNoun("onion", 2) == "onions")

	Assert(t, PluralizeNoun("gas", 2) == "gases")

	Assert(t, PluralizeNoun("Excalibur", 2) == "Excaliburs")
	Assert(t, PluralizeNoun("GLAMDRING", 2) == "GLAMDRINGs")

	Assert(t, PluralizeNoun("apex", 2) == "apexes")
	Assert(t, PluralizeNoun("beau", 2) == "beaux")
	Assert(t, PluralizeNoun("quiz", 2) == "quizzes")
	Assert(t, PluralizeNoun("elf", 2) == "elves")
}


func TestPluralizeNounPhrase(t *testing.T) {
	s := "a short sword"
	Assert(t, PluralizeNounPhrase(s, 0) == "zero short swords")
	Assert(t, PluralizeNounPhrase(s, 1) == "one short sword")
	Assert(t, PluralizeNounPhrase(s, 3) == "three short swords")

	b := "a bag of holding"
	Assert(t, PluralizeNounPhrase(b, 0) == "zero bags of holding")
	Assert(t, PluralizeNounPhrase(b, 1) == "one bag of holding")
	Assert(t, PluralizeNounPhrase(b, 11) == "eleven bags of holding")

	l := "one loaf of crusty bread"
	Assert(t, PluralizeNounPhrase(l, 0) == "zero loaves of crusty bread")
	Assert(t, PluralizeNounPhrase(l, 1) == "one loaf of crusty bread")
	Assert(t, PluralizeNounPhrase(l, 42) == "forty-two loaves of crusty bread")

	e := "an Excalibur"
	Assert(t, PluralizeNounPhrase(e, 1) == "one Excalibur")
	Assert(t, PluralizeNounPhrase(e, 7) == "seven Excaliburs")

	g := "THE GLAMDRING"
	Assert(t, PluralizeNounPhrase(g, 1) == "one GLAMDRING")
	Assert(t, PluralizeNounPhrase(g, 2) == "two GLAMDRINGs")
}

func TestPluralizeVerb(t *testing.T) {
	Assert(t, PluralizeVerb("") == "")

	Assert(t, PluralizeVerb("has") == "have")
	Assert(t, PluralizeVerb("isn't") == "aren't")

	Assert(t, PluralizeVerb("relaxes") == "relax")
	Assert(t, PluralizeVerb("blesses") == "bless")
	Assert(t, PluralizeVerb("bashes") == "bash")
	Assert(t, PluralizeVerb("wrenches") == "wrench")
	Assert(t, PluralizeVerb("fuzzes") == "fuzz")

	Assert(t, PluralizeVerb("bloodies") == "bloody")
	Assert(t, PluralizeVerb("parries") == "parry")

	Assert(t, PluralizeVerb("assays") == "assay")

	Assert(t, PluralizeVerb("moans") == "moan")

	Assert(t, PluralizeVerb("test") == "test")
	Assert(t, PluralizeVerb("flies") == "fly")
}

