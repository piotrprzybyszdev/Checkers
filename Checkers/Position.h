#pragma once

#include <cuda.h>
#include <cuda_runtime.h>

#ifdef __CUDA_ARCH__

#include <cuda/std/bit>
#include <cuda/std/cassert>
#include <cuda/std/type_traits>

namespace stl = cuda::std;

#define CONSTANT __constant__

#else

#include <bit>
#include <cassert>
#include <cstdint>
#include <random>
#include <type_traits>
#include <utility>

namespace stl = std;
#define CONSTANT

#endif

namespace Checkers
{

using Bitboard = uint32_t;

namespace Board
{

static inline constexpr Bitboard Empty = 0x0u;
static inline constexpr Bitboard Full = 0xffffffffu;

__host__ __device__ __inline__ constexpr bool IsEmpty(Bitboard board)
{
	return board == Empty;
}

__host__ __device__ __inline__ constexpr Bitboard FromIndex(int index)
{
	assert(0 <= index && index < 32);
	return 0x1u << index;
}

__host__ __device__ __inline__ constexpr bool IsWhiteSquare(int i, int j)
{
	assert(0 <= i && i < 8 && 0 <= j && j < 8);
	return (i + j) % 2 == 1;
}

__host__ __device__ __inline__ constexpr int CoordsToIndex(int i, int j)
{
	assert(!IsWhiteSquare(i, j));
	return j * 4 + i / 2;
}

__host__ __device__ __inline__ constexpr void IndexToCoords(int index, int &i, int &j)
{
	assert(0 <= index && index < 32);
	j = index / 4;
	i = (index - j * 4) * 2 + j % 2;
}

__host__ __device__ __inline__ constexpr Bitboard FromCoords(int i, int j)
{
	return FromIndex(CoordsToIndex(i, j));
}

__host__ __device__ __inline__ constexpr bool HasBit(Bitboard board, int index)
{
	return !Board::IsEmpty(board & Board::FromIndex(index));
}

__host__ __device__ __inline__ constexpr int GetBits(Bitboard board, int choices[32])
{
	int choiceCnt = 0;
	while (board)
	{
		choices[choiceCnt++] = stl::countr_zero(board);
		board &= board - 1;
	}

	return choiceCnt;
}

template<typename G>
__host__ __device__ __inline__ int RandomBit(G &generator, Bitboard board)
{
	if (stl::has_single_bit(board))
		return stl::countr_zero(board);

	int choices[32];
	int choiceCnt = GetBits(board, choices);

	int idx = generator.GetUniform(0, choiceCnt - 1);

	return choices[idx];
}

}

namespace Impl
{

/*
* 8   28  29  30  31
* 7 24  25  26  27
* 6   20  21  22  23
* 5 16  17  18  19
* 4   12  13  14  15
* 3 08  09  10  11
* 2   04  05  06  07
* 1 00  01  02  03
*   A B C D E F G H
*/

static inline constexpr Bitboard CanShiftLeft3 = 0x0e0e0e0eu;
static inline constexpr Bitboard CanShiftLeft4 = 0x0fffffffu;
static inline constexpr Bitboard CanShiftLeft5 = 0x00707070u;
static inline constexpr Bitboard CanShiftRight3 = 0x70707070u;
static inline constexpr Bitboard CanShiftRight4 = 0xfffffff0u;
static inline constexpr Bitboard CanShiftRight5 = 0x0e0e0e00u;

static inline constexpr Bitboard BlackPromotion = 0xf0000000u;
static inline constexpr Bitboard WhitePromotion = 0x0000000fu;

static inline constexpr Bitboard DiagA1H8 = 0x88442211u;
static inline constexpr Bitboard DiagA3F8 = 0x44221100u;
static inline constexpr Bitboard DiagA5D8 = 0x22110000u;
static inline constexpr Bitboard DiagA7B8 = 0x11000000u;
static inline constexpr Bitboard DiagC1H6 = 0x00884422u;
static inline constexpr Bitboard DiagE1H4 = 0x00008844u;
static inline constexpr Bitboard DiagG1H2 = 0x00000088u;

CONSTANT static inline constexpr Bitboard DiagBL2TR[32] = {
	DiagA1H8, DiagC1H6, DiagE1H4, DiagG1H2,
	DiagA1H8, DiagC1H6, DiagE1H4, DiagG1H2,
	DiagA3F8, DiagA1H8, DiagC1H6, DiagE1H4,
	DiagA3F8, DiagA1H8, DiagC1H6, DiagE1H4,
	DiagA5D8, DiagA3F8, DiagA1H8, DiagC1H6,
	DiagA5D8, DiagA3F8, DiagA1H8, DiagC1H6,
	DiagA7B8, DiagA5D8, DiagA3F8, DiagA1H8,
	DiagA7B8, DiagA5D8, DiagA3F8, DiagA1H8
};

static inline constexpr Bitboard DiagA7G1 = 0x01122448u;
static inline constexpr Bitboard DiagA5E1 = 0x00011224u;
static inline constexpr Bitboard DiagA3C1 = 0x00000112u;
static inline constexpr Bitboard DiagA1A1 = 0x00000001u;
static inline constexpr Bitboard DiagB8H2 = 0x12244880u;
static inline constexpr Bitboard DiagD8H4 = 0x24488000u;
static inline constexpr Bitboard DiagF8H6 = 0x48800000u;
static inline constexpr Bitboard DiagH8H8 = 0x80000000u;

CONSTANT static inline constexpr Bitboard DiagTL2BR[32] = {
	DiagA1A1, DiagA3C1, DiagA5E1, DiagA7G1,
	DiagA3C1, DiagA5E1, DiagA7G1, DiagB8H2,
	DiagA3C1, DiagA5E1, DiagA7G1, DiagB8H2,
	DiagA5E1, DiagA7G1, DiagB8H2, DiagD8H4,
	DiagA5E1, DiagA7G1, DiagB8H2, DiagD8H4,
	DiagA7G1, DiagB8H2, DiagD8H4, DiagF8H6,
	DiagA7G1, DiagB8H2, DiagD8H4, DiagF8H6,
	DiagB8H2, DiagD8H4, DiagF8H6, DiagH8H8
};

}

struct Position
{
	Bitboard Black;
	Bitboard White;
	Bitboard Queens;

	int8_t SinceCapture;
	bool BlackTurn;

private:
	static inline constexpr uint8_t MovesTillDraw = 30;

	__host__ __device__ __inline__ constexpr Bitboard GetQueenMovesDiag(int index, Bitboard diag) const
	{
		Bitboard taken = diag & (Black | White);

		int bits[8];
		int count = Board::GetBits(taken, bits);

		//                        l            r
		//						 -----       ----
		// we can move like this nXnnXyyyyQyyXnnn
		// but we have to & it with the diag mask

		int l = 0, r = 0;

		int i = 0;
		for (; i < count && bits[i] <= index; i++)
		{
			if (bits[i] < index)
				l = bits[i] + 1;
		}

		if (i < count)
			r = 32 - bits[i];

		Bitboard mask = Board::Full;
		mask >>= l; mask <<= l + r; mask >>= r;
		mask ^= Board::FromIndex(index);

		return mask & diag;
	}

	__host__ __device__ __inline__ constexpr Bitboard GetQueenMoves(int index) const
	{
		return GetQueenMovesDiag(index, Impl::DiagTL2BR[index]) | GetQueenMovesDiag(index, Impl::DiagBL2TR[index]);
	}

	__host__ __device__ __inline__ constexpr Bitboard GetQueenCapturesDiag(int index, Bitboard diag) const
	{
		Bitboard taken = diag & (Black | White);

		int bits[8];
		int count = Board::GetBits(taken, bits);

		//     l       r   
		//  l              r
		// nXyyYnnnnQnnYyyy
		// nnyynnnnnnnnnyyy
		//   --         ---
		
		int il = -1, ir = count;

		int i = 0;
		for (; i < count && bits[i] <= index; i++)
		{
			if (bits[i] < index)
				il = i;
		}
		
		ir = i;

		int l1 = -1, l2 = index, r1 = index, r2 = 32;
		
		if (il >= 0) l2 = bits[il];
		if (il - 1 >= 0) l1 = bits[il - 1];
		if (ir < count) r1 = bits[ir];
		if (ir + 1 < count) r2 = bits[ir + 1];
		
		constexpr Bitboard masks[2] = { Board::Empty, Board::Full };

		Bitboard left = masks[l1 + 1 != l2];
		left >>= l1 + 1; left <<= l1 + 1 + 32 - l2; left >>= 32 - l2;
		Bitboard right = masks[r1 + 1 != r2];
		right >>= r1 + 1; right <<= r1 + 1 + 32 - r2; right >>= 32 - r2;

		Bitboard opponent = GetOpponent();

		bool canCaptureLeft = Board::HasBit(opponent, l2);
		bool canCaptureRight = Board::HasBit(opponent, r1);

		return ((left & masks[canCaptureLeft]) | (right & masks[canCaptureRight])) & diag;
	}

	__host__ __device__ __inline__ constexpr Bitboard GetQueenCaptures(int index) const
	{
		return GetQueenCapturesDiag(index, Impl::DiagTL2BR[index]) | GetQueenCapturesDiag(index, Impl::DiagBL2TR[index]);
	}

public:
	__host__ __device__ __inline__ constexpr Bitboard GetMoving(Bitboard from) const
	{
		Bitboard free = ~(Black | White);

		Bitboard moving;
		if (BlackTurn)
		{
			Bitboard r3 = (free & Impl::CanShiftRight3) >> 3;
			Bitboard r4 = (free & Impl::CanShiftRight4) >> 4;
			Bitboard r5 = (free & Impl::CanShiftRight5) >> 5;

			moving = (r3 | r4 | r5) & from;
		}
		else
		{
			Bitboard l3 = (free & Impl::CanShiftLeft3) << 3;
			Bitboard l4 = (free & Impl::CanShiftLeft4) << 4;
			Bitboard l5 = (free & Impl::CanShiftLeft5) << 5;

			moving = (l3 | l4 | l5) & from;
		}

		Bitboard queens = from & Queens;
		if (Board::IsEmpty(queens))
			return moving;

		int bits[12];
		int count = Board::GetBits(queens, bits);

		constexpr Bitboard masks[2] = { Board::Empty, Board::Full };
		for (int i = 0; i < count; i++)
		{
			int index = bits[i];

			bool canMove = !Board::IsEmpty(GetQueenMoves(index));

			Bitboard mask = Board::FromIndex(index);

			moving |= mask & masks[canMove];
		}

		return moving;
	}

	__host__ __device__ __inline__ constexpr Bitboard GetMoves(Bitboard from) const
	{
		Bitboard free = ~(Black | White);

		Bitboard moves;
		if (BlackTurn)
		{
			Bitboard l3 = (from & Impl::CanShiftLeft3) << 3;
			Bitboard l4 = (from & Impl::CanShiftLeft4) << 4;
			Bitboard l5 = (from & Impl::CanShiftLeft5) << 5;

			moves = (l3 | l4 | l5) & free;
		}
		else
		{
			Bitboard r3 = (from & Impl::CanShiftRight3) >> 3;
			Bitboard r4 = (from & Impl::CanShiftRight4) >> 4;
			Bitboard r5 = (from & Impl::CanShiftRight5) >> 5;

			moves = (r3 | r4 | r5) & free;
		}

		Bitboard queens = from & Queens;

		int bits[12];
		int count = Board::GetBits(queens, bits);

		for (int i = 0; i < count; i++)
			moves |= GetQueenMoves(bits[i]);

		return moves;
	}

	__host__ __device__ __inline__ constexpr Bitboard GetCapturing(Bitboard from) const
	{
		Bitboard free = ~(Black | White);

		Bitboard r3 = (free & Impl::CanShiftRight3) >> 3;
		Bitboard r4 = (free & Impl::CanShiftRight4) >> 4;
		Bitboard r5 = (free & Impl::CanShiftRight5) >> 5;

		Bitboard l3 = (free & Impl::CanShiftLeft3) << 3;
		Bitboard l4 = (free & Impl::CanShiftLeft4) << 4;
		Bitboard l5 = (free & Impl::CanShiftLeft5) << 5;

		Bitboard opponent = BlackTurn ? White : Black;

		Bitboard r34 = (r3 & opponent & Impl::CanShiftRight4) >> 4;
		Bitboard r43 = (r4 & opponent & Impl::CanShiftRight3) >> 3;
		Bitboard r45 = (r4 & opponent & Impl::CanShiftRight5) >> 5;
		Bitboard r54 = (r5 & opponent & Impl::CanShiftRight4) >> 4;

		Bitboard l34 = (l3 & opponent & Impl::CanShiftLeft4) << 4;
		Bitboard l43 = (l4 & opponent & Impl::CanShiftLeft3) << 3;
		Bitboard l45 = (l4 & opponent & Impl::CanShiftLeft5) << 5;
		Bitboard l54 = (l5 & opponent & Impl::CanShiftLeft4) << 4;

		Bitboard capturing = (r34 | r43 | r45 | r54 | l34 | l43 | l45 | l54) & from;
		
		Bitboard queens = from & Queens;

		int bits[12];
		int count = Board::GetBits(queens, bits);

		constexpr Bitboard masks[2] = { Board::Empty, Board::Full };
		for (int i = 0; i < count; i++)
		{
			int index = bits[i];

			bool canCapture = !Board::IsEmpty(GetQueenCaptures(index));

			Bitboard mask = Board::FromIndex(index);

			capturing |= mask & masks[canCapture];
		}

		return capturing;
	}

	__host__ __device__ __inline__ constexpr Bitboard GetCaptures(Bitboard from) const
	{
		Bitboard l3 = (from & Impl::CanShiftLeft3) << 3;
		Bitboard l4 = (from & Impl::CanShiftLeft4) << 4;
		Bitboard l5 = (from & Impl::CanShiftLeft5) << 5;

		Bitboard r3 = (from & Impl::CanShiftRight3) >> 3;
		Bitboard r4 = (from & Impl::CanShiftRight4) >> 4;
		Bitboard r5 = (from & Impl::CanShiftRight5) >> 5;

		Bitboard opponent = BlackTurn ? White : Black;

		Bitboard l34 = (l3 & opponent & Impl::CanShiftLeft4) << 4;
		Bitboard l43 = (l4 & opponent & Impl::CanShiftLeft3) << 3;
		Bitboard l45 = (l4 & opponent & Impl::CanShiftLeft5) << 5;
		Bitboard l54 = (l5 & opponent & Impl::CanShiftLeft4) << 4;

		Bitboard r34 = (r3 & opponent & Impl::CanShiftRight4) >> 4;
		Bitboard r43 = (r4 & opponent & Impl::CanShiftRight3) >> 3;
		Bitboard r45 = (r4 & opponent & Impl::CanShiftRight5) >> 5;
		Bitboard r54 = (r5 & opponent & Impl::CanShiftRight4) >> 4;

		Bitboard captures = (l34 | l43 | l45 | l54 | r34 | r43 | r45 | r54) & ~(Black | White);

		Bitboard queens = from & Queens;

		if (Board::IsEmpty(queens))
			return captures;

		int bits[12];
		int count = Board::GetBits(queens, bits);

		for (int i = 0; i < count; i++)
			captures |= GetQueenCaptures(bits[i]);

		return captures;
	}

	__host__ __device__ __inline__ constexpr void Move(int fromIndex, int toIndex)
	{
		Bitboard move = Board::FromIndex(fromIndex) | Board::FromIndex(toIndex);

		if (Board::HasBit(Queens, fromIndex))
			Queens ^= move;
		else
			SinceCapture = -1;

		if (BlackTurn)
			Black ^= move;
		else
			White ^= move;
	}

	__host__ __device__ __inline__ constexpr void Capture(int fromIndex, int toIndex)
	{
		SinceCapture = -1;
		Move(fromIndex, toIndex);

		bool bl2tr = Impl::DiagBL2TR[fromIndex] == Impl::DiagBL2TR[toIndex];

		constexpr const Bitboard *tables[2] = {Impl::DiagTL2BR, Impl::DiagBL2TR};
		Bitboard diag = tables[bl2tr][fromIndex];

		if (fromIndex > toIndex)
			stl::swap(fromIndex, toIndex);

		Bitboard captured = Board::Full;
		captured >>= fromIndex + 1; captured <<= fromIndex + 1 + 32 - toIndex; captured >>= 32 - toIndex;
		captured &= diag;

		if (BlackTurn)
			White &= ~captured;
		else
			Black &= ~captured;

		Queens &= ~captured;
	}

	__host__ __device__ __inline__ constexpr void EndTurn()
	{
		Queens |= Black & Impl::BlackPromotion;
		Queens |= White & Impl::WhitePromotion;

		BlackTurn = !BlackTurn;
		SinceCapture++;
	}

	__host__ __device__ __inline__ constexpr Bitboard GetCheckers() const
	{
		return BlackTurn ? Black : White;
	}

	__host__ __device__ __inline__ constexpr Bitboard GetOpponent() const
	{
		return BlackTurn ? White : Black;
	}

	__host__ __device__ __inline__ constexpr Bitboard GetAllMoving() const
	{
		return GetMoving(GetCheckers());
	}

	__host__ __device__ __inline__ constexpr Bitboard GetAllCapturing() const
	{
		return GetCapturing(GetCheckers());
	}

	__host__ __device__ __inline__ constexpr bool HasLost() const
	{
		return Board::IsEmpty(GetAllMoving() | GetAllCapturing());
	}

	__host__ __device__ __inline__ constexpr bool IsDraw() const
	{
		return SinceCapture >= MovesTillDraw;
	}

	template<typename G>
	__host__ __device__ __inline__ constexpr void RandomMove(G &generator)
	{
		Bitboard capturing = GetAllCapturing();

		if (capturing)
		{
			// if there are capturing pieces we have to make a capturing move

			int fromIndex = Board::RandomBit(generator, capturing);

			Bitboard captures = GetCaptures(Board::FromIndex(fromIndex));
			assert(!Board::IsEmpty(captures));

			do {
				int toIndex = Board::RandomBit(generator, captures);

				Capture(fromIndex, toIndex);

				fromIndex = toIndex;

				captures = GetCaptures(Board::FromIndex(fromIndex));
			} while (captures);

			return;
		}

		Bitboard moving = GetAllMoving();

		int fromIndex = Board::RandomBit(generator, moving);

		Bitboard moves = GetMoves(Board::FromIndex(fromIndex));
		assert(!Board::IsEmpty(moves));

		int toIndex = Board::RandomBit(generator, moves);

		Move(fromIndex, toIndex);
	}

	template<typename G>
	__host__ __device__ __inline__ void SimulateOne(G &generator, int &blackInc, int &whiteInc)
	{
		static constexpr int MaxMoves = 40;

		int i = 0;
		while (!HasLost() && !IsDraw() && i < MaxMoves)
		{
			RandomMove(generator);
			EndTurn();
		}

		if (IsDraw() || i == MaxMoves)
		{
			blackInc = 1;
			whiteInc = 1;
			return;
		}

		blackInc = (!BlackTurn) * 2;
		whiteInc = BlackTurn * 2;
	}
};

static_assert(stl::is_standard_layout_v<Position> == true);

static inline constexpr Position StartingPosition{ 0x00000fffu, 0xfff00000u, Board::Empty, 0, false };

}
