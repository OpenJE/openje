// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stddef.h>
#include "JE/ModelCache.hpp"

namespace JE {

	// 0x70C528 (g_model_cache)
	ModelCache* g_model_cache = NULL;

	// 0x585c00
	// Observed side effects of the original constructor:
	//   0x585c32  vftptr = &ModelCache_vftable_0x69b9bc
	//   0x585c41  mbr_0x8  = buynode_0x45c940()  (0x38-byte tree-A header node, self-linked), mbr_0xc = 0
	//   0x585c76  mbr_0x14 = buynode_0x59ec40()  (0x30-byte tree-B header node, self-linked), mbr_0x18 = 0
	//   0x585ca1  if ( g_model_cache ) throw Exception("ModelCache object already exists.")
	//   0x585cce  g_model_cache = this
	//   0x585cf5  UNRESOLVED: loop that formats decimal keys "1", "2", "3", ... and probes tree-A with
	//             lower_bound_0x4339b0 + _stricmp (case-insensitive std::string keys).
	// The two std::map members are not reconstructed here: their node/value template arguments are still
	// unconfirmed (see docs/je/ModelCache.md "Open questions").
	ModelCache::ModelCache() {
		tracing::instrument( tracing::LOCATION, "" );

		if ( g_model_cache ) {
			// 0x585cb5: the original raises _CxxThrowException(Exception("ModelCache object already
			// exists.")). The throw is deliberately not reproduced yet, because the string payload is
			// not modelled by JE::Common::Exception; keep the first instance instead of clobbering it.
			return;
		}
		g_model_cache = this;
	} // ModelCache

	// 0x585b10
	// Observed side effects of the original destructor:
	//   0x585b31  vftptr = &ModelCache_vftable_0x69b9bc (destructing)
	//   0x585b4c  walks tree-B (mbr_0x14); for each element destroys + frees the cached object at
	//             node+0x28 (0x567f50 then free)
	//   0x585b8a  g_model_cache = 0
	//   0x585ba5  destroys tree-B (0x585640) and frees its header node (0x585bae)
	//   0x585bd5  destroys tree-A (0x45d1d0) and frees its header node (0x585bde)
	ModelCache::~ModelCache() {
		tracing::instrument( tracing::LOCATION, "" );

		g_model_cache = NULL;
	} // ~ModelCache

} // namespace JE
