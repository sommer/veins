/**
 * Convenience class to contain all object and nodes in the world
 *
 * (C) 2008, Otto Visser
 */

#ifndef __WORLDTABLEMODEL_H__
#define __WORLDTABLEMODEL_H__

#include <QVariant>
#include <QStandardItemModel>

#include "../miximConfiguratorCommon.h"

class WorldTableModel : public QAbstractTableModel {
	Q_OBJECT

	public:
		WorldTableModel(QObject* parent = 0) : QAbstractTableModel(parent), rows(0) {}
		int rowCount(const QModelIndex & UNUSED(parent) = QModelIndex()) const {printf("returning %d rows\n", rows); return rows;}
		int columnCount(const QModelIndex & UNUSED(parent) = QModelIndex()) const {return 3;}
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
		bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
		bool insertData(QStandardItemModel* item);

	private:
		int rows;
		QList<QStandardItemModel*> contents;
};

#endif

