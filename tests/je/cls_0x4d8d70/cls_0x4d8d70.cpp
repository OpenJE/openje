// Host test for JE::cls_0x4d8d70 (command registry tree). Build (see comment at bottom):
//   i686-w64-mingw32-g++ -std=c++98 -Isrc -Imodules/openje/src test.cpp modules/openje/src/JE/cls_0x4d8d70.cpp -o test
//   wine test.exe
#include "JE/cls_0x4d8d70.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int g_fail = 0;
static void check(int cond, const char* what) {
	if (!cond) { printf("FAIL: %s\n", what); ++g_fail; }
}

// Build a RegisterCommand-style tempString (cls_0x50db20 shape: std::string sub-object at +4,
// capacity 15 SSO, size 0). Returns object base pointer.
struct QueryString {
	unsigned int word0;
	char buffer[16];        // +0x04 inline buffer / heap pointer
	unsigned int size;      // +0x14
	unsigned int capacity;  // +0x18
};

static void setQuery(QueryString* q, const char* text) {
	q->word0 = 0;
	size_t len = strlen(text);
	if (len < 16) {
		q->capacity = 15;
		memcpy(q->buffer, text, len + 1);
		q->size = (unsigned)len;
	} else {
		// MSVC4 heap mode: the pointer occupies the inline buffer slot.
		char* heap = (char*)malloc(len + 1);
		memcpy(heap, text, len + 1);
		memcpy(q->buffer, &heap, sizeof(heap));
		q->size = (unsigned)len;
		q->capacity = (unsigned)len;
	}
}

int main() {
	JE::cls_0x4d8d70 tree;
	QueryString q;

	// Empty tree: lower_bound returns the header sentinel.
	setQuery(&q, "quit");
	check(tree.LowerBound(&q) == (void*)tree.mbr_0x4, "empty lower_bound == header");
	check(tree.FindValue(&q) == 0, "find missing == 0");

	// Insert three handlers at the value slots, distinct cases: short keys.
	void* s1 = tree.InsertOrGet((setQuery(&q, "quit"), (void*)&q));
	*(int*)s1 = 111;
	void* s2 = tree.InsertOrGet((setQuery(&q, "save"), (void*)&q));
	*(int*)s2 = 222;
	void* s3 = tree.InsertOrGet((setQuery(&q, "load"), (void*)&q));
	*(int*)s3 = 333;
	check(s1 != s2 && s2 != s3 && s1 != s3, "distinct value slots");
	check(tree.mbr_0x8 == 3, "node count == 3");

	// Lookups, including case-insensitivity.
	setQuery(&q, "quit"); check(tree.FindValue(&q) == 111, "find quit");
	setQuery(&q, "QUIT"); check(tree.FindValue(&q) == 111, "find QUIT case-insensitive");
	setQuery(&q, "save"); check(tree.FindValue(&q) == 222, "find save");
	setQuery(&q, "load"); check(tree.FindValue(&q) == 333, "find load");
	setQuery(&q, "nope"); check(tree.FindValue(&q) == 0, "find missing 0");

	// Insert-or-get of an existing key returns the SAME slot (update path, 0x59f3e0 semantics).
	void* s1b = tree.InsertOrGet((setQuery(&q, "quit"), (void*)&q));
	check(s1b == s1, "re-insert returns same value slot");
	check(tree.mbr_0x8 == 3, "no duplicate node");

	// Heap (>= 0x16 chars... precisely capacity>=0x10) key path.
	setQuery(&q, "averyveryverylongcommandname");
	void* s4 = tree.InsertOrGet(&q);
	*(int*)s4 = 444;
	check(tree.FindValue(&q) == 444, "long key insert+find");
	check(tree.mbr_0x8 == 4, "count after long key");

	// Lower-bound key text matches the original walk semantics: first key >= query (ci).
	setQuery(&q, "load");
	void* lb = tree.LowerBound(&q);
	check(lb != (void*)tree.mbr_0x4 && _stricmp(JE::cls_0x4d8d70::NodeKeyText(lb), "load") == 0,
		"lower_bound exact key");

	// Destructor frees everything; valgrind-style double-free would abort here.
	// (Explicit placement new scope end runs ~cls_0x4d8d70.)

	// Free the tempString heap buffer (RegisterCommand does the same when capacity >= 0x10).
	setQuery(&q, "anotherlongcommandname1234");
	void* s5 = tree.InsertOrGet((setQuery(&q, "anotherlongcommandname1234"), (void*)&q));
	*(int*)s5 = 555;
	check(tree.FindValue(&q) == 555, "second long key");
	check(tree.mbr_0x8 == 5, "count after second long key");
	if (q.capacity >= 16) free(*(void**)q.buffer);

	if (!g_fail) { printf("OK: cls_0x4d8d70 registry tests passed\n"); return 0; }
	printf("%d failures\n", g_fail);
	return 1;
}
