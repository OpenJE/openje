// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include <string.h>
#include "JE/SpatialGrid.hpp"

namespace JE {
	// Port-side spatial index (implementer design; door-rebase + entity-shape docs). No shipped
	// counterpart fns for these four lifecycle methods: the shipped side inserts entity-owned
	// tick-stamped convex quads into the .rle grid (0x58-byte records, coverage virt 0x40A4B0,
	// query era 0x405F80 — RLE-convex-quad doc); this Cell array models that insert surface.
	SpatialGrid::~SpatialGrid() {
		if ( m_cells ) {
			for ( unsigned i = 0; i < m_cols * m_rows; ++i ) {
				free( m_cells[i].items );
			}
			free( m_cells );
		}
	} // ~SpatialGrid

	// Port design (no shipped fn; era cites at ~SpatialGrid above: records 0x58B, 0x40A4B0).
	void SpatialGrid::Init( unsigned cols, unsigned rows ) {
		if ( cols > 0xFFFF || rows > 0xFFFF || !cols || !rows ) {
			return;
		}
		m_cols = cols;
		m_rows = rows;
		m_cells = (Cell*)calloc( cols * rows, sizeof( Cell ) );
	} // Init

	// Port design (no shipped fn): growth realloc shape stands in for the shipped per-entity
	// .rle insertion path (coverage virt 0x40A4B0, RLE-convex-quad doc).
	int SpatialGrid::Insert( int col, int row, void* entity ) {
		tracing::instrument( tracing::LOCATION, "col=%d row=%d", col, row );
		if ( !m_cells || col < 0 || row < 0
		     || (unsigned)col >= m_cols || (unsigned)row >= m_rows || !entity ) {
			return 0;
		}
		Cell& c = m_cells[(unsigned)col + (unsigned)row * m_cols];
		if ( c.count == c.cap ) {
			unsigned cap = c.cap ? c.cap * 2 : 4;
			void** grown = (void**)realloc( c.items, sizeof( void* ) * cap );
			if ( !grown ) {
				return 0;
			}
			c.items = grown;
			c.cap  = cap;
		}
		c.items[c.count++] = entity;
		return 1;
	} // Insert

	// Port design (no shipped fn): unlink mirrors Init/Insert era above (0x40A4B0/0x405F80).
	int SpatialGrid::Remove( int col, int row, void* entity ) {
		tracing::instrument( tracing::LOCATION, "" );
		if ( !m_cells || col < 0 || row < 0
		     || (unsigned)col >= m_cols || (unsigned)row >= m_rows ) {
			return 0;
		}
		Cell& c = m_cells[(unsigned)col + (unsigned)row * m_cols];
		for ( unsigned i = 0; i < c.count; ++i ) {
			if ( c.items[i] == entity ) {
				memmove( &c.items[i], &c.items[i + 1], sizeof( void* ) * ( c.count - i - 1 ) );
				--c.count;
				return 1;
			}
		}
		return 0;
	} // Remove

	// 0x405F80 port. The window math mirrors the original exactly:
	//   x0 = p5 + run.x - p3; shipped callers pass p5 == p3 (mover cell x) so x0 = run.x
	//   (ABSOLUTE columns), row = p6;
	//   skip unless 0<=row<rows, x0<cols, x0+len>0;
	//   negative x0 folds into length (x0=0); far edge clamped to cols;
	//   per covered column, append the cell's entities (deduped).
	// Deviation: cells are 1-column wide here (the original stores run-spanned cells built
	// from the .rle runs and jumps column := cell.x + cell.span at 0x406053; equivalent
	// coverage, per-column stepping).
	unsigned SpatialGrid::QueryEntities( Run* runs, unsigned runCount,
	                                     int cx, int cy,
	                                     void** results, unsigned resultsCap ) const {
		tracing::instrument( tracing::LOCATION, "cx=%d cy=%d", cx, cy );
		unsigned found = 0;
		if ( !m_cells ) {
			return 0;
		}
		for ( unsigned r = 0; r < runCount; ++r ) {
			int x  = runs[r].x;
			int len = runs[r].len;
			int x0 = cx + x - cx;          // = run.x, faithful to the original expression
			(void)cx;
			if ( cy < 0 || (unsigned)cy >= m_rows || x0 >= (int)m_cols || x0 + len <= 0 ) {
				continue;
			}
			if ( x0 >= 0 ) {
				if ( x0 + len >= (int)m_cols ) {
					len = (int)m_cols - x0;
				}
			} else {
				len += x0;
				x0 = 0;
			}
			int end = x0 + len;
			for ( int xw = x0; xw < end; ++xw ) {
				const Cell& c = m_cells[(unsigned)xw + (unsigned)cy * m_cols];
				for ( unsigned i = 0; i < c.count; ++i ) {
					void* e = c.items[i];
					unsigned k;
					for ( k = 0; k < found; ++k ) {
						if ( results[k] == e ) {
							break;
						}
					}
					if ( k == found && found < resultsCap ) {
						results[found++] = e;
					}
				}
			}
		}
		return found;
	} // QueryEntities
} // namespace JE
