/**
 * Convenience class to contain all object and nodes in the world
 *
 * (C) 2008, Otto Visser
 */

#include "WorldTableModel.h"

QVariant WorldTableModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();
	switch (index.column()) {
		case 0:
			return "1";
		case 1:
			return "BaseNode";
		case 2:
			return "BaseApplication";
		default:
			return "Error";
	}
}

QVariant WorldTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal) {
		switch (section) {
			case 0:
				return "Count";
			case 1:
				return "Type";
			case 2:
				return "Name";
			default:
				return "Error";
		}
	} else {
		return section;
	}
}

bool WorldTableModel::insertRows(int row, int count, const QModelIndex &parent) {
	beginInsertRows(parent, row, row + count);
	printf("inserting %d rows at loc %d\n", count, row);	// does nothing; handled by insertData
	rows++;
	endInsertRows();
	return true;
}

bool WorldTableModel::removeRows(int row, int count, const QModelIndex &parent) {
	return false;	// TODO
}

bool WorldTableModel::insertData(QStandardItemModel* item) {
	contents.append(item);
	return true;
}

