#include "Node.h"

Module* macModules;
Module* phyModules;
QStringList macNames;
QStringList phyNames;
bool inited = false;
bool stopToggling = false;
bool threeD = false;
bool square = true;

Node::Node(Q3Wizard *parent): Q3Wizard(parent) {
	setupUi(this);

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
	nicTable->setItem(0, 1, new Q3ComboTableItem(nicTable, macNames));
	nicTable->setItem(0, 2, new Q3ComboTableItem(nicTable, phyNames));
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

void Node::on_addNICButton_clicked() {
	nicTable->insertRows(0);
	nicTable->setText(0, 0, "new");
	nicTable->setItem(0, 1, new Q3ComboTableItem(nicTable, macNames));
	nicTable->setItem(0, 2, new Q3ComboTableItem(nicTable, phyNames));
}

void Node::on_regularGridRB_toggled( bool ) {
	if (stopToggling)
		stopToggling = false;
	else {
		stopToggling =true;
		randomRB->toggle();
	}
}

void Node::on_randomRB_toggled( bool ) {
	if (stopToggling)
		stopToggling = false;
	else {
		stopToggling =true;
		regularGridRB->toggle();
	}
}

void Node::on_squareCB_toggled( bool ) {
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

void Node::on_threeDCB_toggled( bool ) {
	threeD = !threeD;
	dimZTB->setEnabled(threeD && !square);
	if (threeD)
		dimZTB->setText(dimXTB->text());
	else
		dimZTB->setText("-");
}

void Node::on_dimXTB_textChanged( const QString & ) {
	if (square) {
		dimYTB->setText(dimXTB->text());
		if (threeD)
			dimZTB->setText(dimXTB->text());
	}
}

void Node::on_fixedAnchorCB_toggled( bool ) {
	bool enable = fixedAnchorCB->isChecked();
	sinkIDTB->setEnabled(enable);
	sinkXTB->setEnabled(enable);
	sinkYTB->setEnabled(enable);
	sinkZTB->setEnabled(enable);
}

