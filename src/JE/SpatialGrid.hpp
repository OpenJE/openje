// Copyright 2024 OpenJE

#ifndef JE_SPATIALGRID_HPP_
#define JE_SPATIALGRID_HPP_

#include "types.hpp"

namespace JE {
	// JE::SpatialGrid (original cls_0x405f80) - per-cell occupancy lists over the map's cell
	// grid; map owns it at map+0x88 (created by CreateGrid_0x57a8a0 with the .rle dimensions;
	// grid header fields mbr_0x0=cols, mbr_0x4=rows match QueryEntities' reads at 0x405F94/9F).
	//
	// Entity footprint: each entity registers a run list (per-row {xOff, len} entries; the
	// query's param_2 vector nodes carry {+4 x, +8 len}) reachable at Entity+0x16C (+12 =
	// run-vector base, observed in OnServerUpdate_0x4060b0 at 0x406170). Model/bbox-driven
	// footprints are unmodeled => default single run {0,1} (1x1 cell) documented as deviation.
	//
	// Deviations: per-cell containers are flat growable arrays instead of the original's
	// intrusive 0xC-node lists; removal is compaction, not relinking. Query semantics
	// (window clamp, per-x column walk, dedupe insertion) are ported faithfully.
	class SpatialGrid {
	public:
		struct Run { int x; int len; };          // footprint row-run (relative to anchor cell)

		SpatialGrid() : m_cols(0), m_rows(0), m_cells(0) {}
		~SpatialGrid();

		void Init( unsigned cols, unsigned rows );

		int  Insert( int col, int row, void* entity );     // 0 = out of grid
		int  Remove( int col, int row, void* entity );     // 1 removed

		// 0x405F80 - collect entities overlapping runs placed at anchor cell (cx,cy).
		// results receives unique entity pointers (original dedupes via RB-set insert; we
		// linear-scan - same content). Returns count collected.
		unsigned QueryEntities( Run* runs, unsigned runCount,
		                         int cx, int cy,
		                         void** results, unsigned resultsCap ) const;

		unsigned Cols() const { return m_cols; }
		unsigned Rows() const { return m_rows; }

	private:
		struct Cell {
			void** items;
			unsigned count;
			unsigned cap;
		};
		unsigned m_cols;
		unsigned m_rows;
		Cell*    m_cells;   // m_cols * m_rows
	};
} // namespace JE

#endif // JE_SPATIALGRID_HPP_
