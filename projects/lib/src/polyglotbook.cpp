/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDataStream>
#include "polyglotbook.h"


static BookMove moveFromBits(quint16 pgMove)
{
	using Chess::Square;
	
	Square target((quint16)(pgMove << 13) >> 13,
		      (quint16)(pgMove << 10) >> 13);
	Square source((quint16)(pgMove << 7) >> 13,
		      (quint16)(pgMove << 4) >> 13);
	int promotion = (quint16)(pgMove << 1) >> 13;
	if (promotion > 0)
		promotion++;
	
	return BookMove(source, target, promotion);
}

static quint16 moveToBits(const BookMove& move)
{
	using Chess::Square;
	
	const Square& src = move.sourceSquare();
	const Square& trg = move.targetSquare();
	
	quint16 target = trg.file() | (trg.rank() << 3);
	quint16 source = (src.file() << 6) | (src.rank() << 9);
	quint16 promotion = 0;
	if (move.promotion() > 0)
		promotion = (move.promotion() - 1) << 12;
	
	return target | source | promotion;
}


Chess::Variant PolyglotBook::variant() const
{
	return Chess::Variant::Standard;
}

void PolyglotBook::loadEntry(QDataStream& in)
{
	quint64 key;
	quint16 pgMove;
	quint16 weight;
	quint32 learn;
	
	// Read the data. No need to worry about endianess,
	// because QDataStream uses big-endian by default.
	in >> key >> pgMove >> weight >> learn;
	
	Entry entry = { moveFromBits(pgMove), weight };
	addEntry(entry, key);
}

void PolyglotBook::saveEntry(const Map::const_iterator& it,
                             QDataStream& out) const
{
	quint32 learn = 0;
	quint64 key = it.key();
	quint16 pgMove = moveToBits(it.value().move);
	quint16 weight = it.value().weight;
	
	// Store the data. Again, big-endian is used by default.
	out << key << pgMove << weight << learn;
}
