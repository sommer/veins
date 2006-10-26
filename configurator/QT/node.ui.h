/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include "../miximConfiguratorCommon.h"

Module* macModules;
Module* phyModules;
QStringList macNames;
QStringList phyNames;
bool inited = false;
bool stopToggling = false;
bool threeD = false;
bool square = true;

void Node::init() {
	Module* baseModules = findBaseModules();
	Module* nodeModules = findModules(NODE);
	Module* networkModules = findModules(NETWORK);
	Module* appModules = findModules(APPLICATION);
	macModules = findModules(MAC);
	phyModules = findModules(PHY);
	
	setFinishEnabled(page(1), true);
	
	if (nodeModules != NULL) {
		Module* it = nodeModules;
		while (it) {
			nodeTypeCB->insertItem(it->name);
			it = it->next;
		}
	}
	
	if (networkModules != NULL) {
		Module* it = networkModules;
		while (it) {
			networkLayerCB->insertItem(it->name);
			it = it->next;
		}
	}
	
	if (appModules != NULL) {
		Module* it = appModules;
		while (it) {
			applicationNameCB->insertItem(it->name);
			it = it->next;
		}
	}
	
	if (!inited) {
		macNames.append("BaseMAC");
		phyNames.append("BasePHY");
		applicationNameCB->insertItem("BaseApplication");
		inited = true;
	}
	
	if (macModules != NULL) {
		Module* it = macModules;
		while (it) {
			macNames.append(it->name);
			it = it->next;
		}
	}
	
	
	if (phyModules != NULL) {
		Module* it = phyModules;
		while (it) {
			phyNames.append(it->name);
			it = it->next;
		}
	}
	
	nicTable->insertRows(0);
	// add default item
	nicTable->setText(0, 0, "default");
	nicTable->setItem(0, 1, new QComboTableItem(nicTable, macNames));
	nicTable->setItem(0, 2, new QComboTableItem(nicTable, phyNames));
}

void Node::destroy() {
	// transfer data to app?
}

unsigned Node::getCount() {
	return nodeCountSB->value();
}

QString Node::getType() {
	return nodeTypeCB->currentText();
}

QString Node::getNetworkLayer() {
	return networkLayerCB->currentText();
}

QString Node::getApplicationName() {
	return applicationNameCB->currentText();
}

void Node::addNICButton_clicked() {
	nicTable->insertRows(0);
	nicTable->setText(0, 0, "new");
	nicTable->setItem(0, 1, new QComboTableItem(nicTable, macNames));
	nicTable->setItem(0, 2, new QComboTableItem(nicTable, phyNames));
}


void Node::regularGridRB_toggled( bool ) {
	if (stopToggling)
		stopToggling = false;
	else {
		stopToggling =true;
		randomRB->toggle();
	}
}

void Node::randomRB_toggled( bool ) {
	if (stopToggling)
		stopToggling = false;
	else {
		stopToggling =true;
		regularGridRB->toggle();
	}
}

void Node::squareCB_toggled( bool ) {
		square = !square;
	dimYTB->setEnabled(!square);
	if (threeD)
		dimZTB->setEnabled(!square);
	if (square) {
		dimYTB->setText(dimXTB->text());
		if (threeD)
			dimZTB->setText(dimXTB->text());
	}
}

void Node::threeDCB_toggled( bool ) {
	threeD = !threeD;
	dimZTB->setEnabled(threeD && !square);
	if (threeD)
		dimZTB->setText(dimXTB->text());
	else
		dimZTB->setText("-");
}

void Node::dimXTB_textChanged( const QString & ) {
	if (square) {
		dimYTB->setText(dimXTB->text());
		if (threeD)
			dimZTB->setText(dimXTB->text());
	}
}

void Node::fixedAnchorCB_toggled( bool ) {
	bool enable = fixedAnchorCB->isChecked();
	sinkIDTB->setEnabled(enable);
	sinkXTB->setEnabled(enable);
	sinkYTB->setEnabled(enable);
	sinkZTB->setEnabled(enable);
}
