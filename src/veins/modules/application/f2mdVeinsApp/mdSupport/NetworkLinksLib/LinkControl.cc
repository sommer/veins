//
// LinkControl - models Links that block radio transmissions
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <veins/modules/application/f2mdVeinsApp/mdSupport/NetworkLinksLib/LinkControl.h>


LinkControl::~LinkControl() {

}

void LinkControl::initialize(TraCICommandInterface* traci) {
    for (auto const& id : traci->getLaneIds()) {
       std::list<Coord> slist = traci->lane(id).getShape();
       vector<Coord> vec1;
       vec1.insert(vec1.begin(), slist.begin(),slist.end());
       addShape(vec1);
    }
}

void LinkControl::finish() {
    for (LinkList::iterator i = Links.begin(); i != Links.end(); ++i) {
        for (LinkGridRow::iterator j = i->begin(); j != i->end(); ++j) {
            while (j->begin() != j->end())
                erase(*j->begin());
        }
    }
    Links.clear();
}


void LinkControl::addShape(std::vector<Coord> shape) {
    Link link = Link();
    link.setShape(shape);
    add(link);
}

void LinkControl::add(Link link) {
    Link *o =  new Link(link);

    size_t fromRow = std::max(0, int(o->getBboxP1().x / GRIDCELL_SIZE));
    size_t toRow = std::max(0, int(o->getBboxP2().x / GRIDCELL_SIZE));
    size_t fromCol = std::max(0, int(o->getBboxP1().y / GRIDCELL_SIZE));
    size_t toCol = std::max(0, int(o->getBboxP2().y / GRIDCELL_SIZE));
    for (size_t row = fromRow; row <= toRow; ++row) {
        for (size_t col = fromCol; col <= toCol; ++col) {
            if (Links.size() < col + 1)
                Links.resize(col + 1);
            if (Links[col].size() < row + 1)
                Links[col].resize(row + 1);
            (Links[col])[row].push_back(o);
        }
    }

    cacheEntries.clear();
}

void LinkControl::erase(const Link* link) {
    for (LinkList::iterator i = Links.begin(); i != Links.end(); ++i) {
        for (LinkGridRow::iterator j = i->begin(); j != i->end(); ++j) {
            for (LinkGridCell::iterator k = j->begin(); k != j->end();) {
                Link* o = *k;
                if (o == link) {
                    k = j->erase(k);
                } else {
                    ++k;
                }
            }
        }
    }

    delete link;

    cacheEntries.clear();
}


double LinkControl::calculateDistance(const Coord& pos, double deltaX, double deltaY) {
    // calculate bounding box of transmission
    Coord bboxP1 = Coord(std::min(pos.x - deltaX, pos.x + deltaX),
            std::min(pos.y - deltaY, pos.y + deltaY));
    Coord bboxP2 = Coord(std::max(pos.x - deltaX, pos.x + deltaX),
            std::max(pos.y - deltaY, pos.y + deltaY));

    size_t fromRow = std::max(0, int(bboxP1.x / GRIDCELL_SIZE));
    size_t toRow = std::max(0, int(bboxP2.x / GRIDCELL_SIZE));
    size_t fromCol = std::max(0, int(bboxP1.y / GRIDCELL_SIZE));
    size_t toCol = std::max(0, int(bboxP2.y / GRIDCELL_SIZE));

    if (fromRow > 0) {
        fromRow = fromRow - 1;
    }
    if (fromCol > 0) {
        fromCol = fromCol - 1;
    }
    toRow = toRow + 1;
    toCol = toCol + 1;

    std::set<Link*> processedLinks;

    double distance = DBL_MAX;
    double localD = 0;
    for (size_t col = fromCol; col <= toCol; ++col) {
        if (col >= Links.size())
            break;
        for (size_t row = fromRow; row <= toRow; ++row) {
            if (row >= Links[col].size())
                break;
            const LinkGridCell& cell = (Links[col])[row];
            for (LinkGridCell::const_iterator k = cell.begin();
                    k != cell.end(); ++k) {

                Link* o = *k;
                localD = o->getDistance(&pos);
                if(localD<distance){
                    distance = localD;
                    if(localD<MAX_DISTANCE_FROM_ROUTE){
                        return localD;
                    }
                }
            }
        }
    }
    return distance;
}



