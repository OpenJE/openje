// Copyright 2024 OpenJE
#include "types.hpp"
#include "JE/cls_0x4d8d70.hpp"
#include <string.h>
#include <stdlib.h>

#ifdef TRACING
#include "tracing.hpp"
#endif

namespace JE {

	// ---- node layout constants (0x30-byte nodes, see cls_0x4d8d70.hpp) --------------------------
	// Confirmed against the meth_0x545710 disassembly at 0x545710-0x54576c (cmp [esi+24h],10h /
	// mov al,[esi+2Dh] / lea eax,[esi+10h]) and sub_59EC40 (operator new(0x30), +0x2C=1, +0x2D=0).
	static const int Node_Left    = 0x00;   // _Left
	static const int Node_Parent  = 0x04;   // _Parent
	static const int Node_Right   = 0x08;   // _Right
	static const int Node_Key     = 0x10;   // std::string _Bx (inline buffer or heap pointer)
	static const int Node_KeySize = 0x20;   // std::string _Mysize
	static const int Node_KeyCap  = 0x24;   // std::string _Myres (capacity)
	static const int Node_Value   = 0x28;   // handler pointer written by F3::RegisterCommand
	static const int Node_Color   = 0x2C;   // _Color
	static const int Node_Isnil   = 0x2D;   // _Isnil

	// ---- accessors -------------------------------------------------------------------------------
	// A "string object" passed to the tree (the cls_0x50db20-style tempString in RegisterCommand) is an
	// MSVC4 std::string living at object offset +4: { inline buffer or heap pointer at +4, size at +0x14,
	// capacity at +0x18 }. Evidence: meth_0x545710 prologue `mov eax,[esp+14h]; cmp 16,[eax+18h]` and
	// `lea edi,[eax+4]`. Node keys are raw std::strings at node+0x10 (same shape at offset 0).

	static const char* StringData( const void* stringStart ) {
		const byte* base = reinterpret_cast<const byte*>( stringStart );
		dword capacity = *reinterpret_cast<const dword*>( base + 0x14 );
		if ( capacity < 0x10u ) {
			return reinterpret_cast<const char*>( base );
		}
		return *reinterpret_cast<const char* const*>( base );
	}

	static dword StringCapacity( const void* stringStart ) {
		return *reinterpret_cast<const dword*>( reinterpret_cast<const byte*>( stringStart ) + 0x14 );
	}

	// Accessors for a string OBJECT (cls_0x50db20 shape): the std::string sub-object is at +4.
	static const char* QueryData( const void* stringObject ) {
		return StringData( reinterpret_cast<const byte*>( stringObject ) + 4 );
	}

	static dword QueryCapacity( const void* stringObject ) {
		return StringCapacity( reinterpret_cast<const byte*>( stringObject ) + 4 );
	}

	// No shipped single-fn counterpart: node linking is INLINED per _Tree clone in the shipped
	// binary (0x50DBB0/0x50DB20 families, T-057 attribution tier); this accessor is the port's
	// extraction over the b3 node layout (edge selected by linkOffset).
	static byte* NodeLink( byte* node, int linkOffset ) {
		return *reinterpret_cast<byte**>( node + linkOffset );
	}

	// Same tier as NodeLink (no shipped single fn): std::string keys are read inline in shipped
	// clones; SSO-buffer-first layout per QueryData above (offset 0 = buffer).
	static const char* NodeKey( byte* node ) {
		return StringData( node + Node_Key );   // node keys are plain std::strings (offset 0 = buffer)
	}

	static bool NodeIsNil( byte* node ) {
		return node[Node_Isnil] != 0;
	}

	// 0x59EC40 - buynode: operator new(0x30) in the original, then _Left/_Parent/_Right = 0,
	// _Color (+0x2C) = 1, _Isnil (+0x2D) = 0. The header node and inserted nodes are released with
	// j__free by the destructor at 0x59F4D0, so malloc/free is used here to keep the allocation pair
	// matched.
	static byte* Buynode_0x59ec40() {
		byte* node = reinterpret_cast<byte*>( malloc( 0x30 ) );
		if ( node ) {
			memset( node, 0, 0x0C );          // _Left, _Parent, _Right
			node[ Node_Color ] = 1;           // _Color
			node[ Node_Isnil ] = 0;           // _Isnil
		}
		return node;
	} // Buynode_0x59ec40

	// Copy the key string into node storage the way the original pair constructor (0x570EB0 via
	// sub_59F170 inside meth_0x59f200) does: SSO inline buffer below 0x10 characters, heap buffer
	// (malloc'd, NUL-terminated) at or above it.
	static void NodeSetKey( byte* node, const char* text ) {
		size_t length = strlen( text );
		byte* slot = node + Node_Key;
		if ( length < 0x10u ) {
			memcpy( slot, text, length );
			slot[length] = '\0';
			*reinterpret_cast<dword*>( slot + 0x10 ) = static_cast<dword>( length ); // size
			*reinterpret_cast<dword*>( slot + 0x14 ) = 15u;                          // capacity
		} else {
			char* buffer = reinterpret_cast<char*>( malloc( length + 1 ) );
			if ( buffer ) {
				memcpy( buffer, text, length );
				buffer[length] = '\0';
			}
			*reinterpret_cast<char**>( slot ) = buffer;
			*reinterpret_cast<dword*>( slot + 0x10 ) = static_cast<dword>( length ); // size
			*reinterpret_cast<dword*>( slot + 0x14 ) = static_cast<dword>( length );  // capacity
		}
	}

	static void NodeFreeKey( byte* node ) {
		if ( StringCapacity( node + Node_Key ) >= 0x10u ) {
			free( *reinterpret_cast<void**>( node + Node_Key ) );
		}
	}

	// 0x59F500
	// Observed side effects of the original constructor:
	//   0x59f508  byte [this+0x00] = AL (from the caller's stack slot; unspecified at the only shipped
	//             caller, the static-init stub 0x643F60, so deliberately NOT reproduced)
	//   0x59f50f  mbr_0x4 = Buynode_0x59EC40()            (0x30-byte header node)
	//   0x59f512  byte [header+0x2D] = 1                  (_Isnil: header is the end sentinel)
	//   0x59f519  header->_Parent = header
	//   0x59f51f  header->_Left   = header
	//   0x59f524  header->_Right  = header
	//   0x59f527  mbr_0x8 = 0                             (node count)
	cls_0x4d8d70::cls_0x4d8d70() {
		// The original ctor writes only mbr_0x4/mbr_0x8 (+ the AL-sourced flag byte at 0x00);
		// the shipped instance lives in zeroed .bss. Model the untouched words as explicit zeros.
		this->mbr_0x0 = 0;
		this->mbr_0xc = 0;  this->mbr_0x10 = 0; this->mbr_0x14 = 0;
		this->mbr_0x18 = 0; this->mbr_0x1c = 0; this->mbr_0x20 = 0;
		this->mbr_0x24 = 0; this->mbr_0x28 = 0; this->mbr_0x2c = 0;
		this->mbr_0x30 = 0; this->mbr_0x34 = 0;

		byte* header = Buynode_0x59ec40();
		header[ Node_Isnil ] = 1;
		*reinterpret_cast<byte**>( header + Node_Parent ) = header;
		*reinterpret_cast<byte**>( header + Node_Left ) = header;
		*reinterpret_cast<byte**>( header + Node_Right ) = header;

		this->mbr_0x4 = reinterpret_cast<dword>( header );
		this->mbr_0x8 = 0;
	} // cls_0x4d8d70

	// 0x59F4D0 - destructor, reached through the _atexit thunk 0x646B90: erases every element
	// (meth_0x59F0B0 range-erase over the header, which frees node keys/values via the value
	// teardown, relinks the header to itself and zeroes mbr_0x8), frees the header node, zeroes
	// mbr_0x4/mbr_0x8.
	cls_0x4d8d70::~cls_0x4d8d70() {
		byte* header = reinterpret_cast<byte*>( this->mbr_0x4 );
		if ( header ) {
			// Post-order teardown of the element subtree rooted at header->_Parent.
			byte* root = NodeLink( header, Node_Parent );
			if ( root != header ) {
				byte* stack[ 64 ];
				int depth = 0;
				byte* current = root;
				stack[ depth++ ] = current;
				while ( depth > 0 && depth < 64 ) {
					current = stack[ --depth ];
					byte* left = NodeLink( current, Node_Left );
					byte* right = NodeLink( current, Node_Right );
					if ( !NodeIsNil( left ) ) {
						stack[ depth++ ] = left;
					}
					if ( !NodeIsNil( right ) ) {
						stack[ depth++ ] = right;
					}
					NodeFreeKey( current );
					free( current );
				}
			}
			free( header );
		}
		this->mbr_0x4 = 0;
		this->mbr_0x8 = 0;
	} // ~cls_0x4d8d70

	// 0x545710 - _Tree::_Lower_bound. Original disassembly walk: start at header->_Parent (root),
	// remember the last node whose key is >= query, descend left/right accordingly, stop at any
	// _Isnil (the header). Returns the remembered node or the header (end sentinel).
	void* cls_0x4d8d70::LowerBound( const void* query ) {
		byte* header = reinterpret_cast<byte*>( this->mbr_0x4 );
		byte* node = NodeLink( header, Node_Parent );
		byte* result = header;

		const char* queryText = QueryData( query );

		while ( !NodeIsNil( node ) ) {
			if ( _stricmp( NodeKey( node ), queryText ) >= 0 ) {
				result = node;
				node = NodeLink( node, Node_Left );
			} else {
				node = NodeLink( node, Node_Right );
			}
		}
		return result;
	} // LowerBound

	// 0x59F540 - insert-or-get, returning the value slot (node + 0x28, the original's `return v3 + 40`).
	// Composes the original meth_0x59F3E0 (lower_bound + equivalence test via sub_433930) and
	// meth_0x59F200 (_Insert_at_bottom + buynode via sub_59F170). Red-black fixup/rotations from
	// 0x59F200 are intentionally omitted here (see the class header's DEVIATION note); BST shape only
	// affects internal balance, not the results the shipped call sites can observe.
	void* cls_0x4d8d70::InsertOrGet( const void* query ) {
		byte* header = reinterpret_cast<byte*>( this->mbr_0x4 );
		const char* queryText = QueryData( query );

		byte* node = reinterpret_cast<byte*>( LowerBound( query ) );
		if ( node != header && _stricmp( NodeKey( node ), queryText ) == 0 ) {
			return node + Node_Value;
		}

		byte* fresh = Buynode_0x59ec40();
		if ( !fresh ) {
			return header + Node_Value; // never happens for the tiny registry sizes; original would fault
		}
		NodeSetKey( fresh, queryText );
		*reinterpret_cast<dword*>( fresh + Node_Value ) = 0; // pair value defaults to 0 before assignment

		byte* root = NodeLink( header, Node_Parent );
		byte* parent;
		bool wentLeft;
		if ( NodeIsNil( root ) ) {
			// empty tree: the new node is the root; header keeps linking to itself for left/right.
			parent = header;
			wentLeft = false;
			*reinterpret_cast<byte**>( header + Node_Parent ) = fresh;
			*reinterpret_cast<byte**>( header + Node_Left ) = fresh;
			*reinterpret_cast<byte**>( header + Node_Right ) = fresh;
		} else {
			// shipped descent 0x59f3f0-0x59f443 (StringTree::meth_0x59f3e0, the _Insert_hint walk):
			// from the root compare _stricmp(query, nodeKey) < 0 ? left (0x59f43a) : right (0x59f43e),
			// stopping AT the nil child slot. THE slot found is the only place a new link may go -
			// a slice-5b defect caught by the Commands suite: the previous placement attached under
			// the lower_bound node's LEFT slot unconditionally, orphaning whatever subtree lived
			// there (registering 'alpha' after 'probe'/'simple' made 'alpha' unreachable while
			// mbr_0x8 still counted it - keys lost silently).
			byte* cur = root;
			for (;;) {
				if ( _stricmp( queryText, NodeKey( cur ) ) < 0 ) {
					byte* left = NodeLink( cur, Node_Left );
					if ( NodeIsNil( left ) ) { parent = cur; wentLeft = true; break; }
					cur = left;
				} else {
					byte* right = NodeLink( cur, Node_Right );
					if ( NodeIsNil( right ) ) { parent = cur; wentLeft = false; break; }
					cur = right;
				}
			}
			if ( wentLeft ) {
				*reinterpret_cast<byte**>( parent + Node_Left ) = fresh;
				if ( parent == NodeLink( header, Node_Left ) ) {
					*reinterpret_cast<byte**>( header + Node_Left ) = fresh;   // new leftmost
				}
			} else {
				*reinterpret_cast<byte**>( parent + Node_Right ) = fresh;
				if ( parent == NodeLink( header, Node_Right ) ) {
					*reinterpret_cast<byte**>( header + Node_Right ) = fresh;  // new rightmost
				}
			}
		}
		*reinterpret_cast<byte**>( fresh + Node_Parent ) = parent;
		*reinterpret_cast<byte**>( fresh + Node_Left ) = header;
		*reinterpret_cast<byte**>( fresh + Node_Right ) = header;

		this->mbr_0x8 += 1;   // original: ++this->mbr_0x8 at 0x59f2a9
		return fresh + Node_Value;
	} // InsertOrGet

	// 0x5ACE40 - console command lookup: exact case-insensitive match -> stored handler, else 0.
	// The original routes through meth_0x56F520/_Upper_bound; lower_bound + equality is equivalent for
	// unique keys (RegisterCommand rejects duplicates with a FatalError before inserting).
	int cls_0x4d8d70::FindValue( const void* query ) {
		byte* header = reinterpret_cast<byte*>( this->mbr_0x4 );
		byte* node = reinterpret_cast<byte*>( LowerBound( query ) );
		if ( node == header || _stricmp( NodeKey( node ), QueryData( query ) ) != 0 ) {
			return 0;
		}
		return static_cast<int>( *reinterpret_cast<dword*>( node + Node_Value ) );
	} // FindValue

	const char* cls_0x4d8d70::NodeKeyText( void* node ) {
		return NodeKey( reinterpret_cast<byte*>( node ) );
	} // NodeKeyText

} // namespace JE
