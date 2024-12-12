## pluralizer code snippet

Code to list stacks of dikumud-like short item descriptions in a more natural
way. Turns a phrase like "a short sword" into "six short swords". Requires glib
2.0 hash table for an irregualr noun lookup, and scnprintf to keep printing in
bound. Also provides code to add a verb $v() operator to act(), that changes
the pluraliztion of the verb to match the gender of the the subject.

## usage

Add the .c and .h files into your project.  Include the .h file anyplace that
you want to use the code.  See the .h file for the important bits, and the
comments in the .c for how to use them.

See the file <README>(README) (not REAME.md) for info on how to patch this code into a
dikumud family game.
